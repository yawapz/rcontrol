#pragma once

#include "message_box.h"

#include "commands/commands.h"
#include "pproto/func_invoker.h"
#include "pproto/transport/tcp.h"
#include "pproto/commands/base.h"
#include "pproto/commands/pool.h"
#include "pproto/logger_operators.h"
#include "shared/defmac.h"
#include "shared/logger/logger.h"
#include "shared/logger/format.h"
#include "shared/config/appl_conf.h"
#include "shared/qt/logger_operators.h"

#include <QDialog>
#include <QCloseEvent>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QInputDialog>

using namespace pproto;
using namespace pproto::transport;

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QDialog
{
signals:
    void signalCancel(bool);
    void signalShowRegistrationWindow();

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

    bool init(tcp::Socket::Ptr socket);

public slots:

private slots:
    void on_btnClose_clicked(bool checked);
    void on_btnLogin_clicked(bool checked);
    void on_btnRegistration_clicked(bool checked);

private:
    Q_OBJECT

    Ui::LoginWindow *ui;
    tcp::Socket::Ptr _socket;

    QString _hoverStyle;
    QString _btnStyle;
    QString _lineStyle;

protected:
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject *, QEvent *) override;
};
