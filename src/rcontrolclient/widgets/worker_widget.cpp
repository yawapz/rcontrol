#include "worker_widget.h"
#include "ui_worker_widget.h"

WorkerWidget::WorkerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WorkerWidget)
{
    ui->setupUi(this);

    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(3); // Устанавливаем радиус размытия
    shadowEffect->setOffset(0.5); // Устанавливаем смещение тени
    this->setGraphicsEffect(shadowEffect); // Устанавливаем эффект тени на окно
}

WorkerWidget::~WorkerWidget()
{
    delete ui;
}

bool WorkerWidget::init(const tcp::Socket::Ptr socket, const data::workerData& workerData, const QStringList& workersNames)
{
    _socket = socket;
    chk_connect_q(_socket.get(), &tcp::Socket::disconnected,
                  this, &WorkerWidget::close);

    _workersNames = workersNames;

    ui->lwGPU->setStyleSheet(" QListWidget:item{     "
                             " border-style: outset; "
                             " border-width: 1px;    "
                             " border-radius: 3px;   "
                             " border-color: black;} ");

    // Пробелы не трогать!
    ui->lblTitle->setText("TEMP            "
                          "FAN        "
                          "CORE         "
                          "MEM               "
                          "PL             ");
    ui->lblTitle->setStyleSheet(" background-color: transparent; "
                                " font-size: 14px                ");

    _lblTotalPowerText = new QLabel();
    _la1 = new QLabel();
    _la5 = new QLabel();
    _la15 = new QLabel();

    _socket = socket;
    _worker = workerData;

    renderData(NEW);

    return true;
}

void WorkerWidget::deinit()
{

}

void WorkerWidget::renderData(const DataType& dataType)
{
    setName(dataType);
    setUptime();
    setMinerUptime();
    setGPUWidgets();
    setGPUList();
    setCPU(dataType);
    setMB();
    setDisk(dataType);
    setRAM(dataType);
    setIP(dataType);
    setVersion(dataType);

    ui->lblName->setText(_worker.workerName.isEmpty() ? digest(_worker.id) : _worker.workerName);
    ui->lblName->setStyleSheet(" background-color: transparent; "
                               " font-size: 18px                ");

    if (_worker.status)
    {
        ui->lblStatus->setText("online");
        ui->lblStatus->setStyleSheet(" margin-left: 10px;             "
                                     " font-size: 18px;               "
                                     " color: #84BF40;                "
                                     " background-color: transparent; ");

    }
    else
    {
        ui->lblStatus->setText("offline");
        ui->lblStatus->setStyleSheet(" margin-left: 10px;            "
                                     " font-size: 18px;              "
                                     " color: #FF3733;               "
                                     " background-color: transparent ");
    }

    auto clearContentFunc = [&]()
    {
        ui->lblMinerName->clear();
        ui->lblWallet1->clear();
        ui->lblWallet2->clear();
        ui->lblAlg1->clear();
        ui->lblAlg2->clear();
        ui->lblPool1->clear();
        ui->lblPool2->clear();
        ui->lblSpeed1->clear();
        ui->lblSpeed2->clear();
        ui->lblRejected->clear();
        ui->lblAccepted->clear();
        ui->lblStaled->clear();
        ui->lblStat->clear();
    };

    if (_worker.status)
    {
        if (!_worker.minerName.isEmpty())
        {
            ui->lblMinerName->setText(_worker.minerName);
            ui->lblMinerName->setStyleSheet(" background-color: transparent; "
                                            " font-size: 16px                ");
            ui->lblPool1->setText(_worker.server1);
            ui->lblPool1->setStyleSheet(" background-color: transparent; "
                                        " font-size: 12px                ");
            ui->lblPool2->setText(_worker.server2);
            ui->lblPool2->setStyleSheet(" background-color: transparent; "
                                        " font-size: 12px                ");

            if (!_worker.algorithm1.isEmpty())
            {
                ui->lblAlg1->setText(_worker.algorithm1);
                ui->lblAlg1->setStyleSheet(" background-color: transparent; "
                                           " color: black; font-size: 16px  ");
            }
            else
                ui->lblAlg1->setText("");

            if (!_worker.algorithm2.isEmpty())
            {
                ui->lblAlg2->setText(_worker.algorithm2);
                ui->lblAlg2->setStyleSheet(" background-color: transparent; "
                                           " color: black; font-size: 16px  ");
            }
            else
                ui->lblAlg2->setText("");
        }
        else
            clearContentFunc();

        setLA(dataType);
        setPower(dataType);
        setStat();
        setSpeed();
        setWallet();
    }
    else
        clearContentFunc();
}

