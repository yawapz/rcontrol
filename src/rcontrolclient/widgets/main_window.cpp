#include "main_window.h"
#include "ui_main_window.h"

#define log_error_m   alog::logger().error   (alog_line_location, "MainWin")
#define log_warn_m    alog::logger().warn    (alog_line_location, "MainWin")
#define log_info_m    alog::logger().info    (alog_line_location, "MainWin")
#define log_verbose_m alog::logger().verbose (alog_line_location, "MainWin")
#define log_debug_m   alog::logger().debug   (alog_line_location, "MainWin")
#define log_debug2_m  alog::logger().debug2  (alog_line_location, "MainWin")

#define KILL_TIMER(TIMER) {if (TIMER != -1) {killTimer(TIMER); TIMER = -1;}}

QUuidEx MainWindow::_applId;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _socket(new tcp::Socket)
{
    ui->setupUi(this);

    QList<QScreen*> screens = QGuiApplication::screens();

    enableButtons(false);

    _labelConnectStatus = new QLabel(u8"Нет подключения", this);
    ui->statusBar->addWidget(_labelConnectStatus);
    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::VLine);
    line->setLineWidth(1);
    ui->statusBar->addWidget(line);

    _labelAuthStatus = new QLabel(u8"Не авторизован", this);
    ui->statusBar->addWidget(_labelAuthStatus);

    QString vers = u8"Версия: %1 (gitrev: %2)";
    vers = vers.arg(VERSION_PROJECT).arg(GIT_REVISION);
    ui->statusBar->addPermanentWidget(new QLabel(vers, this));

    chk_connect_q(_socket.get(), &tcp::Socket::message,
                  this, &MainWindow::message)

    chk_connect_q(_socket.get(), &tcp::Socket::connected,
                  this, &MainWindow::socketConnected)

    chk_connect_q(_socket.get(), &tcp::Socket::disconnected,
                  this, &MainWindow::socketDisconnected)

    chk_connect_q(&_reconnectTimer, &QTimer::timeout,
                  this, &MainWindow::reconnectCheck);

    #define FUNC_REGISTRATION(COMMAND) \
        _funcInvoker.registration(command:: COMMAND, &MainWindow::command_##COMMAND, this);

    FUNC_REGISTRATION(UserLogin)
    FUNC_REGISTRATION(UserWorkerList)
    FUNC_REGISTRATION(AddWorker)
    FUNC_REGISTRATION(DeleteWorker)
    FUNC_REGISTRATION(UpdateWorkerInfoClient)

    #undef FUNC_REGISTRATION

    qRegisterMetaType<QUuidEx>("QUuidEx");
    qRegisterMetaType<widgetListSizeNormalize>("widgetListSizeNormalize");

    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(7); // Устанавливаем радиус размытия
    shadowEffect->setOffset(3); // Устанавливаем смещение тени
    ui->centralwidget->setGraphicsEffect(shadowEffect); // Устанавливаем эффект тени на окно
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::init()
{
    if (config::base().getValue("application.id", _applId, false))
    {
        log_verbose_m << "Read ApplId: " << _applId;
    }
    else
    {
        _applId = QUuidEx::createUuid();
        config::base().setValue("application.id", _applId);
        config::base().saveFile();
        log_verbose_m << "Generated new ApplId: " << _applId;
    }

    loadGeometry();
    loadSettings();

    QPixmap iconPlus(":/worker/resources/worker/plus.svg");
    QPainter qp = QPainter(&iconPlus);
    qp.setCompositionMode(QPainter::CompositionMode_SourceIn);
    qp.fillRect(iconPlus.rect(), Qt::GlobalColor::darkGreen);
    qp.end();

    QPixmap iconMinus(":/worker/resources/worker/minus.svg");
    QPainter qp2 = QPainter(&iconMinus);
    qp2.setCompositionMode(QPainter::CompositionMode_SourceIn);
    qp2.fillRect(iconMinus.rect(), Qt::GlobalColor::darkRed);
    qp2.end();



    QString style = " QPushButton        "
                    " {                  "
                    " margin: 0;         "
                    " padding: 0;        "
                    " }                  "
                    " QToolTip           "
                    " {                  "
                    " background: black; "
                    " color: orange      "
                    " }                  ";
    ui->btnAddWorker->setIcon(iconPlus);
    ui->btnDelWorker->setIcon(iconMinus);
    ui->btnAddWorker->setStyleSheet(style);
    ui->btnDelWorker->setStyleSheet(style);

    ui->lblDelMode->setStyleSheet(" background-color: transparent; "
                                  " font-size: 14px;               "
                                  " color: #FF3733                 ");
    //ui->mainWorkerWidgetList->setStyleSheet("QListWidget:item{border-style: outset; border-width: 1px; border-radius: 3px; border-color: black;}");

    QString toolButtonsStyle = " QPushButton:hover                    "
                               " {                                    "
                               " background-color: transparent;       "
                               " border-style: double;                "
                               " border-color: rgba(35, 146, 220, 1); "
                               " border-width: 3px;                   "
                               " border-radius: 3px;                  "
                               " }                                    "
                               " QToolTip                             "
                               " {                                    "
                               " background: black;                   "
                               " color: orange                        "
                               " }                                    ";

    ui->btnConnect->setStyleSheet(toolButtonsStyle);
    ui->btnRefresh->setStyleSheet(toolButtonsStyle);
    ui->btnSettings->setStyleSheet(toolButtonsStyle);
    ui->btnExit->setStyleSheet(toolButtonsStyle);

    ui->btnConnect->installEventFilter(this);
    ui->btnRefresh->installEventFilter(this);
    ui->btnSettings->installEventFilter(this);
    return true;
}

void MainWindow::deinit()
{
    if (_socket)
        _socket->disconnect();
}

void MainWindow::message(const pproto::Message::Ptr& message)
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

void MainWindow::socketConnected(pproto::SocketDescriptor)
{
    ui->btnConnect->setChecked(true);
    connectActiveMessage();
    enableButtons(true);

    if (!_settings.login.isEmpty() && !_settings.password.isEmpty())
    {
        Message::Ptr m = createMessage(data::UserLogin(_settings.login,
                                                       _settings.password));
        _socket->send(m);
    }
    else
        showLoginForm();
}

void MainWindow::socketDisconnected(pproto::SocketDescriptor)
{
    if (ui->tabWidget->count() > 1)
        for (int i = 1; i < ui->tabWidget->count(); ++i)
            ui->tabWidget->removeTab(i);

    ui->mainWorkerWidgetList->clear();

    ui->btnConnect->setChecked(false);
    enableButtons(false);

    _userID = QUuidEx(); // Нулевой Uuid

    if (_socket->isConnected())
        _labelConnectStatus->setText(u8"Переподключение");
    else
    {
        _labelConnectStatus->setText(u8"Нет подключения");
        _labelAuthStatus->setText(u8"Не авторизован");
    }

    QMetaObject::invokeMethod(this, "reconnectCheck", Qt::QueuedConnection);

    KILL_TIMER(_autoRefId);
}

void MainWindow::showWorkerById(const QUuidEx& id)
{
    for (auto& worker : _workers)
    {
        if (worker.id == id)
        {
            QString workerName = worker.workerName.isEmpty() ? digest(worker.id)
                                                             : worker.workerName;

            if (!_delWorkerMode)
            {
                for (int i = 0; i < ui->tabWidget->count(); ++i)
                    if (ui->tabWidget->tabText(i) == workerName)
                    {
                        ui->tabWidget->setCurrentIndex(i);
                        return;
                    }
                QStringList workersNames;
                for (auto& w : _workers)
                    workersNames.append(w.workerName);

                WorkerWidget* w = new WorkerWidget();
                w->init(_socket, worker, workersNames);
                chk_connect_q(this, &MainWindow::newWorkerName,
                              w, &WorkerWidget::slotUpdateNamesList);

                ui->tabWidget->addTab(w, workerName);

                qint16 lastIndex = ui->tabWidget->count() - 1;
                ui->tabWidget->setCurrentIndex(lastIndex);
                ui->tabWidget->setTabToolTip(lastIndex, u8"Сlose - double click");
                ui->tabWidget->setStyleSheet(" QToolTip                             "
                                             " {                                    "
                                             " background: black;                   "
                                             " color: orange                        "
                                             " }                                    ");
                break;
            }
            else
            {
                _delWorkerBuffId = worker.id;

                QString msg = QString(
                u8"Вы действительно хотите удалить %1?").arg(workerName);

                QMessageBox* box = new QMessageBox(this);
                box->setWindowModality(Qt::WindowModal);
                box->setWindowFlag(Qt::WindowTitleHint);
                box->setWindowFlag(Qt::FramelessWindowHint);
                box->setStyleSheet(
                " QMessageBox           "
                " {                     "
                " border-style: solid;  "
                " border-width: 1px;    "
//                " border-radius: 15px;  "
                " border-color: black;  "
                " }                     "
                );
                QString hoverStyle = " QPushButton:hover                          "
                                     " {                                          "
                                     " padding: 3px;                              "
                                     " border-style: outset;                      "
                                     " border-width: 1px;                         "
                                     " border-radius: 3px;                        "
                                     " border-color: rgba(35, 146, 220, 0.5);     "
                                     " font-size: 16px;                           "
                                     " background-color: rgba(35, 146, 220, 0.5); "
                                     " }                                          ";


                box->setText(msg);
                box->setIcon(QMessageBox::Icon::Warning);

                QPushButton* yesBtn = new QPushButton(u8"Да");
                yesBtn->setStyleSheet(hoverStyle);
                chk_connect_q(yesBtn, &QPushButton::clicked, this, &MainWindow::deleteWorker);
                QPushButton* noBtn = new QPushButton(u8"Нет");
                noBtn->setStyleSheet(hoverStyle);
                chk_connect_q(noBtn, &QPushButton::clicked, box, &QMessageBox::close);

                box->addButton(yesBtn, QMessageBox::ButtonRole::AcceptRole);
                box->addButton(noBtn, QMessageBox::ButtonRole::NoRole);
                box->show();

                ui->lblDelMode->setText("");
                _delWorkerMode = false;
            }
        }
    }
}

void MainWindow::on_btnConnect_clicked(bool checked)
{
    if (!checked)
    {
        _reconnectTimer.stop();
        _socket->disconnect();
        _labelConnectStatus->setText(u8"Нет подключения");

        return;
    }

    qint32 port = 61121;
    if (!config::base().getValue("server.port", port))
        config::base().setValue("server.port", port);

    QString msg, host;
    if (!config::base().getValue("server.address", host))
        config::base().setValue("server.address", QString("127.0.0.1"));

    QHostInfo hostInfo = QHostInfo::fromName(host);
    if (hostInfo.error() != QHostInfo::NoError
        || hostInfo.addresses().isEmpty())
    {
        //log_error_m << "Failed to resolve address " << strAddr;
        msg = u8"Сетевой адрес %1 запрещен";
        ui->btnConnect->setChecked(false);
        messageBox(this, QMessageBox::Icon::Warning, msg.arg(host));
        return;
    }
    QHostAddress addr = hostInfo.addresses()[0];

    if (!_socket->init({addr, port}))
    {
        msg = u8"Не удалось инициализировать сокет";
        ui->btnConnect->setChecked(false);
        messageBox(this, QMessageBox::Icon::Warning, msg);
        return;
    }

    setCursor(Qt::WaitCursor);

    _socket->setEchoTimeout(5);
    _socket->connect();

    qint16 attempts = 0;
    while (attempts++ < 30 /*ждем 6 сек*/)
    {
        qApp->processEvents();
        usleep(200*1000 /*0.2 сек*/);
        if (_socket->isConnected())
            break;
    }
    setCursor(Qt::ArrowCursor);

    if (!_socket->isConnected())
    {
        _socket->stop();
        ui->btnConnect->setChecked(false);
        _labelConnectStatus->setText(u8"Нет подключения");

        msg = u8"Не удалось подключиться к хосту %1:%2";
        msg = msg.arg(addr.toString()).arg(port);
        messageBox(this, QMessageBox::Icon::Warning, msg);
    }
    return;
}

void MainWindow::on_btnSettings_clicked(bool)
{
    SettingsWindow sw {this};

    sw.init(_socket, _settings, _userID);
    chk_connect_q(&sw, &::SettingsWindow::paramsChanged, this,
                  &MainWindow::paramsChanged);
    sw.exec();
    sw.deinit();
}

void MainWindow::on_btnExit_clicked(bool)
{
    close();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // В основном окне приложения метод saveGeometry() нужно вызывать в этой
    // точке, иначе геометрия окна будет сохранятся по разному в разных ОС
    saveGeometry();
}

void MainWindow::showEvent(QShowEvent* event)
{
    event->accept();
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == _autoRefId)
    {
        ui->btnRefresh->click();
        return;
    }
}

