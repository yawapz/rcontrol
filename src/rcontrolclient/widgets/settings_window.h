#pragma once

#include "message_box.h"
#include "registration_window.h"

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
#include <QInputDialog>

using namespace pproto;
using namespace pproto::transport;

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QDialog
{
signals:
    void paramsChanged();

public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

    bool init(tcp::Socket::Ptr socket, Settings& settings, QUuidEx userID);
    void deinit();

public slots:

private slots:
    void message(const pproto::Message::Ptr&);
    void on_btnChangePassword_clicked(bool checked);
    void on_btnDeleteUser_clicked(bool checked);
    void sendCMDDeleteUser(bool);
    void on_btnOk_clicked(bool checked);
    void on_btnCancel_clicked(bool checked);

private:
    Q_OBJECT

    void command_DeleteUser(const Message::Ptr&);
    void command_ChangeUserPassword(const Message::Ptr&);

    void loadGeometry();
    void saveGeometry();

private:
    Ui::SettingsWindow *ui;

    tcp::Socket::Ptr _socket;
    Settings*  _settings;
    FunctionInvoker  _funcInvoker;
    QUuidEx _userID;

    QString _hoverStyle;
    QString _btnStyle;
    QString _lineStyle;

protected:
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject *, QEvent *) override;
};
