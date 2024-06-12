#include "rcontrolagent_appl.h"

#include "shared/spin_locker.h"
#include "shared/logger/logger.h"
#include "shared/logger/format.h"
#include "shared/config/appl_conf.h"
#include "shared/config/logger_conf.h"
#include "shared/qt/logger_operators.h"
#include "shared/qt/version_number.h"

#include "pproto/commands/base.h"
#include "pproto/commands/pool.h"
#include "pproto/transport/tcp.h"
#include "pproto/transport/udp.h"
#include "pproto/logger_operators.h"

#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include <thread>
#include <QHostInfo>

#define log_error_m   alog::logger().error   (alog_line_location, "Application")
#define log_warn_m    alog::logger().warn    (alog_line_location, "Application")
#define log_info_m    alog::logger().info    (alog_line_location, "Application")
#define log_verbose_m alog::logger().verbose (alog_line_location, "Application")
#define log_debug_m   alog::logger().debug   (alog_line_location, "Application")
#define log_debug2_m  alog::logger().debug2  (alog_line_location, "Application")

#define KILL_TIMER(TIMER) {if (TIMER != -1) {killTimer(TIMER); TIMER = -1;}}

QUuidEx Application::_applId;
volatile bool Application::_stop = false;
std::atomic_int Application::_exitCode = {0};

Application::Application(int& argc, char** argv)
    : QCoreApplication(argc, argv)
{
    _stopTimerId = startTimer(1000);

    chk_connect_a(&config::observerBase(), &config::ObserverBase::changed,
                  this, &Application::reloadConfig)

    #define FUNC_REGISTRATION(COMMAND) \
        _funcInvokerAgent.registration(command:: COMMAND, &Application::command_##COMMAND, this);

    FUNC_REGISTRATION(AgentGetGPUData)
    FUNC_REGISTRATION(UpdateWorkerInfoAgent)
    FUNC_REGISTRATION(CheckWorkerInfoAgent)
    FUNC_REGISTRATION(UpdateGPUSettings)

    #undef FUNC_REGISTRATION
}

bool Application::init()
{
    loadSettings();

    getCpuData();
    getCpuTemp();
    getRamData();
    getStartTime();
    getDiskData();
    getIpData();
    getMacData();
    getKernelData();
    getLoadAverageData();
    getMotherboardData();
    getNvidiaData();
    getGPUsSystemData();
    speedCorrect();

    if (!config::base().getValue("application.id", _applId))
    {
        log_error_m << "Application id undefined. Set application.id in config";
        return false;
    }
    _worker.id = _applId;

    _socket.setMessageWebFlags(true);

    chk_connect_q(&_socket, &tcp::Socket::message,
                      this, &Application::message)

    chk_connect_q(&_socket, &tcp::Socket::connected,
                      this, &Application::socketConnected)

    chk_connect_q(&_socket, &tcp::Socket::disconnected,
                      this, &Application::socketDisconnected)

    chk_connect_q(&_networkManager, &QNetworkAccessManager::finished,
                  this,             &Application::readJsonDataFromMiner)

    QHostAddress host = QHostAddress::AnyIPv4;
    config::readHostAddress("host.address", host);

    int port = DEFAULT_PORT;
    config::base().getValue("host.port", port);

    _hostPoint.setAddress(host);
    _hostPoint.setPort(port);

    if (!_socket.init({QHostAddress::AnyIPv4, port}))
    {
        log_error << "Socket can't be initialized";
        return false;
    }

    connectToHost();

    if (!_socket.isConnected())
        _connectTimerId = startTimer(5*1000 /*5 сек*/);
//    loadSettings();

    return true;
}

void Application::deinit()
{
    _socket.stop();

    // Установить настройки разгона по умолчанию
    for (const data::gpuData& GPU : _worker.devices)
        setDefaultGPUSettings(GPU);
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
            KILL_TIMER(_connectTimerId)

            exit(_exitCode);
            return;
        }
    }
    else if (event->timerId() == _statTimerId)
    {
        if (_socket.isConnected() && _lockSendData)
        {
            getJsonDataFromMiner();

            getCpuData();
            getCpuTemp();
            getRamData();
            getStartTime();
            getDiskData();
            getIpData();
            getMacData();
            getKernelData();
            getLoadAverageData();
            getMotherboardData();
            getNvidiaData();
            getGPUsSystemData();

            speedCorrect();
            data::AgentStat worker;
            worker.agent = _worker;

            Message::Ptr m = createMessage(worker);
            m->appendDestinationPoint(_hostPoint);

            _socket.send(m);
        }

        return;
    }
    else if (event->timerId() == _connectTimerId)
    {
        connectToHost();

        return;
    }
}

void Application::command_AgentGetGPUData(const Message::Ptr& message)
{
    if (message->type() == Message::Type::Answer)
    {
        data::AgentGetGPUData gpuData;
        readFromMessage(message, gpuData);

        bool needUpdate = false;

        if (gpuData.gpuList.isEmpty())
            needUpdate = true;

        for (data::gpuData& workerGPU : _worker.devices)
        {
            bool GPUIsFindInDBData = false;
            for (const data::gpuData& dbGPU : gpuData.gpuList)
            {
                if (workerGPU == dbGPU)
                {
                    workerGPU.setFanSpeed = dbGPU.setFanSpeed;
                    workerGPU.setCore     = dbGPU.setCore    ;
                    workerGPU.setMem      = dbGPU.setMem     ;
                    workerGPU.setPl       = dbGPU.setPl      ;

                    acceptGPUSettings(workerGPU);
                    GPUIsFindInDBData = true;
                    break;
                }
            }

            if (!GPUIsFindInDBData)
            {
                workerGPU.setCore = 0;
                workerGPU.setMem = 0;
                workerGPU.setPl = workerGPU.defaultPl.toInt();
                workerGPU.fanSpeed = 50;

                acceptGPUSettings(workerGPU);
                needUpdate = true;
            }
        }

        if (needUpdate)
            _socket.send(createMessage(data::AgentUpdateGPUData(_worker)));
    }
}