void MainWindow::loadGeometry()
{
    QVector<int> v {10, 10, 800, 600};
    config::base().getValue("windows.main_window.geometry", v);
    setGeometry(v[0], v[1], v[2], v[3]);

    int tabIndex = 0;
    config::base().getValue("windows.main_window.tab_index", tabIndex);
    ui->tabWidget->setCurrentIndex(tabIndex);
}

void MainWindow::saveGeometry()
{
    QRect g = geometry();
    QVector<int> v {g.x(), g.y(), g.width(), g.height()};
    config::base().setValue("windows.main_window.geometry", v);

    config::base().setValue("windows.main_window.tab_index", ui->tabWidget->currentIndex());
}

void MainWindow::loadSettings()
{
    config::base().getValue("server.address", _settings.host);
    config::base().getValue("server.port", _settings.port);
    config::base().getValue("user.login", _settings.login);
    config::base().getValue("user.password", _settings.password);
}

void MainWindow::command_Settings(const Message::Ptr& message)
{
}

void MainWindow::command_UserLogin(const Message::Ptr& message)
{
    if (message->type() == Message::Type::Answer)
    {
        data::UserLogin userLogin;
        readFromMessage(message, userLogin);

        if (userLogin.resault)
        {
            _userID = userLogin.id;
            _labelAuthStatus->setText(QString(u8"Авторизован: %1").arg(userLogin.login));

            ui->btnRefresh->setEnabled(true);
            ui->btnAddWorker->setVisible(true);
            ui->btnDelWorker->setVisible(true);

            _settings.login = userLogin.login;
            _settings.password = userLogin.password;

            // Выполняем запрос id всех ригов пользователя
            Message::Ptr m = createMessage(data::UserWorkerList(userLogin.id));
            _socket->send(m);
        }
        else
        {
            QString msg = u8"Ошибка авторизации";
            messageBox(this, QMessageBox::Icon::Warning, msg);

            LoginWindow lw(this);
            lw.init(_socket);
            chk_connect_q(&lw,  &LoginWindow::signalCancel,
                          this, &MainWindow::on_btnConnect_clicked);
            chk_connect_q(&lw,  &LoginWindow::signalShowRegistrationWindow,
                          this, &MainWindow::showRegistrationWindow);

            lw.exec();
            return;
        }
    }
}

