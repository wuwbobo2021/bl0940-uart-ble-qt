#include "mainwindow.h"

#include <QApplication>
#include <QLoggingCategory>
#include <QLocale>
#include <QTranslator>

#include "bl0940handler.h"

static void installTranslator(QApplication& app);

int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    installTranslator(app);

    MainWindow win;

    DeviceHandler deviceHandler;
    Bl0940Handler* bl0940Handler = createBl0940Handler(&deviceHandler);

    bl0940Handler->connect(bl0940Handler, &Bl0940Handler::readElecData,
                           &win, &MainWindow::receivedElecData,
                           Qt::BlockingQueuedConnection);

    deviceHandler.startScan();
    deviceHandler.connect(&deviceHandler, &DeviceHandler::bleDeviceNotFound,
                          &deviceHandler, &DeviceHandler::startScan);
    deviceHandler.connect(&deviceHandler, &DeviceHandler::bleDisconnected,
                          &deviceHandler, &DeviceHandler::startScan);

    win.show();
    return app.exec();
}

static void installTranslator(QApplication& app)
{
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "bl0940-uart-ble-qt_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }
}
