#include "settings_window.h"
#include "ui_settings_window.h"

using namespace pproto;
using namespace pproto::transport;

SettingsWindow::SettingsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);
    setWindowModality(Qt::WindowModal);
    setWindowFlag(Qt::WindowTitleHint);
    setWindowFlag(Qt::FramelessWindowHint);

    #define FUNC_REGISTRATION(COMMAND) \
        _funcInvoker.registration(command:: COMMAND, &SettingsWindow::command_##COMMAND, this);

    FUNC_REGISTRATION(DeleteUser)
    FUNC_REGISTRATION(ChangeUserPassword)

    #undef FUNC_REGISTRATION

    QPainterPath painPath;
    painPath.addRoundedRect(rect(), 15, 15, Qt::AbsoluteSize);
    this->setMask(painPath.toFillPolygon().toPolygon());

    setStyleSheet(
    " SettingsWindow        "
    " {                     "
    " border-style: double; "
    " border-width: 6px;    "
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
    ui->lineHost->setStyleSheet(_lineStyle);
    ui->linePort->setStyleSheet(_lineStyle);

    ui->btnChangePassword->setStyleSheet(_btnStyle);
    ui->btnDeleteUser->setStyleSheet(_btnStyle);
    ui->btnCancel->setStyleSheet(_btnStyle);
    ui->btnOk->setStyleSheet(_btnStyle);

    ui->btnChangePassword->installEventFilter(this);
    ui->btnDeleteUser->installEventFilter(this);
    ui->btnCancel->installEventFilter(this);
    ui->btnOk->installEventFilter(this);

    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(9); // Устанавливаем радиус размытия
    shadowEffect->setOffset(1); // Устанавливаем смещение тени
    ui->mainWidget->setGraphicsEffect(shadowEffect); // Устанавливаем эффект тени на окно

    QGraphicsDropShadowEffect *shadowEffectTabServer = new QGraphicsDropShadowEffect(this);
    shadowEffectTabServer->setBlurRadius(9); // Устанавливаем радиус размытия
    shadowEffectTabServer->setOffset(3); // Устанавливаем смещение тени
    ui->tabServer->setGraphicsEffect(shadowEffectTabServer); // Устанавливаем эффект тени на окно

    QGraphicsDropShadowEffect *shadowEffectTabAuth = new QGraphicsDropShadowEffect(this);
    shadowEffectTabAuth->setBlurRadius(9); // Устанавливаем радиус размытия
    shadowEffectTabAuth->setOffset(3); // Устанавливаем смещение тени
    ui->tabAuth->setGraphicsEffect(shadowEffectTabAuth); // Устанавливаем эффект тени на окно
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

bool SettingsWindow::init(tcp::Socket::Ptr socket, Settings& settings, QUuidEx userID)
{
    _socket = socket;
    _settings = &settings;
    _userID = userID;

    chk_connect_q(_socket.get(), &tcp::Socket::message,
                  this, &SettingsWindow::message)

    ui->lineHost->setText(_settings->host);
    ui->linePort->setText(QString::number(_settings->port));
    ui->lineLogin->setText(_settings->login);
    ui->linePassword->setText(_settings->password);

    chk_connect_q(_socket.get(), &tcp::Socket::disconnected, this, &SettingsWindow::close);

    if (_socket->isConnected() && _userID != QUuidEx())
    {
        ui->btnChangePassword->setEnabled(true);
        ui->btnDeleteUser->setEnabled(true);
    }

    return true;
}

void SettingsWindow::deinit()
{
    saveGeometry();
}

void SettingsWindow::message(const pproto::Message::Ptr& message)
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

void SettingsWindow::loadGeometry()
{
    QVector<int> v {10, 10, 450, 300};
    config::base().getValue("windows.settings_window.geometry", v);
    setGeometry(v[0], v[1], v[2], v[3]);
}

void SettingsWindow::saveGeometry()
{
    QRect g = geometry();
    QVector<int> v {g.x(), g.y(), g.width(), g.height()};
    config::base().setValue("windows.settings_window.geometry", v);
}

void SettingsWindow::keyPressEvent(QKeyEvent *event)
{
    if(QString::number(event->key()) == "16777220" || QString::number(event->key()) == "16777221")
        ui->btnOk->click();
    else if (QString::number(event->key()) == "16777216")
        ui->btnCancel->click();
}

bool SettingsWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->btnChangePassword && ui->btnChangePassword->isEnabled())
    {
        QEvent::Type type = event->type();
        if  (type == QEvent::HoverLeave)
            ui->btnChangePassword->setStyleSheet(_btnStyle);
        else if (type == QEvent::HoverEnter)
            ui->btnChangePassword->setStyleSheet(_hoverStyle);
    }
    if (obj == ui->btnDeleteUser && ui->btnDeleteUser->isEnabled())
    {
        QEvent::Type type = event->type();
        if  (type == QEvent::HoverLeave)
            ui->btnDeleteUser->setStyleSheet(_btnStyle);
        else if (type == QEvent::HoverEnter)
            ui->btnDeleteUser->setStyleSheet(_hoverStyle);
    }
    if (obj == ui->btnCancel && ui->btnCancel->isEnabled())
    {
        QEvent::Type type = event->type();
        if  (type == QEvent::HoverLeave)
            ui->btnCancel->setStyleSheet(_btnStyle);
        else if (type == QEvent::HoverEnter)
            ui->btnCancel->setStyleSheet(_hoverStyle);
    }
    if (obj == ui->btnOk && ui->btnOk->isEnabled())
    {
        QEvent::Type type = event->type();
        if  (type == QEvent::HoverLeave)
            ui->btnOk->setStyleSheet(_btnStyle);
        else if (type == QEvent::HoverEnter)
            ui->btnOk->setStyleSheet(_hoverStyle);
    }

return QWidget::eventFilter(obj, event);
}