void WorkerWidget::setName(const DataType& workerData)
{
    if (workerData == NEW)
    {
        QPixmap iconEdit(":/worker/resources/worker/edit.svg");
        QPainter qpEdit = QPainter(&iconEdit);
        qpEdit.setCompositionMode(QPainter::CompositionMode_SourceIn);
        qpEdit.fillRect(iconEdit.rect(), Qt::black);
        qpEdit.end();

        ui->lblEditIcon->setPixmap(iconEdit);
        ui->lblEditIcon->setFixedSize(18,23);
        ui->lblEditIcon->setContentsMargins(0,2,0,2);
        ui->lblEditIcon->setScaledContents(true);
        ui->lblEditIcon->setStyleSheet(" background-color: transparent; "
                                       " color: orange;                 ");
        ui->lblEditIcon->setToolTip("Редактировать параметры воркера");
        ui->lblEditIcon->installEventFilter(this);
    }

    ui->lblName->setText(_worker.workerName.isEmpty() ? digest(_worker.id) : _worker.workerName);
    ui->lblName->setStyleSheet(" background-color: transparent; "
                               " font-size: 18px                ");
}

void WorkerWidget::updateData(const data::workerData &workerData)
{
    _lastState = _worker.status;
    _worker = workerData;

    for (int i = 0; i < ui->gpuLay->count(); ++i)
    {
        ui->gpuLay->takeAt(i)->widget()->deleteLater();
        ui->gpuLay->removeItem(ui->gpuLay->takeAt(i));
    }

    if (!_lastState || !_worker.status)
    {
        if(ui->lblTotalPower->layout())
        {
            while (auto item = ui->lblTotalPower->layout()->takeAt(0))
                item->widget()->deleteLater();
            ui->lblTotalPower->layout()->deleteLater();
        }

        if (ui->lblLA->layout())
        {
            while (auto item = ui->lblLA->layout()->takeAt(0))
                item->widget()->deleteLater();
            ui->lblLA->layout()->deleteLater();
        }
    }

    if (!_lastState && _worker.status)
    {
        _lblTotalPowerText = new QLabel();
        _la1 = new QLabel();
        _la5 = new QLabel();
        _la15 = new QLabel();
    }

    renderData(UPDATE);
}

void WorkerWidget::slotUpdateNamesList(const QStringList& namesList)
{
    _workersNames = namesList;
}

bool WorkerWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->lblWallet1)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(ui->lblWallet1->toolTip());
        }
    }
    if (watched == ui->lblWallet2)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(ui->lblWallet2->toolTip());
        }
    }
    if (watched == ui->lblExtIPCOPY)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(_worker.extIp);
        }
    }
    if (watched == ui->lblLocalIPCOPY)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(_worker.localIp);
        }
    }
    if (watched == ui->lblEditIcon)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            // Вызов окна редактирования параметров воркера
            EditWindow lw(this);
            lw.init(_socket, _worker, _workersNames);
            lw.exec();
        }
    }

    return QWidget::eventFilter(watched, event);
}

void WorkerWidget::setPower(const DataType& dataType)
{
    double totalPower = 0;

    for (auto& gpu : _worker.devices)
        totalPower += gpu.powerUsage;

    if (totalPower > 0 && _worker.status)
    {
        if (dataType == NEW || !_lastState)
        {
            QPixmap iconPower(":/worker/resources/worker/consumption.svg");
            QPainter qp = QPainter(&iconPower);
            qp.setCompositionMode(QPainter::CompositionMode_SourceIn);
            qp.fillRect(iconPower.rect(), Qt::black);
            qp.end();

            QHBoxLayout* hlay = new QHBoxLayout();
            hlay->setAlignment(Qt::AlignmentFlag::AlignRight
                             | Qt::AlignmentFlag::AlignVCenter);
            hlay->setContentsMargins(2,0,2,0);
            hlay->setSpacing(0);

            QLabel* lblPixmap = new QLabel();

            hlay->addWidget(lblPixmap);
            hlay->addWidget(_lblTotalPowerText);

            lblPixmap->setPixmap(iconPower);
            lblPixmap->setFixedSize(23,23);
            lblPixmap->setContentsMargins(2,0,2,0);
            lblPixmap->setScaledContents(true);
            lblPixmap->setStyleSheet("background-color: transparent;");

            ui->lblTotalPower->setLayout(hlay);
            ui->lblTotalPower->setStyleSheet(" background-color: transparent; "
                                             " font-size: 14px                ");
        }
            //Рассчёт для перевода в W - kW
            if (totalPower > 1000)
            {
                totalPower /= 1000;
                _lblTotalPowerText->setText(QString("%1%2").arg(QString::number(totalPower, 'f', 3), "kW"));
            }
            else
                _lblTotalPowerText->setText(QString("%1%2").arg(QString::number(totalPower, 'f', 1), "W"));
    }
}

