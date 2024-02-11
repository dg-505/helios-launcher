#pragma once

#include <QMainWindow>
#include <QProcess>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
} // namespace Ui
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT // NOLINT

public:
    MainWindow(const MainWindow&) = delete;
    MainWindow(MainWindow&&) = delete;
    auto operator=(const MainWindow&) -> MainWindow& = delete;
    auto operator=(MainWindow&&) -> MainWindow& = delete;

    explicit MainWindow(QSettings* settings, QWidget* parent = nullptr);
    ~MainWindow() override;

    // Redirect console output of Helios++ to QTextBrowser
    void startHeliospp();
    void redirectStdout();
    void exitHeliospp();

public Q_SLOTS:
    void writeLastSurveyToSettings();
    void writeHeliosBaseDirToSettings();

private:
    Ui::MainWindow* _ui;
    QSettings* _settings;
    QProcess _process;
    QString _outputDir;
};
