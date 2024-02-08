#include "mainwindow.h"
#include "../ui/ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QSettings* settings, QWidget* parent)
    : QMainWindow(parent), _ui(new Ui::MainWindow), _settings(settings)
{
    _ui->setupUi(this);
    this->setWindowIcon(QIcon(":/heliospp.png"));

    _ui->lineEdit->setText(_settings->value("DIRS/HeliosBaseDir").toString());
    _ui->lineEdit_2->setText(_settings->value("DIRS/LastSurvey").toString());

    QObject::connect(_ui->buttonBox, &QDialogButtonBox::clicked, this, [=]()
                     {
                         const QString heliosBaseDir = QFileDialog::getExistingDirectory(this, tr("Select Helios++ base directory"), QDir::homePath());
                         if (!heliosBaseDir.isEmpty())
                         {
                             _ui->lineEdit->setText(heliosBaseDir);
                             _settings->setValue("DIRS/HeliosBaseDir", heliosBaseDir);
                         } });

    QObject::connect(_ui->buttonBox_2, &QDialogButtonBox::clicked, this, [=]()
                     {
                         const QString survey = QFileDialog::getOpenFileName(this, tr("Select survey XML file"), _settings->value("DIRS/HeliosBaseDir").toString() + "/data/surveys", tr("XML files (*.xml)"));
                         if (!survey.isEmpty())
                         {
                             _ui->lineEdit_2->setText(survey);
                             _settings->setValue("DIRS/LastSurvey", survey);
                         } });
}

MainWindow::~MainWindow()
{
    delete _ui;
}
