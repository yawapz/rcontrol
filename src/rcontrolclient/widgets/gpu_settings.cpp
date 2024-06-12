#include "gpu_settings.h"
#include "ui_gpu_settings.h"

GPUSettings::GPUSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GPUSettings)
{
    ui->setupUi(this);

    setWindowModality(Qt::WindowModal);
    setWindowFlag(Qt::WindowTitleHint);
    setWindowFlag(Qt::FramelessWindowHint);

    QPainterPath painPath;
    painPath.addRoundedRect(rect(), 15, 15, Qt::AbsoluteSize);
    this->setMask(painPath.toFillPolygon().toPolygon());

    setStyleSheet(
    " GPUSettings {         "
    " border-style: double; "
    " border-width: 4px;    "
    " border-radius: 15px;  "
    " border-color: black;  "
    " }                     "
    );

    _btnStyle = " padding: 3px;                  "
                " border-style: outset;          "
                " border-width: 1px;             "
                " border-radius: 3px;            "
                " border-color: black;           "
                " font-size: 18px;               "
                " background-color: transparent; ";

    _hoverStyle = " padding: 3px;                              "
                  " border-style: outset;                      "
                  " border-width: 1px;                         "
                  " border-radius: 3px;                        "
                  " border-color: rgba(35, 146, 220, 0.5);     "
                  " font-size: 18px;                           "
                  " background-color: rgba(35, 146, 220, 0.5); ";

    _lineStyle = " QLineEdit {                          "
                 " border-style: outset;                "
                 " border-width: 1px;                   "
                 " border-radius: 3px;                  "
                 " border-color: black;                 "
                 " font-size: 18px;                     "
                 " background-color: transparent;}      "
                 " QLineEdit:focus {                    "
                 " border-style: outset;                "
                 " border-width: 2px;                   "
                 " border-radius: 3px;                  "
                 " border-color: rgba(35, 146, 220, 1); "
                 " font-size: 18px;                     "
                 " background-color: transparent;}      ";


    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(5); // Устанавливаем радиус размытия
    shadowEffect->setOffset(1); // Устанавливаем смещение тени
    ui->Widget->setGraphicsEffect(shadowEffect); // Устанавливаем эффект тени на окно

    ui->lineCore->setStyleSheet(_lineStyle);
    ui->lineFan->setStyleSheet(_lineStyle);
    ui->lineMemory->setStyleSheet(_lineStyle);
    ui->linePL->setStyleSheet(_lineStyle);
    ui->btnAccept->setStyleSheet(_btnStyle);
    ui->btnCancel->setStyleSheet(_btnStyle);

    ui->btnAccept->installEventFilter(this);
    ui->btnCancel->installEventFilter(this);

    setTabOrder(ui->lineCore, ui->lineMemory);
    setTabOrder(ui->lineMemory, ui->linePL);
    setTabOrder(ui->linePL, ui->lineFan);
    setTabOrder(ui->lineFan, ui->btnCancel);
    setTabOrder(ui->btnCancel, ui->btnAccept);

    setFocus();
}

GPUSettings::~GPUSettings()
{
    delete ui;
}

bool GPUSettings::init(const data::gpuData& newData, tcp::Socket::Ptr socket)
{
    _gpuData = newData;
    _socket = socket;

    QLocale locale(QLocale::English);
    QIntValidator* validatorCore = new QIntValidator(0, 9999);
    validatorCore->setLocale(locale);
    QIntValidator* validatorMemory = new QIntValidator(0, 99999);
    validatorCore->setLocale(locale);
    QIntValidator* validatorPL = new QIntValidator(_gpuData.minPl.toFloat(),
                                                   _gpuData.maxPl.toFloat());
    validatorCore->setLocale(locale);
    QIntValidator* validatorFan = new QIntValidator(0, 100);
    validatorCore->setLocale(locale);

    ui->lineCore->setValidator(validatorCore);
    ui->lineMemory->setValidator(validatorMemory);
    ui->linePL->setValidator(validatorPL);
    ui->lineFan->setValidator(validatorFan);

    setID();
    setModel();
    setMemSize();
    setFAN();
    setPL();
    setCoreClock();
    setMemoryClock();

    return true;
}

void GPUSettings::setID()
{
    QString style = (" background-color: transparent; "
                     " font-size: 18px;               ");

    ui->lblIndex->setStyleSheet(style);
    ui->lblIndex->setText(QString("<b>GPU %1</b>").arg(QString::number(_gpuData.gpuId)));
}

