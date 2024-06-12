/*****************************************************************************
  В модуле представлен список идентификаторов команд для коммуникации между
  клиентской и серверной частями приложения.
  В данном модуле представлен список команд персональный для этого приложения.

  Требование надежности коммуникаций: однажды назначенный идентификатор коман-
  ды не должен более меняться.
*****************************************************************************/

#pragma once

#include "pproto/commands/base.h"

namespace pproto {
namespace command {

//----------------------------- Список команд --------------------------------

/**
  Команда используется для поиска экземпляров BoostpumpCtl. Команда отправляется
  как широковещательное сообщение, так же она используется в качестве ответа
*/
extern const QUuidEx CtlDiscovery;

/**
  Команда используется для отправки статистики с агента на ctl
*/
extern const QUuidEx AgentStat;

/**
  Команда получения агентом данных о картах из БД
*/
extern const QUuidEx AgentGetGPUData;

/**
  Команда обновления данных о картах  агента в БД
*/
extern const QUuidEx AgentUpdateGPUData;

/**
  Команда авторизации пользователя
*/
extern const QUuidEx UserLogin;

/**
  Команда получения списка воркеров для конкретного пользователя
*/
extern const QUuidEx UserWorkerList;

/**
  Команда добавления воркера
*/
extern const QUuidEx AddWorker;

/**
  Команда удаления воркера
*/
extern const QUuidEx DeleteWorker;

/**
  Команда регистрации пользователя
*/
extern const QUuidEx AddUser;

/**
  Команда удаления пользователя
*/
extern const QUuidEx DeleteUser;

/**
  Команда изменения пароля пользователя
*/
extern const QUuidEx ChangeUserPassword;

/**
  Команда обновления данных воркера для стороны пользователя
*/
extern const QUuidEx UpdateWorkerInfoClient;

/**
  Команда обновления данных воркера для стороны агента
*/
extern const QUuidEx UpdateWorkerInfoAgent;

/**
  Команда обновления данных воркера для стороны агента при подключении к серверу
*/
extern const QUuidEx CheckWorkerInfoAgent;

/**
  Команда обновления данных видеокарты для агента и ctl
*/
extern const QUuidEx UpdateGPUSettings;

} // namespace command

struct widgetListSizeNormalize
{
    qint16 WorkerName   = {0};
    qint16 WorkerStatus = {0};
    qint16 MinerUpTime  = {0};
    qint16 WorkerSpeed  = {0};
    qint16 MinerName    = {0};
    qint16 workerErr    = {0};
    qint16 GpuCount     = {0};
    qint16 workerLA     = {0};
    qint16 workerFAN    = {0};
    qint16 WorkerPower  = {0};
};

//---------------- Структуры данных используемые в сообщениях ----------------

struct Settings
{
    QString host = {"127.0.0.1"};
    qint32 port = {61121};
    QString login = {"user"};
    QString password = {"unknown"};
};

namespace data {

struct gpuData
{
    QUuidEx id;

    // Данные из конфига

    unsigned int setFanSpeed       = {0};
    int setCore                    = {0};
    int setMem                     = {0};
    int setPl                      = {0};

    // Данные из системы

    QString name                        ;
    QString busId                       ;
    QString vendor                      ;
    QString totalMemory                 ;
    QString vbiosVersion                ;
    QString minPl                       ;
    QString defaultPl                   ;
    QString maxPl                       ;

    unsigned int gpuId             = {0};
    unsigned int fanSpeed          = {0};
    unsigned int coreClock         = {0};
    unsigned int memoryClock       = {0};
    double powerUsage              = {0};
    unsigned int coreTemp          = {0};
    unsigned int memoryTemp        = {0};
//    unsigned int maxCoreFreq       = {0};
//    unsigned int maxMemFreq        = {0};

    // Данные из майнера

    long long int speed1           = {0};
    long long int speed2           = {0};
    long long int speedZil         = {0};

    unsigned int acceptedShares1   = {0};
    unsigned int acceptedShares2   = {0};
    unsigned int acceptedSharesZil = {0};

    unsigned int rejectedShares1   = {0};
    unsigned int rejectedShares2   = {0};
    unsigned int rejectedSharesZil = {0};

