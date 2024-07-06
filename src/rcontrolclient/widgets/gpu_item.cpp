#include "gpu_item.h"
#include "ui_gpu_item.h"

GPUItem::GPUItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GPUItem)
{
    ui->setupUi(this);

    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(3); // Устанавливаем радиус размытия
    shadowEffect->setOffset(0.5); // Устанавливаем смещение тени
    this->setGraphicsEffect(shadowEffect); // Устанавливаем эффект тени на окно

    qRegisterMetaType<data::gpuData>("data::gpuData");
}

GPUItem::~GPUItem()
{
    delete ui;
}

bool GPUItem::init(const data::gpuData& gpu, tcp::Socket::Ptr socket)
{
    _GPU = gpu;
    _socket = socket;

    renderData(NEW);

    return true;
}

void GPUItem::setID()
{
    QString style = (" background-color: transparent; "
                     " font-size: 14px;               ");
    ui->lblID->setText(QString("<b>GPU %1</b>").arg(QString::number(_GPU.gpuId)));
    ui->lblID->setStyleSheet(style);
}

void GPUItem::setBUS()
{
    QString style = (" background-color: transparent; "
                     " font-size: 12px;               ");
    ui->lblBUS->setText(_GPU.busId);
    ui->lblBUS->setStyleSheet(style);
}

void GPUItem::setModel()
{
    QString style = " background-color: transparent;   "
                    " font-size: 14px; color: #84BF40; ";
    if (_GPU.name.toLower().contains("amd"))
        style = " background-color: transparent;   "
                " font-size: 14px; color: #b46fc3; ";

    ui->lblModel->setText(QString("<b>%1</b>").arg(_GPU.name));
    ui->lblModel->setStyleSheet(style);
}

void GPUItem::setMemSze()
{
    QString style = (" background-color: transparent;      "
                     " font-size: 12px; margin-left: 10px; ");
    ui->lblMemSize->setText(QString("%1 MB").arg(_GPU.totalMemory));
    ui->lblMemSize->setStyleSheet(style);
}

void GPUItem::setVendor()
{
    QString style = (" background-color: transparent; "
                     " font-size: 12px;               ");
    ui->lblVendor->setText(_GPU.vendor);
    ui->lblVendor->setStyleSheet(style);
}

void GPUItem::setvBIOS()
{
    QString style = (" background-color: transparent; "
                     " font-size: 12px;               ");
    ui->lblvBIOS->setText(_GPU.vbiosVersion);
    ui->lblvBIOS->setStyleSheet(style);
}

void GPUItem::setPLInfo()
{
    QString style = (" background-color: transparent; "
                     " font-size: 12px;               ");
    ui->lblPL->setText(QString("PL %1 w, %2 w, %3 w").arg(_GPU.minPl, _GPU.defaultPl, _GPU.maxPl));
    ui->lblPL->setStyleSheet(style);
}

void GPUItem::setStat()
{
    ui->lblAccept->clear();
    ui->lblStale->clear();
    ui->lblReject->clear();
    ui->lblInvalid->clear();

    unsigned long int total;

    total = _GPU.acceptedShares1 + _GPU.acceptedShares2 + _GPU.acceptedSharesZil;
    if (total)
    {
        ui->lblAccept->setText(QString("A %1").arg(QString::number(total)));
        QString styleAccept = (" background-color:             "
                               " transparent; font-size: 14px; "
                               " color: #84BF40;               "
                               " margin-left: 10px;            ");
        ui->lblAccept->setStyleSheet(styleAccept);
    }

    total = _GPU.staleShares1 + _GPU.staleShares2 + _GPU.staleSharesZil;
    if (total)
    {
        ui->lblStale->setText(QString("S %1").arg(QString::number(total)));
        QString styleStale = (" background-color: transparent; "
                              " font-size: 14px;               "
                              " color: #995c00;                ");
        ui->lblStale->setStyleSheet(styleStale);
    }

    total = _GPU.rejectedShares1 + _GPU.rejectedShares2 + _GPU.rejectedSharesZil;
    if (total)
    {
        ui->lblReject->setText(QString("R %1").arg(QString::number(total)));
        QString styleReject = (" background-color: transparent; "
                               " font-size: 14px;               "
                               " color: #FF3733;                ");
        ui->lblReject->setStyleSheet(styleReject);
    }

    total = _GPU.invalidShares1 + _GPU.invalidShares2 + _GPU.invalidSharesZil;
    if (total)
    {
        ui->lblInvalid->setText(QString("I %1").arg(QString::number(total)));
        QString styleInvalid = (" background-color: transparent; "
                                " font-size: 14px;               "
                                " color: #FF3733;                ");
        ui->lblInvalid->setStyleSheet(styleInvalid);
    }
}

