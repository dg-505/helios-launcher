#include <QFileDialog>
#include <QMessageBox>

#include "../ui/ui_basedir.h"
#include "basedir.h"

BaseDir::BaseDir(QWidget* parent)
    : QDialog(parent),
      _ui(new Ui::BaseDir)
{
    _ui->setupUi(this);

    QObject::connect(_ui->browseButton, &QPushButton::clicked, this, &BaseDir::browse);
    QObject::connect(_ui->proceedButton, &QPushButton::clicked, this, &BaseDir::proceed);
    QObject::connect(_ui->cancelButton, &QPushButton::clicked, this, &BaseDir::reject);
}

BaseDir::~BaseDir()
{
    delete _ui;
}
auto BaseDir::getBaseDir() const -> QString
{
    return _ui->baseDirLine->text();
}

void BaseDir::browse()
{
    auto browseDir = _ui->baseDirLine->text();
    if (browseDir.isEmpty())
    {
        browseDir = QDir::homePath();
    }
    const auto dir = QFileDialog::getExistingDirectory(this, tr("Specify the HELIOS++ base directory"), browseDir, QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty())
    {
        _ui->baseDirLine->setText(dir);
    }
}

void BaseDir::proceed()
{
#ifdef _WIN32
    auto exec = QFile(_ui->baseDirLine->text() + "\\run\\helios.exe");
#else
    auto exec = QFile(_ui->baseDirLine->text() + "/run/helios");
#endif
    if (!exec.exists())
    {
        QMessageBox msgBox(QMessageBox::Warning, tr("No valid HELIOS++ installation"), tr("The specified directory does not seem to contain a valid HELIOS++ installation. Please choose another directory."), QMessageBox::Ok, this);
        msgBox.exec();
    }
    else
    {
        this->accept();
    }
}
