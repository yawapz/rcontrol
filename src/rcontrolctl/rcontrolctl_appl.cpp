#include "rcontrolctl_appl.h"

#define log_error_m   alog::logger().error   (alog_line_location, "Application")
#define log_warn_m    alog::logger().warn    (alog_line_location, "Application")
#define log_info_m    alog::logger().info    (alog_line_location, "Application")
#define log_verbose_m alog::logger().verbose (alog_line_location, "Application")
#define log_debug_m   alog::logger().debug   (alog_line_location, "Application")
#define log_debug2_m  alog::logger().debug2  (alog_line_location, "Application")

#define KILL_TIMER(TIMER) {if (TIMER != -1) {killTimer(TIMER); TIMER = -1;}}

using namespace sql;

QUuidEx Application::_applId;
volatile bool Application::_stop = false;
std::atomic_int Application::_exitCode = {0};

Application::Application(int& argc, char** argv)
    : QCoreApplication(argc, argv)
{
    _stopTimerId = startTimer(1000);
    _workerChkRelevanceDataTimerId = startTimer(10 * 1000 /*10 сек*/);

    chk_connect_a(&config::observerBase(), &config::ObserverBase::changed,
                  this, &Application::reloadConfig)

    #define FUNC_REGISTRATION(COMMAND) \
        _funcInvoker.registration(command:: COMMAND, &Application::command_##COMMAND, this);

    FUNC_REGISTRATION(CtlDiscovery)
    FUNC_REGISTRATION(AgentStat)
    FUNC_REGISTRATION(UserLogin)
    FUNC_REGISTRATION(UserWorkerList)
    FUNC_REGISTRATION(AddWorker)
    FUNC_REGISTRATION(DeleteWorker)
    FUNC_REGISTRATION(AgentGetGPUData)
    FUNC_REGISTRATION(AgentUpdateGPUData)
    FUNC_REGISTRATION(AddUser)
    FUNC_REGISTRATION(DeleteUser)
    FUNC_REGISTRATION(ChangeUserPassword)
    FUNC_REGISTRATION(UpdateWorkerInfoClient)
    FUNC_REGISTRATION(CheckWorkerInfoAgent)
    FUNC_REGISTRATION(UpdateGPUSettings)

    #undef FUNC_REGISTRATION
}

bool Application::init()
{
    if (!config::base().getValue("application.id", _applId))
    {
        log_error_m << "Application id undefined. Set application.id in config";
        return false;
    }

    config::base().getValue("application.name", _applName);
    config::base().getValue("application.line", _applIndex);
    if (!config::base().getValue("database.users_table", _usersTable))
        config::base().setValue("database.users_table", _usersTable);
    if (!config::base().getValue("database.workers_table", _workersTable))
        config::base().setValue("database.workers_table", _workersTable);
    if (!config::base().getValue("database.gpus_table", _gpusTable))
        config::base().setValue("database.gpus_table", _gpusTable);

    if (!checkSQLTables())
    {
        log_error_m << "Tables were not found and can't be created, check database settings";
        return false;
    }

    auto tfunc = [this]()
    {
        trd::ThreadIdLock threadIdLock(&_threadIds); (void) threadIdLock;
        db::postgres::Driver::Ptr dbcon = pgpool().connect();
        db::postgres::Transaction::Ptr transact = dbcon->createTransact();
        QSqlQuery q {db::postgres::createResult(transact)};
        QString sql = QString(" SELECT * FROM %1 ").arg(_workersTable);

        if (!sql::exec(q, sql))
            return;

        while (q.next())
        {
            QVector<QUuidEx> gpuIds;
            data::workerData readWorker;

            QSqlRecord r = q.record();
            sql::assignValue(readWorker.id, r, "ID"                           );
            //sql::assignValue(, r, "USER_ID"                                   );
            sql::assignValue(gpuIds                    ,r, "GPU_LIST_ID"      );
            sql::assignValue(readWorker.workerName     ,r, "WORKER_NAME"      );
            sql::assignValue(readWorker.electricityCost,r, "ELECTRICITY_COST" );
            sql::assignValue(readWorker.kernelVersion  ,r, "KERNEL_VERSION"   );
            sql::assignValue(readWorker.nvidiaVersion  ,r, "NVIDIA_VERSION"   );
            sql::assignValue(readWorker.amdVersion     ,r, "AMD_VERSION"      );
            sql::assignValue(readWorker.motherboardInfo,r, "MOTHERBOARD_INFO" );
            sql::assignValue(readWorker.cpuInfo        ,r, "CPU_INFO"         );
            sql::assignValue(readWorker.diskModel      ,r, "DISK_MODEL"       );
            sql::assignValue(readWorker.diskSize       ,r, "DISK_SIZE"        );
            sql::assignValue(readWorker.ramTotal       ,r, "RAM_TOTAL"        );
            sql::assignValue(readWorker.mac            ,r, "MAC"              );
            sql::assignValue(readWorker.localIp        ,r, "LOCAL_IP"         );
            sql::assignValue(readWorker.extIp          ,r, "EXT_IP"           );
            sql::assignValue(readWorker.lastOnline     ,r, "LAST_ONLINE"      );

            readWorker.devices = readWorkerGPUDataFromDB(gpuIds, transact);

            emit signalWorkerDataComing(readWorker);
        }
    };
    std::thread t {tfunc};
    t.detach();

    return true;
}

void Application::deinit()
{
    auto tfunc = [this]()
    {
        trd::ThreadIdLock threadIdLock(&_threadIds); (void) threadIdLock;
        db::postgres::Driver::Ptr dbcon = pgpool().connect();
        db::postgres::Transaction::Ptr transact = dbcon->createTransact();
        QSqlQuery q {db::postgres::createResult(transact)};
        QString sql = QString(" SELECT ID, USER_ID FROM %1 ").arg(_workersTable);

        if (!sql::exec(q, sql))
            return;

        QList<QPair<QUuidEx, QUuidEx>> pairs;
        while (q.next())
        {
            QUuidEx wid;
            QUuidEx uid;

            QSqlRecord r = q.record();
            sql::assignValue(wid, r, "ID");
            sql::assignValue(uid, r, "USER_ID");

            pairs.append(QPair(wid, uid));
        }

        QString fields =
        " ID                "
        " ,USER_ID          "
        " ,GPU_LIST_ID      "
        " ,WORKER_NAME      "
        " ,ELECTRICITY_COST "
        " ,KERNEL_VERSION   "
        " ,NVIDIA_VERSION   "
        " ,AMD_VERSION      "
        " ,MOTHERBOARD_INFO "
        " ,CPU_INFO         "
        " ,DISK_MODEL       "
        " ,DISK_SIZE        "
        " ,RAM_TOTAL        "
        " ,MAC              "
        " ,LOCAL_IP         "
        " ,EXT_IP           "
        " ,LAST_ONLINE      ";

        QSqlQuery qResult {db::postgres::createResult(transact)};

        for (auto& worker : wdc::wdc()._workers)
        {
            QUuidEx userId;

            for (auto& p : pairs)
                if (p.first == worker.id)
                {
                    userId = p.second;
                    break;
                }
                else
                    continue;

            QString sql = sql::INSERT_OR_UPDATE_PG(_workersTable, fields, "ID");

            QVector<QUuidEx> devIds;
            for (auto& dev : worker.devices)
                devIds.append(dev.id);

            if (qResult.prepare(sql))
            {
                sql::bindValue(qResult, ":ID"               , worker.id             );
                sql::bindValue(qResult, ":USER_ID"          , userId                );
                sql::bindValue(qResult, ":GPU_LIST_ID"      , devIds                );
                sql::bindValue(qResult, ":WORKER_NAME"      , worker.workerName     );
                sql::bindValue(qResult, ":ELECTRICITY_COST" , worker.electricityCost);
                sql::bindValue(qResult, ":KERNEL_VERSION"   , worker.kernelVersion  );
                sql::bindValue(qResult, ":NVIDIA_VERSION"   , worker.nvidiaVersion  );
                sql::bindValue(qResult, ":AMD_VERSION"      , worker.amdVersion     );
                sql::bindValue(qResult, ":MOTHERBOARD_INFO" , worker.motherboardInfo);
                sql::bindValue(qResult, ":CPU_INFO"         , worker.cpuInfo        );
                sql::bindValue(qResult, ":DISK_MODEL"       , worker.diskModel      );
                sql::bindValue(qResult, ":DISK_SIZE"        , worker.diskSize       );
                sql::bindValue(qResult, ":RAM_TOTAL"        , worker.ramTotal       );
                sql::bindValue(qResult, ":MAC"              , worker.mac            );
                sql::bindValue(qResult, ":LOCAL_IP"         , worker.localIp        );
                sql::bindValue(qResult, ":EXT_IP"           , worker.extIp          );
                sql::bindValue(qResult, ":LAST_ONLINE"      , worker.lastOnline     );

                if (qResult.exec())
                       writeOrUpdateWorkerGPUDataInDB(worker, transact);
            }
        }
    };
    std::thread t {tfunc};
    t.detach();

    _threadIds.lock([](const std::vector<pid_t>& tids)
    {
        for (pid_t tid : tids)
            pgpool().abortOperation(tid);
    });

    while (!_threadIds.empty())
        usleep(100*1000);
}

void Application::sendSettings()
{

}

void Application::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == _stopTimerId)
    {
        if (_stop)
        {
            KILL_TIMER(_stopTimerId)
            KILL_TIMER(_clearHistoryTimerId)
            KILL_TIMER(_workerChkRelevanceDataTimerId)

            exit(_exitCode);
            return;
        }
    }
    else if (event->timerId() == _workerChkRelevanceDataTimerId)
    {
        setWorkerStatusByRelevanceData();
    }
}

