#include "worker_item.h"
#include "ui_worker_item.h"

WorkerItem::WorkerItem(QWidget *parent) :
    QPushButton(parent),
    ui(new Ui::WorkerItem)
{
    ui->setupUi(this);

    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(9); // Устанавливаем радиус размытия
    shadowEffect->setOffset(1); // Устанавливаем смещение тени
    this->setGraphicsEffect(shadowEffect); // Устанавливаем эффект тени на окно
}

WorkerItem::~WorkerItem()
{
    delete ui;
}

bool WorkerItem::init(const data::workerData &worker)
{
    QString style = "QLabel {border: none; background-color: transparent};";

    ui->workerName->setStyleSheet(style);
    ui->minerUptime->setStyleSheet(style);
    ui->workerSpeed->setStyleSheet(style);
    ui->minerName->setStyleSheet(style);
    ui->workerErr->setStyleSheet(style);
    ui->workerStatus->setStyleSheet(style);
    ui->workerFAN->setStyleSheet(style);
    ui->workerLA->setStyleSheet(style);
    ui->workerPower->setStyleSheet(style);
    ui->gpuCount->setStyleSheet(style);

    setWokerStatus(worker);
    setGPURects(worker);
    setFANSpeed(worker);
    setPower(worker);
    setLA(worker);

    ui->workerName->setText(worker.workerName.isEmpty() ? digest(worker.id) : worker.workerName);
    ui->minerUptime->setText(transformMinerUpTime(worker.status, worker.minerUptime, worker.lastOnline));

    ui->workerSpeed->setText(transformWorkerSpeed(worker));
    ui->minerName->setText(transformMinerName(worker));

    _id = worker.id;

    setStyleSheet("QPushButton {border: none; background-color: transparent};");

    return true;
}

widgetListSizeNormalize WorkerItem::getListNormalize()
{
    widgetListSizeNormalize n;

    n.WorkerName    = ui->workerName->sizeHint().width();
    n.WorkerStatus  = ui->workerStatus->sizeHint().width();
    n.MinerUpTime   = ui->minerUptime->sizeHint().width();
    n.WorkerSpeed   = ui->workerSpeed->sizeHint().width();
    n.MinerName     = ui->minerName->sizeHint().width();
    n.workerErr     = ui->workerErr->sizeHint().width();
    n.GpuCount      = ui->gpuCount->sizeHint().width();
    n.workerLA      = ui->workerLA->sizeHint().width();
    n.workerFAN     = ui->workerFAN->sizeHint().width();
    n.WorkerPower   = ui->workerPower->sizeHint().width();

    return n;
}

void WorkerItem::show()
{
    emit showWorker(_id);
}

void WorkerItem::normalizeSize(const widgetListSizeNormalize& n)
{
    ui->workerName->setMinimumWidth(n.WorkerName);
    ui->workerStatus->setMinimumWidth(n.WorkerStatus);
    ui->minerUptime->setMinimumWidth(n.MinerUpTime);
    ui->workerSpeed->setMinimumWidth(n.WorkerSpeed);
    ui->minerName->setMinimumWidth(n.MinerName);
    ui->workerErr->setMinimumWidth(n.workerErr);
    ui->gpuCount->setMinimumWidth(n.GpuCount);
    ui->workerLA->setMinimumWidth(n.workerLA);
    ui->workerFAN->setMinimumWidth(n.workerFAN);
    ui->workerPower->setMinimumWidth(n.WorkerPower);

    setMinimumWidth(n.WorkerName +
                    n.WorkerStatus +
                    n.MinerUpTime +
                    n.WorkerSpeed +
                    n.MinerName +
                    n.workerErr +
                    n.GpuCount +
                    n.workerLA +
                    n.workerFAN +
                    n.WorkerPower);
}

void WorkerItem::setName(const QString &newName, const QUuidEx& id)
{
    ui->workerName->setText(newName.isEmpty() ? digest(id) : newName);
}

