#include "registration_window.h"
#include "ui_registration_window.h"

RegistrationWindow::RegistrationWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegistrationWindow)
{
    ui->setupUi(this);

    setWindowModality(Qt::WindowModal);
    setWindowFlag(Qt::WindowTitleHint);
    setWindowFlag(Qt::FramelessWindowHint);

    QPainterPath painPath;
    painPath.addRoundedRect(rect(), 15, 15, Qt::AbsoluteSize);
    this->setMask(painPath.toFillPolygon().toPolygon());

    setStyleSheet("RegistrationWindow {  "
                  "border-style: outset; "
                  "border-width: 4px;    "
                  "border-radius: 15px;  "
                  "border-color: black;  "
                  "}                     "
    );

    _btnStyle = " border-style: outset;          "
                " border-width: 1px;             "
                " border-radius: 3px;            "
                " border-color: black;           "
                " font-size: 18px;               "
                " background-color: transparent; ";

    _hoverStyle = " border-style: outset;                      "
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
                 " background-color: transparent;       "
                 " }                                    "
                 " QLineEdit:focus {                    "
                 " border-style: outset;                "
                 " border-width: 2px;                   "
                 " border-radius: 3px;                  "
                 " border-color: rgba(35, 146, 220, 1); "
                 " font-size: 18px;                     "
                 " background-color: transparent;}      ";

    ui->lineLogin->setStyleSheet(_lineStyle);
    ui->linePassword->setStyleSheet(_lineStyle);
    ui->linePassword2->setStyleSheet(_lineStyle);

    ui->btnRegistration->setStyleSheet(_btnStyle);
    ui->btnCancel->setStyleSheet(_btnStyle);

    ui->btnRegistration->installEventFilter(this);
    ui->btnCancel->installEventFilter(this);

    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(3); // Устанавливаем радиус размытия
    shadowEffect->setOffset(0.5); // Устанавливаем смещение тени
    ui->Widget->setGraphicsEffect(shadowEffect); // Устанавливаем эффект тени на окно

#define FUNC_REGISTRATION(COMMAND) \
    _funcInvoker.registration(command:: COMMAND, &RegistrationWindow::command_##COMMAND, this);

    FUNC_REGISTRATION(AddUser)


#undef FUNC_REGISTRATION
}

RegistrationWindow::~RegistrationWindow()
{
    delete ui;
}

bool RegistrationWindow::init(tcp::Socket::Ptr socket)
{
    _socket = socket;

    chk_connect_q(_socket.get(), &tcp::Socket::disconnected,
                  this, &RegistrationWindow::close);
    chk_connect_q(_socket.get(), &tcp::Socket::message,
                  this, &RegistrationWindow::message)

    setTabOrder(ui->lineLogin, ui->linePassword);
    setTabOrder(ui->linePassword, ui->linePassword2);
    setTabOrder(ui->linePassword2, ui->btnRegistration);
    setTabOrder(ui->btnRegistration, ui->btnCancel);

    setFocus();

    return true;
}

bool RegistrationWindow::initChangePassword(tcp::Socket::Ptr socket,
                                            const QString &login,
                                            const QUuidEx& userID)
{
    _userID = userID;
    _socket = socket;

    chk_connect_q(_socket.get(), &tcp::Socket::disconnected,
                  this, &RegistrationWindow::close);
    chk_connect_q(_socket.get(), &tcp::Socket::message,
                  this, &RegistrationWindow::message)

    setTabOrder(ui->lineLogin, ui->linePassword);
    setTabOrder(ui->linePassword, ui->linePassword2);
    setTabOrder(ui->linePassword2, ui->btnRegistration);
    setTabOrder(ui->btnRegistration, ui->btnCancel);

    setFocus();

    changeUserPasswordMode = true;
    ui->lineLogin->setText(login);
    ui->lineLogin->setEnabled(false);
    ui->label->setText("Смена пароля");
    ui->btnRegistration->setText("Обновить");

    return true;
}

void RegistrationWindow::message(const Message::Ptr& message)
{
    if (message->processed())
        return;

    if (lst::FindResult fr = _funcInvoker.findCommand(message->command()))
    {
        if (command::pool().commandIsSinglproc(message->command()))
            message->markAsProcessed();
        _funcInvoker.call(message, fr);
    }
}

void RegistrationWindow::command_AddUser(const Message::Ptr& message)
{
    if (message->type() == Message::Type::Answer)
    {
        setCursor(Qt::ArrowCursor);

        data::AddUser addUser;
        readFromMessage(message, addUser);

        if (addUser.resault)
        {
            QString msg = QString(u8"Пользователь %1 зарегестрирован!").arg(addUser.login);
            messageBox(this, QMessageBox::Icon::Information, msg);
            emit signalShowLoginForm();
            close();
        }
        else
        {
            QString msg = QString(u8"Не смогли зарегистрировать %1.\n"
                                     "Пожалуйста, попробуйте ещё раз!").arg(addUser.login);
            messageBox(this, QMessageBox::Icon::Warning, msg);
        }

        setEnabled(true);
    }
}

void RegistrationWindow::keyPressEvent(QKeyEvent *event)
{
    if(QString::number(event->key()) == "16777220" || QString::number(event->key()) == "16777221")
        ui->btnRegistration->click();
    else if (QString::number(event->key()) == "16777216")
        ui->btnCancel->click();
}

bool RegistrationWindow::eventFilter(QObject *obj, QEvent *event)
{
    for (int i = 0; i < ui->gridLayout->count(); ++i)
    {
        if (obj == ui->btnRegistration && isEnabled())
        {
            QEvent::Type type = event->type();
            if  (type == QEvent::HoverLeave)
                ui->btnRegistration->setStyleSheet(_btnStyle);
            else if (type == QEvent::HoverEnter)
                ui->btnRegistration->setStyleSheet(_hoverStyle);
        }
        if (obj == ui->btnCancel && isEnabled())
        {
            QEvent::Type type = event->type();
            if  (type == QEvent::HoverLeave)
                ui->btnCancel->setStyleSheet(_btnStyle);
            else if (type == QEvent::HoverEnter)
                ui->btnCancel->setStyleSheet(_hoverStyle);
        }
    }
    return QWidget::eventFilter(obj, event);
}

void RegistrationWindow::on_btnCancel_clicked(bool checked)
{
    if (!changeUserPasswordMode)
        emit signalCancel(checked);
    else
        close();
}


void RegistrationWindow::on_btnRegistration_clicked(bool checked)
{
    setEnabled(false);
    setCursor(Qt::WaitCursor);

    QString msg;

    if (!changeUserPasswordMode)
        if (ui->lineLogin->text().length() < 8)
        {
            setCursor(Qt::ArrowCursor);
            msg = u8"Логин не соответствует требованиям!";
            messageBox(this, QMessageBox::Icon::Warning, msg);
            setEnabled(true);
            return;
        }

    if (ui->linePassword->text() != ui->linePassword2->text())
    {
        setCursor(Qt::ArrowCursor);
        msg = u8"Пароли не совпадают!";
        messageBox(this, QMessageBox::Icon::Warning, msg);
        setEnabled(true);
        return;
    }

    QRegExp re("^(?=.*\\d)(?=.*[a-z])(?=.*[A-Z])(?=.*\\W).{8,}$");
    if (re.indexIn(ui->linePassword2->text()))
    {
        setCursor(Qt::ArrowCursor);
        msg = u8"Пароль не соответствует требованиям!";
        messageBox(this, QMessageBox::Icon::Warning, msg);
        setEnabled(true);
        return;
    }

    if (!changeUserPasswordMode)
        _socket->send(createMessage(data::AddUser(ui->lineLogin->text(),
                                                  ui->linePassword2->text())));
    else
    {
        _socket->send(createMessage(data::ChangeUserPassword(
                                        _userID,
                                        ui->lineLogin->text(),
                                        ui->linePassword2->text())));
        close();
    }
}

