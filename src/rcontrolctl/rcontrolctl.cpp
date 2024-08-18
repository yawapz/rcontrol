#include "rcontrolctl_appl.h"
#include "rcontrolctl_wdc.h"

#include "shared/defmac.h"
#include "shared/utils.h"
#include "shared/type_name.h"
#include "shared/logger/logger.h"
#include "shared/logger/format.h"
#include "shared/logger/config.h"
#include "shared/config/appl_conf.h"
#include "shared/config/logger_conf.h"
#include "shared/qt/logger_operators.h"
#include "shared/qt/version_number.h"
#include "pproto/commands/base.h"
#include "pproto/commands/pool.h"
#include "pproto/transport/tcp.h"
#include "pproto/transport/udp.h"
#include "database/postgres_pool.h"

#include <QtCore>
#include <QNetworkProxy>

#ifdef MINGW
#include <windows.h>
#else
#include <csignal>
#endif
#include <unistd.h>
#include <limits>

using namespace std;
using namespace pproto;
using namespace pproto::transport;

/**
  Используется для уведомления основного потока о завершении работы программы.
*/
void stopProgramHandler(int sig)
{
    if ((sig == SIGTERM) || (sig == SIGINT))
    {
        const char* sigName = (sig == SIGTERM) ? "SIGTERM" : "SIGINT";
        log_verbose << "Signal " << sigName << " is received. Program will be stopped";
        Application::stop();
    }
    else
        log_verbose << "Signal " << sig << " is received";
}

void stopProgram()
{
    #define STOP_THREAD(THREAD_FUNC, NAME, TIMEOUT) \
        if (!THREAD_FUNC.stop(TIMEOUT * 1000)) { \
            log_info << "Thread '" NAME "': Timeout expired, thread will be terminated"; \
            THREAD_FUNC.terminate(); \
        }

    STOP_THREAD(udp::socket()        , "TransportUDP"       , 15)
    STOP_THREAD(wdc::wdc()           , "WorkerDataControl"  , 15)

    #undef STOP_THREAD

    tcp::listener().close();

    log_info << log_format("'%?' service is stopped", APPLICATION_NAME);

    // Удаляем сэйвер для записи лог-сообщений в БД
    alog::logger().removeSaver("eventlog");

    // Перед отключением БД нужно сбросить все лог-сообщения
    alog::logger().flush();
    alog::logger().waitingFlush();

    alog::stop();

    db::postgres::pool().close();
}

void helpInfo()
{
    alog::logger().clearSavers();
    alog::logger().addSaverStdOut(alog::Level::Info, true);

    log_info << log_format(
        "'%?' service (version: %?; protocol version: %?-%?; gitrev: %?)",
        APPLICATION_NAME, productVersion().toString(),
        PPROTO_VERSION_LOW, PPROTO_VERSION_HIGH, GIT_REVISION);
    log_info << "Usage: rcontrolctl";
    log_info << "  -n do not daemonize";
    log_info << "  -h this help";
    alog::logger().flush();
}