void WorkerItem::setWokerStatus(const data::workerData &worker)
{
    if (worker.status)
    {
        ui->workerStatus->setText("online");
        ui->workerStatus->setStyleSheet(" font-size: 18px;               "
                                        " color: #84BF40;                "
                                        " background-color: transparent; ");
    }
    else
    {
        ui->workerStatus->setText("offline");
        ui->workerStatus->setStyleSheet(" font-size: 18px;              "
                                        " color: #FF3733;               "
                                        " background-color: transparent ");
    }

    // TODO Вкрутить иконку Wi-Fi
}

void WorkerItem::setGPURects(const data::workerData &worker)
{
    if (worker.status && worker.devices.size())
    {
        QHBoxLayout *lay = new QHBoxLayout();
        lay->setAlignment(Qt::AlignmentFlag::AlignLeft);
        lay->setSpacing(2);
        lay->setContentsMargins(0,0,0,0);

        for (const data::gpuData& GPU : worker.devices)
        {
            QWidget *unit = new QWidget();
            unit->setFixedSize(15,15);
            unit->setContentsMargins(0,0,0,0);
            unit->setCursor(Qt::WhatsThisCursor);
            unit->setToolTip(QString("GPU %1 TEMP %2°C FAN %3%").arg(
                                 QString::number(GPU.gpuId),
                                 QString::number(GPU.coreTemp),
                                 QString::number(GPU.fanSpeed)));

            // TODO сделать отлов ошибок (отвалов), и иконку перегрева

            if(true)
                unit->setStyleSheet(" QWidget                    "
                                    " {                          "
                                    " background-color: #84BF40; "
                                    " border-radius: 0px         "
                                    " }                          "
                                    " QToolTip                   "
                                    " {                          "
                                    " background: black;         "
                                    " color: orange              "
                                    " }                          ");
            else
                unit->setStyleSheet(" QWidget                        "
                                    " {                              "
                                    " background-color: transparent; "
                                    " border: 2px;                   "
                                    " border-color: #FFAE00          "
                                    " }                              "
                                    " QToolTip                       "
                                    " {                              "
                                    " background: black;             "
                                    " color: orange                  "
                                    " }                              ");

            lay->addWidget(unit);
        }
        ui->gpuCount->setLayout(lay);
    }
}

void WorkerItem::setFANSpeed(const data::workerData &worker)
{
    if (worker.status)
    {
        QPixmap iconFan(":/worker/resources/worker/cooler.svg");
        QPainter qp = QPainter(&iconFan);
        qp.setCompositionMode(QPainter::CompositionMode_SourceIn);
        qp.fillRect(iconFan.rect(), Qt::black);
        qp.end();

        QHBoxLayout* hlay = new QHBoxLayout();
        hlay->setAlignment(Qt::AlignmentFlag::AlignCenter);
        hlay->setContentsMargins(2,0,2,0);
        hlay->setSpacing(0);

        QLabel* lblText = new QLabel();
        QLabel* lblPixmap = new QLabel();

        hlay->addWidget(lblPixmap);
        hlay->addWidget(lblText);

        lblPixmap->setPixmap(iconFan);
        lblPixmap->setFixedSize(28,23);
        lblPixmap->setContentsMargins(2,0,2,0);
        lblPixmap->setScaledContents(true);
        lblPixmap->setStyleSheet("background-color: transparent;");

        unsigned int maxSpeed = 0;
        for (auto& gpu : worker.devices)
            if (gpu.fanSpeed > maxSpeed)
                maxSpeed = gpu.fanSpeed;

        lblText->setText(QString("%1%").arg(QString::number(maxSpeed)));
        ui->workerFAN->setLayout(hlay);
    }
}