void MainWindow::command_UserWorkerList(const Message::Ptr& message)
{
    if (message->type() == Message::Type::Answer)
    {
        data::UserWorkerList workerList;
        readFromMessage(message, workerList);

        _workerIdList = workerList.userIdWorkers;
        _workers = workerList.workersData;

        ui->mainWorkerWidgetList->clear();
        _normalizeItemSizeTable = widgetListSizeNormalize();

        QLabel* lbl = new QLabel();
        QHBoxLayout* hlay = new QHBoxLayout();
        hlay->setContentsMargins(2,0,2,0);
        hlay->setSpacing(0);

        QString style = "QLabel {font-size: 18px; border: none; background-color: transparent};";

        QLabel* lblWorkerName = new QLabel("Name");
        lblWorkerName->setStyleSheet(style);
        lblWorkerName->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lblWorkerName->setMinimumWidth(lblWorkerName->sizeHint().width());

        QLabel* lblWorkerStatus = new QLabel("Status");
        lblWorkerStatus->setStyleSheet(style);
        lblWorkerStatus->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lblWorkerStatus->setMinimumWidth(lblWorkerStatus->sizeHint().width());

        QLabel* lblMinerUpTime = new QLabel("uptime");
        lblMinerUpTime->setStyleSheet(style);
        lblMinerUpTime->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lblWorkerName->setMinimumWidth(lblWorkerName->sizeHint().width());

        QLabel* lblWorkerSpeed = new QLabel("Worker speed");
        lblWorkerSpeed->setStyleSheet(style);
        lblWorkerSpeed->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lblWorkerSpeed->setMinimumWidth(lblWorkerSpeed->sizeHint().width());

        QLabel* lblMinerName = new QLabel("Miner");
        lblMinerName->setStyleSheet(style);
        lblMinerName->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lblMinerName->setMinimumWidth(lblMinerName->sizeHint().width());

        QLabel* lblErr = new QLabel("");
        lblErr->setStyleSheet(style);
        lblErr->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lblErr->setMinimumWidth(lblErr->sizeHint().width());

        QLabel* lblGpuCount = new QLabel("");
        lblGpuCount->setAlignment(Qt::AlignmentFlag::AlignLeft);
        lblGpuCount->setMinimumWidth(lblGpuCount->sizeHint().width());

        QLabel* lblLA = new QLabel("LA");
        lblLA->setStyleSheet(style);
        lblLA->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lblLA->setMinimumWidth(lblLA->sizeHint().width());

        QLabel* lblFAN = new QLabel("FAN");
        lblFAN->setStyleSheet(style);
        lblFAN->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lblFAN->setMinimumWidth(lblFAN->sizeHint().width());

        QLabel* lblWorkerPower = new QLabel("Power");
        lblWorkerPower->setStyleSheet(style);
        lblWorkerPower->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lblWorkerPower->setMinimumWidth(lblWorkerPower->sizeHint().width());

        hlay->addWidget(lblWorkerName);
        hlay->addWidget(lblWorkerStatus);
        hlay->addWidget(lblMinerUpTime);
        hlay->addWidget(lblWorkerSpeed);
        hlay->addWidget(lblMinerName);
        hlay->addWidget(lblErr);
        hlay->addWidget(lblGpuCount);
        hlay->addWidget(lblLA);
        hlay->addWidget(lblFAN);
        hlay->addWidget(lblWorkerPower);

        lbl->setLayout(hlay);

        QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
        shadowEffect->setBlurRadius(9); // Устанавливаем радиус размытия
        shadowEffect->setOffset(5); // Устанавливаем смещение тени
        lbl->setGraphicsEffect(shadowEffect); // Устанавливаем эффект тени на окно

        QListWidgetItem* item = new QListWidgetItem();
        ui->mainWorkerWidgetList->addItem(item);
        ui->mainWorkerWidgetList->setItemWidget(item, lbl);

        _normalizeItemSizeTable.WorkerName = lblWorkerName->sizeHint().width();
        _normalizeItemSizeTable.WorkerStatus = lblWorkerStatus->sizeHint().width();
        _normalizeItemSizeTable.MinerUpTime = lblMinerUpTime->sizeHint().width();
        _normalizeItemSizeTable.WorkerSpeed = lblWorkerSpeed->sizeHint().width();
        _normalizeItemSizeTable.MinerName = lblMinerName->sizeHint().width();
        _normalizeItemSizeTable.workerErr = lblErr->sizeHint().width();
        _normalizeItemSizeTable.GpuCount = lblGpuCount->sizeHint().width();
        _normalizeItemSizeTable.workerLA = lblLA->sizeHint().width();
        _normalizeItemSizeTable.workerFAN = lblFAN->sizeHint().width();
        _normalizeItemSizeTable.WorkerPower = lblWorkerPower->sizeHint().width();

        for (auto& worker : _workers)
        {
            WorkerItem *w = new WorkerItem();
            w->init(worker);

            chk_connect_q(w, &WorkerItem::clicked, w, &WorkerItem::show);
            chk_connect_q(this, &MainWindow::normalizeSize, w, &WorkerItem::normalizeSize);
            chk_connect_q(w, &WorkerItem::showWorker, this, &MainWindow::showWorkerById);

            QListWidgetItem* item = new QListWidgetItem();
            item->setData(Qt::UserRole, worker.id);
            item->setSizeHint(w->sizeHint() + QSize(0, 30));
            ui->mainWorkerWidgetList->addItem(item);
            ui->mainWorkerWidgetList->setItemWidget(item, w);

            w->setAttribute(Qt::WA_Hover);
            w->installEventFilter(this);

            checkNormalize(w->getListNormalize());
        }

        emit normalizeSize(_normalizeItemSizeTable);

        lblWorkerName->setMinimumWidth(_normalizeItemSizeTable.WorkerName);
        lblWorkerStatus->setMinimumWidth(_normalizeItemSizeTable.WorkerStatus);
        lblMinerUpTime->setMinimumWidth(_normalizeItemSizeTable.MinerUpTime);
        lblWorkerSpeed->setMinimumWidth(_normalizeItemSizeTable.WorkerSpeed);
        lblMinerName->setMinimumWidth(_normalizeItemSizeTable.MinerName);
        lblErr->setMinimumWidth(_normalizeItemSizeTable.workerErr);
        lblGpuCount->setMinimumWidth(_normalizeItemSizeTable.GpuCount);
        lblLA->setMinimumWidth(_normalizeItemSizeTable.workerLA);
        lblFAN->setMinimumWidth(_normalizeItemSizeTable.workerFAN);
        lblWorkerPower->setMinimumWidth(_normalizeItemSizeTable.WorkerPower);

        ui->mainWorkerWidgetList->setMinimumWidth(
        _normalizeItemSizeTable.WorkerName +
        _normalizeItemSizeTable.WorkerStatus +
        _normalizeItemSizeTable.MinerUpTime +
        _normalizeItemSizeTable.WorkerSpeed +
        _normalizeItemSizeTable.MinerName +
        _normalizeItemSizeTable.workerErr +
        _normalizeItemSizeTable.GpuCount +
        _normalizeItemSizeTable.workerLA +
        _normalizeItemSizeTable.workerFAN +
        _normalizeItemSizeTable.WorkerPower);

        setMinimumWidth(ui->mainWorkerWidgetList->minimumWidth() * 2);

        for (auto& worker : _workers)
        {
            for (int i = 0; i < ui->tabWidget->count(); ++i)
                if (ui->tabWidget->tabText(i) == worker.workerName)
                    qobject_cast<WorkerWidget*>(ui->tabWidget->widget(i))->updateData(worker);
        }

        KILL_TIMER(_autoRefId);
        _autoRefId = startTimer(5*1000);
    } // if answer
}

