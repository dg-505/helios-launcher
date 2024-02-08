#include "mainwindow.h"
#include "../ui/ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/heliospp.png"));

    QObject::connect(ui->buttonBox, &QDialogButtonBox::clicked, this, [=]()
                     {
                         const QString directory = QFileDialog::getExistingDirectory(this, tr("Select helios base directory"), QDir::homePath());
                         if (!directory.isEmpty())
                         {
                             ui->lineEdit->setText(directory);
                         } });
}

MainWindow::~MainWindow()
{
    delete ui;
}
