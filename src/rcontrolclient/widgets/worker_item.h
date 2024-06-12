#pragma once

#include "commands/commands.h"

#include <QWidget>
#include <QPainter>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>

using namespace pproto;
using namespace pproto::transport;

namespace Ui {
class WorkerItem;
}

class WorkerItem : public QPushButton
{

signals:
    void showWorker(const QUuidEx&);

public:
    explicit WorkerItem(QWidget *parent = nullptr);
    ~WorkerItem();

    bool init(const data::workerData& worker);
    void setName(const QString& newName, const QUuidEx& id);
    widgetListSizeNormalize getListNormalize();

public slots:
    void show();
    void normalizeSize(const widgetListSizeNormalize&);

private:
    Q_OBJECT

    Ui::WorkerItem *ui;
    QUuidEx _id;

private:
    void setWokerStatus(const data::workerData& worker);
    void setGPURects(const data::workerData& worker);
    void setFANSpeed(const data::workerData& worker);
    void setPower(const data::workerData& worker);
    void setLA(const data::workerData& worker);

    QString transformMinerUpTime(bool, long long, QDateTime);
    QString transformMinerName(const data::workerData& worker);
    QString transformWorkerSpeed(const data::workerData& worker);
};