void MainWindow::command_AddWorker(const Message::Ptr& message)
{
    if (message->type() == Message::Type::Answer)
    {
        data::AddWorker addWorker;
        readFromMessage(message, addWorker);

        if (addWorker.resault)
        {
            _socket->send(createMessage(data::UserWorkerList(_userID)));
            QString msg = u8"Воркер добавлен";
            messageBox(this, QMessageBox::Icon::Information, msg, 1);
        }
        else
        {
            QString msg = u8"Ошибка создания воркера";
            messageBox(this, QMessageBox::Icon::Warning, msg, 1);
        }
    }
}

void MainWindow::command_DeleteWorker(const Message::Ptr& message)
{
    if (message->type() == Message::Type::Answer)
    {
        data::DeleteWorker deleteWorker;
        readFromMessage(message, deleteWorker);

        if (deleteWorker.resault)
        {
            _socket->send(createMessage(data::UserWorkerList(_userID)));
            QString msg = QString(u8"%1 удален").arg(deleteWorker.workerName);
            messageBox(this, QMessageBox::Icon::Information, msg, 1);
        }
        else
        {
            QString msg = QString(u8"Ошибка удаления %1").arg(deleteWorker.workerName);
            messageBox(this, QMessageBox::Icon::Information, msg, 1);
        }
    }
}