void WorkerWidget::setLA(const DataType& dataType)
{
    if (dataType == NEW || !_lastState)
    {
        QHBoxLayout *hlay = new QHBoxLayout();
        hlay->setContentsMargins(0,0,5,0);
        hlay->setSpacing(0);
        hlay->setAlignment(Qt::AlignmentFlag::AlignRight);

        QLabel *la = new QLabel("Load Average");
        la->setContentsMargins(0,0,5,0);
        la->setStyleSheet(" background-color: transparent; "
                          " font-size: 16px                ");
        la->setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);

        QLabel *min1 = new QLabel("1min");
        min1->setContentsMargins(0,0,0,0);
        min1->setStyleSheet(" background-color: transparent; "
                            " font-size: 10px                ");
        min1->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop);

        QLabel *min5 = new QLabel("5min");
        min5->setContentsMargins(0,0,0,0);
        min5->setStyleSheet(" background-color: transparent; "
                            " font-size: 10px                ");
        min5->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop);

        QLabel *min15 = new QLabel("15min");
        min15->setContentsMargins(0,0,0,0);
        min15->setStyleSheet(" background-color: transparent; "
                             " font-size: 10px                ");
        min15->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop);

        hlay->addWidget(la);
        hlay->addWidget(min1);
        hlay->addWidget(_la1);
        hlay->addWidget(min5);
        hlay->addWidget(_la5);
        hlay->addWidget(min15);
        hlay->addWidget(_la15);

        ui->lblLA->setLayout(hlay);
        ui->lblLA->setAlignment(Qt::AlignmentFlag::AlignRight);

        _la1->setAlignment( Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);
        _la1->setContentsMargins(2,0,0,0);
        _la5->setAlignment( Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);
        _la5->setContentsMargins(2,0,0,0);
        _la15->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);
        _la15->setContentsMargins(2,0,0,0);
    }

    if (_worker.status)
    {
        _la1->setText(_worker.la1);
        if(_worker.la1.toDouble() >= 0.8)
            _la1->setStyleSheet(" background-color: transparent;  "
                                " color: #FF3733; font-size: 16px ");
        else
            _la1->setStyleSheet(" background-color: transparent;  "
                                " color: #818E9C; font-size: 16px ");

        _la5->setText(_worker.la5);
        if(_worker.la5.toDouble() >= 0.8)
            _la5->setStyleSheet(" background-color: transparent; "
                                " color: #FF3733;                "
                                " font-size: 16px                ");
        else
            _la5->setStyleSheet(" background-color: transparent; "
                                " color: #818E9C;                "
                                " font-size: 16px                ");

        _la15->setText(_worker.la15);
        if(_worker.la15.toDouble() >= 0.8)
            _la15->setStyleSheet(" background-color: transparent; "
                                 " color: #FF3733;                "
                                 " font-size: 16px                ");
        else
            _la15->setStyleSheet(" background-color: transparent; "
                                 " color: #818E9C;                "
                                 " font-size: 16px                ");
    }
}

