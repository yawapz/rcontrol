#include "edit_window.h"
#include "ui_edit_window.h"

EditWindow::EditWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditWindow)
{
    ui->setupUi(this);

    setWindowModality(Qt::WindowModal);
    setWindowFlag(Qt::WindowTitleHint);
    setWindowFlag(Qt::FramelessWindowHint);

    QPainterPath painPath;
    painPath.addRoundedRect(rect(), 15, 15, Qt::AbsoluteSize);
    this->setMask(painPath.toFillPolygon().toPolygon());

    setStyleSheet(
    " EditWindow {          "
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

    _lineStyle = " QLineEdit {                                 "
                 " padding: 2px;                               "
                 " border-style: hidden;                       "
                 " border-width: 1px;                          "
                 " border-radius: 3px;                         "
                 " border-color: black;                        "
                 " font-size: 18px;                            "
                 " background-color: transparent;}             "
                 " QLineEdit:focus {                           "
                 " padding: 2px;                               "
                 " border-bottom-style: dashed;                "
                 " border-bottom-width: 2px;                   "
                 " border-radius: 3px;                         "
                 " border-bottom-color: rgba(35, 146, 220, 1); "
                 " font-size: 18px;                            "
                 " background-color: transparent;}             ";

    ui->lineWorkerName->setStyleSheet(_lineStyle);
    ui->linePowerPrice->setStyleSheet(_lineStyle);
    ui->btnUpdate->setStyleSheet(_btnStyle);
    ui->btnClose->setStyleSheet(_btnStyle);

    ui->btnUpdate->installEventFilter(this);
    ui->btnClose->installEventFilter(this);
    ui->lblCOPYIDICON->installEventFilter(this);

    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(3); // Устанавливаем радиус размытия
    shadowEffect->setOffset(0.5); // Устанавливаем смещение тени
    ui->Widget->setGraphicsEffect(shadowEffect); // Устанавливаем эффект тени на окно
}

EditWindow::~EditWindow()
{
    delete ui;
}

bool EditWindow::init(tcp::Socket::Ptr socket, const data::workerData& workerData, const QStringList& workersNames)
{
    _socket = socket;
    _id = workerData.id;
    _workerNames = workersNames;

    ui->lblMain->setText(workerData.workerName.isEmpty() ? digest(workerData.id)
                                                         : workerData.workerName);
    ui->lblID->setText(toString(workerData.id));
    if (!workerData.workerName.isEmpty())
        ui->lineWorkerName->setText(workerData.workerName);
    if (workerData.electricityCost)
        ui->linePowerPrice->setText(QString::number(workerData.electricityCost, 'f', 2));

    chk_connect_q(_socket.get(), &tcp::Socket::disconnected, this, &EditWindow::close);
    chk_connect_q(ui->btnClose, &QPushButton::clicked, this, &EditWindow::close);

    setTabOrder(ui->lineWorkerName, ui->linePowerPrice);
    setTabOrder(ui->linePowerPrice, ui->btnClose);
    setTabOrder(ui->btnClose, ui->btnUpdate);

    setFocus();

    QPixmap iconCopyID(":/worker/resources/worker/copy.svg");
    QPainter qpCopy = QPainter(&iconCopyID);
    qpCopy.setCompositionMode(QPainter::CompositionMode_SourceIn);
    qpCopy.fillRect(iconCopyID.rect(), Qt::black);
    qpCopy.end();

    QString copyStyle = "background-color: transparent; "
                        "color: orange;                 ";
    ui->lblCOPYIDICON->setPixmap(iconCopyID);
    ui->lblCOPYIDICON->setFixedSize(12,12);
    ui->lblCOPYIDICON->setContentsMargins(0,0,0,0);
    ui->lblCOPYIDICON->setScaledContents(true);
    ui->lblCOPYIDICON->installEventFilter(this);
    ui->lblCOPYIDICON->setToolTip("Копировать ID");
    ui->lblCOPYIDICON->setStyleSheet(copyStyle);

    QLocale locale(QLocale::English);
    QDoubleValidator* validator = new QDoubleValidator(0.0, 5, 2, this);
    validator->setLocale(locale);

    ui->linePowerPrice->setValidator(validator);

    return true;
}

void EditWindow::on_btnUpdate_clicked(bool checked)
{
    // Проверка на первую и последнюю точку
    if (ui->linePowerPrice->text().length())
    {
        if (ui->linePowerPrice->text().front() == '.')
            ui->linePowerPrice->setText(QString("0%1").arg(ui->linePowerPrice->text()));

        if (ui->linePowerPrice->text().back() == '.')
            ui->linePowerPrice->setText(QString("%10").arg(ui->linePowerPrice->text()));
    }
    else
        ui->linePowerPrice->setText("0");

    if (ui->lineWorkerName->text().length())
    {
        while (ui->lineWorkerName->text().front() == ' ')
            ui->lineWorkerName->setText(QString("%1").arg(
                                    ui->lineWorkerName->text().remove(0,1)));

        while (ui->lineWorkerName->text().back() == ' ')
            ui->lineWorkerName->setText(QString("%1").arg(
                                    ui->lineWorkerName->text().remove(
                                        ui->lineWorkerName->text().length() - 1,1)));
    }

    // Проверка на занятость имени из списка
    for (const QString& name : _workerNames)
    {
        if (name == ui->lineWorkerName->text() && ui->lblMain->text()
                                               != ui->lineWorkerName->text())
        {
            const QString msg = "Воркер с таким названием уже существует!";
            messageBox(this, QMessageBox::Icon::Warning, msg);
            return;
        }
    }

    // Отправка команды
    _socket->send(createMessage(data::UpdateWorkerInfoClient(_id,
                                        ui->lineWorkerName->text(),
                                        ui->lblMain->text(),
                                        ui->linePowerPrice->text().toDouble())));
    close();
}

void EditWindow::keyPressEvent(QKeyEvent *event)
{
    if(QString::number(event->key()) == "16777220" || QString::number(event->key()) == "16777221")
        ui->btnUpdate->click();
    else if (QString::number(event->key()) == "16777216")
        ui->btnClose->click();
}

bool EditWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->btnUpdate)
    {
        QEvent::Type type = event->type();
        if  (type == QEvent::HoverLeave)
            ui->btnUpdate->setStyleSheet(_btnStyle);
        else if (type == QEvent::HoverEnter)
            ui->btnUpdate->setStyleSheet(_hoverStyle);
    }
    if (obj == ui->btnClose)
    {
        QEvent::Type type = event->type();
        if  (type == QEvent::HoverLeave)
            ui->btnClose->setStyleSheet(_btnStyle);
        else if (type == QEvent::HoverEnter)
            ui->btnClose->setStyleSheet(_hoverStyle);
    }
    if (obj == ui->lblCOPYIDICON)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(toString(_id));
        }
    }

    return QWidget::eventFilter(obj, event);
}