void GPUItem::setSpeed()
{
    auto calcValue = [](QString& value, const long long& speed, long double& transformSpeed)
    {
        if(speed >= 1000 && speed < 100000)
        {
            transformSpeed /= 1000;
            value = "kH";
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
    long double transformedSpeed1 = _GPU.speed1;
    long double transformedSpeed2 = _GPU.speed2;
    //long double transformedSpeedZil = 0;

    calcValue(value1, _GPU.speed1, transformedSpeed1);
    calcValue(value2, _GPU.speed2, transformedSpeed2);
    //calcValue(valueZil, _GPU.speedZil, transformedSpeedZil);

    QString style = " background-color: transparent; "
                    " font-size: 14px                ";
    QString algorithmText;
    ui->lblSpeed1->setAlignment(Qt::AlignmentFlag::AlignRight);
    ui->lblSpeed1->setStyleSheet(style);

    ui->lblSpeed2->setAlignment(Qt::AlignmentFlag::AlignRight);
    ui->lblSpeed2->setStyleSheet(style);

    if(_GPU.speed1 > 0 && _GPU.speed2 > 0)
    {
        ui->lblSpeed1->setText(QString("%1 %2").arg(QString::number(transformedSpeed1, 'f', 2), value1));
        ui->lblSpeed2->setText(QString("%1 %2").arg(QString::number(transformedSpeed2, 'f', 2), value2));

        ui->lblSpeed1->setHidden(false);
        ui->lblSpeed2->setHidden(false);
    }
    else if (_GPU.speed1 > 0 && _GPU.speed2 == 0)
    {
        ui->lblSpeed1->setText(QString("%1 %2").arg(QString::number(transformedSpeed1, 'f', 2), value1));
        ui->lblSpeed1->setHidden(false);
        ui->lblSpeed2->setHidden(true);
    }
    else if (_GPU.speed1 == 0 && _GPU.speed2 > 0)
    {
        ui->lblSpeed2->setText(QString("%1 %2").arg(QString::number(transformedSpeed2, 'f', 2), value2));
        ui->lblSpeed1->setText(QString("n/a"));

        ui->lblSpeed1->setHidden(false);
        ui->lblSpeed2->setHidden(false);
    }
    else
    {
        ui->lblSpeed1->setHidden(true);
        ui->lblSpeed2->setHidden(true);
    }
}

void GPUItem::setTemp(const DataType& dataType)
{
    if (dataType == NEW)
    {
        QPixmap iconTemp(":/worker/resources/worker/temperature.svg");
        QPainter qpTemp = QPainter(&iconTemp);
        qpTemp.setCompositionMode(QPainter::CompositionMode_SourceIn);
        qpTemp.fillRect(iconTemp.rect(), Qt::black);
        qpTemp.end();

        QString styleIcon = "background-color: transparent; color: orange;";
        ui->lblTempIcon->setPixmap(iconTemp);
        ui->lblTempIcon->setFixedSize(23,23);
        ui->lblTempIcon->setContentsMargins(0,0,0,0);
        ui->lblTempIcon->setScaledContents(true);
        ui->lblTempIcon->setStyleSheet(styleIcon);
    }

    QString coreStyle, memStyle;
    if(_GPU.coreTemp >= 60)
        coreStyle = " background-color: transparent; "
                    " color: #995c00;                "
                    " font-size: 14px                ";
    else if (_GPU.coreTemp > 65)
        coreStyle = " background-color: transparent; "
                    " color: #FF3733;                "
                    " font-size: 14px                ";
    else
        coreStyle = " background-color: transparent; "
                    " color: black;                  "
                    " font-size: 14px                ";

    if (_GPU.coreTemp)
        ui->lblCoreTemp->setText(QString("%1 °C").arg(QString::number(_GPU.coreTemp)));
    else
        ui->lblCoreTemp->setText("---/---");

    ui->lblCoreTemp->setStyleSheet(coreStyle);

    if (!_GPU.memoryTemp)
    {
        ui->lblMemTemp->setHidden(true);
        ui->lblTempIcon->setToolTip("CORE");
    }
    else
    {
        ui->lblMemTemp->setHidden(false);
        ui->lblTempIcon->setToolTip("CORE / MEM");

        if(_GPU.memoryTemp >= 80)
            memStyle = " background-color: transparent; "
                       " color: #995c00;                "
                       " font-size: 14px                ";
        else if (_GPU.memoryTemp > 95)
            memStyle = " background-color: transparent; "
                       " color: #FF3733;                "
                       " font-size: 14px                ";
        else
            memStyle = " background-color: transparent; "
                       " color: black;                  "
                       " font-size: 14px                ";

        ui->lblMemTemp->setText(QString("%1 °C").arg(QString::number(_GPU.memoryTemp)));
        ui->lblTempIcon->setToolTip("CORE / MEM");
        ui->lblMemTemp->setStyleSheet(memStyle);
    }
}

void GPUItem::setFAN(const DataType& dataType)
{
    if (dataType == NEW)
    {
        QPixmap iconFAN(":/worker/resources/worker/cooler.svg");
        QPainter qpFAN = QPainter(&iconFAN);
        qpFAN.setCompositionMode(QPainter::CompositionMode_SourceIn);
        qpFAN.fillRect(iconFAN.rect(), Qt::black);
        qpFAN.end();

        QString styleIcon = " background-color: transparent; "
                            " margin-left: 5px;              "
                            " margin-right: 5px              ";
        ui->lblFANIcon->setPixmap(iconFAN);
        ui->lblFANIcon->setFixedSize(33,23);
        ui->lblFANIcon->setContentsMargins(0,0,0,0);
        ui->lblFANIcon->setScaledContents(true);
        ui->lblFANIcon->setStyleSheet(styleIcon);
    }

    QString style;
    if (_GPU.fanSpeed == 0)
        style = " background-color: transparent; "
                " color: black;                  "
                " font-size: 14px;               ";
    else if (_GPU.fanSpeed > 0 && _GPU.fanSpeed <= 35)
        style = " background-color: transparent; "
                " color: black;                  "
                " font-size: 14px;               ";
    else if (_GPU.fanSpeed > 35 && _GPU.fanSpeed <= 65)
        style = " background-color: transparent; "
                " color: black;                  "
                " font-size: 14px;               ";
    else if (_GPU.fanSpeed > 65 && _GPU.fanSpeed <= 80)
        style = " background-color: transparent; "
                " color: #995c00;                "
                " font-size: 14px;               ";
    else if (_GPU.fanSpeed > 80 && _GPU.fanSpeed <= 100)
        style = " background-color: transparent; "
                " color: #ff504c;                "
                " font-size: 14px;               ";

    if (_GPU.fanSpeed)
        ui->lblFANValue->setText(QString("%1 %").arg(QString::number(_GPU.fanSpeed)));
    else
        ui->lblFANValue->setText("---/---");

    ui->lblFANValue->setStyleSheet(style);

    style = " background-color: transparent; "
            " font-size: 14px;               ";
    ui->lblFANSet->setText(QString("%1 %").arg(QString::number(_GPU.setFanSpeed)));
    ui->lblFANSet->setStyleSheet(style);
}

void GPUItem::setPower(const DataType& dataType)
{
    if (dataType == NEW)
    {
        QPixmap iconPower(":/worker/resources/worker/consumption.svg");
        QPainter qpSet = QPainter(&iconPower);
        qpSet.setCompositionMode(QPainter::CompositionMode_SourceIn);
        qpSet.fillRect(iconPower.rect(), Qt::black);
        qpSet.end();

        QString styleIcon = "background-color: transparent;";
        ui->lblPowerIcon->setPixmap(iconPower);
        ui->lblPowerIcon->setFixedSize(23,23);
        ui->lblPowerIcon->setContentsMargins(0,0,0,0);
        ui->lblPowerIcon->setScaledContents(true);
        ui->lblPowerIcon->setStyleSheet(styleIcon);
    }

    QString style = (" background-color: transparent; "
                     " font-size: 14px;               ");
    if (_GPU.powerUsage)
        ui->lblPowerValue->setText(QString("%1 w").arg(QString::number(_GPU.powerUsage, 'f', 0)));
    else
        ui->lblPowerValue->setText("----/----");
    ui->lblPowerValue->setStyleSheet(style);

    ui->lblSetPl->setText(QString("%1 w").arg(QString::number(_GPU.setPl)));
    ui->lblSetPl->setStyleSheet(style);
}

void GPUItem::setClocks()
{
    //QString curStyle = "background-color: transparent; font-size: 14px; margin-left: 10px; color: #FFAE00;";
    QString style = " background-color: transparent; "
                    " font-size: 14px;               "
                    " margin-left: 10px;             ";

    if (_GPU.coreClock)
        ui->lblCurrentCoreClock->setText(QString("%1 Mhz").arg(QString::number(_GPU.coreClock)));
    else
        ui->lblCurrentCoreClock->setText("------/------");
    if (_GPU.memoryClock)
        ui->lblCurrentMemClock->setText(QString("%1 Mhz").arg(QString::number(_GPU.memoryClock)));
    else
        ui->lblCurrentMemClock->setText("------/------");

    ui->lblSetCoreClock->setText(QString("%1 Mhz").arg(QString::number(_GPU.setCore)));
    ui->lblSetMemClock->setText(QString("%1 Mhz").arg(QString::number(_GPU.setMem)));

    ui->lblCurrentCoreClock->setStyleSheet(style);
    ui->lblCurrentMemClock->setStyleSheet(style);
    ui->lblSetCoreClock->setStyleSheet(style);
    ui->lblSetMemClock->setStyleSheet(style);
}

void GPUItem::renderData(const DataType& dataType)
{
    if (dataType == NEW)
    {
        setID();
        setBUS();
        setModel();
        setMemSze();
        setVendor();
        setvBIOS();
        setPLInfo();

        QPixmap* iconSet = new QPixmap(":/worker/resources/worker/speed.svg");
        QPainter qpSet = QPainter(iconSet);
        qpSet.setCompositionMode(QPainter::CompositionMode_SourceIn);
        qpSet.fillRect(iconSet->rect(), Qt::black);
        qpSet.end();

        QString setStyle = " background-color: transparent; "
                           " color: orange;                 ";
        ui->lblSetIcon->setPixmap(*iconSet);
        ui->lblSetIcon->setFixedSize(23,23);
        ui->lblSetIcon->setContentsMargins(0,0,0,0);
        ui->lblSetIcon->setScaledContents(true);
        ui->lblSetIcon->setToolTip("Задать параметры");
        ui->lblSetIcon->setStyleSheet(setStyle);
        ui->lblSetIcon->installEventFilter(this);

        ui->vLayID->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->vLayID->setSpacing(5);
        ui->vLayID->setContentsMargins(0,0,0,0);

        ui->vLayInfo->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->vLayInfo->setSpacing(5);
        ui->vLayInfo->setContentsMargins(0,0,0,0);

        ui->hLayStat->setAlignment(Qt::AlignmentFlag::AlignCenter);
        ui->hLayStat->setSpacing(5);
        ui->hLayStat->setContentsMargins(0,0,0,0);

        ui->vLaySpeed->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->vLaySpeed->setSpacing(5);
        ui->vLaySpeed->setContentsMargins(0,0,0,0);

        ui->hLayTemp->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->hLayTemp->setSpacing(5);
        ui->hLayTemp->setContentsMargins(0,0,0,0);

        ui->vLayTemp->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->vLayTemp->setSpacing(5);
        ui->vLayTemp->setContentsMargins(0,0,0,0);

        ui->hLayFAN->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->hLayFAN->setSpacing(5);
        ui->hLayFAN->setContentsMargins(0,0,0,0);

        ui->hLayPower->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->hLayPower->setSpacing(5);
        ui->hLayPower->setContentsMargins(0,0,0,0);

        ui->vLayPower->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->vLayPower->setSpacing(5);
        ui->vLayPower->setContentsMargins(0,0,0,0);

        ui->vLayClocks->setAlignment(Qt::AlignmentFlag::AlignLeft);
        ui->vLayClocks->setSpacing(5);
        ui->vLayClocks->setContentsMargins(0,0,0,0);
    }

    setStat();
    setSpeed();
    setTemp(dataType);
    setFAN(dataType);
    setPower(dataType);
    setClocks();
}

bool GPUItem::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->lblSetIcon)
    {
        QEvent::Type type = event->type();
        if (type == QMouseEvent::MouseButtonRelease)
        {
            GPUSettings settings(this);
            settings.init(_GPU, _socket);
            chk_connect_q(&settings, &GPUSettings::signalUpdateSetts,
                          this, &GPUItem::updateData)

            settings.exec();
        }
    }
    return QWidget::eventFilter(obj, event);
}

void GPUItem::updateData(const data::gpuData &GPU)
{
    _GPU = GPU;
    renderData(UPDATE);
}

void GPUItem::setOffline()
{
    ui->lblCoreTemp->setText("---/---");
    ui->lblCurrentCoreClock->setText("------/------");
    ui->lblCurrentMemClock->setText("------/------");
    ui->lblPowerValue->setText("---/---");
    ui->lblFANValue->setText("---/---");

    ui->lblAccept->clear();
    ui->lblStale->clear();
    ui->lblReject->clear();
    ui->lblInvalid->clear();
}
