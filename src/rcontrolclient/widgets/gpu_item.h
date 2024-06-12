#pragma once

#include <QWidget>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include "commands/commands.h"
#include "gpu_settings.h"

using namespace pproto;
using namespace pproto::transport;

namespace Ui {
class GPUItem;
}

class GPUItem : public QWidget
{
    Q_OBJECT

    enum DataType
    {
        NEW = 0,
        UPDATE = 1,
    };

public:
    explicit GPUItem(QWidget *parent = nullptr);
    ~GPUItem();

    bool init(const data::gpuData& gpu, tcp::Socket::Ptr socket);
    void setOffline();

public slots:
    void updateData(const data::gpuData& GPU);

private:
    Ui::GPUItem *ui;
    data::gpuData _GPU;
    tcp::Socket::Ptr _socket;

private:
    void setID();
    void setBUS();
    void setModel();
    void setMemSze();
    void setVendor();
    void setvBIOS();
    void setPLInfo();
    void setStat();
    void setSpeed();
    void setTemp(const DataType&);
    void setFAN(const DataType&);
    void setPower(const DataType&);
    void setClocks();

    void renderData(const DataType&);

protected:
    bool eventFilter(QObject *, QEvent *) override;
};