void Application::stop(int exitCode)
{
    _exitCode = exitCode;
    stop();
}

void Application::message(const pproto::Message::Ptr& message)
{
    // Не обрабатываем сообщения если приложение получило команду на остановку
    if (_stop)
        return;

    if (message->processed())
        return;

    if (message->execStatus() == Message::ExecStatus::Error)
    {
        data::MessageError error;
        readFromMessage(message, error);
        log_error_m << "Error message"
                    << ". Detail: "     << error.description;
        return;
    }

    if (lst::FindResult fr = _funcInvoker.findCommand(message->command()))
    {
        if (command::pool().commandIsSinglproc(message->command()))
            message->markAsProcessed();
        _funcInvoker.call(message, fr);
    }
}

void Application::socketConnected(pproto::SocketDescriptor socketDescript)
{
    base::Socket::Ptr sock = tcp::listener().socketByDescriptor(socketDescript);
    if (sock->messageFormat() == SerializeFormat::Json)
    {
        sock->setMessageWebFlags(true);
        return;
    }
}

void Application::socketDisconnected(pproto::SocketDescriptor socketDescript)
{

}

void Application::reloadConfig()
{

}

QList<data::gpuData> Application::readWorkerGPUDataFromDB(const QVector<QUuidEx>& idVec, db::postgres::Transaction::Ptr transact)
{
    QList<data::gpuData> gpuList;

    if (idVec.isEmpty())
        return gpuList;

    QStringList listIds;
    for (QUuidEx id : idVec)
        listIds.append("'" + toString(id) + "'");

    QSqlQuery q {db::postgres::createResult(transact)};

    QString sql = QString(" SELECT * FROM %1   "
                          " WHERE id IN (%2) "
                         ).arg(_gpusTable, listIds.join(','));

    if (sql::exec(q, sql))
    {
        while (q.next())
        {
            data::gpuData gpu;

            QSqlRecord r = q.record();
            sql::assignValue(gpu.id           ,r, "ID"           );
            sql::assignValue(gpu.setFanSpeed  ,r, "SET_FAN_SPEED");
            sql::assignValue(gpu.setCore      ,r, "SET_CORE"     );
            sql::assignValue(gpu.setMem       ,r, "SET_MEM"      );
            sql::assignValue(gpu.setPl        ,r, "SET_PL"       );
            sql::assignValue(gpu.name         ,r, "NAME"         );
            sql::assignValue(gpu.busId        ,r, "BUS_ID"       );
            sql::assignValue(gpu.vendor       ,r, "VENDOR"       );
            sql::assignValue(gpu.totalMemory  ,r, "TOTAL_MEMORY" );
            sql::assignValue(gpu.vbiosVersion ,r, "VBIOS_VERSION");
            sql::assignValue(gpu.minPl        ,r, "MIN_PL"       );
            sql::assignValue(gpu.defaultPl    ,r, "DEFAULT_PL"   );
            sql::assignValue(gpu.maxPl        ,r, "MAX_PL"       );
            sql::assignValue(gpu.gpuId        ,r, "GPU_SYS_ID"   );

            gpuList.append(gpu);
        }
    }

    return gpuList;
}

void Application::writeOrUpdateWorkerGPUDataInDB(const data::workerData& worker, db::postgres::Transaction::Ptr transact)
{
    const QString fields =
    "  ID            "
    " ,WORKER_ID     "
    " ,SET_FAN_SPEED "
    " ,SET_CORE      "
    " ,SET_MEM       "
    " ,SET_PL        "
    " ,NAME          "
    " ,BUS_ID        "
    " ,VENDOR        "
    " ,TOTAL_MEMORY  "
    " ,VBIOS_VERSION "
    " ,MIN_PL        "
    " ,DEFAULT_PL    "
    " ,MAX_PL        "
    " ,GPU_SYS_ID    ";

    const QString sql = sql::INSERT_OR_UPDATE_PG(_gpusTable, fields, "ID");
    for (const data::gpuData& g : worker.devices)
    {
        QSqlQuery qResult {db::postgres::createResult(transact)};

        if (qResult.prepare(sql))
        {
            sql::bindValue(qResult, ":ID"            , g.id          );
            sql::bindValue(qResult, ":WORKER_ID"     , worker.id     );
            sql::bindValue(qResult, ":SET_FAN_SPEED" , g.setFanSpeed );
            sql::bindValue(qResult, ":SET_CORE"      , g.setCore     );
            sql::bindValue(qResult, ":SET_MEM"       , g.setMem      );
            sql::bindValue(qResult, ":SET_PL"        , g.setPl       );
            sql::bindValue(qResult, ":NAME"          , g.name        );
            sql::bindValue(qResult, ":BUS_ID"        , g.busId       );
            sql::bindValue(qResult, ":VENDOR"        , g.vendor      );
            sql::bindValue(qResult, ":TOTAL_MEMORY"  , g.totalMemory );
            sql::bindValue(qResult, ":VBIOS_VERSION" , g.vbiosVersion);
            sql::bindValue(qResult, ":MIN_PL"        , g.minPl       );
            sql::bindValue(qResult, ":DEFAULT_PL"    , g.defaultPl   );
            sql::bindValue(qResult, ":MAX_PL"        , g.maxPl       );
            sql::bindValue(qResult, ":GPU_SYS_ID"    , g.gpuId       );

            qResult.exec();
        }
    }
}

