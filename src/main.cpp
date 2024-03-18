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

    QApplication::setStyle(QStringLiteral("fusion"));
    const QApplication launcherApp(argc, argv);

    // settings to preserve between sessions
    // default location on Linux: ~/.config/heliospp/helios-launcher.ini
    // default location on Windows: C:\Users\<username>\AppData\Roaming\heliospp\helios-launcher.ini
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("heliospp"), QStringLiteral("helios-launcher"));

    // if not existing, specify HELIOS++ base directory
    if (settings.value(QStringLiteral("DIRS/HeliosBaseDir")).isNull() || settings.value(QStringLiteral("DIRS/HeliosBaseDir")).toString() == QLatin1String(""))
    {
        auto* baseDir = std::make_unique<BaseDir>(nullptr).release();
        if (baseDir->exec() == QDialog::Accepted)
        {
            settings.setValue(QStringLiteral("DIRS/HeliosBaseDir"), baseDir->getBaseDir());
        }
        else
        {
            return 1;
        }
        // set default values for optional arguments
        settings.beginGroup(QStringLiteral("ARGS"));
        settings.setValue(QStringLiteral("General"), "");
        settings.setValue(QStringLiteral("Output"), "");
        settings.setValue(QStringLiteral("ZipOutput"), "false");
        settings.setValue(QStringLiteral("LasScale"), "default");
        settings.setValue(QStringLiteral("Parallelization"), "default");
        settings.setValue(QStringLiteral("nthreads"), "default");
        settings.setValue(QStringLiteral("ChunkSize"), "default");
        settings.setValue(QStringLiteral("WarehouseFactor"), "default");
        settings.setValue(QStringLiteral("SplitByChannel"), "false");
        settings.setValue(QStringLiteral("WriteWaveform"), "false");
        settings.setValue(QStringLiteral("CalcEchoWidth"), "false");
        settings.setValue(QStringLiteral("FullwaveNoise"), "false");
        settings.setValue(QStringLiteral("FixedIncidenceAngle"), "false");
        settings.setValue(QStringLiteral("DisablePlatformNoise"), "false");
        settings.setValue(QStringLiteral("DisableLegNoise"), "false");
        settings.setValue(QStringLiteral("Seed"), "default");
        settings.setValue(QStringLiteral("GpsStartTime"), "default");
        settings.setValue(QStringLiteral("LogFile"), "false");
        settings.setValue(QStringLiteral("LogFileOnly"), "false");
        settings.setValue(QStringLiteral("Silent"), "false");
        settings.setValue(QStringLiteral("Quiet"), "false");
        settings.setValue(QStringLiteral("Vt"), "false");
        settings.setValue(QStringLiteral("V"), "false");
        settings.setValue(QStringLiteral("VV"), "false");
        settings.setValue(QStringLiteral("RebuildScene"), "false");
        settings.setValue(QStringLiteral("KDTree"), "default");
        settings.setValue(QStringLiteral("KDTreeThreads"), "default");
        settings.setValue(QStringLiteral("KDTreeGeomThreads"), "default");
        settings.setValue(QStringLiteral("SAHnodes"), "default");
        settings.setValue(QStringLiteral("Unzip"), "false");
        settings.setValue(QStringLiteral("UnzipInput"), "");
        settings.setValue(QStringLiteral("UnzipOutput"), "");
        settings.setValue(QStringLiteral("AssetsPathFlag"), "false");
        settings.setValue(QStringLiteral("AssetsPath"), "");
        settings.setValue(QStringLiteral("OutputPathFlag"), "false");
        settings.setValue(QStringLiteral("OutputPath"), "");
        settings.setValue(QStringLiteral("LiveTrajectoryPlot"), "false");
        settings.setValue(QStringLiteral("Polyscope"), "false");
        settings.setValue(QStringLiteral("Open3D"), "false");
        settings.endGroup();
        settings.setValue(QStringLiteral("MISC/ArgsSource"), "GUI");
        settings.setValue(QStringLiteral("MISC/CurrentTab"), "0");
        settings.setValue(QStringLiteral("MISC/ExecMode"), "default");
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

    std::string version;
    version = "0.2.0";
    Launcher launcher(version, &settings);
    launcher.show();
    return QApplication::exec();
}