void MainWindow::command_UpdateWorkerInfoClient(const Message::Ptr& message)
{
    data::UpdateWorkerInfoClient updateWL;
    readFromMessage(message, updateWL);

    if (message->type() == Message::Type::Answer)
    {
        if (updateWL.resault)
        {
            for (data::workerData& worker : _workers)
            {
                if (worker.id == updateWL.workerID)
                {
                    worker.workerName = updateWL.newWorkerName;
                    worker.electricityCost = updateWL.newPowerPrice;
                    updateTabName(updateWL.oldName, updateWL.newWorkerName, updateWL.workerID);

                    QStringList workerList;
                    for (const data::workerData& worker : _workers)
                    {
                        if (worker.workerName.isEmpty())
                            workerList.append(digest(worker.id));
                        else
                            workerList.append(worker.workerName);
                    }

                    emit newWorkerName(workerList);

                    for (int i = 0; i < ui->tabWidget->count(); ++i)
                        if (ui->tabWidget->tabText(i) == worker.workerName)
                        {
                            qobject_cast<WorkerWidget*>(
                                  ui->tabWidget->widget(i))->updateData(worker);
                            break;
                        }

                    break;
                } // if uuid
            } // for
        } // if resault
        else
        {
            const QString msg = QString(u8"Не удалось обновить данные воркера %1.").arg(
                        updateWL.oldName);
            messageBox(this, QMessageBox::Icon::Warning, msg);
        }
    }
}

