#include "login_window.h"
#include "ui_login_window.h"

using namespace pproto;
using namespace pproto::transport;

LoginWindow::LoginWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
    setWindowFlag(Qt::WindowTitleHint);
    setWindowFlag(Qt::FramelessWindowHint);

    QPainterPath painPath;
    painPath.addRoundedRect(rect(), 15, 15, Qt::AbsoluteSize);
    this->setMask(painPath.toFillPolygon().toPolygon());

    setStyleSheet(
    " LoginWindow {         "
    " border-style: outset; "
    " border-width: 4px;    "
    " border-radius: 15px;  "
    " border-color: black;  "
    " }                     "
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
                 " background-color: transparent;}      "
                 " QLineEdit:focus {                    "
                 " border-style: outset;                "
                 " border-width: 2px;                   "
                 " border-radius: 3px;                  "
                 " border-color: rgba(35, 146, 220, 1); "
                 " font-size: 18px;                     "
                 " background-color: transparent;}      ";


    ui->lineLogin->setStyleSheet(_lineStyle);
    ui->linePassword->setStyleSheet(_lineStyle);

    ui->btnLogin->setStyleSheet(_btnStyle);
    ui->btnRegistration->setStyleSheet(_btnStyle);
    ui->btnClose->setStyleSheet(_btnStyle);

    ui->btnLogin->installEventFilter(this);
    ui->btnRegistration->installEventFilter(this);
    ui->btnClose->installEventFilter(this);

    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(3); // Устанавливаем радиус размытия
    shadowEffect->setOffset(0.5); // Устанавливаем смещение тени
    ui->Widget->setGraphicsEffect(shadowEffect); // Устанавливаем эффект тени на окно
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

bool LoginWindow::init(tcp::Socket::Ptr socket)
{
    _socket = socket;

    chk_connect_q(_socket.get(), &tcp::Socket::disconnected, this, &LoginWindow::close);

    QString login = "";
    QString password = "";

    config::base().getValue("user.login", login);
    config::base().getValue("user.password", password);

    ui->lineLogin->setText(login);
    ui->linePassword->setText(password);

    setTabOrder(ui->lineLogin, ui->linePassword);
    setTabOrder(ui->linePassword, ui->btnLogin);
    setTabOrder(ui->btnLogin, ui->btnRegistration);
    setTabOrder(ui->btnRegistration, ui->btnClose);

    setFocus();

    return true;
}

void LoginWindow::on_btnClose_clicked(bool checked)
{
    emit signalCancel(checked);
}


void LoginWindow::on_btnLogin_clicked(bool checked)
{
    if (!ui->lineLogin->text().isEmpty() && !ui->linePassword->text().isEmpty())
    {
        _socket->send(createMessage(data::UserLogin(ui->lineLogin->text(),
                                                    ui->linePassword->text())));
        config::base().setValue("user.login", ui->lineLogin->text());
        config::base().setValue("user.password", ui->linePassword->text());

        close();
    }
    else
    {
        QString msg = u8"Поля ввода логина и пароля должны быть заполнены";
        messageBox(this, QMessageBox::Icon::Warning, msg);
        return;
    }
}


void LoginWindow::on_btnRegistration_clicked(bool checked)
{
    emit signalShowRegistrationWindow();
    close();
}

void LoginWindow::keyPressEvent(QKeyEvent *event)
{
    if(QString::number(event->key()) == "16777220" || QString::number(event->key()) == "16777221")
        ui->btnLogin->click();
    else if (QString::number(event->key()) == "16777216")
        ui->btnClose->click();
}

bool LoginWindow::eventFilter(QObject *obj, QEvent *event)
{
    for (int i = 0; i < ui->gridLayout->count(); ++i)
    {
        if (obj == ui->btnLogin)
        {
            QEvent::Type type = event->type();
            if  (type == QEvent::HoverLeave)
                ui->btnLogin->setStyleSheet(_btnStyle);
            else if (type == QEvent::HoverEnter)
                ui->btnLogin->setStyleSheet(_hoverStyle);
        }
        if (obj == ui->btnClose)
        {
            QEvent::Type type = event->type();
            if  (type == QEvent::HoverLeave)
                ui->btnClose->setStyleSheet(_btnStyle);
            else if (type == QEvent::HoverEnter)
                ui->btnClose->setStyleSheet(_hoverStyle);
        }
        if (obj == ui->btnRegistration)
        {
            QEvent::Type type = event->type();
            if  (type == QEvent::HoverLeave)
                ui->btnRegistration->setStyleSheet(_btnStyle);
            else if (type == QEvent::HoverEnter)
                ui->btnRegistration->setStyleSheet(_hoverStyle);
        }
    }
    return QWidget::eventFilter(obj, event);
}