void SettingsWindow::command_DeleteUser(const Message::Ptr& message)
{
    if (message->type() == Message::Type::Answer)
    {
        setEnabled(true);
        setCursor(Qt::ArrowCursor);

        data::DeleteUser deleteUser;
        readFromMessage(message, deleteUser);
        QString msg;

        if (deleteUser.resault)
        {
            config::base().setValue("server.address", ui->lineHost->text());
            config::base().setValue("server.port", ui->linePort->text().toInt());
            config::base().setValue("user.login", QString(""));
            config::base().setValue("user.password", QString(""));

            config::base().saveFile();

            msg = QString(u8"Пользователь %1 удален!").arg(deleteUser.login);
            messageBox(this, QMessageBox::Icon::Warning, msg);

            emit paramsChanged();
        }
        else
        {
            msg = QString(u8"Не смогли удалить %1.\n"
                            "Пожалуйста, попробуйте ещё раз!").arg(deleteUser.login);
            messageBox(this, QMessageBox::Icon::Warning, msg);
        }
    }
}

void SettingsWindow::command_ChangeUserPassword(const Message::Ptr& message)
{
    if (message->type() == Message::Type::Answer)
    {
        data::ChangeUserPassword changeUserPassword;
        readFromMessage(message, changeUserPassword);
        QString msg;

        if (changeUserPassword.resault)
        {
            config::base().setValue("server.address", ui->lineHost->text());
            config::base().setValue("server.port", ui->linePort->text().toInt());
            config::base().setValue("user.login", changeUserPassword.login);
            config::base().setValue("user.password", changeUserPassword.password);

            config::base().saveFile();

            msg = u8"Пароль изменен!";
            messageBox(this, QMessageBox::Icon::Warning, msg);

            emit paramsChanged();
        }
        else
        {
            msg = u8"Не смогли изменить пароль!\n"
                    "Пожалуйста, попробуйте ещё раз!";
            messageBox(this, QMessageBox::Icon::Warning, msg);

            ui->btnChangePassword->click();
        }
    }
}

void SettingsWindow::on_btnChangePassword_clicked(bool checked)
{
    RegistrationWindow rw(this);
    rw.initChangePassword(_socket, _settings->login, _userID);
    rw.exec();
}


void SettingsWindow::on_btnDeleteUser_clicked(bool checked)
{
    QString msg = QString(u8"Вы действительно хотите удалить %1?").arg(_settings->login);

    QMessageBox* box = new QMessageBox(this);
    box->setText(msg);
    box->setIcon(QMessageBox::Icon::Warning);

    QPushButton* yesBtn = new QPushButton(u8"Да");
    chk_connect_q(yesBtn, &QPushButton::clicked, this, &SettingsWindow::sendCMDDeleteUser);
    QPushButton* noBtn = new QPushButton(u8"Нет");
    chk_connect_q(noBtn, &QPushButton::clicked, box, &QMessageBox::close);

    box->addButton(yesBtn, QMessageBox::ButtonRole::AcceptRole);
    box->addButton(noBtn, QMessageBox::ButtonRole::NoRole);
    box->show();
}

void SettingsWindow::sendCMDDeleteUser(bool)
{
    _socket->send(createMessage(data::DeleteUser(_userID,
                                                 _settings->login,
                                                 _settings->password)));
    setEnabled(false);
    setCursor(Qt::WaitCursor);
}


void SettingsWindow::on_btnOk_clicked(bool checked)
{
    config::base().setValue("server.address", ui->lineHost->text());
    config::base().setValue("server.port", ui->linePort->text().toInt());
    config::base().setValue("user.login", ui->lineLogin->text());
    config::base().setValue("user.password", ui->linePassword->text());

    config::base().saveFile();

    emit paramsChanged();
    close();
}


void SettingsWindow::on_btnCancel_clicked(bool checked)
{
    close();
}