void Application::setWorkerStatusByRelevanceData()
{
    auto tfunc = [this]()
    {
        SpinLocker locker {_relevanceDataLocker}; (void) locker;

        qint64 time = QDateTime::currentDateTime().toSecsSinceEpoch();

        for (auto& w : wdc::wdc()._workers)
            if (w.status && (time - w.lastOnline.toSecsSinceEpoch()) > 30)
                w.setOffline();
    };

    std::thread t {tfunc};
    t.detach();
}

bool Application::checkSQLTables()
{
    db::postgres::Driver::Ptr dbcon = pgpool().connect();
    db::postgres::Transaction::Ptr transact = dbcon->createTransact();

    auto check = [transact](const QString& tableName)
    {
        QSqlQuery q {db::postgres::createResult(transact)};

        QString sql = " SELECT EXISTS (                "
                      " SELECT 1                       "
                      " FROM information_schema.tables "
                      " WHERE table_schema = 'public'  "
                      " AND table_name = '%1'          "
                      ") AS RES;                       ";

        sql = sql.arg(tableName);
        sql::exec(q, sql);
        bool flag = false;

        if (q.next())
        {
            QSqlRecord r = q.record();
            sql::assignValue(flag,r, "RES");
        }

        if (flag)
            return true;
        else
            return false;
    };

    auto createTable = [transact, check](const QString& tableName, const QString& sql)
    {
        QSqlQuery q {db::postgres::createResult(transact)};
        sql::exec(q, sql);
        if (check(tableName))
            return true;
        else
            return false;
    };

    if (!check(_gpusTable))
    {
        QString sqlCreateGpusTable = " CREATE TABLE public.gpus( "
                                     " id uuid NOT NULL,         "
                                     " worker_id uuid NOT NULL,  "
                                     " set_fan_speed smallint,   "
                                     " set_core smallint,        "
                                     " set_mem integer,          "
                                     " set_pl smallint,          "
                                     " name text,                "
                                     " bus_id text,              "
                                     " vendor text,              "
                                     " total_memory text,        "
                                     " vbios_version text,       "
                                     " min_pl text,              "
                                     " default_pl text,          "
                                     " max_pl text,              "
                                     " gpu_sys_id smallint,      "
                                     " PRIMARY KEY (id));        ";
        if (!createTable(_gpusTable, sqlCreateGpusTable))
            return false;
    }
    if (!check(_workersTable))
    {
        QString sqlCreateWorkerTable = " CREATE TABLE public.workers(            "
                                       " id uuid NOT NULL,                       "
                                       " user_id uuid NOT NULL,                  "
                                       " gpu_list_id uuid[],                     "
                                       " worker_name text,                       "
                                       " electricity_cost float,                 "
                                       " kernel_version text,                    "
                                       " nvidia_version text,                    "
                                       " amd_version text,                       "
                                       " motherboard_info text,                  "
                                       " cpu_info text,                          "
                                       " disk_model text,                        "
                                       " disk_size text,                         "
                                       " ram_total text,                         "
                                       " mac text,                               "
                                       " local_ip text,                          "
                                       " ext_ip text,                            "
                                       " last_online timestamp without time zone,"
                                       " PRIMARY KEY (id))                       ";
        if (!createTable(_gpusTable, sqlCreateWorkerTable))
            return false;
    }
    if (!check(_usersTable))
    {
        QString sqlCreateUsersTable = "CREATE TABLE public.users(    "
                    "id uuid NOT NULL,                               "
                    "login text NOT NULL,                            "
                    "password text NOT NULL,                         "
                    "last_visit timestamp without time zone NOT NULL,"
                    "PRIMARY KEY (id));                              ";
        if (!createTable(_gpusTable, sqlCreateUsersTable))
            return false;
    }

    return true;
}

void Application::netInterfacesUpdate()
{
    if (_netInterfacesTimer.elapsed() > 15*1000 /*15 сек*/)
    {
        network::Interface::List nl = network::getInterfaces();
        _netInterfaces.swap(nl);
    }
}

void Application::loadSettings()
{
    // Закладка "Журнал"
//    _settings.journal.keepHistory = {1, 10};
//    config::state().getValue("settings.journal.keep_history",
//                             _settings.journal.keepHistory);
}

void Application::saveSettings()
{
//    // Закладка "Журнал"
//    config::state().setValue("settings.journal.keep_history",
//                             _settings.journal.keepHistory);

    // Закладка "Настройки камеры"

    config::state().saveFile();
}

void Application::command_CtlDiscovery(const Message::Ptr& message)
{
    // Обрабатываем сообщения только от UDP сокета
    if (message->socketType() != SocketType::Udp)
        return;

    Message::Ptr answer = message->cloneForAnswer();

    int port = DEFAULT_PORT;
    config::base().getValue("listener.port", port);

    data::CtlDiscovery ctlDiscovery;
    ctlDiscovery.info = "RControl";
    ctlDiscovery.applId = _applId;

    // От локального хоста обрабатываем сообщения индивидуально
    if (message->sourcePoint().address() == QHostAddress::LocalHost)
    {
        ctlDiscovery.hostPoint = {QHostAddress::LocalHost, port};
        writeToMessage(ctlDiscovery, answer);
        udp::socket().send(answer);
        return;
    }

    netInterfacesUpdate();

    // Ответ отправляем только в известные планировщику сети. Хорошо это или
    // плохо, пока непонятно.
    for (network::Interface* intf : _netInterfaces)
        if (message->sourcePoint().address().isInSubnet(intf->subnet(),
                                                        intf->subnetPrefixLength()))
        {
            ctlDiscovery.hostPoint = {intf->ip(), port};
            writeToMessage(ctlDiscovery, answer);
            udp::socket().send(answer);
            return;
        }
}