void WorkerWidget::setUptime()
{
    QString negativeStyle = " QLabel {                       "
                            " font-size: 16px;               "
                            " color: #FF3733;                "
                            " border: none;                  "
                            " background-color: transparent; "
                            " margin-left: 10px;};           ";
    QString normalStyle = " QLabel {                       "
                          " font-size: 16px;               "
                          " color: black;                  "
                          " border: none;                  "
                          " background-color: transparent; "
                          " margin-left: 10px;};           ";
    ui->lblUptime->setStyleSheet(!_worker.lastOnline.toSecsSinceEpoch() ? negativeStyle : normalStyle);

    long long timeSeconds = _worker.sysUptime;
    if(!_worker.status)
    {
        if (_worker.lastOnline.toMSecsSinceEpoch() == 0)
        {
            ui->lblUptime->setText("uptime never");
            return;
        }
        else
            timeSeconds = QDateTime::currentDateTime().toSecsSinceEpoch()
                                  - _worker.lastOnline.toSecsSinceEpoch();
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

    ui->lblUptime->setText(_worker.status ? ("uptime " + normalizeTime) : QString(
                                                "%1 %2 %3").arg("was", normalizeTime, "ago"));
}

void WorkerWidget::setMinerUptime()
{
    QString normalStyle = " QLabel {                       "
                          " font-size: 16px;               "
                          " color: black;                  "
                          " border: none;                  "
                          " background-color: transparent; "
                          " margin-left: 10px;};           ";
    ui->lblMinerUptime->setStyleSheet(normalStyle);

    long long timeSeconds = _worker.minerUptime;
    if(!_worker.status)
    {
        ui->lblMinerUptime->setText("");
        return;
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

    if(!_worker.minerUptime && _worker.status)
        ui->lblMinerUptime->setText("");
    else
         ui->lblMinerUptime->setText("miner " + normalizeTime);
}

void WorkerWidget::setCPU(const DataType& dataType)
{
    if (dataType == NEW)
    {
        QPixmap iconCpu(":/worker/resources/worker/cpu.svg");
        QPainter qpCpu = QPainter(&iconCpu);
        qpCpu.setCompositionMode(QPainter::CompositionMode_SourceIn);
        qpCpu.fillRect(iconCpu.rect(), Qt::black);
        qpCpu.end();

        ui->lblCPUIcon->setPixmap(iconCpu);
        ui->lblCPUIcon->setFixedSize(50,50);
        ui->lblCPUIcon->setContentsMargins(0,0,0,0);
        ui->lblCPUIcon->setScaledContents(true);
        ui->lblCPUIcon->setStyleSheet("background-color: transparent;");

        ui->layCPUText->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->layCPUText->setSpacing(5);
        ui->layCPUText->setContentsMargins(0,0,0,0);

        ui->layCPUText->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->layCPUText->setSpacing(5);
        ui->layCPUText->setContentsMargins(0,0,0,0);
    }

    QString style = (" background-color: transparent; "
                     " font-size: 10px;               ");
    if (_worker.cpuInfo.isEmpty())
        ui->lblCPUText->setText(QString("<b>CPU</b> -/-"));
    else
        ui->lblCPUText->setText(QString("<b>CPU</b> %1").arg(_worker.cpuInfo));

    ui->lblCPUText->setStyleSheet(style);

    if (_worker.status)
        ui->lblCPUTemperature->setText(QString("<b>CPU TEMP</b> %1°C").arg(
                                                              _worker.cpuTemp));
    else
        ui->lblCPUTemperature->setText("<b>CPU TEMP</b> -/-");

    ui->lblCPUTemperature->setStyleSheet(style);
}

void WorkerWidget::setMB()
{
    QString style = (" background-color: transparent; "
                     " font-size: 10px;               ");

    if (_worker.motherboardInfo.isEmpty())
        ui->lblMBText->setText(QString("<b>Motherboard</b> -/-"));
    else
        ui->lblMBText->setText(QString("<b>Motherboard</b> %1").arg(
                                                      _worker.motherboardInfo));
    ui->lblMBText->setStyleSheet(style);
}

void WorkerWidget::setDisk(const DataType& dataType)
{
    if (dataType == NEW)
    {
        ui->vBlock2->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->vBlock2->setSpacing(5);
        ui->vBlock2->setContentsMargins(0,0,0,0);
    }

    QString style = (" background-color: transparent; "
                     " font-size: 10px;               ");

    if (_worker.diskModel.isEmpty())
        ui->lblDiskModel->setText(QString("<b>Disk Model</b> -/-"));
    else
        ui->lblDiskModel->setText(QString("<b>Disk Model</b> %1").arg(
                                                            _worker.diskModel));
    ui->lblDiskModel->setStyleSheet(style);

    if (_worker.diskSize.isEmpty())
        ui->lblDiskSize->setText(QString("<b>Disk Size</b> -/-"));
    else
        ui->lblDiskSize->setText(QString("<b>Disk Size</b> %1").arg(
                                                             _worker.diskSize));
    ui->lblDiskSize->setStyleSheet(style);

    ui->lblDiskFreeSpace->setStyleSheet(style);
    if (_worker.status)
        ui->lblDiskFreeSpace->setText(QString("<b>Free Space</b> %1").arg(
                                                        _worker.diskFreeSpace));
    else
        ui->lblDiskFreeSpace->setText("<b>Free Space</b> -/-");

}

void WorkerWidget::setRAM(const DataType& dataType)
{
    if (dataType == NEW)
    {
        ui->vBlock3->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->vBlock3->setSpacing(5);
        ui->vBlock3->setContentsMargins(0,0,0,0);
    }

    QString style = (" background-color: transparent; "
                     " font-size: 10px;               ");

    ui->lblRAMTotal->setStyleSheet(style);
    ui->lblRAMUsed->setStyleSheet(style);
    ui->lblRAMFree->setStyleSheet(style);

    if (_worker.status)
    {
        QString ramUsed = QString::number(std::stof(_worker.ramTotal.toStdString())
                                - std::stof(_worker.ramFree.toStdString()), 'f', 2);

        ui->lblRAMTotal->setText(QString("<b>RAM Total</b> %1").arg(_worker.ramTotal));
        ui->lblRAMUsed->setText(QString("<b>RAM Used</b> %1 Gb").arg(ramUsed));
        ui->lblRAMFree->setText(QString("<b>RAM Free</b> %1").arg(_worker.ramFree));
    }
    else
    {
         ui->lblRAMTotal->setText("<b>RAM Total</b> -/-");
        ui->lblRAMUsed->setText("<b>RAM Used</b> -/-");
        ui->lblRAMFree->setText("<b>RAM Free</b> -/-");
    }
}

void WorkerWidget::setIP(const DataType& dataType)
{
    if (dataType == NEW)
    {
        ui->vBlock4->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->vBlock4->setSpacing(5);
        ui->vBlock4->setContentsMargins(0,0,0,0);

        ui->hLayExtIP->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->hLayExtIP->setSpacing(5);
        ui->hLayExtIP->setContentsMargins(0,0,0,0);

        ui->hLayLocalIP->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->hLayLocalIP->setSpacing(5);
        ui->hLayLocalIP->setContentsMargins(0,0,0,0);

        QPixmap iconCopyExt(":/worker/resources/worker/copy.svg");
        QPainter qpExt = QPainter(&iconCopyExt);
        qpExt.setCompositionMode(QPainter::CompositionMode_SourceIn);
        qpExt.fillRect(iconCopyExt.rect(), Qt::black);
        qpExt.end();

        QPixmap iconCopyLocal(":/worker/resources/worker/copy.svg");
        QPainter qpLocal = QPainter(&iconCopyLocal);
        qpLocal.setCompositionMode(QPainter::CompositionMode_SourceIn);
        qpLocal.fillRect(iconCopyLocal.rect(), Qt::black);
        qpLocal.end();

        QString copyStyle = " background-color: transparent; "
                            " color: orange;                 ";
        ui->lblExtIPCOPY->setPixmap(iconCopyExt);
        ui->lblExtIPCOPY->setFixedSize(12,12);
        ui->lblExtIPCOPY->setContentsMargins(0,0,0,0);
        ui->lblExtIPCOPY->setScaledContents(true);
        ui->lblExtIPCOPY->installEventFilter(this);
        ui->lblExtIPCOPY->setToolTip("Копировать Remote IP");
        ui->lblExtIPCOPY->setStyleSheet(copyStyle);

        ui->lblLocalIPCOPY->setPixmap(iconCopyLocal);
        ui->lblLocalIPCOPY->setFixedSize(12,12);
        ui->lblLocalIPCOPY->setContentsMargins(0,0,0,0);
        ui->lblLocalIPCOPY->setScaledContents(true);
        ui->lblLocalIPCOPY->installEventFilter(this);
        ui->lblLocalIPCOPY->setToolTip("Копировать Local IP");
        ui->lblLocalIPCOPY->setStyleSheet(copyStyle);
    }

    QString style = (" background-color: transparent; "
                     " font-size: 10px;               ");

    if (_worker.extIp.isEmpty())
    {
        ui->lblExtIPTRext->setText(QString("<b>Remote IP</b> -/-"));
        ui->lblExtIPCOPY->setHidden(true);
    }
    else
    {
        ui->lblExtIPTRext->setText(QString("<b>Remote IP</b> %1").arg(
                                                                _worker.extIp));
        ui->lblExtIPCOPY->setHidden(false);
    }
    ui->lblExtIPTRext->setStyleSheet(style);

    if (_worker.localIp.isEmpty())
    {
        ui->lblLocalIPText->setText(QString("<b>Local IP</b> -/-"));
        ui->lblLocalIPCOPY->setHidden(true);
    }
    else
    {
        ui->lblLocalIPText->setText(QString("<b>Local IP</b> %1").arg(
                                                              _worker.localIp));
        ui->lblLocalIPCOPY->setHidden(false);
    }
    ui->lblLocalIPText->setStyleSheet(style);

    if (_worker.mac.isEmpty())
        ui->lblMAC->setText(QString("<b>MAC Address</b> -/-"));
    else
        ui->lblMAC->setText(QString("<b>MAC Address</b> %1").arg(_worker.mac));
    ui->lblMAC->setStyleSheet(style);
}

void WorkerWidget::setVersion(const DataType& dataType)
{
    if (dataType == NEW)
    {
        ui->hBlock5->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->hBlock5->setSpacing(5);
        ui->hBlock5->setContentsMargins(0,0,0,0);

        ui->vBlock5Text->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->vBlock5Text->setSpacing(5);
        ui->vBlock5Text->setContentsMargins(0,0,0,0);

        QPixmap iconLinux(":/worker/resources/worker/linux.svg");
        QPainter qpLnx = QPainter(&iconLinux);
        qpLnx.setCompositionMode(QPainter::CompositionMode_SourceIn);
        qpLnx.fillRect(iconLinux.rect(), Qt::black);
        qpLnx.end();

        QString style = "background-color: transparent;";
        ui->lblKernelLogo->setPixmap(iconLinux);
        ui->lblKernelLogo->setFixedSize(50,50);
        ui->lblKernelLogo->setStyleSheet(style);
        ui->lblKernelLogo->setContentsMargins(0,0,0,0);
        ui->lblKernelLogo->setScaledContents(true);
    }

    QString style = (" background-color: transparent; "
                     " font-size: 10px;               ");

    if (_worker.kernelVersion.isEmpty())
        ui->lblKernelText->setText(QString("<b>Kernel</b> -/-"));
    else
        ui->lblKernelText->setText(QString("<b>Kernel</b> %1").arg(_worker.kernelVersion));
    ui->lblKernelText->setStyleSheet(style);

    ui->lblNvidia->setText(QString("<b>NVIDIA</b> %1").arg(
        _worker.nvidiaVersion.isEmpty() ? "not installed" : _worker.nvidiaVersion));
    ui->lblNvidia->setStyleSheet(style);
    ui->lblAMD->setText(QString("<b>AMD</b> %1").arg(
        _worker.amdVersion.isEmpty() ? "not installed" : _worker.amdVersion));
    ui->lblAMD->setStyleSheet(style);
}

void WorkerWidget::setWallet()
{
    if (_worker.minerName.length())
    {
        QPixmap iconBag(":/worker/resources/worker/bag.png");
        QPainter qp = QPainter(&iconBag);
        qp.setCompositionMode(QPainter::CompositionMode_SourceIn);
        qp.fillRect(iconBag.rect(), Qt::black);
        qp.end();

        ui->lblWallet1->setPixmap(iconBag);
        ui->lblWallet1->setFixedSize(18,18);
        ui->lblWallet1->setContentsMargins(2,0,2,0);
        ui->lblWallet1->setScaledContents(true);
        ui->lblWallet1->setStyleSheet(" background-color: transparent; "
                                      " color: orange;                 ");
        ui->lblWallet1->setToolTip(_worker.user1);
        ui->lblWallet1->installEventFilter(this);

        if (_worker.algorithm2.length())
        {
            ui->lblWallet2->setPixmap(iconBag);
            ui->lblWallet2->setFixedSize(18,18);
            ui->lblWallet2->setContentsMargins(2,0,2,0);
            ui->lblWallet2->setScaledContents(true);
            ui->lblWallet2->setStyleSheet(" background-color: transparent; "
                                          " color: orange;                 ");
            ui->lblWallet2->setToolTip(_worker.user2);
            ui->lblWallet2->installEventFilter(this);
        }
    }
}

void WorkerWidget::setStat()
{
    if (_worker.minerName.length())
    {
        double totalR = _worker.totalRejectedShares1
                      + _worker.totalRejectedShares2
                      + _worker.totalRejectedSharesZil;

        double totalS = _worker.totalStaleShares1
                      + _worker.totalStaleShares2
                      + _worker.totalStaleSharesZil;

        double totalA = _worker.totalAcceptedShares1
                      + _worker.totalAcceptedShares2
                      + _worker.totalAcceptedSharesZil;

        double keff = 100 - double(totalR + totalS) / totalA * 100;

        if(keff >= 100)
        {
            keff = 100;
            ui->lblStat->setText(QString::number(int(keff)) + '%');
            ui->lblStat->setStyleSheet(" margin-left: 10px;             "
                                       " background-color: transparent; "
                                       " color: black;                  "
                                       " font-size: 16px                ");
        }
        else if(keff < 100 && totalA >= 10)
        {
            ui->lblStat->setText(QString::number(keff, 'f', 2) + '%');
            if (keff <= 98 && keff >= 97)
                ui->lblStat->setStyleSheet(" margin-left: 10px;             "
                                           " background-color: transparent; "
                                           " color: #FFAE00;                "
                                           " font-size: 16px                ");
            else if (keff < 97)
                ui->lblStat->setStyleSheet(" margin-left: 10px;             "
                                           " background-color: transparent; "
                                           " color: #FF3733;                "
                                           " font-size: 16px                ");
            else
                ui->lblStat->setStyleSheet(" margin-left: 10px;             "
                                           " background-color: transparent; "
                                           " color: black;                  "
                                           " font-size: 16px                ");
        }
        else
            ui->lblStat->setText("");

        // ACCEPTED
        ui->lblAccepted->setStyleSheet(" background-color: transparent; "
                                       " color: black;                  "
                                       " font-size: 16px                ");
        qint32 totalAccepted = _worker.totalAcceptedShares1 +
                               _worker.totalAcceptedShares2 +
                               _worker.totalAcceptedSharesZil;
        if(totalAccepted)
            ui->lblAccepted->setText("A " + QString::number(totalAccepted));
        else
            ui->lblAccepted->setText("");

        // STALED
        ui->lblStaled->setStyleSheet(" background-color: transparent; "
                                     " color: #FFAE00;                "
                                     " font-size: 16px                ");
        qint32 totalStaled = _worker.totalStaleShares1
                           + _worker.totalStaleShares2
                           + _worker.totalStaleSharesZil;
        if(totalStaled)
            ui->lblStaled->setText("S " + QString::number(totalStaled));
        else
            ui->lblStaled->setText("");

        // REJECTED + INVALIDED
        ui->lblRejected->setStyleSheet(" background-color: transparent; "
                                       " color: #FF3733;                "
                                       " font-size: 16px                ");
        qint32 totalNegative = _worker.totalRejectedShares1
                             + _worker.totalRejectedShares2
                             + _worker.totalRejectedSharesZil
                             + _worker.totalInvalidShares1
                             + _worker.totalInvalidShares2
                             + _worker.totalInvalidSharesZil;

        if(totalNegative)
            ui->lblRejected->setText("R " + QString::number(totalNegative));
        else
            ui->lblRejected->setText("");
    }
}

void WorkerWidget::setSpeed()
{
    if (!_worker.minerName.isEmpty())
    {
        ui->lblSpeed1->setStyleSheet(" background-color: transparent; "
                                     " color: black;                  "
                                     " font-size: 16px                ");
        ui->lblSpeed2->setStyleSheet(" background-color: transparent; "
                                     " color: black;                  "
                                     " font-size: 16px                ");

        long double algorithm1 = 0;
        long double algorithm2 = 0;
        long double algorithmZil = 0;
        for (auto& GPU : _worker.devices)
        {
            algorithm1 += GPU.speed1;
            algorithm2 += GPU.speed2;
            algorithmZil += GPU.speedZil;
        }

        auto calcValue = [](QString& value, const long double& speed, long double& transformSpeed)
        {
            if(speed >= 1000 && speed < 100000)
            {
                transformSpeed /= 1000;
                value = "KH";
            }
            else if(speed >= 100000 && speed < 100000000)
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
            ui->lblSpeed1->setText(QString("%1 %2").arg(
                                   QString::number(transformedSpeed1, 'f', 2),
                                                                     value1));
            ui->lblSpeed2->setText(QString("%1 %2").arg(
                                   QString::number(transformedSpeed2, 'f', 2),
                                                                     value2));
        }
        else if (algorithm1 > 0 && algorithm2 == 0)
        {
            ui->lblSpeed1->setText(QString("%1 %2").arg(
                                   QString::number(transformedSpeed1, 'f', 2),
                                                                     value1));
            ui->lblSpeed2->setText(QString(""));
        }
        else if (algorithm1 == 0 && algorithm2 > 0)
        {
            ui->lblSpeed2->setText(QString("%1 %2").arg(
                                   QString::number(transformedSpeed2, 'f', 2),
                                                                     value2));
            ui->lblSpeed1->setText(QString("n/a"));
        }
        else
        {
            ui->lblSpeed1->setText("n/a");
            if (_worker.algorithm2.length())
                ui->lblSpeed2->setText("n/a");
            else
                ui->lblSpeed2->setText("");
        }
    }
    else
    {
        ui->lblSpeed1->setText("");
        ui->lblSpeed2->setText("");
    }
}

void WorkerWidget::setGPUWidgets()
{
    // TODO ДОРАБОТАТЬ ИЗМЕНИТЬ НАЗВАНИЯ ПОДОГНАТЬ КРАСОТУ
    if (_worker.minerName.length())
    {
        for (auto& gpu : _worker.devices)
        {
            QWidget *GPUWidget = new QWidget();
            GPUWidget->setContentsMargins(0,0,0,0);
            GPUWidget->setStyleSheet(" background-color: transparent; "
                                     " color: black;                  "
                                     " font-size: 12px;               "
                                     " border-width: 1px;             "
                                     " border-style: outset;          "
                                     " border-color: black;           "
                                     " border-radius: 3px;            ");


            QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
            shadowEffect->setBlurRadius(9); // Устанавливаем радиус размытия
            shadowEffect->setOffset(2); // Устанавливаем смещение тени
            GPUWidget->setGraphicsEffect(shadowEffect); // Устанавливаем эффект тени на окно

            GPUWidget->setFixedSize(52, 70);
            if(!_worker.algorithm2.isEmpty())
                GPUWidget->setFixedHeight(75);
            QVBoxLayout *GPUlay = new QVBoxLayout();
            GPUlay->setContentsMargins(0,0,0,0);
            GPUlay->setSpacing(2);

            QLabel *temp = new QLabel(QString(" %1°C").arg(QString::number(gpu.coreTemp)));
            temp->setFixedHeight(12);
            temp->setAlignment(Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignTop);
            temp->setContentsMargins(0,0,0,0);
            temp->setStyleSheet(" background-color: transparent; "
                                " color: black;                  "
                                " font-size: 12px;               "
                                " border: none;                  ");

            QLabel *fan = new QLabel(QString(" %1%").arg(QString::number(gpu.fanSpeed)));
            fan->setAlignment(Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignTop);
            fan->setContentsMargins(0,0,0,0);
            fan->setFixedHeight(17);
            if (gpu.fanSpeed == 0)
                fan->setStyleSheet(" margin-left: 5px;              "
                                   " margin-right: 5px;             "
                                   " background-color: transparent; "
                                   " color: black;                  "
                                   " font-size: 12px;               "
                                   " border: none;                  ");
            else if (gpu.fanSpeed > 0 && gpu.fanSpeed <= 35)
                fan->setStyleSheet(" margin-left: 5px;          "
                                   " margin-right: 5px;         "
                                   " background-color: #5c6bc0; "
                                   " color: #16191d;            "
                                   " font-size: 12px;           "
                                   " border: none;              ");
            else if (gpu.fanSpeed > 35 && gpu.fanSpeed <= 65)
                fan->setStyleSheet(" margin-left: 5px;          "
                                   " margin-right: 5px;         "
                                   " background-color: #2392dc; "
                                   " color: #16191d;            "
                                   " font-size: 12px;           "
                                   " border: none;              ");
            else if (gpu.fanSpeed > 65 && gpu.fanSpeed <= 80)
                fan->setStyleSheet(" margin-left: 5px;          "
                                   " margin-right: 5px;         "
                                   " background-color: #fcce00; "
                                   " color: #16191d;            "
                                   " font-size: 12px;           "
                                   " border: none;              " );
            else if (gpu.fanSpeed > 80 && gpu.fanSpeed <= 100)
                fan->setStyleSheet(" margin-left: 5px;          "
                                   " margin-right: 5px;         "
                                   " background-color: #ff504c; "
                                   " color: #16191d;            "
                                   " font-size: 12px;           "
                                   " border: none;              ");

            long double algorithm1 = gpu.speed1;
            long double algorithm2 = gpu.speed2;

            if(algorithm1 >= 1000 && algorithm1 < 100000)
                algorithm1 /= 1000;
            else if(algorithm1 >= 100000 && algorithm1 < 100000000)
                algorithm1 /= 1000000;
            else if(algorithm1 >= 100000000 && algorithm1 < 1000000000000)
                algorithm1 /= 1000000000;
            if(algorithm2 >= 1000 && algorithm2 < 100000)
                algorithm2 /= 1000;
            else if(algorithm2 >= 100000 && algorithm2 < 1000000000)
                algorithm2 /= 1000000;
            else if(algorithm2 >= 100000000 && algorithm2 < 1000000000000)
                algorithm2 /= 1000000000;

            QLabel *speed = nullptr;
            QLabel *speed2 = nullptr;

            if (algorithm1)
            {
                speed = new QLabel(QString::number(algorithm1, 'f', 2));
                speed->setStyleSheet(" margin-top: 5px; "
                                     " color: black;    "
                                     " font-size: 14px; "
                                     " border: none     ");
            }
            else
            {
                speed = new QLabel("n/a");
                speed->setStyleSheet(" margin-top: 5px; "
                                     " color: yellow;   "
                                     " font-size: 14px; "
                                     " border: none     ");
            }

            if (algorithm2)
            {
                speed2 = new QLabel(QString::number(algorithm2, 'f', 2));
                speed2->setStyleSheet(" color: black;    "
                                      " font-size: 14px; "
                                      " border: none     ");
            }
            else
            {
                speed2 = new QLabel("n/a");
                speed2->setStyleSheet(" color: yellow;   "
                                      " font-size: 14px; "
                                      " border: none     ");
            }

            speed->setAlignment(Qt::AlignmentFlag::AlignCenter);
            speed2->setAlignment(Qt::AlignmentFlag::AlignCenter);
            speed->setFixedHeight(17);
            speed2->setFixedHeight(17);

            GPUlay->addWidget(temp);
            GPUlay->addWidget(fan);
            GPUlay->addWidget(speed);

            if(algorithm2 > 0)
                GPUlay->addWidget(speed2);

            GPUWidget->setLayout(GPUlay);
            ui->gpuLay->addWidget(GPUWidget);
            ui->gpuLay->setAlignment(Qt::AlignmentFlag::AlignTop);
        }
    }
}

void WorkerWidget::setGPUList()
{
    if (_worker.devices.count() < ui->lwGPU->count())
            ui->lwGPU->clear();

    for (const data::gpuData& gpu : _worker.devices)
    {
        bool isFind = false;
        for (int i = 0; i < ui->lwGPU->count(); ++i)
        {
            QListWidgetItem* item = ui->lwGPU->item(i);
            QUuidEx id = item->data(Qt::UserRole).toUuid();
            if (id == gpu.id)
            {
                GPUItem* widget = qobject_cast<GPUItem*>(ui->lwGPU->itemWidget(item));
                widget->updateData(gpu);

                if (!_worker.status)
                    widget->setOffline();

                isFind = true;
                continue;
            }
        }

        if (!isFind)
        {
            GPUItem *g = new GPUItem();
            g->init(gpu, _socket);

            QListWidgetItem* item = new QListWidgetItem();
            item->setData(Qt::UserRole, gpu.id);
            item->setSizeHint(g->sizeHint());
            ui->lwGPU->addItem(item);
            ui->lwGPU->setItemWidget(item, g);

    //        g->setAttribute(Qt::WA_Hover);
    //        g->installEventFilter(this);
        }
    }
}
