#include "commands.h"
#include "pproto/commands/pool.h"

namespace pproto {
namespace command {

#define REGISTRY_COMMAND_SINGLPROC(COMMAND, UUID) \
    const QUuidEx COMMAND = command::Pool::Registry{UUID, #COMMAND, false};

#define REGISTRY_COMMAND_MULTIPROC(COMMAND, UUID) \
    const QUuidEx COMMAND = command::Pool::Registry{UUID, #COMMAND, true};

REGISTRY_COMMAND_SINGLPROC(CtlDiscovery            ,"0568213e-94c8-4dba-8125-9144ebd4e01d")
REGISTRY_COMMAND_SINGLPROC(AgentStat               ,"831ef8d7-743e-4920-a604-3821500d5952")
REGISTRY_COMMAND_SINGLPROC(UserLogin               ,"cab5e72d-b557-40e5-bd84-869d8c63eb36")
REGISTRY_COMMAND_SINGLPROC(UserWorkerList          ,"620b83ac-0a73-4d18-b5f8-3ed7a7440b94")
REGISTRY_COMMAND_SINGLPROC(AddWorker               ,"72b2b855-0094-44a5-9348-3cb4e2d63acf")
REGISTRY_COMMAND_SINGLPROC(DeleteWorker            ,"f74a0363-1ae7-4ee0-96ad-3edb9080cd0a")
REGISTRY_COMMAND_SINGLPROC(AgentGetGPUData         ,"013868c4-eebc-4812-b978-c2a080f9197c")
REGISTRY_COMMAND_SINGLPROC(AgentUpdateGPUData      ,"885082ca-398f-44d9-b746-5f41d7394698")
REGISTRY_COMMAND_SINGLPROC(AddUser                 ,"f8c9bf81-9062-4376-a33e-3c0bf7158501")
REGISTRY_COMMAND_SINGLPROC(DeleteUser              ,"5ea5e390-5f7d-4cb6-a671-f25db9d35a95")
REGISTRY_COMMAND_SINGLPROC(ChangeUserPassword      ,"1218ee85-b56e-4529-99c5-a9e8c4abe8dd")
REGISTRY_COMMAND_SINGLPROC(UpdateWorkerInfoClient  ,"fd2b4e8c-496f-4537-befb-21259c427e04")
REGISTRY_COMMAND_SINGLPROC(UpdateWorkerInfoAgent   ,"dd073cff-297f-4ff6-929d-fc67300d8dc9")
REGISTRY_COMMAND_SINGLPROC(CheckWorkerInfoAgent    ,"35dbdd6b-7236-4dc2-a7f2-11cfb950ae77")
REGISTRY_COMMAND_SINGLPROC(UpdateGPUSettings       ,"1cea42e0-228c-4e05-9587-7ac2983b4c8e")

#undef REGISTRY_COMMAND_SINGLPROC
#undef REGISTRY_COMMAND_MULTIPROC

} // namespace command

namespace data {

QJsonObject workerData::getJSON()
{
    QJsonArray gpuArr;
    for (auto gpu : this->devices)
    {
        QJsonObject obj;
        obj.insert("id"                  ,     toString(gpu.id)         );
        obj.insert("name"                ,     gpu.name                 );
        obj.insert("bus_id"              ,     gpu.busId                );
        obj.insert("gpu_id"              ,(int)gpu.gpuId                );
        obj.insert("fan_speed"           ,(int)gpu.fanSpeed             );
        obj.insert("core_clock"          ,(int)gpu.coreClock            );
        obj.insert("memory_clock"        ,(int)gpu.memoryClock          );
        obj.insert("power_usage"         ,(int)gpu.powerUsage           );
        obj.insert("speed1"              ,(int)gpu.speed1               );
        obj.insert("speed2"              ,(int)gpu.speed2               );
        obj.insert("speed_zil"           ,(int)gpu.speedZil             );
        obj.insert("accepted_shares1"    ,(int)gpu.acceptedShares1      );
        obj.insert("accepted_shares2"    ,(int)gpu.acceptedShares2      );
        obj.insert("accepted_shares_zil" ,(int)gpu.acceptedSharesZil    );
        obj.insert("rejected_shares1"    ,(int)gpu.rejectedShares1      );
        obj.insert("rejected_shares2"    ,(int)gpu.rejectedShares2      );
        obj.insert("rejected_shares_zil" ,(int)gpu.rejectedSharesZil    );
        obj.insert("stale_shares1"       ,(int)gpu.staleShares1         );
        obj.insert("stale_shares2"       ,(int)gpu.staleShares2         );
        obj.insert("stale_shares_zil"    ,(int)gpu.staleSharesZil       );
        obj.insert("invalid_shares1"      ,(int)gpu.invalidShares1       );
        obj.insert("invalid_shares2"     ,(int)gpu.invalidShares2       );
        obj.insert("invalid_shares_zil"  ,(int)gpu.invalidSharesZil     );
        obj.insert("core_temperature"    ,(int)gpu.coreTemp             );
        obj.insert("memory_temperature"  ,(int)gpu.memoryTemp           );
        obj.insert("set_fan_speed"       ,(int)gpu.setFanSpeed          );
        obj.insert("set_core"            ,     gpu.setCore              );
        obj.insert("set_mem"             ,     gpu.setMem               );
        obj.insert("set_pl"              ,     gpu.setPl                );
        obj.insert("vendor"              ,     gpu.vendor               );
        obj.insert("VBIOS_version"       ,     gpu.vbiosVersion         );
        obj.insert("total_memory"        ,     gpu.totalMemory          );
        obj.insert("min_pl"              ,     gpu.minPl                );
        obj.insert("default_pl"          ,     gpu.defaultPl            );
        obj.insert("max_pl"              ,     gpu.maxPl                );
//        obj.insert("max_core_freq"       ,(int)gpu.maxCoreFreq          );
//        obj.insert("max_mem_freq"        ,(int)gpu.maxMemFreq           );
        gpuArr.push_back(obj);
    }

    QJsonObject doc;

    doc.insert("id"                ,toString(this->id)                  );
    doc.insert("devices"           ,      gpuArr                        );
    doc.insert("status"            ,this->status                        );
    doc.insert("worker_name"       ,this->workerName                    );
    doc.insert("miner"             ,this->minerName                     );
    doc.insert("miner_uptime"      ,this->minerName                     );
    doc.insert("algorithm1"        ,this->algorithm1                    );
    doc.insert("algorithm2"        ,this->algorithm2                    );
    doc.insert("algorithm_zil"     ,this->algorithmZil                  );
    doc.insert("server1"           ,this->server1                       );
    doc.insert("server2"           ,this->server2                       );
    doc.insert("server_zil"        ,this->serverZil                     );
    doc.insert("user1"             ,this->user1                         );
    doc.insert("user2"             ,this->user2                         );
    doc.insert("user_zil"          ,this->userZil                       );
    doc.insert("local_ip"          ,this->localIp                       );
    doc.insert("ext_ip"            ,this->extIp                         );
    doc.insert("la1"               ,this->la1                           );
    doc.insert("la5"               ,this->la5                           );
    doc.insert("la15"              ,this->la15                          );
    doc.insert("electricity_cost"  ,this->electricityCost               );
    doc.insert("kernel_version"    ,this->kernelVersion                 );
    doc.insert("nvidia_version"    ,this->nvidiaVersion                 );
    doc.insert("amd_version"       ,this->amdVersion                    );
    doc.insert("motherboard_data"  ,this->motherboardInfo               );
    doc.insert("cpu_info"          ,this->cpuInfo                       );
    doc.insert("cpu_temperature"   ,this->cpuTemp                       );
    doc.insert("disk_model"        ,this->diskModel                     );
    doc.insert("disk_size"         ,this->diskSize                      );
    doc.insert("disk_free_space"   ,this->diskFreeSpace                 );
    doc.insert("ram_total"         ,this->ramTotal                      );
//    doc.insert("RAM_used"          ,this->ramUsed                       );
    doc.insert("ram_free"          ,this->ramFree                       );
    doc.insert("mac"               ,this->mac                           );
    doc.insert("version"           ,this->version                       );

    doc.insert("total_accepted_shares1"    ,(qint64)this->totalAcceptedShares1  );
    doc.insert("total_accepted_shares2"    ,(qint64)this->totalAcceptedShares2  );
    doc.insert("total_accepted_shares_zil" ,(qint64)this->totalAcceptedSharesZil);
    doc.insert("total_rejected_shares1"    ,(qint64)this->totalRejectedShares1  );
    doc.insert("total_rejected_shares2"    ,(qint64)this->totalRejectedShares2  );
    doc.insert("total_rejected_shares_zil" ,(qint64)this->totalRejectedSharesZil);
    doc.insert("total_invalid_shares1"     ,(qint64)this->totalInvalidShares1   );
    doc.insert("total_invalid_shares2"     ,(qint64)this->totalInvalidShares2   );
    doc.insert("total_invalid_sharesZil"   ,(qint64)this->totalInvalidSharesZil );
    doc.insert("total_stale_shares1"       ,(qint64)this->totalStaleShares1     );
    doc.insert("total_stale_shares2"       ,(qint64)this->totalStaleShares2     );
    doc.insert("total_stale_shares_zil"    ,(qint64)this->totalStaleSharesZil   );
    doc.insert("miner_uptime"              ,(qint64)this->minerUptime           );
    doc.insert("system_uptime"             ,(qint64)this->sysUptime             );
    doc.insert("last_online"               ,(qint64)this->lastOnline.
                                                              toSecsSinceEpoch());

    return doc;
}

void workerData::setOffline()
{
    status = false;

    algorithm1.clear        ();
    algorithm2.clear        ();
    algorithmZil.clear      ();
    server1.clear           ();
    server2.clear           ();
    serverZil.clear         ();
    user1.clear             ();
    user2.clear             ();
    userZil.clear           ();
    totalAcceptedShares1   = 0;
    totalAcceptedShares2   = 0;
    totalAcceptedSharesZil = 0;
    totalRejectedShares1   = 0;
    totalRejectedShares2   = 0;
    totalRejectedSharesZil = 0;
    totalStaleShares1      = 0;
    totalStaleShares2      = 0;
    totalStaleSharesZil    = 0;
    totalInvalidShares1    = 0;
    totalInvalidShares2    = 0;
    totalInvalidSharesZil  = 0;

    cpuTemp.clear           ();
    la1.clear               ();
    la5.clear               ();
    la15.clear              ();
    minerName.clear         ();
    ramFree.clear           ();
    ramTotal.clear          ();
    version.clear           ();
    minerUptime            = 0;
    sysUptime              = 0;

    for (auto& g : devices)
    {
        g.speed1            = 0;
        g.speed2            = 0;
        g.speedZil          = 0;
        g.fanSpeed          = 0;
        g.coreTemp          = 0;
        g.memoryTemp        = 0;
        g.coreClock         = 0;
        g.memoryClock       = 0;
        g.powerUsage        = 0;
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
}

bserial::RawVector gpuData::toRaw() const
{
    B_SERIALIZE_V1(stream)

    stream << id                   ;
    stream << setFanSpeed          ;
    stream << setCore              ;
    stream << setMem               ;
    stream << setPl                ;
    stream << name                 ;
    stream << busId                ;
    stream << vendor               ;
    stream << totalMemory          ;
    stream << vbiosVersion         ;
    stream << minPl                ;
    stream << defaultPl            ;
    stream << maxPl                ;
    stream << gpuId                ;
    stream << fanSpeed             ;
    stream << coreClock            ;
    stream << memoryClock          ;
    stream << powerUsage           ;
    stream << coreTemp             ;
    stream << memoryTemp           ;
//    stream << maxCoreFreq          ;
//    stream << maxMemFreq           ;
    stream << speed1               ;
    stream << speed2               ;
    stream << speedZil             ;
    stream << acceptedShares1      ;
    stream << acceptedShares2      ;
    stream << acceptedSharesZil    ;
    stream << rejectedShares1      ;
    stream << rejectedShares2      ;
    stream << rejectedSharesZil    ;
    stream << staleShares1         ;
    stream << staleShares2         ;
    stream << staleSharesZil       ;
    stream << invalidShares1       ;
    stream << invalidShares2       ;
    stream << invalidSharesZil     ;

    B_SERIALIZE_RETURN
}

void gpuData::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)

    stream >> id                   ;
    stream >> setFanSpeed          ;
    stream >> setCore              ;
    stream >> setMem               ;
    stream >> setPl                ;
    stream >> name                 ;
    stream >> busId                ;
    stream >> vendor               ;
    stream >> totalMemory          ;
    stream >> vbiosVersion         ;
    stream >> minPl                ;
    stream >> defaultPl            ;
    stream >> maxPl                ;
    stream >> gpuId                ;
    stream >> fanSpeed             ;
    stream >> coreClock            ;
    stream >> memoryClock          ;
    stream >> powerUsage           ;
    stream >> coreTemp             ;
    stream >> memoryTemp           ;
//    stream >> maxCoreFreq          ;
//    stream >> maxMemFreq           ;
    stream >> speed1               ;
    stream >> speed2               ;
    stream >> speedZil             ;
    stream >> acceptedShares1      ;
    stream >> acceptedShares2      ;
    stream >> acceptedSharesZil    ;
    stream >> rejectedShares1      ;
    stream >> rejectedShares2      ;
    stream >> rejectedSharesZil    ;
    stream >> staleShares1         ;
    stream >> staleShares2         ;
    stream >> staleSharesZil       ;
    stream >> invalidShares1       ;
    stream >> invalidShares2       ;
    stream >> invalidSharesZil     ;

    B_DESERIALIZE_END
}

bserial::RawVector workerData::toRaw() const
{
    B_SERIALIZE_V1(stream)

    stream << workerName                            ;
    stream << id                                    ;
    stream << electricityCost                       ;
    stream << version                               ;
    stream << devices                               ;
    stream << startup                               ;
    stream << sysUptime                             ;
    stream << la1                                   ;
    stream << la5                                   ;
    stream << la15                                  ;
    stream << kernelVersion                         ;
    stream << nvidiaVersion                         ;
    stream << amdVersion                            ;
    stream << motherboardInfo                       ;
    stream << cpuInfo                               ;
    stream << cpuTemp                               ;
    stream << diskModel                             ;
    stream << diskSize                              ;
    stream << diskFreeSpace                         ;
    stream << ramTotal                              ;
//    stream <<  ramUsed                              ;
    stream << ramFree                               ;
    stream << mac                                   ;
    stream << localIp                               ;
    stream << extIp                                 ;
    stream << minerName                             ;
    stream << minerUptime                           ;
    stream << algorithm1                            ;
    stream << algorithm2                            ;
    stream << algorithmZil                          ;
    stream << server1                               ;
    stream << server2                               ;
    stream << serverZil                             ;
    stream << user1                                 ;
    stream << user2                                 ;
    stream << userZil                               ;
    stream << totalAcceptedShares1                  ;
    stream << totalAcceptedShares2                  ;
    stream << totalAcceptedSharesZil                ;
    stream << totalInvalidShares1                   ;
    stream << totalInvalidShares2                   ;
    stream << totalInvalidSharesZil                 ;
    stream << totalRejectedShares1                  ;
    stream << totalRejectedShares2                  ;
    stream << totalRejectedSharesZil                ;
    stream << totalStaleShares1                     ;
    stream << totalStaleShares2                     ;
    stream << totalStaleSharesZil                   ;
    stream << status                                ;
    stream << lastOnline                            ;

    B_SERIALIZE_RETURN
}

void workerData::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)