void Application::command_AgentStat(const Message::Ptr& message)
{
    auto tfunc = [this, message]()
    {
        trd::ThreadIdLock threadIdLock(&_threadIds); (void) threadIdLock;
        data::AgentStat worker;
        readFromMessage(message, worker);
        worker.agent.status = true;

        // Проверка на изменения info для воркера
        for (int i = 0; i < _updateWorkerInfoList.size(); ++i)
        {
            if (_updateWorkerInfoList[i].first.workerID == worker.agent.id)
            {
                worker.agent.workerName =
                        _updateWorkerInfoList[i].first.newWorkerName;
                worker.agent.electricityCost =
                        _updateWorkerInfoList[i].first.newPowerPrice;

                Message::Ptr answer = createMessage(
                            data::UpdateWorkerInfoAgent(
                                _updateWorkerInfoList[i].first));

                answer->appendDestinationPoint(message->sourcePoint());
                answer->appendDestinationSocket(message->socketDescriptor());
                answer->setProxyId(message->proxyId());
                answer->setAccessId(message->accessId());
                answer->setAuxiliary(message->auxiliary());

                tcp::listener().send(answer);

                _updateWorkerInfoList.removeOne(_updateWorkerInfoList[i]);
                break;
            }

            // Если запрос "протух", то скорее всего воркер оффлайн
            if (QDateTime::currentDateTime().toSecsSinceEpoch()
                    - _updateWorkerInfoList[i].second.toSecsSinceEpoch() > 100)
                _updateWorkerInfoList.removeOne(_updateWorkerInfoList[i--]);

            if (i < 0)
                break;
        }

        // Проверка на изменение разгона для gpu воркера
        for (int i = 0; i < _updateWorkerGPUSettings.size(); ++i)
        {
            if (_updateWorkerGPUSettings[i].first.first == worker.agent.id)
            {
                for (data::gpuData& g : worker.agent.devices)
                {
                    g = _updateWorkerGPUSettings[i].first.second.GPUData;
                }

                Message::Ptr answer = createMessage(
                            data::UpdateGPUSettings(
                                _updateWorkerGPUSettings[i].first.second.GPUData));

                answer->appendDestinationPoint(message->sourcePoint());
                answer->appendDestinationSocket(message->socketDescriptor());
                answer->setProxyId(message->proxyId());
                answer->setAccessId(message->accessId());
                answer->setAuxiliary(message->auxiliary());

                tcp::listener().send(answer);
                _updateWorkerGPUSettings.removeOne(_updateWorkerGPUSettings[i]);
                break;
            }

            // Если запрос "протух", то скорее всего воркер оффлайн
            if (QDateTime::currentDateTime().toSecsSinceEpoch()
                    - _updateWorkerGPUSettings[i].second.toSecsSinceEpoch() > 100)
                _updateWorkerGPUSettings.removeOne(_updateWorkerGPUSettings[i--]);

            if (i < 0)
                break;
        }

        emit signalWorkerDataComing(worker.agent);

    };
    std::thread t {tfunc};
    t.detach();
}

void Application::command_UserLogin(const Message::Ptr& message)
{
    auto tfunc = [this, message]()
    {
        trd::ThreadIdLock threadIdLock(&_threadIds); (void) threadIdLock;

        data::UserLogin userLogin;
        readFromMessage(message, userLogin);

        if (message->contentFormat() == SerializeFormat::Json)
        {
            QJsonDocument jsonDoc(QJsonDocument::fromJson(message->content()));
            userLogin.login = jsonDoc.object().take("login").toString();
            userLogin.password = jsonDoc.object().take("password").toString();
        }

        db::postgres::Driver::Ptr dbcon = pgpool().connect();
        db::postgres::Transaction::Ptr transact = dbcon->createTransact();
        QSqlQuery q {db::postgres::createResult(transact)};

        QString sql = QString(" SELECT * FROM %1   "
                              " WHERE login = '%2' "
                             ).arg(_usersTable, userLogin.login);

        if (sql::exec(q, sql))
        {
            QUuidEx id;
            QString l = "";
            QString p = "";

            if (q.next())
            {
                QSqlRecord r = q.record();
                sql::assignValue(id ,r, "ID"      );
                sql::assignValue(l  ,r, "LOGIN"   );
                sql::assignValue(p  ,r, "PASSWORD");
            }

            if (userLogin.login == l && userLogin.password == p)
            {
                userLogin.resault = true;
                userLogin.id = id;

                db::postgres::Driver::Ptr dbcon = pgpool().connect();
                db::postgres::Transaction::Ptr transact = dbcon->createTransact();
                QSqlQuery q {db::postgres::createResult(transact)};

                QString sql = QString(" UPDATE %1 last_visit   "
                                      " SET last_visit = now() "
                                      " WHERE id = '%2'        "
                                     ).arg(_usersTable, toString(id));

               sql::exec(q, sql);
            }
        } // exec

        Message::Ptr answer = message->cloneForAnswer();
        writeToMessage(userLogin, answer, message->contentFormat());
        tcp::listener().send(answer);

    };
    std::thread t {tfunc};
    t.detach();
}

void Application::command_UserWorkerList(const Message::Ptr& message)
{
    auto tfunc = [this, message]()
    {
        trd::ThreadIdLock threadIdLock(&_threadIds); (void) threadIdLock;

        data::UserWorkerList workerList;
        readFromMessage(message, workerList);

        if (message->contentFormat() == SerializeFormat::Json)
        {
            QJsonDocument jsonDoc(QJsonDocument::fromJson(message->content()));
            workerList.userId = jsonDoc.object().take("id").toString();
        }

        db::postgres::Driver::Ptr dbcon = pgpool().connect();
        db::postgres::Transaction::Ptr transact = dbcon->createTransact();
        QSqlQuery q {db::postgres::createResult(transact)};

        QString sql = QString(" SELECT ID FROM %1     "
                              " WHERE user_id = '%2' "
                             ).arg(_workersTable, toString(workerList.userId));

        if (sql::exec(q, sql))
        {
            while (q.next())
            {
                QUuidEx id;
                QSqlRecord r = q.record();
                sql::assignValue(id ,r, "ID");
                workerList.userIdWorkers.append(id);
            }
        }

        QList<QUuidEx> notFoundedWorkersIds;
        for (auto& id : workerList.userIdWorkers)
        {
            bool isFind = false;

            for (const data::workerData& w : wdc::wdc()._workers)
            {
                if (id == w.id)
                {
                    isFind = true;
                    workerList.workersData.append(w);
                    break;
                }
            }

            if (!isFind)
                notFoundedWorkersIds.append(id);
        }

        if (!notFoundedWorkersIds.isEmpty())
        {
            QStringList listIds;
            for (QUuidEx& id : notFoundedWorkersIds)
                listIds.append("'" + toString(id) + "'");

            QSqlQuery qq {db::postgres::createResult(transact)};

            sql = QString(" SELECT * FROM %1   "
                          " WHERE id IN ('%2') "
                          ).arg(_workersTable, listIds.join(','));

            if (sql::exec(qq, sql))
            {
                while (q.next())
                {
                    QVector<QUuidEx> devicesIds;
                    data::workerData newWorker;

                    QSqlRecord r = q.record();
                    sql::assignValue(newWorker.id              ,r, "ID"              );
                    //sql::assignValue(newWorker.userID ,r, "USER_ID"         );
                    sql::assignValue(devicesIds                ,r, "GPU_LIST_ID"     );
                    sql::assignValue(newWorker.workerName      ,r, "WORKER_NAME"     );
                    sql::assignValue(newWorker.electricityCost ,r, "ELECTRICITY_COST");
                    sql::assignValue(newWorker.kernelVersion   ,r, "KERNEL_VERSION"  );
                    sql::assignValue(newWorker.nvidiaVersion   ,r, "NVIDIA_VERSION"  );
                    sql::assignValue(newWorker.amdVersion      ,r, "AMD_VERSION"     );
                    sql::assignValue(newWorker.motherboardInfo ,r, "MOTHERBOARD_INFO");
                    sql::assignValue(newWorker.cpuInfo         ,r, "CPU_INFO"        );
                    sql::assignValue(newWorker.diskModel       ,r, "DISK_MODEL"      );
                    sql::assignValue(newWorker.diskSize        ,r, "DISK_SIZE"       );
                    sql::assignValue(newWorker.ramTotal        ,r, "RAM_TOTAL"       );
                    sql::assignValue(newWorker.mac             ,r, "MAC"             );
                    sql::assignValue(newWorker.localIp         ,r, "LOCAL_IP"        );
                    sql::assignValue(newWorker.extIp           ,r, "EXT_IP"          );
                    sql::assignValue(newWorker.lastOnline      ,r, "LAST_ONLINE"     );

                    newWorker.devices = readWorkerGPUDataFromDB(devicesIds, transact);

                    emit signalWorkerDataComing(newWorker);
                }
            }
        }

        Message::Ptr answer = message->cloneForAnswer();
        writeToMessage(workerList, answer, message->contentFormat());
        tcp::listener().send(answer);
    };

    std::thread t {tfunc};
    t.detach();
}

