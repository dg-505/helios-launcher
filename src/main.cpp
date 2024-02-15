#include <QApplication>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

#include "basedir.h"
#include "launcher.h"

auto main(int argc, char* argv[]) -> int
{
    Q_INIT_RESOURCE(resources); // NOLINT

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    const QApplication launcherApp(argc, argv);
    QApplication::setStyle("fusion");

    // settings to preserve between sessions
    // default location on Linux: ~/.config/heliospp/helios-launcher.ini
    // default location on Windows: C:\Users\<username>\AppData\Roaming\heliospp\helios-launcher.ini
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "heliospp", "helios-launcher");

    // if not existing, specify HELIOS++ base directory
    if (settings.value("DIRS/HeliosBaseDir").isNull() || settings.value("DIRS/HeliosBaseDir").toString() == "")
    {
        auto* baseDir = std::make_unique<BaseDir>(nullptr).release();
        if (baseDir->exec() == QDialog::Accepted)
        {
            settings.setValue("DIRS/HeliosBaseDir", baseDir->getBaseDir());
        }
        else
        {
            return 1;
        }
    }

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
