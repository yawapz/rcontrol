#pragma once

#include "qlistwidget.h"
#include "qtablewidget.h"
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
#include <cstddef>
#include <winsock2.h>
#endif

#include "worker_item.h"
#include "worker_widget.h"
#include "login_window.h"
#include "registration_window.h"
#include "settings_window.h"
#include "message_box.h"

#include "shared/simple_ptr.h"
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
#include "commands/commands.h"
#include "commands/error.h"

#include <QMainWindow>
#include <QCloseEvent>
#include <QApplication>
#include <QHostInfo>
#include <QScreen>
#include <QLabel>
#include <QFileInfo>
#include <QInputDialog>
#include <QGraphicsItem>
#include <unistd.h>
#include <QFileDialog>
#include <QListView>

using namespace std;
using namespace pproto;
using namespace pproto::transport;

namespace Ui {
class MainWindow;
}

class QLabel;

class MainWindow : public QMainWindow
{
signals:
    void normalizeSize(const widgetListSizeNormalize&);
    void newWorkerName(const QStringList&);

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool init();
    void deinit();

private slots:
    void message(const pproto::Message::Ptr&);
    void socketConnected(pproto::SocketDescriptor);
    void socketDisconnected(pproto::SocketDescriptor);

    void showWorkerById(const QUuidEx&);

private slots:
    void on_btnConnect_clicked(bool);
    void on_btnSettings_clicked(bool);
    void on_btnExit_clicked(bool);

    void reconnectCheck(); // Проверка необходимости переподключения
    void paramsChanged();

    void on_tabWidget_tabBarDoubleClicked(int index);
    void on_btnRefresh_clicked(bool checked);
    void on_btnDelWorker_clicked(bool checked);
    void on_btnAddWorker_clicked(bool checked);

    void showRegistrationWindow();
    void showLoginForm();

    void deleteWorker();
    void updateTabName(const QString& oldName, const QString& newName, const QUuidEx& id);

    void on_tabWidget_currentChanged(int index);

private:
    Q_OBJECT
    void closeEvent(QCloseEvent*) override;
    void showEvent(QShowEvent*) override;
    void timerEvent(QTimerEvent* event) override;

    void loadGeometry();
    void saveGeometry();

    void loadSettings();

    void enableButtons(bool);
    void connectActiveMessage();

    void command_Settings(const Message::Ptr&);
    void command_UserLogin(const Message::Ptr&);
    void command_UserWorkerList(const Message::Ptr&);
    void command_AddWorker(const Message::Ptr&);
    void command_DeleteWorker(const Message::Ptr&);
    void command_UpdateWorkerInfoClient(const Message::Ptr&);

private:
    qint16 _autoRefId = {-1}; // Таймер автообновления
    QUuidEx _delWorkerBuffId; // Буффер id для удаления
    Ui::MainWindow *ui;
    static QUuidEx _applId;

    tcp::Socket::Ptr _socket;
    Settings   _settings;
    FunctionInvoker  _funcInvoker;

    QString _windowTitle;
    QLabel* _labelConnectStatus;
    QLabel* _labelAuthStatus;

    QTimer _reconnectTimer;

    QUuidEx _userID;
    QList<QUuidEx> _workerIdList;
    QList<data::workerData> _workers;
    bool _delWorkerMode = {false};

    widgetListSizeNormalize _normalizeItemSizeTable;
    void checkNormalize(const widgetListSizeNormalize&);

protected:
    bool eventFilter(QObject *, QEvent *) override;
};
