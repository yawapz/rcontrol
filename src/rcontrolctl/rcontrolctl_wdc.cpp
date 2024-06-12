#include "rcontrolctl_wdc.h"

#define log_error_m   alog::logger().error   (alog_line_location, "WorkerDataControl")
#define log_warn_m    alog::logger().warn    (alog_line_location, "WorkerDataControl")
#define log_info_m    alog::logger().info    (alog_line_location, "WorkerDataControl")
#define log_verbose_m alog::logger().verbose (alog_line_location, "WorkerDataControl")
#define log_debug_m   alog::logger().debug   (alog_line_location, "WorkerDataControl")
#define log_debug2_m  alog::logger().debug2  (alog_line_location, "WorkerDataControl")

namespace wdc {

bool WorkerDataControl::init()
{
    return true;
}

void WorkerDataControl::slotWorkerDataComing(data::workerData worker)
{
    _incomeWorkerData.append(worker);
}

void WorkerDataControl::run()
{
    log_info_m << "Worker data control started";

    while (true)
    {
        CHECK_QTHREADEX_STOP

        QList<data::workerData> workers;

        { //Block for SpinLocker
            SpinLocker locker {_dataLock}; (void) locker;
            workers.swap(_incomeWorkerData);
        }

        if (workers.isEmpty())
        {
            msleep(50);
            continue;
        }

        for (data::workerData& w2 : workers)
        {
            bool flag = false;

            for (data::workerData& w1 : _workers)
            {
                if (w1.id == w2.id)
                {
                    w1 = w2;
                    w1.lastOnline = QDateTime::currentDateTime();

                    flag = true;
                    break;
                }
            }

            if (!flag)
                _workers.append(w2);
        }

    } // while (true)

    log_info_m << "Worker data control stopped";
}

WorkerDataControl& wdc()
{
    return safe::singleton<WorkerDataControl>();
}

} // namespace image