void MainWindow::checkNormalize(const widgetListSizeNormalize& n)
{
    if (n.WorkerName > _normalizeItemSizeTable.WorkerName)
        _normalizeItemSizeTable.WorkerName = n.WorkerName;
    if (n.WorkerStatus > _normalizeItemSizeTable.WorkerStatus)
        _normalizeItemSizeTable.WorkerStatus = n.WorkerStatus;
    if (n.MinerUpTime > _normalizeItemSizeTable.MinerUpTime)
        _normalizeItemSizeTable.MinerUpTime = n.MinerUpTime;
    if (n.WorkerSpeed > _normalizeItemSizeTable.WorkerSpeed)
        _normalizeItemSizeTable.WorkerSpeed = n.WorkerSpeed;
    if (n.MinerName > _normalizeItemSizeTable.MinerName)
        _normalizeItemSizeTable.MinerName = n.MinerName;
    if (n.workerErr > _normalizeItemSizeTable.workerErr)
        _normalizeItemSizeTable.workerErr = n.workerErr;
    if (n.GpuCount > _normalizeItemSizeTable.GpuCount)
        _normalizeItemSizeTable.GpuCount = n.GpuCount;
    if (n.workerLA > _normalizeItemSizeTable.workerLA)
        _normalizeItemSizeTable.workerLA = n.workerLA;
    if (n.workerFAN > _normalizeItemSizeTable.workerFAN)
        _normalizeItemSizeTable.workerFAN = n.workerFAN;
    if (n.WorkerPower > _normalizeItemSizeTable.WorkerPower)
        _normalizeItemSizeTable.WorkerPower = n.WorkerPower;

//    qDebug() << width() << "x" << height();
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    QEvent::Type type = event->type();

    for (int i = 0; i < ui->mainWorkerWidgetList->count(); ++i)
    {
        QListWidgetItem* item = ui->mainWorkerWidgetList->item(i);
        if (obj == ui->mainWorkerWidgetList->itemWidget(item))
        {

            if  (type == QEvent::HoverLeave)
            {
                ui->mainWorkerWidgetList->itemWidget(item)->setStyleSheet(
                            "QWidget {                          "
                            "background-color: transparent;     "
                            "border: none; border-radius: 0px}; ");
            }
            else if (type == QEvent::HoverEnter)
                ui->mainWorkerWidgetList->itemWidget(item)->setStyleSheet(
                            "QWidget {                                 "
                            "background-color: rgba(35, 146, 220, 0.5);"
                            "border: none;                             "
                            "border-radius: 0px;};                     ");
        }
    }

    if (obj == ui->btnRefresh && !ui->btnRefresh->isEnabled())
        return QWidget::eventFilter(obj, event);

    if (ui->btnConnect || ui->btnRefresh || ui->btnSettings)
    {
        QPushButton* btn = dynamic_cast<QPushButton*>(obj);
        if (type == QMouseEvent::MouseButtonPress)
            btn->setIconSize(QSize(52, 46));
        if (type == QMouseEvent::MouseButtonRelease)
            btn->setIconSize(QSize(52, 52));
    }


    return QWidget::eventFilter(obj, event);
}

