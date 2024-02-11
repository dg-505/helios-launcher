#include <QApplication>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

#include "mainwindow.h"

auto main(int argc, char* argv[]) -> int
{
    Q_INIT_RESOURCE(resources); // NOLINT

    const QApplication heliosGuiApp(argc, argv);

    // settings to preserve between sessions
    // default location on Linux: ~/.config/heliospp/heliosGui.ini
    // default location on Windows: C:\Users\<username>\AppData\Roaming\heliospp\heliosGui.ini
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "heliospp", "heliosGui");

    // possible later: add support for translations
    QTranslator translator(nullptr);
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString& locale : uiLanguages)
    {
        if (translator.load(":/i18n/" + QLocale(locale).name()))
        {
            QApplication::installTranslator(&translator);
            break;
        }
    }
    MainWindow heliosGuiMainWindow(&settings);
    heliosGuiMainWindow.show();
    return QApplication::exec();
}