void WorkerItem::setPower(const data::workerData &worker)
{
    double totalPower = 0;

    for (auto& gpu : worker.devices)
        totalPower += gpu.powerUsage;

    if(totalPower > 0)
    {
        QPixmap iconPower(":/worker/resources/worker/consumption.svg");
        QPainter qp = QPainter(&iconPower);
        qp.setCompositionMode(QPainter::CompositionMode_SourceIn);
        qp.fillRect(iconPower.rect(), Qt::black);
        qp.end();

        QHBoxLayout* hlay = new QHBoxLayout();
        hlay->setAlignment(Qt::AlignmentFlag::AlignCenter);
        hlay->setContentsMargins(2,0,2,0);
        hlay->setSpacing(0);

        QLabel* lblText = new QLabel();
        QLabel* lblPixmap = new QLabel();

        hlay->addWidget(lblPixmap);
        hlay->addWidget(lblText);

        lblPixmap->setPixmap(iconPower);
        lblPixmap->setFixedSize(23,23);
        lblPixmap->setContentsMargins(2,0,2,0);
        lblPixmap->setScaledContents(true);
        lblPixmap->setStyleSheet("background-color: transparent;");

        QString powerValue;
        //Рассчёт для перевода в W - kW
        if(totalPower > 1000)
        {
            totalPower /= 1000;
            lblText->setText(QString("%1%2").arg(QString::number(totalPower, 'f', 2), "kW"));
            ui->workerPower->setLayout(hlay);
        }
        else
        {
            lblText->setText(QString("%1%2").arg(QString::number(totalPower, 'f', 1), "W"));
            ui->workerPower->setLayout(hlay);
        }
    }
}

void WorkerItem::setLA(const data::workerData &worker)
{
    if (worker.status)
    {
        ui->workerLA->setText(QString("%1").arg(worker.la1));
        if(worker.la1.toDouble() >= 0.8)
            ui->workerLA->setStyleSheet(" background-color: transparent; "
                                        " color: #FF3733;                ");
        else
            ui->workerLA->setStyleSheet(" background-color: transparent; "
                                        " color: black;                  ");
    }
}

QString WorkerItem::transformMinerUpTime(bool status, long long minerUptime, QDateTime lastOnline)
{
    QString negativeStyle = " QLabel {                        "
                            " font-size: 16px;                "
                            " color: #FF3733;                 "
                            " border: none;                   "
                            " background-color: transparent}; ";
    QString normalStyle = " QLabel {                        "
                          " font-size: 16px;                "
                          " color: #84BF40;                 "
                          " border: none;                   "
                          " background-color: transparent}; ";
    ui->minerUptime->setStyleSheet(!lastOnline.toSecsSinceEpoch() ? negativeStyle : normalStyle);

    long long timeSeconds = minerUptime;
    if(!status)
    {
        if (lastOnline.toMSecsSinceEpoch() == 0)
            return "never";
        else
            timeSeconds = QDateTime::currentDateTime().toSecsSinceEpoch() - lastOnline.toSecsSinceEpoch();
    }

    int seconds =  timeSeconds % 60;
    int minutes = (timeSeconds / 60) % 60;
    int hours   = (timeSeconds / 60 /  60);
    int days    = (timeSeconds / 60 /  60 / 24);
    QString normalizeTime = "";
    if(days > 0)
        normalizeTime = QString::number(days) + "d " + QString::number(hours % 24) + "h";
    if(days == 0 && hours > 0)
        normalizeTime = QString::number(hours) + "h " + QString::number(minutes % 60) + "m";
    if(days == 0 && hours == 0)
        normalizeTime = QString::number(minutes) + "m " + QString::number(seconds % 60) + "s";
    if(days == 0 && hours == 0 && minutes == 0)
        normalizeTime = QString::number(seconds) + "s";

    if(minerUptime == 0 && status)
    return normalizeTime = "";

    if (!status)
         ui->minerUptime->setStyleSheet(negativeStyle);

    return status ? normalizeTime : QString("%1 %2 %3").arg("was", normalizeTime, "ago");
}

