#pragma once

#include "routing.h"
#include "shared/simple_timer.h"
#include "shared/steady_timer.h"
#include "shared/qt/network/interfaces.h"

#include "pproto/func_invoker.h"
#include "pproto/transport/tcp.h"

#include "commands/commands.h"
//#include "commands/error.h"

#include <QtCore>
#include <QCoreApplication>
#include <QNetworkInterface>
#include <atomic>
#include <chrono>

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
    void command_AgentGetGPUData(const Message::Ptr&);
    void command_UpdateWorkerInfoAgent(const Message::Ptr&);
    void command_CheckWorkerInfoAgent(const Message::Ptr&);
    void command_UpdateGPUSettings(const Message::Ptr&);

    void loadSettings();
    void saveSettings();
    void reloadConfig();

    QString linuxTerminalRequest(const QString&);

private:
    static QUuidEx _applId;
    static volatile bool _stop;
    static std::atomic_int _exitCode;

    HostPoint _hostPoint;
    data::workerData _worker;
    tcp::Socket _socket;
    QNetworkAccessManager _networkManager;

    unsigned int _minerPort   = {5000};
    QString      _minerPrefix = {"/stat"};

    int _stopTimerId = {-1};
    int _statTimerId = {-1};
    int _connectTimerId = {-1};
    unsigned short int _statTimerPeriod = {5};

    int _deviceIndex = 0;

    qint32  _applIndex = {0};

    FunctionInvoker _funcInvokerAgent;

    bool _lockSendData = {false};

private:
    void connectToHost();

    void getCpuData();
    void getCpuTemp();

    void getDiskData();
    void getRamData();

    void getIpData();
    void getMacData();

    void getKernelData();
    void getLoadAverageData();
    void getMotherboardData();

    void getNvidiaData();
    void getRadeonData();

    void getStartTime();
    void getGPUsSystemData();

    void getJsonDataFromMiner();

    void GMinerParser(QJsonObject&);
    void lolMinerParser(QJsonObject&);
    void RigelMinerParser(QJsonObject&);
    void clearDataGPUsInsideParsers(const unsigned short int&, const bool&);

    void speedCorrect();

    void acceptGPUSettings(const data::gpuData&);
    void setDefaultGPUSettings(const data::gpuData&);
private slots:
    void readJsonDataFromMiner(QNetworkReply*);
};
