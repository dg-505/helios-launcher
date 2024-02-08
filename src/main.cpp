#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

auto main(int argc, char* argv[]) -> int
{
    Q_INIT_RESOURCE(resources); // NOLINT

    const QApplication heliosGuiApp(argc, argv);

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
    MainWindow heliosGuiMainWindow;
    heliosGuiMainWindow.show();
    return QApplication::exec();
}
