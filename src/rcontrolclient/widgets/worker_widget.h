#pragma once

#include <QWidget>
#include <QPainter>
#include <QLabel>
#include <QClipboard>
#include <QGraphicsDropShadowEffect>

#include "commands/commands.h"
#include "pproto/func_invoker.h"
#include "pproto/transport/tcp.h"
#include "pproto/commands/pool.h"

#include "gpu_item.h"
#include "edit_window.h"

using namespace pproto;
using namespace pproto::transport;

namespace Ui {
class WorkerWidget;
}

class WorkerWidget : public QWidget
{
    Q_OBJECT

    enum DataType
    {
        NEW = 0,
        UPDATE = 1,
    };

public:
    explicit WorkerWidget(QWidget *parent = nullptr);
    ~WorkerWidget();

    bool init(const tcp::Socket::Ptr socket, const data::workerData& workerData, const QStringList& workersList);
    void deinit();

    void updateData(const data::workerData &workerData);

public slots:
    void slotUpdateNamesList(const QStringList&);

private:
    bool _lastState = {false};

    QLabel* _lblTotalPowerText;
    QLabel* _la1;
    QLabel* _la5;
    QLabel* _la15;

private:
    bool eventFilter(QObject *watched, QEvent *event);

    Ui::WorkerWidget *ui;

    tcp::Socket::Ptr _socket;
    data::workerData _worker;
    QStringList _workersNames;

    void renderData(const DataType&);

    void setName(const DataType&);
    void setPower(const DataType&);
    void setLA(const DataType&);
    void setUptime();
    void setMinerUptime();
    void setCPU(const DataType&);
    void setMB();
    void setDisk(const DataType&);
    void setRAM(const DataType&);
    void setIP(const DataType&);
    void setVersion(const DataType&);
    void setWallet();
    void setStat();
    void setSpeed();
    void setGPUWidgets();
    void setGPUList();
};