    stream >> workerName                            ;
    stream >> id                                    ;
    stream >> electricityCost                       ;
    stream >> version                               ;
    stream >> devices                               ;
    stream >> startup                               ;
    stream >> sysUptime                             ;
    stream >> la1                                   ;
    stream >> la5                                   ;
    stream >> la15                                  ;
    stream >> kernelVersion                         ;
    stream >> nvidiaVersion                         ;
    stream >> amdVersion                            ;
    stream >> motherboardInfo                       ;
    stream >> cpuInfo                               ;
    stream >> cpuTemp                               ;
    stream >> diskModel                             ;
    stream >> diskSize                              ;
    stream >> diskFreeSpace                         ;
    stream >> ramTotal                              ;
//    stream >>  ramUsed                              ;
    stream >> ramFree                               ;
    stream >> mac                                   ;
    stream >> localIp                               ;
    stream >> extIp                                 ;
    stream >> minerName                             ;
    stream >> minerUptime                           ;
    stream >> algorithm1                            ;
    stream >> algorithm2                            ;
    stream >> algorithmZil                          ;
    stream >> server1                               ;
    stream >> server2                               ;
    stream >> serverZil                             ;
    stream >> user1                                 ;
    stream >> user2                                 ;
    stream >> userZil                               ;
    stream >> totalAcceptedShares1                  ;
    stream >> totalAcceptedShares2                  ;
    stream >> totalAcceptedSharesZil                ;
    stream >> totalInvalidShares1                   ;
    stream >> totalInvalidShares2                   ;
    stream >> totalInvalidSharesZil                 ;
    stream >> totalRejectedShares1                  ;
    stream >> totalRejectedShares2                  ;
    stream >> totalRejectedSharesZil                ;
    stream >> totalStaleShares1                     ;
    stream >> totalStaleShares2                     ;
    stream >> totalStaleSharesZil                   ;
    stream >> status                                ;
    stream >> lastOnline                            ;