void Application::command_AddWorker(const Message::Ptr& message)
{
    auto tfunc = [this, message]()
    {
        trd::ThreadIdLock threadIdLock(&_threadIds); (void) threadIdLock;

        data::AddWorker addWorker;
        readFromMessage(message, addWorker);

        if (message->contentFormat() == SerializeFormat::Json)
        {
            QJsonDocument jsonDoc(QJsonDocument::fromJson(message->content()));
            addWorker.userId = jsonDoc.object().take("id").toString();
        }

        Message::Ptr answer = message->cloneForAnswer();

        db::postgres::Driver::Ptr dbcon = pgpool().connect();
        db::postgres::Transaction::Ptr transact = dbcon->createTransact();
        QSqlQuery q0 {db::postgres::createResult(transact)};

        QUuidEx newId = QUuidEx::createUuid();
        bool flag = false;
        QString sql;

        // Подбор свободного Uuid
        while (!flag)
        {
            QString sql = QString(" SELECT ID FROM %1 "
                                  " WHERE ID = '%2'   "
                                 ).arg(_workersTable, toString(newId));

            sql::exec(q0, sql);

            if (q0.next())
                newId = QUuidEx::createUuid();
            else
                flag = true;
        }

        QString fields =
        " ID               "
        ",USER_ID          "
        ",GPU_LIST_ID      "
        ",WORKER_NAME      "
        ",ELECTRICITY_COST "
        ",KERNEL_VERSION   "
        ",NVIDIA_VERSION   "
        ",AMD_vERSION      "
        ",MOTHERBOARD_INFO "
        ",CPU_INFO         "
        ",DISK_MODEL       "
        ",DISK_SIZE        "
        ",RAM_TOTAL        "
        ",MAC              "
        ",LOCAL_IP         "
        ",EXT_IP           "
        ",LAST_ONLINE      ";

        sql = sql::INSERT_INTO(_workersTable, fields);

        QSqlQuery qResult {db::postgres::createResult(transact)};

        data::workerData newWorker;
        newWorker.id = newId;

        if (qResult.prepare(sql))
        {
            sql::bindValue(qResult, ":ID"               , newWorker.id             );
            sql::bindValue(qResult, ":USER_ID"          , addWorker.userId         );
            sql::bindValue(qResult, ":GPU_LIST_ID"      , QVector<QUuidEx>()       );
            sql::bindValue(qResult, ":WORKER_NAME"      , newWorker.workerName     );
            sql::bindValue(qResult, ":ELECTRICITY_COST" , newWorker.electricityCost);
            sql::bindValue(qResult, ":KERNEL_VERSION"   , newWorker.kernelVersion  );
            sql::bindValue(qResult, ":NVIDIA_VERSION"   , newWorker.nvidiaVersion  );
            sql::bindValue(qResult, ":AMD_VERSION"      , newWorker.amdVersion     );
            sql::bindValue(qResult, ":MOTHERBOARD_INFO" , newWorker.motherboardInfo);
            sql::bindValue(qResult, ":CPU_INFO"         , newWorker.cpuInfo        );
            sql::bindValue(qResult, ":DISK_MODEL"       , newWorker.diskModel      );
            sql::bindValue(qResult, ":DISK_SIZE"        , newWorker.diskSize       );
            sql::bindValue(qResult, ":RAM_TOTAL"        , newWorker.ramTotal       );
            sql::bindValue(qResult, ":MAC"              , newWorker.mac            );
            sql::bindValue(qResult, ":LOCAL_IP"         , newWorker.localIp        );
            sql::bindValue(qResult, ":EXT_IP"           , newWorker.extIp          );
            sql::bindValue(qResult, ":LAST_ONLINE"      , newWorker.lastOnline     );

            if (qResult.exec())
            {
                addWorker.resault = true;
                wdc::wdc()._workers.append(newWorker);
            }
        }

        writeToMessage(addWorker, answer, message->contentFormat());
        tcp::listener().send(answer);
    };
    std::thread t {tfunc};
    t.detach();
}

void Application::command_DeleteWorker(const Message::Ptr& message)
{
    auto tfunc = [this, message]()
    {
        trd::ThreadIdLock threadIdLock(&_threadIds); (void) threadIdLock;

        data::DeleteWorker deleteWorker;
        readFromMessage(message, deleteWorker);

        if (message->contentFormat() == SerializeFormat::Json)
        {
            QJsonDocument jsonDoc(QJsonDocument::fromJson(message->content()));
            deleteWorker.userId     = jsonDoc.object().take("userId"    ).toString();
            deleteWorker.workerId   = jsonDoc.object().take("workerId"  ).toString();
            deleteWorker.workerName = jsonDoc.object().take("workerName").toString();
            deleteWorker.resault    = jsonDoc.object().take("resault"   ).toBool();
        }

        Message::Ptr answer = message->cloneForAnswer();

        db::postgres::Driver::Ptr dbcon = pgpool().connect();
        db::postgres::Transaction::Ptr transact = dbcon->createTransact();
        QSqlQuery qResult {db::postgres::createResult(transact)};

        QString sql = QString(" DELETE FROM %1 WHERE ID = '%2' AND USER_ID = '%3' ").arg(
           _workersTable, toString(deleteWorker.workerId), toString(deleteWorker.userId));

        if (qResult.prepare(sql))
        {
            sql::bindValue(qResult, ":ID"        , deleteWorker.workerId);
            sql::bindValue(qResult, ":USER_ID"   , deleteWorker.userId  );

            if (qResult.exec())
            {
                for (auto& w : wdc::wdc()._workers)
                    if (w.id == deleteWorker.workerId)
                    {
                        wdc::wdc()._workers.removeAll(w);
                        deleteWorker.resault = true;
                        break;
                    }
            }
        }

        writeToMessage(deleteWorker, answer, message->contentFormat());
        tcp::listener().send(answer);
    };
    std::thread t {tfunc};
    t.detach();
}

