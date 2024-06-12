#pragma once

#include "commands/commands.h"
#include "pproto/func_invoker.h"
#include "pproto/transport/tcp.h"
#include "pproto/commands/base.h"
#include "pproto/commands/pool.h"

#include "message_box.h"

#include <QDialog>
#include <QPainterPath>
#include <QKeyEvent>
#include <QGraphicsDropShadowEffect>

using namespace pproto;
using namespace pproto::transport;

namespace Ui {
class RegistrationWindow;
}

class RegistrationWindow : public QDialog
{
    Q_OBJECT

signals:
    void signalCancel(bool);
    void signalShowLoginForm();

public:
    explicit RegistrationWindow(QWidget *parent = nullptr);
    ~RegistrationWindow();

    bool init(tcp::Socket::Ptr socket);
    bool initChangePassword(tcp::Socket::Ptr socket, const QString& login, const QUuidEx& userID);

private:
    bool changeUserPasswordMode = {false};
    QUuidEx _userID;

    Ui::RegistrationWindow *ui;
    tcp::Socket::Ptr _socket;
    FunctionInvoker  _funcInvoker;

    QString _hoverStyle;
    QString _btnStyle;
    QString _lineStyle;

    void message(const pproto::Message::Ptr&);
    void command_AddUser(const Message::Ptr&);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject *, QEvent *) override;
private slots:
    void on_btnCancel_clicked(bool checked);
    void on_btnRegistration_clicked(bool checked);
};