    unsigned int staleShares1      = {0};
    unsigned int staleShares2      = {0};
    unsigned int staleSharesZil    = {0};

    unsigned int invalidShares1    = {0};
    unsigned int invalidShares2    = {0};
    unsigned int invalidSharesZil  = {0};

    bool operator==(const gpuData& obj) const
    {
        if (this->id  == obj.id)
            return true;
        else
            return false;
    }

    DECLARE_B_SERIALIZE_FUNC
};

struct workerData
{
    // Данные из конфига

    QString workerName             ;
    QUuidEx id                     ;
    double electricityCost = {0.0} ;

    // Данные из системы
    QDateTime startup = {QDateTime::fromSecsSinceEpoch(0)};

    QString version                ;
    QList<gpuData> devices         ;
    long long int sysUptime = {0}  ;
    QString la1                    ;
    QString la5                    ;
    QString la15                   ;
    QString kernelVersion          ;
    QString nvidiaVersion          ;
    QString amdVersion             ;
    QString motherboardInfo        ;
    QString cpuInfo                ;
    QString cpuTemp                ;
    QString diskModel              ;
    QString diskSize               ;
    QString diskFreeSpace          ;
    QString ramTotal               ;
//    QString ramUsed                ;
    QString ramFree                ;
    QString mac                    ;
    QString localIp                ;
    QString extIp                  ;

    // Данные из майнера

    QString minerName              ;
    long long int minerUptime = {0};

    QString algorithm1             ;
    QString algorithm2             ;
    QString algorithmZil           ;

    QString server1                ;
    QString server2                ;
    QString serverZil              ;

    QString user1                  ;
    QString user2                  ;
    QString userZil                ;

    unsigned int totalAcceptedShares1   = {0};
    unsigned int totalAcceptedShares2   = {0};
    unsigned int totalAcceptedSharesZil = {0};

    unsigned int totalInvalidShares1    = {0};
    unsigned int totalInvalidShares2    = {0};
    unsigned int totalInvalidSharesZil  = {0};

    unsigned int totalRejectedShares1   = {0};
    unsigned int totalRejectedShares2   = {0};
    unsigned int totalRejectedSharesZil = {0};

    unsigned int totalStaleShares1      = {0};
    unsigned int totalStaleShares2      = {0};
    unsigned int totalStaleSharesZil    = {0};

    // Данные с поля заполняются на стороне сервера

    bool status = {false};
    QDateTime lastOnline = {QDateTime::fromSecsSinceEpoch(0)};

    QJsonObject getJSON();
    void setOffline();

    bool operator==(const workerData& obj) const
    {
        if (this->id  == obj.id)
            return true;
        else
            return false;
    }

    workerData& operator=(const workerData& obj)
    {
        this->id                     = obj.id                    ;
        this->workerName             = obj.workerName            ;
        this->electricityCost        = obj.electricityCost       ;
        this->version                = obj.version               ;
        this->startup                = obj.startup               ;
        this->devices                = obj.devices               ;
        this->sysUptime              = obj.sysUptime             ;
        this->la1                    = obj.la1                   ;
        this->la5                    = obj.la5                   ;
        this->la15                   = obj.la15                  ;
        this->kernelVersion          = obj.kernelVersion         ;
        this->nvidiaVersion          = obj.nvidiaVersion         ;
        this->amdVersion             = obj.amdVersion            ;
        this->motherboardInfo        = obj.motherboardInfo       ;
        this->cpuInfo                = obj.cpuInfo               ;
        this->cpuTemp                = obj.cpuTemp               ;
        this->diskModel              = obj.diskModel             ;
        this->diskSize               = obj.diskSize              ;
        this->diskFreeSpace          = obj.diskFreeSpace         ;
        this->ramTotal               = obj.ramTotal              ;
        this->ramFree                = obj.ramFree               ;
        this->mac                    = obj.mac                   ;
        this->localIp                = obj.localIp               ;
        this->extIp                  = obj.extIp                 ;
        this->minerName              = obj.minerName             ;
        this->minerUptime            = obj.minerUptime           ;
        this->algorithm1             = obj.algorithm1            ;
        this->algorithm2             = obj.algorithm2            ;
        this->algorithmZil           = obj.algorithmZil          ;
        this->server1                = obj.server1               ;
        this->server2                = obj.server2               ;
        this->serverZil              = obj.serverZil             ;
        this->user1                  = obj.user1                 ;
        this->user2                  = obj.user2                 ;
        this->userZil                = obj.userZil               ;
        this->totalAcceptedShares1   = obj.totalAcceptedShares1  ;
        this->totalAcceptedShares2   = obj.totalAcceptedShares2  ;
        this->totalAcceptedSharesZil = obj.totalAcceptedSharesZil;
        this->totalInvalidShares1    = obj.totalInvalidShares1   ;
        this->totalInvalidShares2    = obj.totalInvalidShares2   ;
        this->totalInvalidSharesZil  = obj.totalInvalidSharesZil ;
        this->totalRejectedShares1   = obj.totalRejectedShares1  ;
        this->totalRejectedShares2   = obj.totalRejectedShares2  ;
        this->totalRejectedSharesZil = obj.totalRejectedSharesZil;
        this->totalStaleShares1      = obj.totalStaleShares1     ;
        this->totalStaleShares2      = obj.totalStaleShares2     ;
        this->totalStaleSharesZil    = obj.totalStaleSharesZil   ;
        this->lastOnline             = obj.lastOnline            ;
        this->status                 = obj.status                ;

        return *this;
    }