QString WorkerItem::transformMinerName(const data::workerData &worker)
{
    long double keff = 100 - double(double(worker.totalRejectedShares1
                                         + worker.totalRejectedShares2
                                         + worker.totalRejectedSharesZil)
                                  + double(worker.totalStaleShares1
                                         + worker.totalStaleShares2
                                         + worker.totalStaleSharesZil))
                                  / double(worker.totalAcceptedShares1
                                         + worker.totalAcceptedShares2
                                         + worker.totalAcceptedSharesZil) * 100;
    if(worker.minerName.length() > 0)
    {
        if(keff >= 100)
        {
            keff = 100;
            return QString(worker.minerName + "\n%1%").arg(QString::number(int(keff)));
        }
        else if(keff < 100)
            return QString(worker.minerName + "\n%1%").arg(QString::number(keff, 'f', 2));
        else
            return worker.minerName;
    }
    else
        return QString("");
}

QString WorkerItem::transformWorkerSpeed(const data::workerData &worker)
{
    long double algorithm1 = 0;
    long double algorithm2 = 0;
    long double algorithmZil = 0;
    for (auto& GPU : worker.devices)
    {
        algorithm1 += GPU.speed1;
        algorithm2 += GPU.speed2;
        algorithmZil += GPU.speedZil;
    }

    auto calcValue = [](QString& value, const long double& speed, long double& transformSpeed)
    {
        if(speed >= 1000 && speed < 1000000)
        {
            transformSpeed /= 1000;
            value = "KH";
        }
        else if(speed >= 100000 && speed < 1000000000)
        {
            transformSpeed /= 1000000;
            value = "MH";
        }
        else if(speed >= 100000000 && speed < 1000000000000)
        {
            transformSpeed /= 1000000000;
            value = "GH";
        }
        else
            value = "H";
    };

    QString value1, value2, valueZil;
    long double transformedSpeed1 = algorithm1;
    long double transformedSpeed2 = algorithm2;
    long double transformedSpeedZil = algorithmZil;

    calcValue(value1, algorithm1, transformedSpeed1);
    calcValue(value2, algorithm2, transformedSpeed2);
    calcValue(valueZil, algorithmZil, transformedSpeedZil);

    QString algorithmText;
    if(algorithm2 > 0)
    {
        if (value1 == " Gh")
            algorithmText = QString("%1 %2 %3\n%4 %5 %6").arg(worker.algorithm1,
                            QString::number(transformedSpeed1, 'f', 2),
                            value1, worker.algorithm2,
                            QString::number(transformedSpeed2, 'f', 1), value2);
        else if (value1 == " Gh" && value2 == " Gh")
            algorithmText = QString("%1 %2 %3\n%4 %5 %6").arg(worker.algorithm1,
                            QString::number(transformedSpeed1, 'f', 2),
                            value1, worker.algorithm2,
                            QString::number(transformedSpeed2, 'f', 2), value2);
        else if (value2 == " Gh")
            algorithmText = QString("%1 %2 %3\n%4 %5 %6").arg(worker.algorithm1,
                            QString::number(transformedSpeed1, 'f', 1),
                            value1, worker.algorithm2,
                            QString::number(transformedSpeed2, 'f', 2), value2);
        else
            algorithmText = QString("%1 %2 %3\n%4 %5 %6").arg(worker.algorithm1,
                            QString::number(transformedSpeed1, 'f', 1),
                            value1, worker.algorithm2,
                            QString::number(transformedSpeed2, 'f', 1), value2);
    }
    else if (algorithm1 > 0 && algorithm2 == 0)
    {
        if (value1 == " Gh")
            algorithmText = QString("%1 %2 %3").arg(worker.algorithm1,
                            QString::number(transformedSpeed1, 'f', 2), value1);
        else
            algorithmText = QString("%1 %2 %3").arg(worker.algorithm1,
                            QString::number(transformedSpeed1, 'f', 1), value1);
    }
    else
        if (!worker.minerName.isEmpty())
            return algorithmText += "n/a";
        else
            return QString("");

    return algorithmText;
}