void Application::command_AgentGetGPUData(const Message::Ptr& message)
{
    auto tfunc = [this, message]()
    {
        trd::ThreadIdLock threadIdLock(&_threadIds); (void) threadIdLock;

        data::AgentGetGPUData getGPUData;
        readFromMessage(message, getGPUData);

        Message::Ptr answer = message->cloneForAnswer();
        db::postgres::Driver::Ptr dbcon = pgpool().connect();
        db::postgres::Transaction::Ptr transact = dbcon->createTransact();
        QSqlQuery q0 {db::postgres::createResult(transact)};

        QString sql = QString(" SELECT GPU_LIST_ID FROM %1 "
                              " WHERE ID = '%2'            "
                             ).arg(_workersTable, toString(getGPUData.workerId));

        if (sql::exec(q0, sql))
        {
            QVector<QUuidEx> idVec;

            if (q0.next())
            {
                QSqlRecord r = q0.record();
                sql::assignValue(idVec ,r, "GPU_LIST_ID");
            }

            if (!idVec.isEmpty())
                getGPUData.gpuList = readWorkerGPUDataFromDB(idVec, transact);
        }

        writeToMessage(getGPUData, answer);
        tcp::listener().send(answer);
    };
    std::thread t {tfunc};
    t.detach();
}

void Application::command_AgentUpdateGPUData(const Message::Ptr& message)
{
    auto tfunc = [this, message]()
    {
        trd::ThreadIdLock threadIdLock(&_threadIds); (void) threadIdLock;

        data::AgentUpdateGPUData updateGPUData;
        readFromMessage(message, updateGPUData);

        db::postgres::Driver::Ptr dbcon = pgpool().connect();
        db::postgres::Transaction::Ptr transact = dbcon->createTransact();

        // Удалить старые ID GPU
        QVector<QUuidEx> newVec;
        for (data::gpuData g : updateGPUData.worker.devices)
                newVec.append(g.id);

        QSqlQuery q {db::postgres::createResult(transact)};

        QString sql = QString(" DELETE FROM %1         "
                              " WHERE WORKER_ID = '%2' "
                             ).arg(_gpusTable, toString(updateGPUData.worker.id));
        if (sql::exec(q, sql))
        {
            // Обновоить список id у воркера
            QSqlQuery qq {db::postgres::createResult(transact)};

            sql = QString(" SELECT USER_ID FROM %1 "
                          " WHERE id = '%2'        "
                         ).arg(_workersTable, toString(updateGPUData.worker.id));

            if (sql::exec(qq, sql))
            {
                if (qq.next())
                {
                    QUuidEx userId;

                    QSqlRecord r = qq.record();
                    sql::assignValue(userId, r, "USER_ID");

                    QString fields =
                    " ID               "
                    ",USER_ID          "
                    ",GPU_LIST_ID      "
                    ",WORKER_NAME      "
                    ",ELECTRICITY_COST "
                    ",KERNEL_VERSION   "
                    ",NVIDIA_VERSION   "
                    ",AMD_vERSION      "
                    ",MOTHERBOARD_INFO "
                    ",CPU_INFO         "
                    ",DISK_MODEL       "
                    ",DISK_SIZE        "
                    ",RAM_TOTAL        "
                    ",MAC              "
                    ",LOCAL_IP         "
                    ",EXT_IP           "
                    ",LAST_ONLINE      ";

                    sql = sql::INSERT_OR_UPDATE_PG(_workersTable, fields, "ID");
                    QSqlQuery qResult {db::postgres::createResult(transact)};

                    if (qResult.prepare(sql))
                    {
                        sql::bindValue(qResult, ":ID"               , updateGPUData.worker.id             );
                        sql::bindValue(qResult, ":USER_ID"          , userId                              );
                        sql::bindValue(qResult, ":GPU_LIST_ID"      , newVec                              );
                        sql::bindValue(qResult, ":WORKER_NAME"      , updateGPUData.worker.workerName     );
                        sql::bindValue(qResult, ":ELECTRICITY_COST" , updateGPUData.worker.electricityCost);
                        sql::bindValue(qResult, ":KERNEL_VERSION"   , updateGPUData.worker.kernelVersion  );
                        sql::bindValue(qResult, ":NVIDIA_VERSION"   , updateGPUData.worker.nvidiaVersion  );
                        sql::bindValue(qResult, ":AMD_VERSION"      , updateGPUData.worker.amdVersion     );
                        sql::bindValue(qResult, ":MOTHERBOARD_INFO" , updateGPUData.worker.motherboardInfo);
                        sql::bindValue(qResult, ":CPU_INFO"         , updateGPUData.worker.cpuInfo        );
                        sql::bindValue(qResult, ":DISK_MODEL"       , updateGPUData.worker.diskModel      );
                        sql::bindValue(qResult, ":DISK_SIZE"        , updateGPUData.worker.diskSize       );
                        sql::bindValue(qResult, ":RAM_TOTAL"        , updateGPUData.worker.ramTotal       );
                        sql::bindValue(qResult, ":MAC"              , updateGPUData.worker.mac            );
                        sql::bindValue(qResult, ":LOCAL_IP"         , updateGPUData.worker.localIp        );
                        sql::bindValue(qResult, ":EXT_IP"           , updateGPUData.worker.extIp          );
                        sql::bindValue(qResult, ":LAST_ONLINE"      , updateGPUData.worker.lastOnline     );

                        if (qResult.exec())
                        {
                            // Обновить данные GPU в БД
                            writeOrUpdateWorkerGPUDataInDB(updateGPUData.worker, transact);

                            // Обновить данные в буффере
                            emit signalWorkerDataComing(updateGPUData.worker);
                        }
                    }
                } //next
            } //qq exec
        } //q exec
    };
    std::thread t {tfunc};
    t.detach();
}

void Application::command_AddUser(const Message::Ptr& message)
{
    auto tfunc = [this, message]()
    {
        trd::ThreadIdLock threadIdLock(&_threadIds); (void) threadIdLock;
        data::AddUser addUser;
        readFromMessage(message, addUser);

        if (message->contentFormat() == SerializeFormat::Json)
        {
            QJsonDocument jsonDoc(QJsonDocument::fromJson(message->content()));
            addUser.login = jsonDoc.object().take("login").toString();
            addUser.password = jsonDoc.object().take("password").toString();
        }

        Message::Ptr answer = message->cloneForAnswer();

        db::postgres::Driver::Ptr dbcon = pgpool().connect();
        db::postgres::Transaction::Ptr transact = dbcon->createTransact();
        QSqlQuery q0 {db::postgres::createResult(transact)};

        QUuidEx newId = QUuidEx::createUuid();
        bool flag = false;
        QString sql;

        // Подбор свободного Uuid
        while (!flag)
        {
            QString sql = QString(" SELECT ID FROM %1 "
                                  " WHERE ID = '%2'   "
                                 ).arg(_usersTable, toString(newId));

            sql::exec(q0, sql);

            if (q0.next())
                newId = QUuidEx::createUuid();
            else
                flag = true;
        }

        sql = QString(" SELECT LOGIN FROM %1 "
                      " WHERE login = '%2'   "
                      ).arg(_usersTable, addUser.login);

        // Проверка занятости логина
        QSqlQuery q {db::postgres::createResult(transact)};
        sql::exec(q, sql);
        if (q.next())
        {
            writeToMessage(addUser, answer);
            tcp::listener().send(answer);
            return;
        }

        QString fields =
        " ID         "
        ",LOGIN      "
        ",PASSWORD   "
        ",LAST_VISIT ";

        sql = sql::INSERT_INTO(_usersTable, fields);

        QSqlQuery qResult {db::postgres::createResult(transact)};

        if (!qResult.prepare(sql))
        {
            writeToMessage(addUser, answer);
            tcp::listener().send(answer);
            return;
        }

        QDateTime curr = QDateTime::currentDateTime();

        sql::bindValue(qResult, ":ID"        , newId           );
        sql::bindValue(qResult, ":LOGIN"     , addUser.login   );
        sql::bindValue(qResult, ":PASSWORD"  , addUser.password);
        sql::bindValue(qResult, ":LAST_VISIT", curr            );

        if (!qResult.exec())
        {
            writeToMessage(addUser, answer);
            tcp::listener().send(answer);
            return;
        }

        addUser.resault = true;
        writeToMessage(addUser, answer, message->contentFormat());
        tcp::listener().send(answer);
    };
    std::thread t {tfunc};
    t.detach();
}

