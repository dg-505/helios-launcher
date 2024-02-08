#include "mainwindow.h"
#include "../ui/ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QSettings* settings, QWidget* parent)
    : QMainWindow(parent), _ui(new Ui::MainWindow), _settings(settings)
{
    _ui->setupUi(this);
    _ui->lineEdit->setText(_settings->value("DIRS/HeliosBaseDir").toString());
    this->setWindowIcon(QIcon(":/heliospp.png"));

    QObject::connect(_ui->buttonBox, &QDialogButtonBox::clicked, this, [=]()
                     {
                         const QString heliosBaseDir = QFileDialog::getExistingDirectory(this, tr("Select Helios++ base directory"), QDir::homePath());
                         if (!heliosBaseDir.isEmpty())
                         {
                             _ui->lineEdit->setText(heliosBaseDir);
                             _settings->setValue("DIRS/HeliosBaseDir", heliosBaseDir);
                         } });
}

MainWindow::~MainWindow()
{
    delete _ui;
}