int main(int argc, char *argv[])
{
    // Устанавливаем в качестве разделителя целой и дробной части символ '.',
    // если этого не сделать - функции преобразования строк в числа (std::atof)
    // буду неправильно работать.
    qputenv("LC_NUMERIC", "C");

    // Именуем листенер
    //tcp::listener().setName("client");

    int ret = 0;
    try
    {
        alog::logger().start();

#ifdef NDEBUG
        alog::logger().addSaverStdOut(alog::Level::Info, true);
#else
        alog::logger().addSaverStdOut(alog::Level::Debug2);
#endif
        signal(SIGTERM, &stopProgramHandler);
        signal(SIGINT,  &stopProgramHandler);

        // Путь к основному конфиг-файлу
        QString configFile = config::qdir() + "/rcontrolctl.conf";

        int c;
        while ((c = getopt(argc, argv, "hnc:")) != EOF)
        {
            switch (c)
            {
                case 'h':
                    helpInfo();
                    alog::stop();
                    exit(0);
                case 'c':
                    configFile = optarg;
                    break;
                case '?':
                    log_error << "Invalid option";
                    alog::stop();
                    return 1;
            }
        }

        config::dirExpansion(configFile);
        if (!QFile::exists(configFile))
        {
            log_error << "Config file " << configFile << " not exists";
            alog::stop();
            return 1;
        }

        config::base().setReadOnly(true);
        config::base().setSaveDisabled(true);
        if (!config::base().readFile(configFile.toStdString()))
        {
            alog::stop();
            return 1;
        }

        // Путь к конфиг-файлу текущих настроек
        QString configFileS;
        config::base().getValue("state.file", configFileS);

        config::dirExpansion(configFileS);
        config::state().readFile(configFileS.toStdString());

        // Создаем дефолтный сэйвер для логгера
        if (!alog::configDefaultSaver())
        {
            alog::stop();
            return 1;
        }

        log_info << log_format(
            "'%?' service is running (version: %?; protocol version: %?-%?; gitrev: %?)",
            APPLICATION_NAME, productVersion().toString(),
            PPROTO_VERSION_LOW, PPROTO_VERSION_HIGH, GIT_REVISION);
        alog::logger().flush();

        alog::logger().removeSaverStdOut();
        alog::logger().removeSaverStdErr();

        // Создаем дополнительные сэйверы для логгера
        alog::configExtendedSavers();
        alog::printSaversInfo();

        if (!pproto::command::checkUnique())
        {
            stopProgram();
            return 1;
        }

        if (!pproto::error::checkUnique())
        {
            stopProgram();
            return 1;
        }

        Application appl {argc, argv};

        qRegisterMetaType<QUuidEx>("QUuidEx");
        qRegisterMetaType<data::workerData>("data::workerData");

        // Устанавливаем текущую директорию. Эта конструкция работает только
        // когда создан экземпляр QCoreApplication.
        if (QDir::setCurrent(QCoreApplication::applicationDirPath()))
        {
            log_debug << "Set work directory: " << QCoreApplication::applicationDirPath();
        }
        else
        {
            log_error << "Failed set work directory";
            stopProgram();
            return 1;
        }

        QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);

        auto databaseInit = [](db::postgres::Driver::Ptr db) -> bool
        {
            QString hostAddress = "127.0.0.1";
            int port = 5432;
            QString user = "postgres";
            QString password = "postgres";
            QString name;
            QString options;

            YamlConfig::Func loadFunc = [&](YamlConfig* conf, YAML::Node& node, bool /*logWarn*/)
            {
                conf->getValue(node, "address",  hostAddress);
                conf->getValue(node, "port",     port);
                conf->getValue(node, "user",     user);
                conf->getValue(node, "password", password);
                conf->getValue(node, "name",     name);
                conf->getValue(node, "options",  options);
                return true;
            };
            config::base().getValue("database", loadFunc);

            if (name.isEmpty())
            {
                log_error << "Database name is not defined";
                return false;
            }
            return db->open(name, user, password, hostAddress, port, options);
        };

        if (!db::postgres::pool().init(databaseInit))
        {
            stopProgram();
            return 1;
        }

        // Инициализацию Application выполняем после подключения к БД
        if (!appl.init())
        {
            stopProgram();
            return 1;
        }

        // Конфигурирование client-подключений
        QHostAddress hostAddress = QHostAddress::AnyIPv4;
        config::readHostAddress("listener.address", hostAddress);

        int port = DEFAULT_PORT;
        config::base().getValue("listener.port", port);

        if (!tcp::listener().init({hostAddress, port}))
        {
            stopProgram();
            return 1;
        }

        tcp::listener().setMessageWebFlags(true);

        chk_connect_q(&tcp::listener(), &tcp::Listener::message,
                      &appl, &Application::message)

        chk_connect_q(&tcp::listener(), &tcp::Listener::socketConnected,
                      &appl, &Application::socketConnected)

        chk_connect_q(&tcp::listener(), &tcp::Listener::socketDisconnected,
                      &appl, &Application::socketDisconnected)

        // Конфигурирование udp-сокета для обнаружения Агентов
        if (!udp::socket().init({QHostAddress::AnyIPv4, port}))
        {
            stopProgram();
            return 1;
        }

        udp::socket().start();
        udp::socket().waitBinding(3);
        if (!udp::socket().isBound())
        {
            stopProgram();
            return 1;
        }
        chk_connect_q(&udp::socket(), &udp::Socket::message,
                      &appl, &Application::message)

        config::observerBase().start();

        if (!wdc::wdc().init())
        {
            stopProgram();
            return 1;
        }

        chk_connect_q(&appl,       &Application::signalWorkerDataComing,
                      &wdc::wdc(), &wdc::WorkerDataControl::slotWorkerDataComing);

        wdc::wdc().start();

        alog::logger().removeSaverStdOut();
        alog::logger().removeSaverStdErr();

        appl.sendSettings();

        ret = appl.exec();

        appl.deinit();

        config::observerBase().stop();

        if (config::state().changed())
            config::state().saveFile();

        stopProgram();
        return ret;
    }
    catch (std::exception& e)
    {
        log_error << "Failed initialization. Detail: " << e.what();
        ret = 1;
    }
    catch (...)
    {
        log_error << "Failed initialization. Unknown error";
        ret = 1;
    }

    stopProgram();
    return ret;
}
