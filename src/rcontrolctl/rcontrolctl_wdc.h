#pragma once

#include "shared/defmac.h"
#include "shared/steady_timer.h"
#include "shared/safe_singleton.h"
#include "shared/qt/quuidex.h"
#include "shared/qt/qthreadex.h"
#include "shared/break_point.h"
#include "shared/spin_locker.h"
#include "shared/config/appl_conf.h"
#include "shared/logger/logger.h"
#include "shared/qt/logger_operators.h"

#include <QtCore>
#include <atomic>

#include "commands/commands.h"

namespace wdc {

using namespace std;
using namespace pproto;

class WorkerDataControl : public QThreadEx
{
public:
    WorkerDataControl() = default;
    bool init();

    QList<data::workerData> _workers;

    QList<data::workerData> _incomeWorkerData;

signals:


public slots:
    void slotWorkerDataComing(data::workerData);


private:
    Q_OBJECT
    DISABLE_DEFAULT_COPY(WorkerDataControl)
    void run() override final;

private:
    mutable atomic_flag _dataLock = ATOMIC_FLAG_INIT;


};

WorkerDataControl& wdc();

} // namespace image
