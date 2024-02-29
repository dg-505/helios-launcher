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
        // set default values for optional arguments
        settings.beginGroup("ARGS");
        settings.setValue("General", "");
        settings.setValue("Output", "");
        settings.setValue("ZipOutput", "false");
        settings.setValue("LasScale", "default");
        settings.setValue("Parallelization", "default");
        settings.setValue("nthreads", "default");
        settings.setValue("ChunkSize", "default");
        settings.setValue("WarehouseFactor", "default");
        settings.setValue("SplitByChannel", "false");
        settings.setValue("WriteWaveform", "false");
        settings.setValue("CalcEchoWidth", "false");
        settings.setValue("FullwaveNoise", "false");
        settings.setValue("FixedIncidenceAngle", "false");
        settings.setValue("DisablePlatformNoise", "false");
        settings.setValue("DisableLegNoise", "false");
        settings.setValue("Seed", "default");
        settings.setValue("GpsStartTime", "default");
        settings.setValue("LogFile", "false");
        settings.setValue("LogFileOnly", "false");
        settings.setValue("Silent", "false");
        settings.setValue("Quiet", "false");
        settings.setValue("Vt", "false");
        settings.setValue("V", "false");
        settings.setValue("VV", "false");
        settings.setValue("RebuildScene", "false");
        settings.setValue("KDTree", "default");
        settings.setValue("KDTreeThreads", "default");
        settings.setValue("KDTreeGeomThreads", "default");
        settings.setValue("SAHnodes", "default");
        settings.setValue("Unzip", "false");
        settings.setValue("UnzipInput", "");
        settings.setValue("UnzipOutput", "");
        settings.setValue("AssetsPathFlag", "false");
        settings.setValue("AssetsPath", "");
        settings.setValue("OutputPathFlag", "false");
        settings.setValue("OutputPath", "");
        settings.setValue("LiveTrajectoryPlot", "false");
        settings.setValue("Polyscope", "false");
        settings.setValue("Open3D", "false");
        settings.endGroup();
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
    version = "0.1.0";
    Launcher launcher(version, &settings);
    launcher.show();
    return QApplication::exec();
}
