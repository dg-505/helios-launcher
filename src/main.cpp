#include <QApplication>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

#include "launcher.h"

auto main(int argc, char* argv[]) -> int
{
    Q_INIT_RESOURCE(resources); // NOLINT

    const QApplication launcherApp(argc, argv);

    // settings to preserve between sessions
    // default location on Linux: ~/.config/heliospp/helios-launcher.ini
    // default location on Windows: C:\Users\<username>\AppData\Roaming\heliospp\helios-launcher.ini
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "heliospp", "helios-launcher");

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
    Launcher launcher(&settings);
    launcher.show();
    return QApplication::exec();
}
