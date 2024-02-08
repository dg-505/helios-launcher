#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

auto main(int argc, char* argv[]) -> int
{
    Q_INIT_RESOURCE(resources); // NOLINT

    const QApplication heliosGuiApp(argc, argv);

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "heliospp", "heliosGui");

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