void Application::command_UpdateWorkerInfoAgent(const Message::Ptr& message)
{
    data::UpdateWorkerInfoAgent update;
    readFromMessage(message, update);

    _worker.workerName = update.newInfo.newWorkerName;
    _worker.electricityCost = update.newInfo.newPowerPrice;

    saveSettings();
}

void Application::command_CheckWorkerInfoAgent(const Message::Ptr& message)
{
    if (message->type() == Message::Type::Answer)
    {
        data::CheckWorkerInfoAgent check;
        readFromMessage(message, check);

        if (_worker.workerName != check.workerName && !check.workerName.isEmpty())
            _worker.workerName = check.workerName;

        if (_worker.electricityCost != check.powerPrice && check.powerPrice)
            _worker.electricityCost = check.powerPrice;

        saveSettings();

        _lockSendData = true;
    }
}

void Application::command_UpdateGPUSettings(const Message::Ptr& message)
{
    data::UpdateGPUSettings updateGPU;
    readFromMessage(message, updateGPU);

    for (data::gpuData& g : _worker.devices)
    {
        if (g.id == updateGPU.GPUData.id)
        {
            g = updateGPU.GPUData;

            acceptGPUSettings(g);
            break;
        }
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

    if (lst::FindResult fr = _funcInvokerAgent.findCommand(message->command()))
    {
        if (command::pool().commandIsSinglproc(message->command()))
            message->markAsProcessed();
        _funcInvokerAgent.call(message, fr);
    }
}

void Application::socketConnected(pproto::SocketDescriptor socketDescript)
{
    KILL_TIMER(_connectTimerId)
    _socket.send(createMessage(data::AgentGetGPUData(_worker.id)));
    _socket.send(createMessage(data::CheckWorkerInfoAgent(_worker.id)));
}

void Application::socketDisconnected(pproto::SocketDescriptor socketDescript)
{
    KILL_TIMER(_statTimerId)
    _connectTimerId = startTimer(5*1000 /*5 сек*/);
}

void Application::reloadConfig()
{

}

QString Application::linuxTerminalRequest(const QString &request)
{
    QProcess *cmd = new QProcess();
    cmd->start("bash", QStringList() << "-c" << request);
    cmd->waitForFinished();
    QString result = cmd->readAll().data();
    cmd->deleteLater();
    return result;
}

void Application::connectToHost()
{
    _socket.start();

    int cnt = 3;
    while (cnt--)
    {
        usleep(50*1000);
        if (_socket.isConnected())
            break;
    }

    if (!_socket.isConnected())
        _socket.stop();
    else
        _statTimerId = startTimer(5*1000 /*5 сек*/);
}

void Application::getCpuData()
{
    _worker.cpuInfo = linuxTerminalRequest(
     "awk -F ': ' '/model name/ {print $2}' /proc/cpuinfo | uniq").remove('\n');
}

void Application::getCpuTemp()
{
//    QString res = linuxTerminalRequest(
//                      "cat /sys/class/thermal/thermal_zone*/temp").remove('\n');
//    _worker.cpuTemp = QString("%1").arg(res.toInt() / 1000);

    QString res = linuxTerminalRequest(
               "sensors | grep Tctl | awk '{printf $2}' | sed 's/[^0-9.]*//g'");
    _worker.cpuTemp = QString("%1").arg(res.toDouble());
}

void Application::getDiskData()
{
    QStorageInfo storage = QStorageInfo::root();
    double size = storage.bytesTotal();
    double free = storage.bytesAvailable();
    _worker.diskSize = QString("%1 Gb").arg(
                            (size / 1000 / 1000 / 1000));

    _worker.diskFreeSpace = QString("%1 Gb").arg(
                            (free / 1000 / 1000 / 1000));

    QString device = QString::fromStdString(
                                storage.device().toStdString()).remove("/dev/");
    QString res = linuxTerminalRequest("lsblk -d -o NAME,MODEL | awk '{print $1}'");
    QString temp = "";
    QStringList list;
    for (auto& symbol: res)
    {
        if(symbol != '\n')
            temp.push_back(symbol);
        else
        {
            list.push_back(temp);
            temp.clear();
        }
    }

    QString deviceName = "";
    for (auto& str : list)
    {
        if (device.contains(str))
        {
            deviceName = linuxTerminalRequest(
                               QString("lsblk -o MODEL %1").arg("/dev/" + str));
            break;
        }
    }

    while (deviceName.contains('\n'))
        deviceName.remove("\n");
    while (deviceName.contains("MODEL"))
        deviceName.remove("MODEL");

    _worker.diskModel = deviceName;
}

void Application::getRamData()
{
    double resTotal = linuxTerminalRequest(
                      "awk '/MemTotal/ { print $2 }' /proc/meminfo").toDouble();
    double resAvailable = linuxTerminalRequest(
                    "awk '/MemAvailable/ {print $2}' /proc/meminfo").toDouble();
    _worker.ramTotal = QString("%1 Gb").arg(
                        QString::number((resTotal / 1024 / 1024), 'f' , 2));
    _worker.ramFree = QString("%1 Gb").arg(
                QString::number((resAvailable / 1024 / 1024), 'f', 2));
}

void Application::getIpData()
{
    auto tfunc = [this]()
    {
        _worker.localIp = linuxTerminalRequest("hostname -I").remove('\n');
        _worker.extIp = linuxTerminalRequest("curl https://ipinfo.io/ip");
    };

   std::thread t {tfunc};
   t.detach();
}

void Application::getMacData()
{
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for(auto& interface : interfaces)
    {
        // Если этот сетевой интерфейс активируется и работает,
        // и это не адрес цикла, то MAC -адрес, который нам нужен
       if(interface.flags().testFlag(QNetworkInterface::IsUp)
               && interface.flags().testFlag(QNetworkInterface::IsRunning)
               && !interface.flags().testFlag(QNetworkInterface::IsLoopBack))
       {
           _worker.mac = interface.hardwareAddress();
           break;
       }
    }
}

void Application::getKernelData()
{
    _worker.kernelVersion = linuxTerminalRequest("uname -r").remove('\n');
}

void Application::getLoadAverageData()
{
    _worker.la1 = linuxTerminalRequest("awk '{print $1}' /proc/loadavg").remove('\n');
    _worker.la5 = linuxTerminalRequest("awk '{print $2}' /proc/loadavg").remove('\n');
    _worker.la15 = linuxTerminalRequest("awk '{print $3}' /proc/loadavg").remove('\n');
}

void Application::getMotherboardData()
{
    QString boardInfo = linuxTerminalRequest(
                "cat /sys/devices/virtual/dmi/id/board_{vendor,name}");

    while (boardInfo.contains('\n'))
        boardInfo.replace('\n', " ");

    _worker.motherboardInfo = boardInfo;
}

void Application::getNvidiaData()
{
    _worker.nvidiaVersion = linuxTerminalRequest(
                "nvidia-smi | grep Version | gawk '{print $6}'").remove('\n');
}

void Application::getRadeonData()
{
    _worker.amdVersion = "in development";
}

void Application::getStartTime()
{
    QString res = linuxTerminalRequest("awk '{print int($1)}' /proc/uptime");
    _worker.sysUptime = res.toULongLong();
}

void Application::getGPUsSystemData()
{
    // ------------- GPU COUNT
    int gpu_count = linuxTerminalRequest(
           "nvidia-smi --query-gpu=name --format=csv,noheader | wc -l").toInt();

    // ------------- INIT GPUS
    if (_worker.devices.size() != gpu_count)
    {
        _worker.devices.clear();

        for (int i = 0; i < gpu_count; ++i)
        {
            data::gpuData new_gpu;
            new_gpu.gpuId = i;
            _worker.devices.push_back(new_gpu);
        }

        // ------------- GPU IDS
        QString ids = linuxTerminalRequest(
                    "nvidia-smi --query-gpu=uuid --format=csv,noheader");
        QStringList idsList = ids.split('\n');
        for (QString& str : idsList)
        {
            if (str.isEmpty())
            {
                idsList.removeAll(str);
                continue;
            }

            str.remove("GPU-");
        }

        for (int i = 0; i < idsList.length(); ++i)
            _worker.devices[i].id = QUuidEx::fromString(idsList[i]);

        // ------------- GPU NAME
        QString names = linuxTerminalRequest(
                    "nvidia-smi --query-gpu=name --format=csv,noheader");
        QStringList namesList = names.split('\n');
        for (const QString& str : namesList)
            if (str.isEmpty())
                namesList.removeAll(str);

        for (int i = 0; i < namesList.length(); ++i)
            _worker.devices[i].name = namesList[i];

        // ------------- BUS ID
        QString busIds = linuxTerminalRequest(
                    "nvidia-smi --query-gpu=pci.bus_id --format=csv,noheader");
        QStringList BusIdsList = busIds.split('\n');
        for (auto& str : BusIdsList)
            if (str.isEmpty())
                BusIdsList.removeAll(str);

        for (int i = 0; i < BusIdsList.size(); ++i)
            _worker.devices[i].busId = BusIdsList[i].remove(0,9);

        // ------------- VENDOR
        QString vendors = linuxTerminalRequest(
                  "lspci -v -m | grep VGA -A 7 | grep SVendor "
                                                         "| gawk '{print $2}'");
        QStringList vendorsList = vendors.split('\n');
        for (auto& str : vendorsList)
            if (str.isEmpty())
                vendorsList.removeAll(str);

        for (int i = 0; i < vendorsList.size(); ++i)
            _worker.devices[i].vendor = vendorsList[i];

        // ------------- GPU TOTAL MEMORY
        QString totalMems = linuxTerminalRequest(
            "nvidia-smi --query-gpu=memory.total --format=csv,noheader "
                                                         "| gawk '{print $1}'");
        QStringList totalMemsList = totalMems.split('\n');
        for (auto& str : totalMemsList)
            if (str.isEmpty())
                totalMemsList.removeAll(str);

        for (int i = 0; i < totalMemsList.size(); ++i)
            _worker.devices[i].totalMemory = totalMemsList[i];

        // ------------- GPU BIOS VERSION
        QString bioses = linuxTerminalRequest(
            "nvidia-smi --query-gpu=vbios_version --format=csv,noheader");
        QStringList biosesList = bioses.split('\n');
        for (auto& str : biosesList)
            if (str.isEmpty())
                biosesList.removeAll(str);

        for (int i = 0; i < biosesList.size(); ++i)
            _worker.devices[i].vbiosVersion = biosesList[i];

        // ------------- GPU MIN PL
        QString minPls = linuxTerminalRequest(
            "nvidia-smi --query-gpu=power.min_limit --format=csv,noheader"
                                                         "| gawk '{print $1}'");
        QStringList minPlsList = minPls.split('\n');
        for (auto& str : minPlsList)
            if (str.isEmpty())
                minPlsList.removeAll(str);

        for (int i = 0; i < minPlsList.size(); ++i)
            _worker.devices[i].minPl = minPlsList[i];

        // ------------- GPU DEFAULT PL
        QString defPls = linuxTerminalRequest(
            "nvidia-smi --query-gpu=power.default_limit --format=csv,noheader"
                                                         "| gawk '{print $1}'");
        QStringList defPlsList = defPls.split('\n');
        for (auto& str : defPlsList)
            if (str.isEmpty())
                defPlsList.removeAll(str);

        for (int i = 0; i < defPlsList.size(); ++i)
            _worker.devices[i].defaultPl = defPlsList[i];

        // ------------- GPU MAX PL
        QString maxPls = linuxTerminalRequest(
            "nvidia-smi --query-gpu=power.max_limit --format=csv,noheader"
                                                         "| gawk '{print $1}'");
        QStringList maxPlsList = maxPls.split('\n');
        for (auto& str : maxPlsList)
            if (str.isEmpty())
                maxPlsList.removeAll(str);

        for (int i = 0; i < maxPlsList.size(); ++i)
            _worker.devices[i].maxPl = maxPlsList[i];

    } // ------------- INIT GPUS

    // ------------- GPU FAN SPEED
    QString fanSpeeds = linuxTerminalRequest(
        "nvidia-smi --query-gpu=fan.speed --format=csv,noheader"
                                                         "| gawk '{print $1}'");
    QStringList fanSpeedsList = fanSpeeds.split('\n');
    for (auto& str : fanSpeedsList)
        if (str.isEmpty())
            fanSpeedsList.removeAll(str);

    for (int i = 0; i < fanSpeedsList.size(); ++i)
        _worker.devices[i].fanSpeed = fanSpeedsList[i].toUInt();

    // ------------- GPU CORE CLOCK
    QString coreClocks = linuxTerminalRequest(
        "nvidia-smi --query-gpu=clocks.sm --format=csv,noheader"
                                                         "| gawk '{print $1}'");
    QStringList coreClocksList = coreClocks.split('\n');
    for (auto& str : coreClocksList)
        if (str.isEmpty())
            coreClocksList.removeAll(str);

    for (int i = 0; i < coreClocksList.size(); ++i)
        _worker.devices[i].coreClock = coreClocksList[i].toUInt();

    // ------------- GPU MEMORY CLOCK
    QString memClocks = linuxTerminalRequest(
        "nvidia-smi --query-gpu=clocks.current.memory --format=csv,noheader"
                                                         "| gawk '{print $1}'");
    QStringList memClocksList = memClocks.split('\n');
    for (auto& str : memClocksList)
        if (str.isEmpty())
            memClocksList.removeAll(str);

    for (int i = 0; i < memClocksList.size(); ++i)
        _worker.devices[i].memoryClock = memClocksList[i].toUInt();

    // ------------- GPU POWER USAGE
    QString pus = linuxTerminalRequest(
        "nvidia-smi --query-gpu=power.draw --format=csv,noheader"
                                                         "| gawk '{print $1}'");
    QStringList pusList = pus.split('\n');
    for (auto& str : pusList)
        if (str.isEmpty())
            pusList.removeAll(str);

    for (int i = 0; i < pusList.size(); ++i)
        _worker.devices[i].powerUsage = pusList[i].toDouble();

    // ------------- GPU CORE TEMPERATURE
    QString coreTemps = linuxTerminalRequest(
        "nvidia-smi --query-gpu=temperature.gpu --format=csv,noheader");

    QStringList coreTempsList = coreTemps.split('\n');
    for (auto& str : coreTempsList)
        if (str.isEmpty())
            coreTempsList.removeAll(str);

    for (int i = 0; i < coreTempsList.size(); ++i)
        _worker.devices[i].coreTemp = coreTempsList[i].toUInt();

//    // ------------- GPU MAX CORE CLOCK
//    QString maxCoreClocks = linuxTerminalRequest(
//        "nvidia-smi --query-gpu=clocks.max.graphics --format=csv,noheader"
//                                                         "| gawk '{print $1}'");
//    QStringList maxCoreClocksList = maxCoreClocks.split('\n');
//    for (auto& str : maxCoreClocksList)
//        if (str.isEmpty())
//            maxCoreClocksList.removeAll(str);

//    for (int i = 0; i < maxCoreClocksList.size(); ++i)
//        _worker.devices[i].maxCoreFreq = maxCoreClocksList[i].toUInt();

//    // ------------- GPU MAX MEMORY CLOCK
//    QString maxMemClocks = linuxTerminalRequest(
//        "nvidia-smi --query-gpu=clocks.max.memory --format=csv,noheader"
//                                                         "| gawk '{print $1}'");
//    QStringList maxMemClocksList = maxMemClocks.split('\n');
//    for (auto& str : maxMemClocksList)
//        if (str.isEmpty())
//            maxMemClocksList.removeAll(str);

//    for (int i = 0; i < maxMemClocksList.size(); ++i)
//        _worker.devices[i].maxMemFreq = maxMemClocksList[i].toUInt();
}

void Application::getJsonDataFromMiner()
{
    // ps aux | grep -v grep | grep GMiner

    QUrl url("http://0.0.0.0" + _minerPrefix);
    url.setPort(_minerPort);
    QNetworkRequest req(url);
    _networkManager.get(req);
}

void Application::readJsonDataFromMiner(QNetworkReply* reply)
{
    _worker.minerName.clear         ();
    _worker.algorithm1.clear        ();
    _worker.algorithm2.clear        ();
    _worker.algorithmZil.clear      ();
    _worker.server1.clear           ();
    _worker.server2.clear           ();
    _worker.serverZil.clear         ();
    _worker.user1.clear             ();
    _worker.user2.clear             ();
    _worker.userZil.clear           ();
    _worker.totalAcceptedShares1   = 0;
    _worker.totalAcceptedShares2   = 0;
    _worker.totalAcceptedSharesZil = 0;
    _worker.totalRejectedShares1   = 0;
    _worker.totalRejectedShares2   = 0;
    _worker.totalRejectedSharesZil = 0;
    _worker.totalStaleShares1      = 0;
    _worker.totalStaleShares2      = 0;
    _worker.totalStaleSharesZil    = 0;
    _worker.totalInvalidShares1    = 0;
    _worker.totalInvalidShares2    = 0;
    _worker.totalInvalidSharesZil  = 0;
    _worker.minerUptime            = 0;

    for (data::gpuData& g : _worker.devices)
    {
        g.speed1            = 0;
        g.speed2            = 0;
        g.speedZil          = 0;
        g.acceptedShares1   = 0;
        g.acceptedShares2   = 0;
        g.acceptedSharesZil = 0;
        g.rejectedShares1   = 0;
        g.rejectedShares2   = 0;
        g.rejectedSharesZil = 0;
        g.staleShares1      = 0;
        g.staleShares2      = 0;
        g.staleSharesZil    = 0;
        g.invalidShares1    = 0;
        g.invalidShares2    = 0;
        g.invalidSharesZil  = 0;
    }

    if(reply->error() != QNetworkReply::NoError)
    {
//        log_warn_m << "HTTP Request error, check miner API settings";

        return;
    }

    QByteArray bArrayReply = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(bArrayReply);

    if(jsonDoc.isNull())
    {
        log_error_m << "The requested JSON is Null";
        return;
    }

    QJsonObject json = jsonDoc.object();

    QByteArray bLower = bArrayReply.toLower();
    if(bLower.contains("gminer"))
    {
        //qDebug() << "GMiner";
        GMinerParser(json);
    }
    else if (bLower.contains("lolminer"))
    {
        //qDebug() << "lolMiner";
        lolMinerParser(json);
    }
    else if (bLower.contains("rigel"))
    {
        //qDebug() << "RigelMiner";
        RigelMinerParser(json);
    }
    else
    {
        log_error_m << "Unknown miner data";
        //qDebug() << "Unknown miner data";
    }
}

void Application::GMinerParser(QJsonObject& json)
{
    QString algos = json.take("algorithm").toString();
    QStringList algosList = algos.split(" + ");

    unsigned int countAlgos = algosList.size();
    bool zilFlag = algosList.contains("ZIL");

    QJsonArray devices = json.take("devices").toArray();
    for (auto d : devices)
    {
        QJsonObject deviceJson = d.toObject();
        unsigned int gpuId = deviceJson.take("gpu_id").toInt();

        for (auto& g : _worker.devices)
        {
            if (g.gpuId == gpuId)
            {
                if (countAlgos == 1)
                {
                    g.speed1            = deviceJson.take("speed"           ).toInt();
                    g.acceptedShares1   = deviceJson.take("accepted_shares" ).toInt();
                    g.rejectedShares1   = deviceJson.take("rejected_shares" ).toInt();
                    g.staleShares1      = deviceJson.take("stale_shares"    ).toInt();
                    g.invalidShares1    = deviceJson.take("invalid_shares"  ).toInt();
                }

                if (countAlgos == 2 && zilFlag)
                {
                    g.speed1            = deviceJson.take("speed"           ).toInt();
                    g.speedZil          = deviceJson.take("speed2"          ).toInt();
                    g.acceptedShares1   = deviceJson.take("accepted_shares" ).toInt();
                    g.acceptedSharesZil = deviceJson.take("accepted_shares2").toInt();
                    g.rejectedShares1   = deviceJson.take("rejected_shares" ).toInt();
                    g.rejectedSharesZil = deviceJson.take("rejected_shares2").toInt();
                    g.staleShares1      = deviceJson.take("stale_shares"    ).toInt();
                    g.staleSharesZil    = deviceJson.take("stale_shares2"   ).toInt();
                    g.invalidShares1    = deviceJson.take("invalid_shares"  ).toInt();
                    g.invalidSharesZil  = deviceJson.take("invalid_shares2" ).toInt();
                }

                if (countAlgos == 2 && !zilFlag)
                {
                    g.speed1            = deviceJson.take("speed"           ).toInt();
                    g.speed2            = deviceJson.take("speed2"          ).toInt();
                    g.acceptedShares1   = deviceJson.take("accepted_shares" ).toInt();
                    g.acceptedShares2   = deviceJson.take("accepted_shares2").toInt();
                    g.rejectedShares1   = deviceJson.take("rejected_shares" ).toInt();
                    g.rejectedShares2   = deviceJson.take("rejected_shares2").toInt();
                    g.staleShares1      = deviceJson.take("stale_shares"    ).toInt();
                    g.staleShares2      = deviceJson.take("stale_shares2"   ).toInt();
                    g.invalidShares1    = deviceJson.take("invalid_shares"  ).toInt();
                    g.invalidShares2    = deviceJson.take("invalid_shares2" ).toInt();
                }

                if (countAlgos == 3 && zilFlag)
                {
                    g.speed1            = deviceJson.take("speed"           ).toInt();
                    g.speed2            = deviceJson.take("speed2"          ).toInt();
                    g.speedZil          = deviceJson.take("speed3"          ).toInt();
                    g.acceptedShares1   = deviceJson.take("accepted_shares" ).toInt();
                    g.acceptedShares2   = deviceJson.take("accepted_shares2").toInt();
                    g.acceptedSharesZil = deviceJson.take("accepted_shares3").toInt();
                    g.rejectedShares1   = deviceJson.take("rejected_shares" ).toInt();
                    g.rejectedShares2   = deviceJson.take("rejected_shares2").toInt();
                    g.rejectedSharesZil = deviceJson.take("rejected_shares3").toInt();
                    g.staleShares1      = deviceJson.take("stale_shares"    ).toInt();
                    g.staleShares2      = deviceJson.take("stale_shares2"   ).toInt();
                    g.staleSharesZil    = deviceJson.take("stale_shares3"   ).toInt();
                    g.invalidShares1    = deviceJson.take("invalid_shares"  ).toInt();
                    g.invalidShares2    = deviceJson.take("invalid_shares2" ).toInt();
                    g.invalidSharesZil  = deviceJson.take("invalid_shares3" ).toInt();
                }
            }
        } // for worker devices
    } // for devicesJsonArray

    _worker.minerUptime            = json.take("uptime"               ).toInt();
    _worker.minerName              = json.take("miner"                ).toString();
    _worker.server1                = json.take("server"               ).toString();
    _worker.user1                  = json.take("user"                 ).toString();
    _worker.totalAcceptedShares1   = json.take("total_accepted_shares").toInt();
    _worker.totalRejectedShares1   = json.take("total_rejected_shares").toInt();
    _worker.totalStaleShares1      = json.take("total_stale_shares"   ).toInt();
    _worker.totalInvalidShares1    = json.take("total_invalid_shares" ).toInt();

    if (countAlgos)
        _worker.algorithm1   = algosList[0];

    // Второй алго - Zil
    if (countAlgos == 2 && zilFlag)
    {
        _worker.serverZil              = json.take("server2"               ).toString();
        _worker.userZil                = json.take("user2"                 ).toString();
        _worker.totalAcceptedSharesZil = json.take("total_accepted_shares2").toInt();
        _worker.totalRejectedSharesZil = json.take("total_rejected_shares2").toInt();
        _worker.totalStaleSharesZil    = json.take("total_stale_shares2"   ).toInt();
        _worker.totalInvalidSharesZil  = json.take("total_invalid_shares2" ).toInt();

        _worker.algorithmZil = algosList[1];
    }
    // Второй алго - не Zil
    else if (countAlgos == 2 && !zilFlag)
    {
        _worker.server2                = json.take("server2"               ).toString();
        _worker.user2                  = json.take("user2"                 ).toString();
        _worker.totalAcceptedShares2   = json.take("total_accepted_shares2").toInt();
        _worker.totalRejectedShares2   = json.take("total_rejected_shares2").toInt();
        _worker.totalStaleShares2      = json.take("total_stale_shares2"   ).toInt();
        _worker.totalInvalidShares2    = json.take("total_invalid_shares2" ).toInt();

        _worker.algorithm2   = algosList[1];
    }

    // Третий алго - Zil
    if (countAlgos == 3 && zilFlag)
    {
        _worker.server2                = json.take("server2"               ).toString();
        _worker.serverZil              = json.take("server3"               ).toString();
        _worker.user2                  = json.take("user2"                 ).toString();
        _worker.userZil                = json.take("user3"                 ).toString();
        _worker.totalAcceptedShares2   = json.take("total_accepted_shares2").toInt();
        _worker.totalAcceptedSharesZil = json.take("total_accepted_shares3").toInt();
        _worker.totalRejectedShares2   = json.take("total_rejected_shares2").toInt();
        _worker.totalRejectedSharesZil = json.take("total_rejected_shares3").toInt();
        _worker.totalStaleShares2      = json.take("total_stale_shares2"   ).toInt();
        _worker.totalStaleSharesZil    = json.take("total_stale_shares3"   ).toInt();
        _worker.totalInvalidShares2    = json.take("total_invalid_shares2" ).toInt();
        _worker.totalInvalidSharesZil  = json.take("total_invalid_shares3" ).toInt();

        _worker.algorithm2   = algosList[1];
        _worker.algorithmZil = algosList[2];
    }

clearDataGPUsInsideParsers(countAlgos, zilFlag);
}

void Application::lolMinerParser(QJsonObject& json)
{

}

void Application::RigelMinerParser(QJsonObject& json)
{
    QString minerName    = json.take("name"   ).toString();
    QString minerVersion = json.take("version").toString();
    _worker.minerUptime  = json.take("uptime" ).toInt   ();
    _worker.minerName    = QString("%1 %2").arg(minerName, minerVersion);

    QString algos = json.take("algorithm").toString();
    QStringList algosList = algos.split("+");

    for (auto& s : algosList)
        s = s.toLower();

    unsigned short int countAlgos = algosList.size();
    bool zilFlag = algosList.contains("zil");

    QJsonObject statTotal = json.take("solution_stat").toObject();
    QJsonObject pools     = json.take("pools"        ).toObject();

    for (unsigned short int i = 0; i < countAlgos; ++i)
    {
        QJsonObject stat = statTotal.take(algosList[i]).toObject();
        QJsonArray pool = pools.take(algosList[i]).toArray();
        QJsonObject zero = pool.first().toObject();
        QJsonObject connectionDetails = zero.take("connection_details").toObject();

        if (stat.isEmpty())
            continue;

        if (i == 0)
        {
            _worker.algorithm1 = algosList[i];
            _worker.totalAcceptedShares1 = stat.take("accepted").toInt();
            _worker.totalRejectedShares1 = stat.take("rejected").toInt();
            _worker.totalInvalidShares1  = stat.take("invalid" ).toInt();
            _worker.totalStaleShares1    = 0;
            unsigned int port = connectionDetails.take("port").toInt();
            _worker.server1 =
                    QString(connectionDetails.take("hostname").toString()
                                                      + ":%1").arg(port);

            QString user = connectionDetails.take("username").toString();
            short int pos = user.indexOf('.');
            pos != -1 ? _worker.user1 = user.left(pos) : _worker.user1 = user;
        }

        if ((i == 1 && !zilFlag) || (i == 1 && zilFlag && countAlgos >= 3))
        {
            _worker.algorithm2 = algosList[i];
            _worker.totalAcceptedShares2 = stat.take("accepted").toInt();
            _worker.totalRejectedShares2 = stat.take("rejected").toInt();
            _worker.totalInvalidShares2  = stat.take("invalid" ).toInt();
            _worker.totalStaleShares2    = 0;
            unsigned int port = connectionDetails.take("port").toInt();
            _worker.server2 =
                    QString(connectionDetails.take("hostname").toString()
                                                      + ":%1").arg(port);

            QString user = connectionDetails.take("username").toString();
            short int pos = user.indexOf('.');
            pos != -1 ? _worker.user2 = user.left(pos) : _worker.user2 = user;
        }

        if ((i == 1 && zilFlag && countAlgos == 2) || (i == 2 && zilFlag))
        {
            _worker.algorithmZil = algosList[i];
            _worker.totalAcceptedSharesZil = stat.take("accepted").toInt();
            _worker.totalRejectedSharesZil = stat.take("rejected").toInt();
            _worker.totalInvalidSharesZil  = stat.take("invalid" ).toInt();
            _worker.totalStaleSharesZil    = 0;
            unsigned int port = connectionDetails.take("port").toInt();
            _worker.serverZil =
                    QString(connectionDetails.take("hostname").toString()
                                                      + ":%1").arg(port);

            QString user = connectionDetails.take("username").toString();
            short int pos = user.indexOf('.');
            pos != -1 ? _worker.userZil = user.left(pos) : _worker.userZil = user;
        }
    }

    // GPU DATA
    QJsonArray devices = json.take("devices").toArray();
    for (auto d : devices)
    {
        QJsonObject deviceJson = d.toObject();
        unsigned short int gpuId = deviceJson.take("id").toInt();

        for (auto& g : _worker.devices)
        {
            if (g.gpuId == gpuId)
            {
                QJsonObject statAll = deviceJson.take("solution_stat").toObject();
                QJsonObject statSpeed = deviceJson.take("hashrate").toObject();

                unsigned int counter = 1;
                for (unsigned short int i = 0; i < countAlgos; ++i)
                {
                    QJsonObject stat = statAll.take(algosList[i]).toObject();

                    if (stat.isEmpty())
                        continue;

                    if (counter == 1)
                    {
                        g.speed1 = statSpeed.take(algosList[i]).toDouble() / 10000;
                        g.acceptedShares1   = stat.take("accepted").toInt();
                        g.rejectedShares1   = stat.take("rejected").toInt();
                        g.invalidShares1    = stat.take("invalid" ).toInt();
                        g.staleShares1      = 0;
                    }

                    if ((counter == 2 && zilFlag && countAlgos == 2)
                                                 || (counter == 3 && zilFlag))
                    {
                        g.speedZil = statSpeed.take(algosList[i]).toDouble() / 10000;
                        g.acceptedSharesZil   = stat.take("accepted").toInt();
                        g.rejectedSharesZil   = stat.take("rejected").toInt();
                        g.invalidSharesZil    = stat.take("invalid" ).toInt();
                        g.staleSharesZil      = 0;
                    }

                    if ((counter == 2 && !zilFlag)
                                      || (counter == 2 && zilFlag && countAlgos >= 3))
                    {
                        g.speed2 = statSpeed.take(algosList[i]).toDouble() / 10000;
                        g.acceptedShares2   = stat.take("accepted").toInt();
                        g.rejectedShares2   = stat.take("rejected").toInt();
                        g.invalidShares2    = stat.take("invalid" ).toInt();
                        g.staleShares2      = 0;
                    }
                    ++counter;
                }
            } // if gpuId
        } // for worker devices
    } // for json devices

clearDataGPUsInsideParsers(countAlgos, zilFlag);
}

void Application::clearDataGPUsInsideParsers(const unsigned short& countAlgos,
                                             const bool& zilFlag)
{
    if (countAlgos == 1)
    {
        _worker.algorithm2.clear        ();
        _worker.algorithmZil.clear      ();
        _worker.server2.clear           ();
        _worker.serverZil.clear         ();
        _worker.user2.clear             ();
        _worker.userZil.clear           ();
        _worker.totalAcceptedShares2   = 0;
        _worker.totalAcceptedSharesZil = 0;
        _worker.totalRejectedShares2   = 0;
        _worker.totalRejectedSharesZil = 0;
        _worker.totalStaleShares2      = 0;
        _worker.totalStaleSharesZil    = 0;
        _worker.totalInvalidShares2    = 0;
        _worker.totalInvalidSharesZil  = 0;

        for (auto& g : _worker.devices)
        {
            g.speed2            = 0;
            g.speedZil          = 0;
            g.acceptedShares2   = 0;
            g.acceptedSharesZil = 0;
            g.rejectedShares2   = 0;
            g.rejectedSharesZil = 0;
            g.staleShares2      = 0;
            g.staleSharesZil    = 0;
            g.invalidShares2    = 0;
            g.invalidSharesZil  = 0;
        }
    }

    if (countAlgos == 2 && zilFlag)
    {
        _worker.algorithm2.clear      ();
        _worker.server2.clear         ();
        _worker.user2.clear           ();
        _worker.totalAcceptedShares2 = 0;
        _worker.totalRejectedShares2 = 0;
        _worker.totalStaleShares2    = 0;
        _worker.totalInvalidShares2  = 0;

        for (auto& g : _worker.devices)
        {
            g.speed2            = 0;
            g.acceptedShares2   = 0;
            g.rejectedShares2   = 0;
            g.staleShares2      = 0;
            g.invalidShares2    = 0;
        }
    }

    if (countAlgos == 2 && !zilFlag)
    {
        _worker.algorithmZil.clear      ();
        _worker.serverZil.clear         ();
        _worker.userZil.clear           ();
        _worker.totalAcceptedSharesZil = 0;
        _worker.totalRejectedSharesZil = 0;
        _worker.totalStaleSharesZil    = 0;
        _worker.totalInvalidSharesZil  = 0;

        for (auto& g : _worker.devices)
        {
            g.speedZil          = 0;
            g.acceptedSharesZil = 0;
            g.rejectedSharesZil = 0;
            g.staleSharesZil    = 0;
            g.invalidSharesZil  = 0;
        }
    }
}

void Application::speedCorrect()
{
    if (_worker.minerName.toLower().contains("rigel"))
    {
        for (data::gpuData& g : _worker.devices)
        {
            g.speed1 *= 10000;
            g.speed2 *= 10000;
            g.speedZil *= 10000;
        }
    }
}

void Application::acceptGPUSettings(const data::gpuData& GPU)
{
    // Core Clock
    if (GPU.setCore)
    {
        const QString setCoreClock =
                QString(" sudo nvidia-smi --lock-gpu-clocks=%2 -i %1 ")
                        .arg(QString::number(GPU.gpuId),
                             QString::number(GPU.setCore));
        linuxTerminalRequest(setCoreClock);
    }
    else
        linuxTerminalRequest(QString(" sudo nvidia-smi -rgc -i %1").arg(
                                                   QString::number(GPU.gpuId)));
    // Memory Clock
    if (GPU.setMem)
    {
        const QString setMemoryClock =
            QString(" nvidia-settings -a "
                    "[gpu:%1]/GPUMemoryTransferRateOffsetAllPerformanceLevels=%2 ")
                                            .arg(QString::number(GPU.gpuId),
                                                 QString::number(GPU.setMem*2));
//        const QString setMemoryClock =
//                QString(" sudo nvidia-smi --lock-memory-clocks=%2 -i %1 ")
//                        .arg(QString::number(GPU.gpuId),
//                             QString::number(GPU.setMem));
        linuxTerminalRequest(setMemoryClock);
    }
    else
        linuxTerminalRequest(QString(" sudo nvidia-smi -rmc -i %1 ").arg(
                                                   QString::number(GPU.gpuId)));

    // Power Limit
    const QString setPowerLimit =
            QString(" sudo nvidia-smi -i %1 -pl %2 ")
                    .arg(QString::number(GPU.gpuId),
                        GPU.setPl ? QString::number(GPU.setPl) : GPU.defaultPl);
    linuxTerminalRequest(setPowerLimit);

    // FAN Speed
    const QString setFunSpeed =
            QString(" nvidia-settings -c :%1 -a          "
                    " '[gpu:%1]/GPUFanControlState=1' -a "
                    " '[fan:0]/GPUTargetFanSpeed='%2 -a  "
                    " '[fan:1]/GPUTargetFanSpeed=%2'     ")
                    .arg(QString::number(GPU.gpuId),
                         QString::number(GPU.setFanSpeed));
    linuxTerminalRequest(setFunSpeed);
}

void Application::setDefaultGPUSettings(const data::gpuData& GPU)
{
    // Сброс частоты памяти
    linuxTerminalRequest(QString(" sudo nvidia-smi -rmc -i %1 ").arg(
                                               QString::number(GPU.gpuId)));
    const QString setMemoryClock =
        QString(" nvidia-settings -a "
                "[gpu:%1]/GPUMemoryTransferRateOffsetAllPerformanceLevels=0 ")
                                        .arg(QString::number(GPU.gpuId));
    linuxTerminalRequest(setMemoryClock);

    // Сброс частоты ядра
    linuxTerminalRequest(QString(" sudo nvidia-smi -rgc -i %1 ").arg(
                                               QString::number(GPU.gpuId)));

    // Сброс Power Limit
    const QString setPowerLimit =
            QString(" sudo nvidia-smi -i %1 -pl %2 ")
                    .arg(QString::number(GPU.gpuId), GPU.defaultPl);
    linuxTerminalRequest(setPowerLimit);
}

void Application::loadSettings()
{
    if (!config::state().getValue("electricity", _worker.electricityCost))
        config::state().setValue("electricity", QString("0"));
    if (!config::state().getValue("name", _worker.workerName))
        config::state().setValue("name", QString());
}

void Application::saveSettings()
{
    config::state().setValue("electricity", QString::number(
                                            _worker.electricityCost, 'f', 2));
    config::state().setValue("name", _worker.workerName);

    config::state().saveFile();
}