void GPUSettings::setModel()
{
    QString style = " background-color: transparent;   "
                    " font-size: 18px; color: #84BF40; ";

    ui->lblModel->setStyleSheet(style);
    ui->lblModel->setText(QString("<b>%1</b>").arg(_gpuData.name));
}

void GPUSettings::setMemSize()
{
    QString style = (" background-color: transparent;      "
                     " font-size: 16px; margin-left: 10px; ");

    ui->lblMemSize->setStyleSheet(style);
    ui->lblMemSize->setText(QString("<b>%1 MB</b>").arg(_gpuData.totalMemory));
}

void GPUSettings::setFAN()
{
    ui->lineFan->setText(QString::number(_gpuData.setFanSpeed));
}

void GPUSettings::setPL()
{
    ui->linePL->setText(QString::number(_gpuData.setPl));
}

void GPUSettings::setCoreClock()
{
    ui->lineCore->setText(QString::number(_gpuData.setCore));
}

void GPUSettings::setMemoryClock()
{
    ui->lineMemory->setText(QString::number(_gpuData.setMem));
}

void GPUSettings::keyPressEvent(QKeyEvent *event)
{
    if (QString::number(event->key()) == "16777220" || QString::number(event->key()) == "16777221")
        ui->btnAccept->click();
    else if (QString::number(event->key()) == "16777216")
        ui->btnCancel->click();
}

bool GPUSettings::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->btnAccept)
    {
        QEvent::Type type = event->type();
        if  (type == QEvent::HoverLeave)
            ui->btnAccept->setStyleSheet(_btnStyle);
        else if (type == QEvent::HoverEnter)
            ui->btnAccept->setStyleSheet(_hoverStyle);
    }
    if (obj == ui->btnCancel)
    {
        QEvent::Type type = event->type();
        if  (type == QEvent::HoverLeave)
            ui->btnCancel->setStyleSheet(_btnStyle);
        else if (type == QEvent::HoverEnter)
            ui->btnCancel->setStyleSheet(_hoverStyle);
    }

    return QWidget::eventFilter(obj, event);
}

void GPUSettings::on_btnAccept_clicked(bool checked)
{
    _socket->send(createMessage(data::UpdateGPUSettings(_gpuData)));
    emit signalUpdateSetts(_gpuData);
    close();
}

void GPUSettings::on_btnCancel_clicked(bool checked)
{
    close();
}


void GPUSettings::on_lineFan_textEdited(const QString &arg1)
{
    if (ui->lineFan->text().length())
        while (ui->lineFan->text().front() == '0' && ui->lineFan->text().length() > 1)
            ui->lineFan->setText(QString("%1").arg(ui->lineFan->text().remove(0, 1)));
    else
        ui->lineFan->setText("0");

    if (ui->lineFan->text().toInt() > 100)
        ui->lineFan->setText("100");

    _gpuData.setFanSpeed = ui->lineFan->text().toUInt();
}


void GPUSettings::on_linePL_textEdited(const QString &arg1)
{
    if (ui->linePL->text().length())
        while (ui->linePL->text().front() == '0' && ui->linePL->text().length() > 1)
            ui->linePL->setText(QString("%1").arg(ui->linePL->text().remove(0, 1)));

    if (!ui->linePL->text().length())
        _gpuData.setPl = _gpuData.defaultPl.toFloat();

    if (ui->linePL->text().toFloat() > _gpuData.maxPl.toFloat())
    {
        ui->linePL->setText(QString::number(_gpuData.maxPl.toFloat(), 'f', 0));
        _gpuData.setPl = _gpuData.maxPl.toFloat();
    }
    else if (ui->linePL->text().toFloat() < _gpuData.minPl.toFloat())
        _gpuData.setPl = _gpuData.minPl.toFloat();
    else
        _gpuData.setPl = ui->linePL->text().toFloat();
}


void GPUSettings::on_lineMemory_textEdited(const QString &arg1)
{
    if (ui->lineMemory->text().length())
        while (ui->lineMemory->text().front() == '0' && ui->lineMemory->text().length() > 1)
            ui->lineMemory->setText(QString("%1").arg(ui->lineMemory->text().remove(0, 1)));
    else
        ui->lineMemory->setText("0");

    _gpuData.setMem = ui->lineMemory->text().toInt();
}


void GPUSettings::on_lineCore_textEdited(const QString &arg1)
{
    if (ui->lineCore->text().length())
        while (ui->lineCore->text().front() == '0' && ui->lineCore->text().length() > 1)
            ui->lineCore->setText(QString("%1").arg(ui->lineCore->text().remove(0, 1)));
    else
        ui->lineCore->setText("0");


    _gpuData.setCore = ui->lineCore->text().toInt();
}