void Application::command_DeleteUser(const Message::Ptr& message)
{
    auto tfunc = [this, message]()
    {
        trd::ThreadIdLock threadIdLock(&_threadIds); (void) threadIdLock;
        data::DeleteUser deleteUser;
        readFromMessage(message, deleteUser);

        if (message->contentFormat() == SerializeFormat::Json)
        {
            QJsonDocument jsonDoc(QJsonDocument::fromJson(message->content()));
            deleteUser.userID   = jsonDoc.object().take("id"      ).toString();
            deleteUser.login    = jsonDoc.object().take("login"   ).toString();
            deleteUser.password = jsonDoc.object().take("password").toString();
        }

        Message::Ptr answer = message->cloneForAnswer();

        db::postgres::Driver::Ptr dbcon = pgpool().connect();
        db::postgres::Transaction::Ptr transact = dbcon->createTransact();
        QSqlQuery q0 {db::postgres::createResult(transact)};

        // Проверка логина и пароля пароля
        QString sql = QString(" SELECT * FROM %1 "
                              " WHERE ID = '%2'  "
                             ).arg(_usersTable, toString(deleteUser.userID));

        QString l = "";
        QString p = "";

        if (sql::exec(q0, sql))
        {
            if (q0.next())
            {
                QSqlRecord r = q0.record();
                sql::assignValue(l  ,r, "LOGIN"   );
                sql::assignValue(p  ,r, "PASSWORD");
            }
        }

        // Найти и удалить все карты юзера
        if (l == deleteUser.login && p == deleteUser.password)
        {
            sql = QString(" SELECT GPU_LIST_ID FROM %1 "
                          " WHERE USER_ID = '%2'            "
                          ).arg(_workersTable, toString(deleteUser.userID));

            QSqlQuery q1 {db::postgres::createResult(transact)};
            if (sql::exec(q1, sql))
            {
                QStringList allUserGPUID;

                while (q0.next())
                {
                    QVector<QUuidEx> gIDVec;

                    QSqlRecord r = q0.record();
                    sql::assignValue(gIDVec ,r, "GPU_LIST_ID");

                    for (const QUuidEx& gID : gIDVec)
                        allUserGPUID.append("'" + toString(gID) + "'");
                }

                // Удаление карт
                if (!allUserGPUID.isEmpty())
                {
                    sql = QString(" DELETE FROM %1 WHERE id IN (%2) "
                                 ).arg(_gpusTable, allUserGPUID.join(','));

                    QSqlQuery q2 {db::postgres::createResult(transact)};

                    // Если ошибка, вылетаем
                    if (!sql::exec(q2, sql))
                    {
                        writeToMessage(deleteUser, answer);
                        tcp::listener().send(answer);
                        return;
                    }
                }

                // Удалить всех воркеров юзера
                QSqlQuery q3 {db::postgres::createResult(transact)};
                sql = QString(" DELETE FROM %1       "
                              " WHERE USER_ID = '%2' "
                              ).arg(_workersTable,
                                    toString(deleteUser.userID));

                // Удалить самого юзера
                if (sql::exec(q3, sql))
                {
                    sql = QString(" DELETE FROM %1  "
                                  " WHERE ID = '%2' "
                                 ).arg(_usersTable,
                                       toString(deleteUser.userID));

                    QSqlQuery q4 {db::postgres::createResult(transact)};
                    if (sql::exec(q4, sql))
                        deleteUser.resault = true;
                }
            } // if q1
        } // if login password

        writeToMessage(deleteUser, answer, message->contentFormat());
        tcp::listener().send(answer);
    };
    std::thread t {tfunc};
    t.detach();
}

void Application::command_ChangeUserPassword(const Message::Ptr& message)
{
    auto tfunc = [this, message]()
    {
        trd::ThreadIdLock threadIdLock(&_threadIds); (void) threadIdLock;
        data::ChangeUserPassword changeUserPassword;
        readFromMessage(message, changeUserPassword);

        if (message->contentFormat() == SerializeFormat::Json)
        {
            QJsonDocument jsonDoc(QJsonDocument::fromJson(message->content()));
            changeUserPassword.userID   = jsonDoc.object().take("id"      ).toString();
            changeUserPassword.login    = jsonDoc.object().take("login"   ).toString();
            changeUserPassword.password = jsonDoc.object().take("password").toString();
        }

        Message::Ptr answer = message->cloneForAnswer();
        db::postgres::Driver::Ptr dbcon = pgpool().connect();
        db::postgres::Transaction::Ptr transact = dbcon->createTransact();

        QString sql = QString(" SELECT * FROM %1 "
                              " WHERE login = '%2'   "
                              ).arg(_usersTable, changeUserPassword.login);

        QSqlQuery q {db::postgres::createResult(transact)};
        sql::exec(q, sql);

        QUuidEx id;
        QString l = "";
        QString p = "";

        if (q.next())
        {
            QSqlRecord r = q.record();
            sql::assignValue(id ,r, "ID"      );
            sql::assignValue(l  ,r, "LOGIN"   );
            sql::assignValue(p  ,r, "PASSWORD");
        }
        else
        {
            writeToMessage(changeUserPassword, answer);
            tcp::listener().send(answer);
            return;
        }

        if (changeUserPassword.login != l && changeUserPassword.password != p)
        {
            writeToMessage(changeUserPassword, answer, message->contentFormat());
            tcp::listener().send(answer);
            return;
        }

        QString fields =
        " ID         "
        ",LOGIN      "
        ",PASSWORD   "
        ",LAST_VISIT ";

        sql = sql::INSERT_OR_UPDATE_PG(_usersTable, fields, "ID");

        QSqlQuery qResult {db::postgres::createResult(transact)};

        if (!qResult.prepare(sql))
        {
            writeToMessage(changeUserPassword, answer);
            tcp::listener().send(answer);
            return;
        }

        QDateTime curr = QDateTime::currentDateTime();

        sql::bindValue(qResult, ":ID"        , id                         );
        sql::bindValue(qResult, ":LOGIN"     , changeUserPassword.login   );
        sql::bindValue(qResult, ":PASSWORD"  , changeUserPassword.password);
        sql::bindValue(qResult, ":LAST_VISIT", curr                       );

        if (!qResult.exec())
        {
            writeToMessage(changeUserPassword, answer);
            tcp::listener().send(answer);
            return;
        }

        changeUserPassword.resault = true;
        writeToMessage(changeUserPassword, answer);
        tcp::listener().send(answer);
    };
    std::thread t {tfunc};
    t.detach();
}

