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

    BleSerial bleSerial("00:E0:02:12:00:54");
    Bl0940Handler* bl0940Handler = createBl0940Handler(&bleSerial);

    MainWindow win;
    win.setBl0940(bl0940Handler);
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
