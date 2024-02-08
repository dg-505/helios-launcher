#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui {
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

private:
    Ui::MainWindow* _ui;
    QSettings* _settings;
};
#endif // MAINWINDOW_H