void MainWindow::enableButtons(bool val)
{
    ui->btnAddWorker->setEnabled(val);
    ui->btnDelWorker->setEnabled(val);

    if (!val)
    {
        ui->btnRefresh->setEnabled(val);
        ui->btnAddWorker->setVisible(val);
        ui->btnDelWorker->setVisible(val);
    }
}

void MainWindow::connectActiveMessage()
{
    QString msg = u8"Подключено к %1:%2";
    _labelConnectStatus->setText(msg.arg(_socket->peerPoint().address().toString())
                                    .arg(_socket->peerPoint().port()));
}

void MainWindow::reconnectCheck()
{
    if (!_socket->isConnected())
        return;

    _labelConnectStatus->setText(u8"Автоподключение");
    qApp->processEvents();

    _socket->connect();

    int attempts = 0;
    while (attempts++ < 24 /*ждем 6 сек*/)
    {
        qApp->processEvents();
        usleep(250*1000 /*0.25 сек*/);
        if (_socket->isConnected())
            break;
    }

    if (!_socket->isConnected())
    {
        QString msg = u8"Не удалось подключиться к хосту %1:%2. Переподключение через 10 сек";
        _labelConnectStatus->setText(msg.arg(_socket->peerPoint().address().toString())
                                        .arg(_socket->peerPoint().port()));

        _reconnectTimer.start(10*1000 /*10 сек*/);
        return;
    }

    connectActiveMessage();
}