void Application::command_UpdateWorkerInfoClient(const Message::Ptr& message)
{
    auto tfunc = [this, message]()
    {
        trd::ThreadIdLock threadIdLock(&_threadIds); (void) threadIdLock;

        data::UpdateWorkerInfoClient update;
        readFromMessage(message, update);

        if (message->contentFormat() == SerializeFormat::Json)
        {
            QJsonDocument jsonDoc(QJsonDocument::fromJson(message->content()));
            update.workerID      = jsonDoc.object().take("workerId"     ).toString();
            update.newWorkerName = jsonDoc.object().take("newWorkerName").toString();
            update.oldName       = jsonDoc.object().take("oldName"      ).toString();
            update.newPowerPrice = jsonDoc.object().take("newPowerPrice").toDouble();
            update.resault       = jsonDoc.object().take("resault"      ).toBool();
        }
        Message::Ptr answer = message->cloneForAnswer();

        // Обновить данные в БД
        db::postgres::Driver::Ptr dbcon = pgpool().connect();
        db::postgres::Transaction::Ptr transact = dbcon->createTransact();
        QSqlQuery qResult {db::postgres::createResult(transact)};

        const QString sql = QString(" UPDATE %1                    "
                                    " SET WORKER_NAME = '%2'       "
                                    " ,ELECTRICITY_COST = '%3'     "
                                    " WHERE ID = '%4'              "
                                    ).arg(_workersTable, update.newWorkerName,
                                    QString::number(update.newPowerPrice, 'f', 2),
                                    toString(update.workerID));

        if  (!sql::exec(qResult, sql))
        {
            writeToMessage(update, answer);
            tcp::listener().send(answer);
            return;
        }

        // Добавить в буффер задач на answer
        _updateWorkerInfoList.append(QPair(update, QDateTime::currentDateTime()));

        // Изменить в wdc
        for (data::workerData& w : wdc::wdc()._workers)
            if (w.id == update.workerID)
            {
                w.workerName = update.newWorkerName;
                w.electricityCost = update.newPowerPrice;
                break;
            }

        update.resault = true;
        writeToMessage(update, answer, message->contentFormat());
        tcp::listener().send(answer);
    };
    std::thread t {tfunc};
    t.detach();
}

void Application::command_CheckWorkerInfoAgent(const Message::Ptr& message)
{
    auto tfunc = [this, message]()
    {
        trd::ThreadIdLock threadIdLock(&_threadIds); (void) threadIdLock;
        data::CheckWorkerInfoAgent check;
        readFromMessage(message, check);

        Message::Ptr answer = message->cloneForAnswer();
        db::postgres::Driver::Ptr dbcon = pgpool().connect();
        db::postgres::Transaction::Ptr transact = dbcon->createTransact();
        QSqlQuery qResult {db::postgres::createResult(transact)};

        QString sql = QString(" SELECT           "
                              " WORKER_NAME,     "
                              " ELECTRICITY_COST "
                              " FROM %1          "
                              " WHERE ID = '%2'  "
                              ).arg(_workersTable, toString(check.workerID));

        if (sql::exec(qResult, sql))
        {
            while (qResult.next())
            {
                QSqlRecord r = qResult.record();
                sql::assignValue(check.workerName ,r, "WORKER_NAME"     );
                sql::assignValue(check.powerPrice ,r, "ELECTRICITY_COST");
            }
        }

        writeToMessage(check, answer);
        tcp::listener().send(answer);
    };
    std::thread t {tfunc};
    t.detach();
}

void Application::command_UpdateGPUSettings(const Message::Ptr& message)
{
    auto tfunc = [this, message]()
    {
        trd::ThreadIdLock threadIdLock(&_threadIds); (void) threadIdLock;
        data::UpdateGPUSettings updateGPU;
        readFromMessage(message, updateGPU);

        db::postgres::Driver::Ptr dbcon = pgpool().connect();
        db::postgres::Transaction::Ptr transact = dbcon->createTransact();
        QSqlQuery q {db::postgres::createResult(transact)};

        QString sql = QString(" SELECT WORKER_ID FROM %1 WHERE ID = '%2' ").arg(
                        _gpusTable, toString(updateGPU.GPUData.id));

        QUuidEx workerID;
        if (sql::exec(q, sql))
        {
            if (q.next())
            {
                QSqlRecord r = q.record();
                sql::assignValue(workerID ,r, "WORKER_ID");
            }
        }

        if (workerID != QUuidEx())
        {
            QSqlQuery qResult {db::postgres::createResult(transact)};

            // Обновить данные в БД
            const QString fields =
            "  ID            "
            " ,WORKER_ID     "
            " ,SET_FAN_SPEED "
            " ,SET_CORE      "
            " ,SET_MEM       "
            " ,SET_PL        "
            " ,NAME          "
            " ,BUS_ID        "
            " ,VENDOR        "
            " ,TOTAL_MEMORY  "
            " ,VBIOS_VERSION "
            " ,MIN_PL        "
            " ,DEFAULT_PL    "
            " ,MAX_PL        "
            " ,GPU_SYS_ID    ";

            sql = sql::INSERT_OR_UPDATE_PG(_gpusTable, fields, "ID");
            if (qResult.prepare(sql))
            {
                sql::bindValue(qResult, ":ID"            , updateGPU.GPUData.id          );
                sql::bindValue(qResult, ":WORKER_ID"     , workerID                      );
                sql::bindValue(qResult, ":SET_FAN_SPEED" , updateGPU.GPUData.setFanSpeed );
                sql::bindValue(qResult, ":SET_CORE"      , updateGPU.GPUData.setCore     );
                sql::bindValue(qResult, ":SET_MEM"       , updateGPU.GPUData.setMem      );
                sql::bindValue(qResult, ":SET_PL"        , updateGPU.GPUData.setPl       );
                sql::bindValue(qResult, ":NAME"          , updateGPU.GPUData.name        );
                sql::bindValue(qResult, ":BUS_ID"        , updateGPU.GPUData.busId       );
                sql::bindValue(qResult, ":VENDOR"        , updateGPU.GPUData.vendor      );
                sql::bindValue(qResult, ":TOTAL_MEMORY"  , updateGPU.GPUData.totalMemory );
                sql::bindValue(qResult, ":VBIOS_VERSION" , updateGPU.GPUData.vbiosVersion);
                sql::bindValue(qResult, ":MIN_PL"        , updateGPU.GPUData.minPl       );
                sql::bindValue(qResult, ":DEFAULT_PL"    , updateGPU.GPUData.defaultPl   );
                sql::bindValue(qResult, ":MAX_PL"        , updateGPU.GPUData.maxPl       );
                sql::bindValue(qResult, ":GPU_SYS_ID"    , updateGPU.GPUData.gpuId       );

                qResult.exec();
            }
        }

        // Обновить данные в WDC
        for (data::workerData& worker : wdc::wdc()._workers)
            if (worker.id == workerID)
            {
                bool isFind = false;
                for (data::gpuData& g : worker.devices)
                    if (g.id == updateGPU.GPUData.id)
                    {
                        g = updateGPU.GPUData;
                        isFind = true;
                        break;
                    }

                if (isFind)
                    break;
            }

        // Положить в очередь обновлений для Агента
        _updateWorkerGPUSettings.append(QPair(QPair(workerID, updateGPU),
                                              QDateTime::currentDateTime()));

    };
    std::thread t {tfunc};
    t.detach();
}
