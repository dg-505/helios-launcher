#pragma once

#include <QMainWindow>
#include <QProcess>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui
{
class Launcher;
} // namespace Ui
QT_END_NAMESPACE

class Launcher : public QMainWindow
{
    Q_OBJECT // NOLINT

public:
    Launcher(const Launcher&) = delete;
    Launcher(Launcher&&) = delete;
    auto operator=(const Launcher&) -> Launcher& = delete;
    auto operator=(Launcher&&) -> Launcher& = delete;

    explicit Launcher(const std::string& version, QSettings* settings, QWidget* parent = nullptr);
    ~Launcher() override;

public Q_SLOTS:
    void updateCmd();
    // write settings
    void writeHeliosBaseDirToSettings();
    void writeLastSurveyToSettings();
    void writeExecModeToSettings();
    // Redirect console output of HELIOS++ to QTextBrowser
    void startHeliospp();
    void redirectStdout();
    void redirectStderr();
    void exitHeliospp();

private:
    Ui::Launcher* _ui;
    QSettings* _settings;
    QProcess _process;
    QString _outputDir;
};