void MainWindow::paramsChanged()
{
    if (_socket->socketIsConnected())
    {
        ui->btnConnect->click();
        loadSettings();
        ui->btnConnect->click();
    }
    else
        loadSettings();
}

void MainWindow::on_tabWidget_tabBarDoubleClicked(int index)
{
    if (index != 0)
        ui->tabWidget->removeTab(index);
}

void MainWindow::on_btnRefresh_clicked(bool checked)
{
    _socket->send(createMessage(data::UserWorkerList(_userID)));
}

void MainWindow::on_btnDelWorker_clicked(bool checked)
{
    if (_delWorkerMode)
    {
        ui->lblDelMode->setText("");
        _delWorkerMode = false;;
    }
    else
    {
        QString msg = u8"Выберете воркер для удаления";
        ui->lblDelMode->setText(msg);
        _delWorkerMode = true;
    }
}


void MainWindow::on_btnAddWorker_clicked(bool checked)
{
    _socket->send(createMessage(data::AddWorker(_userID)));
}

void MainWindow::showRegistrationWindow()
{
    RegistrationWindow rw(this);
    rw.init(_socket);
    chk_connect_q(&rw,  &RegistrationWindow::signalCancel,
                  this, &MainWindow::on_btnConnect_clicked);
    chk_connect_q(&rw,  &RegistrationWindow::signalShowLoginForm,
                  this, &MainWindow::showLoginForm);

    rw.exec();
}

void MainWindow::showLoginForm()
{
    LoginWindow lw(this);
    lw.init(_socket);
    chk_connect_q(&lw,  &LoginWindow::signalCancel,
                  this, &MainWindow::on_btnConnect_clicked);
    chk_connect_q(&lw,  &LoginWindow::signalShowRegistrationWindow,
                  this, &MainWindow::showRegistrationWindow);

    lw.exec();
}

void MainWindow::deleteWorker()
{
    QString workerName = digest(_delWorkerBuffId);
    for (auto& w : _workers)
    {
        if (w.id == _delWorkerBuffId)
        {
            for (int i = 0; i < ui->tabWidget->count(); ++i)
                if (ui->tabWidget->tabText(i) == w.workerName ||
                        ui->tabWidget->tabText(i) == digest(w.id))
                {
                    ui->tabWidget->removeTab(i);
                    break;
                }

            if (!w.workerName.isEmpty())
            {
                workerName = w.workerName;
                break;
            }
        }
    }

    _socket->send(createMessage(data::DeleteWorker(_userID, _delWorkerBuffId, workerName)));
    _delWorkerBuffId = QUuidEx();
}

void MainWindow::updateTabName(const QString &oldName, const QString &newName, const QUuidEx& id)
{
    for (int i = 0; i < ui->tabWidget->count(); ++i)
    {
        if (ui->tabWidget->tabText(i) == oldName || ui->tabWidget->tabText(i)  == digest(id))
        {
            ui->tabWidget->setTabText(i, (newName.isEmpty() ? digest(id) : newName));
            break;
        }
    }

    for (int i = 0; i < ui->mainWorkerWidgetList->count(); ++i)
    {
        QListWidgetItem* item = ui->mainWorkerWidgetList->item(i);
        if (item->data(Qt::UserRole).toUuid() == id)
        {
            qobject_cast<WorkerItem*>(ui->mainWorkerWidgetList->
                                      itemWidget(item))->setName(newName, id);
        }
    }
}


void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (index)
    {
        ui->btnAddWorker->setVisible(false);
        ui->btnDelWorker->setVisible(false);
    }
    else
    {
        ui->btnAddWorker->setVisible(true);
        ui->btnDelWorker->setVisible(true);
    }
}