    DECLARE_B_SERIALIZE_FUNC
};

struct CtlDiscovery : Data<&command::CtlDiscovery,
                            Message::Type::Answer>
{
    // Краткая информация об экземпляре BoostpumpCtl
    QString info;

    // Идентификатор приложения времени исполнения
    QUuidEx applId;

    // Адрес BoostpumpCtl в локальной сети
    HostPoint hostPoint;

    // Признак что интерфейс с которого пришло сообщение является poin-to-point.
    // Используется для отсечения poin-to-point интерфейсов в конфигураторе
    //bool isPointToPoint = {false};

    // Количество подключенных конфигураторов
    //quint16 configConnectCount = {0};

    DECLARE_B_SERIALIZE_FUNC
};

struct AgentStat : Data<&command::AgentStat,
                         Message::Type::Command>
{
    AgentStat() = default;
    AgentStat(const workerData& workerData)
    {
        this->agent = workerData;
    }

    workerData agent;

    DECLARE_B_SERIALIZE_FUNC
};

struct AgentGetGPUData : Data<&command::AgentGetGPUData,
                               Message::Type::Command,
                               Message::Type::Answer>
{
    AgentGetGPUData() = default;
    AgentGetGPUData(const QUuidEx& workerId)
    {
        this->workerId = workerId;
    }

    QUuidEx workerId;
    QList<gpuData> gpuList;

    DECLARE_B_SERIALIZE_FUNC
};

struct AgentUpdateGPUData : Data<&command::AgentUpdateGPUData,
                                  Message::Type::Command>
{
    AgentUpdateGPUData() = default;
    AgentUpdateGPUData(const workerData& worker)
    {
        this->worker = worker;
    }

    workerData worker;

    DECLARE_B_SERIALIZE_FUNC
};

struct UserLogin : Data<&command::UserLogin,
                     Message::Type::Command,
                     Message::Type::Answer>
{
    UserLogin() = default;
    UserLogin(const QString& login, const QString& password)
    {
        this->login = login;
        this->password = password;
        this->resault = {false};
    }

    QUuidEx id;
    QString login = {"user"};
    QString password = {"unknown"};
    bool resault = {false};

    DECLARE_B_SERIALIZE_FUNC
};

struct UserWorkerList : Data<&command::UserWorkerList,
                              Message::Type::Command,
                              Message::Type::Answer>
{
    UserWorkerList() = default;
    UserWorkerList(const QUuidEx& userId)
    {
        this->userId = userId;
    }

    QUuidEx userId;
    QList<QUuidEx> userIdWorkers;
    QList<workerData> workersData;

    DECLARE_B_SERIALIZE_FUNC
};

struct AddWorker : Data<&command::AddWorker,
                         Message::Type::Command,
                         Message::Type::Answer>
{
    AddWorker() = default;
    AddWorker(const QUuidEx& userId)
    {
        this->userId = userId;
    }
    QUuidEx userId;
    bool resault = {false};

    DECLARE_B_SERIALIZE_FUNC
};

struct DeleteWorker : Data<&command::DeleteWorker,
                            Message::Type::Command,
                            Message::Type::Answer>
{
    DeleteWorker() = default;
    DeleteWorker(const QUuidEx& userId, const QUuidEx& workerId, const QString& workerName)
    {
        this->userId = userId;
        this->workerId = workerId;
        this->workerName = workerName;
    }
    QUuidEx userId;
    QUuidEx workerId;
    QString workerName;
    bool resault = {false};

    DECLARE_B_SERIALIZE_FUNC
};

struct AddUser : Data<&command::AddUser,
                       Message::Type::Command,
                       Message::Type::Answer>
{
    AddUser() = default;
    AddUser(const QString& login, const QString& password)
    {
        this->login = login;
        this->password = password;
    }

    QString login;
    QString password;

    bool resault = {false};

    DECLARE_B_SERIALIZE_FUNC
};

struct DeleteUser : Data<&command::DeleteUser,
                          Message::Type::Command,
                          Message::Type::Answer>
{
    DeleteUser() = default;
    DeleteUser(const QUuidEx& id, const QString& login, const QString& password)
    {
        this->userID = id;
        this->login = login;
        this->password = password;
    }

    QUuidEx userID;
    QString login;
    QString password;
    bool resault = {false};

    DECLARE_B_SERIALIZE_FUNC
};

struct ChangeUserPassword : Data<&command::ChangeUserPassword,
                                  Message::Type::Command,
                                  Message::Type::Answer>
{
    ChangeUserPassword() = default;
    ChangeUserPassword(const QUuidEx& id, const QString& login, const QString& password)
    {
        this->userID = id;
        this->login = login;
        this->password = password;
    }

    QUuidEx userID;
    QString login;
    QString password;
    bool resault = {false};

    DECLARE_B_SERIALIZE_FUNC
};

struct UpdateWorkerInfoClient : Data<&command::UpdateWorkerInfoClient,
                                      Message::Type::Command,
                                      Message::Type::Answer>
{
    UpdateWorkerInfoClient() = default;
    UpdateWorkerInfoClient(const QUuidEx& id, const QString& newWorkerName,
                           const QString& oldName, const double& newPowerPrice)
    {
        this->workerID = id;
        this->newWorkerName = newWorkerName;
        this->oldName = oldName;
        this->newPowerPrice = newPowerPrice;
    }

    bool operator==(const UpdateWorkerInfoClient& obj) const
    {
        if (this->workerID  == obj.workerID)
            return true;
        else
            return false;
    }

    QUuidEx workerID;
    QString newWorkerName;
    QString oldName;
    double newPowerPrice = {0};
    bool resault = {false};

    DECLARE_B_SERIALIZE_FUNC
};

struct UpdateWorkerInfoAgent : Data<&command::UpdateWorkerInfoAgent,
                                     Message::Type::Command>
{
    UpdateWorkerInfoAgent() = default;
    UpdateWorkerInfoAgent(UpdateWorkerInfoClient newInfo)
    {
        this->newInfo = newInfo;
    }

    UpdateWorkerInfoClient newInfo;

    DECLARE_B_SERIALIZE_FUNC
};

struct CheckWorkerInfoAgent : Data<&command::CheckWorkerInfoAgent,
                                    Message::Type::Command,
                                    Message::Type::Answer>
{
    CheckWorkerInfoAgent() = default;
    CheckWorkerInfoAgent(const QUuidEx& id)
    {
        this->workerID = id;
    }

    QUuidEx workerID;
    QString workerName;
    double  powerPrice = {0};

    DECLARE_B_SERIALIZE_FUNC
};

struct UpdateGPUSettings: Data<&command::UpdateGPUSettings,
                                       Message::Type::Command>
{
    UpdateGPUSettings() = default;
    UpdateGPUSettings(const data::gpuData& newGPUData)
    {
        this->GPUData = newGPUData;
    }

    bool operator==(const UpdateGPUSettings& obj) const
    {
        if (this->GPUData == obj.GPUData)
            return true;
        else
            return false;
    }

    data::gpuData GPUData;

    DECLARE_B_SERIALIZE_FUNC
};

} // namespace data
} // namespace pproto