    B_DESERIALIZE_END
}

bserial::RawVector CtlDiscovery::toRaw() const
{
    B_SERIALIZE_V1(stream)
    stream << info;
    stream << applId;
    stream << hostPoint;
    //stream << isPointToPoint;
    //stream << configConnectCount;
    B_SERIALIZE_RETURN
}

void CtlDiscovery::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)
    stream >> info;
    stream >> applId;
    stream >> hostPoint;
    //stream >> isPointToPoint;
    //stream >> configConnectCount;
    B_DESERIALIZE_END
}

bserial::RawVector AgentStat::toRaw() const
{
    B_SERIALIZE_V1(stream)

    stream << agent;

    B_SERIALIZE_RETURN
}

void AgentStat::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)

    stream >> agent;

    B_DESERIALIZE_END
}

bserial::RawVector AgentGetGPUData::toRaw() const
{
    B_SERIALIZE_V1(stream)

    stream << workerId;
    stream << gpuList;

    B_SERIALIZE_RETURN
}

void AgentGetGPUData::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)

    stream >> workerId;
    stream >> gpuList;

    B_DESERIALIZE_END
}

bserial::RawVector AgentUpdateGPUData::toRaw() const
{
    B_SERIALIZE_V1(stream)

    stream << worker;
    stream << worker;

    B_SERIALIZE_RETURN
}

