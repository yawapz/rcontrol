#pragma once

#include "rcontrolctl_wdc.h"

#include "routing.h"
#include "shared/simple_timer.h"
#include "shared/steady_timer.h"
#include "shared/qt/network/interfaces.h"
#include "shared/spin_locker.h"
#include "shared/logger/logger.h"
#include "shared/logger/format.h"
#include "shared/config/appl_conf.h"
#include "shared/config/logger_conf.h"
#include "shared/qt/logger_operators.h"
#include "shared/qt/version_number.h"
#include "commands/commands.h"
#include "commands/error.h"
#include "pproto/func_invoker.h"
#include "pproto/transport/tcp.h"
#include "pproto/commands/base.h"
#include "pproto/commands/pool.h"
#include "pproto/transport/tcp.h"
#include "pproto/transport/udp.h"
#include "pproto/logger_operators.h"
#include "database/sql_func.h"
#include "database/postgres_pool.h"

#include <QtCore>
#include <QCoreApplication>
#include <atomic>
#include <chrono>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include <thread>
#include <QHostInfo>

using namespace std;
using namespace pproto;
using namespace pproto::transport;

class Application : public QCoreApplication
{
public:
    Application(int& argc, char** argv);

    bool init();
    void deinit();

    void sendSettings();

    static void stop() {_stop = true;}
    static bool isStopped() {return _stop;}

signals:
    void signalWorkerDataComing(data::workerData);

public slots:
    void stop(int exitCode);
    void message(const pproto::Message::Ptr&);

    void socketConnected(pproto::SocketDescriptor);
    void socketDisconnected(pproto::SocketDescriptor);

private slots:

private:
    Q_OBJECT
    void timerEvent(QTimerEvent* event) override;

    // --- Обработчики команд ---
    void command_CtlDiscovery(const Message::Ptr&);
    void command_AgentStat(const Message::Ptr&);
    void command_UserLogin(const Message::Ptr&);
    void command_UserWorkerList(const Message::Ptr&);
    void command_AddWorker(const Message::Ptr&);
    void command_DeleteWorker(const Message::Ptr&);
    void command_AgentGetGPUData(const Message::Ptr&);
    void command_AgentUpdateGPUData(const Message::Ptr&);
    void command_AddUser(const Message::Ptr&);
    void command_DeleteUser(const Message::Ptr&);
    void command_ChangeUserPassword(const Message::Ptr&);
    void command_UpdateWorkerInfoClient(const Message::Ptr&);
    void command_CheckWorkerInfoAgent(const Message::Ptr&);
    void command_UpdateGPUSettings(const Message::Ptr&);

    void netInterfacesUpdate();

    void loadSettings();
    void saveSettings();
    void reloadConfig();
    QList<data::gpuData> readWorkerGPUDataFromDB(const QVector<QUuidEx>&,
                                                db::postgres::Transaction::Ptr);
    void writeOrUpdateWorkerGPUDataInDB(const data::workerData&,
                                                db::postgres::Transaction::Ptr);

    void setWorkerStatusByRelevanceData();
    bool checkSQLTables();

private:
    mutable atomic_flag _relevanceDataLocker = ATOMIC_FLAG_INIT;

    static QUuidEx _applId;
    static volatile bool _stop;
    static std::atomic_int _exitCode;

    int _stopTimerId                    = {-1};
    int _workerChkRelevanceDataTimerId  = {-1};
    int _clearHistoryTimerId            = {-1};
    int _clearOldDbRecordTimerId        = {-1};

    QString _usersTable = {"users"};
    QString _workersTable = {"workers"};
    QString _gpusTable = {"gpus"};

    QString _applName;
    qint32  _applIndex = {0};

    FunctionInvoker _funcInvoker;

    network::Interface::List _netInterfaces;
    simple_timer _netInterfacesTimer {0};    

    // Список идентификаторов потоков для выполнения sql-запросов
    trd::ThreadIdList _threadIds;

    // Список задач на обновление информации на агенте
    QList<QPair<data::UpdateWorkerInfoClient, QDateTime>> _updateWorkerInfoList;
    QList<QPair<QPair<QUuidEx, data::UpdateGPUSettings>, QDateTime>>
                                                       _updateWorkerGPUSettings;
};
