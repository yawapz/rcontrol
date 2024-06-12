#include "rcontrolagent_appl.h"

//#include "event_log.h"
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

//#include "database/postgres_pool.h"

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

    STOP_THREAD(udp::socket()        , "TransportUDP"  , 15)

    #undef STOP_THREAD

    tcp::listener().close();

    log_info << log_format("'%?' service is stopped", APPLICATION_NAME);

    alog::logger().flush();
    alog::logger().waitingFlush();

    alog::stop();
}

void helpInfo()
{
    alog::logger().clearSavers();
    alog::logger().addSaverStdOut(alog::Level::Info, true);

    log_info << log_format(
        "'%?' service (version: %?; protocol version: %?-%?; gitrev: %?)",
        APPLICATION_NAME, productVersion().toString(),
        PPROTO_VERSION_LOW, PPROTO_VERSION_HIGH, GIT_REVISION);
    log_info << "Usage: rcontrol";
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
        QString configFile = config::qdir() + "/rcontrolagent.conf";

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

        // Инициализацию Application
        if (!appl.init())
        {
            stopProgram();
            return 1;
        }

        config::observerBase().start();

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