void AgentUpdateGPUData::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)

    stream >> worker;
    stream >> worker;

    B_DESERIALIZE_END
}

bserial::RawVector UserLogin::toRaw() const
{
    B_SERIALIZE_V1(stream)

    stream << id;
    stream << login;
    stream << password;
    stream << resault;

    B_SERIALIZE_RETURN
}

void UserLogin::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)

    stream >> id;
    stream >> login;
    stream >> password;
    stream >> resault;

    B_DESERIALIZE_END
}

bserial::RawVector UserWorkerList::toRaw() const
{
    B_SERIALIZE_V1(stream)

    stream << userId;
    stream << userIdWorkers;
    stream << workersData;

    B_SERIALIZE_RETURN
}

void UserWorkerList::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)

    stream >> userId;
    stream >> userIdWorkers;
    stream >> workersData;

    B_DESERIALIZE_END
}

bserial::RawVector AddWorker::toRaw() const
{
    B_SERIALIZE_V1(stream)

    stream << userId;
    stream << resault;

    B_SERIALIZE_RETURN
}

void AddWorker::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)

    stream >> userId;
    stream >> resault;

    B_DESERIALIZE_END
}

bserial::RawVector DeleteWorker::toRaw() const
{
    B_SERIALIZE_V1(stream)

    stream << userId;
    stream << workerId;
    stream << workerName;
    stream << resault;

    B_SERIALIZE_RETURN
}

void DeleteWorker::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)

    stream >> userId;
    stream >> workerId;
    stream >> workerName;
    stream >> resault;

    B_DESERIALIZE_END
}

bserial::RawVector AddUser::toRaw() const
{
    B_SERIALIZE_V1(stream)

    stream << login;
    stream << password;
    stream << resault;

    B_SERIALIZE_RETURN
}

void AddUser::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)

    stream >> login;
    stream >> password;
    stream >> resault;

    B_DESERIALIZE_END
}

bserial::RawVector DeleteUser::toRaw() const
{
    B_SERIALIZE_V1(stream)

    stream << userID;
    stream << login;
    stream << password;
    stream << resault;

    B_SERIALIZE_RETURN
}

void DeleteUser::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)

    stream >> userID;
    stream >> login;
    stream >> password;
    stream >> resault;

    B_DESERIALIZE_END
}

bserial::RawVector ChangeUserPassword::toRaw() const
{
    B_SERIALIZE_V1(stream)

    stream << userID;
    stream << login;
    stream << password;
    stream << resault;

    B_SERIALIZE_RETURN
}

void ChangeUserPassword::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)

    stream >> userID;
    stream >> login;
    stream >> password;
    stream >> resault;

    B_DESERIALIZE_END
}

bserial::RawVector UpdateWorkerInfoClient::toRaw() const
{
    B_SERIALIZE_V1(stream)

    stream << workerID;
    stream << newWorkerName;
    stream << oldName;
    stream << newPowerPrice;
    stream << resault;

    B_SERIALIZE_RETURN
}

void UpdateWorkerInfoClient::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)

    stream >> workerID;
    stream >> newWorkerName;
    stream >> oldName;
    stream >> newPowerPrice;
    stream >> resault;

    B_DESERIALIZE_END
}

bserial::RawVector UpdateWorkerInfoAgent::toRaw() const
{
    B_SERIALIZE_V1(stream)

    stream << newInfo;

    B_SERIALIZE_RETURN
}

void UpdateWorkerInfoAgent::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)

    stream >> newInfo;

    B_DESERIALIZE_END
}

bserial::RawVector CheckWorkerInfoAgent::toRaw() const
{
    B_SERIALIZE_V1(stream)

    stream << workerID;
    stream << workerName;
    stream << powerPrice;

    B_SERIALIZE_RETURN
}

void CheckWorkerInfoAgent::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)

    stream >> workerID;
    stream >> workerName;
    stream >> powerPrice;

    B_DESERIALIZE_END
}

bserial::RawVector UpdateGPUSettings::toRaw() const
{
    B_SERIALIZE_V1(stream)

    stream << GPUData;

    B_SERIALIZE_RETURN
}

void UpdateGPUSettings::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)

    stream >> GPUData;

    B_DESERIALIZE_END
}
} // namespace data
} // namespace pproto
