#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>

#include "../ui/ui_mainwindow.h"
#include "help.h"
#include "mainwindow.h"

MainWindow::MainWindow(QSettings* settings, QWidget* parent)
    : QMainWindow(parent),
      _ui(new Ui::MainWindow),
      _settings(settings),
      _process(new QProcess(this)),
      _outputDir(_settings->value("DIRS/HeliosBaseDir").toString() + "/output")
{
    _ui->setupUi(this);
    this->setWindowIcon(QIcon(":/heliospp.png"));

    // restore helios base directory, last survey.xml and optional arguments from settings
    _ui->lineEdit->setText(_settings->value("DIRS/HeliosBaseDir").toString());
    _ui->lineEdit_2->setText(_settings->value("DIRS/LastSurvey").toString());
    auto numArgs = _settings->beginReadArray("ARGS");
    for (int i = 0; i < numArgs; i++)
    {
        _settings->setArrayIndex(i);
        if (_settings->value("arg").toString().isEmpty())
        {
            continue;
        }
        _ui->plainTextEdit->insertPlainText(_settings->value("arg").toString() + " ");
        _ui->plainTextEdit->moveCursor(QTextCursor::End);
    }
    _settings->endArray();

    // set up process
    _process.setWorkingDirectory(_settings->value("DIRS/HeliosBaseDir").toString());
    _process.setProcessChannelMode(QProcess::MergedChannels);
#ifndef _WIN32
    // Set LD_LIBRARY_PATH to <heliosBaseDir>/run
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LD_LIBRARY_PATH", _process.workingDirectory() + "/run");
    _process.setProcessEnvironment(env);
#endif

// clang-format off
    // when changing the helios base directory, survey.xml or optional arguments, instantly save new values to settings
    QObject::connect(_ui->lineEdit, &QLineEdit::editingFinished, this, &MainWindow::writeHeliosBaseDirToSettings);
    QObject::connect(_ui->lineEdit_2, &QLineEdit::editingFinished, this, &MainWindow::writeLastSurveyToSettings);

    // Buttons: Open File Dialogs to select helios base directory and survey.xml
    QObject::connect(_ui->pushButton_2, &QPushButton::clicked, this, [this]()
                    {
                        const QString heliosBaseDir = QFileDialog::getExistingDirectory(this, tr("Select Helios++ base directory"), _settings->value("DIRS/HeliosBaseDir").toString());
                        if (!heliosBaseDir.isEmpty())
                        {
                            _ui->lineEdit->setText(heliosBaseDir);
                            _settings->setValue("DIRS/HeliosBaseDir", heliosBaseDir);
                            _process.setWorkingDirectory(heliosBaseDir);
                        }
                    });
    QObject::connect(_ui->pushButton_6, &QPushButton::clicked, this, [this]()
                    {
                        const QString survey = QFileDialog::getOpenFileName(this, tr("Select survey XML file"), _settings->value("DIRS/HeliosBaseDir").toString() + "/data/surveys", tr("XML files (*.xml)"));
                        if (!survey.isEmpty())
                        {
                            _ui->lineEdit_2->setText(survey);
                            _settings->setValue("DIRS/LastSurvey", survey);
                        }
                    });

    // HelpDialog button displays Helios++ main help
    QObject::connect(_ui->pushButton_7, &QPushButton::clicked, this, [this]()
                    {
#ifdef _WIN32
                        _process.start(_process.workingDirectory() + "/run/helios.exe", QStringList() << "--help");
#else
                        _process.start("run/helios", QStringList() << "--help");
#endif
                    });

    // Redirect console output of Helios++ to QTextBrowser
    QObject::connect(_ui->pushButton_4, &QPushButton::clicked, this, &MainWindow::startHeliospp);
    QObject::connect(&_process, &QProcess::readyRead, this, &MainWindow::redirectStdout);
    QObject::connect(&_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::exitHeliospp);

    // Kill running Helioss++
    QObject::connect(_ui->pushButton, &QPushButton::clicked, this, [this]()
                    {
                        if (_process.state() == QProcess::Running)
                        {
                           _process.kill();
                        }
                    });

    // Open Helios++ output directory in file explorer
    QObject::connect(_ui->pushButton_5, &QPushButton::clicked, this, [this]()
                    {
                        const QUrl outputDirUrl = QUrl::fromLocalFile(_outputDir);
                        qDebug() << outputDirUrl;
                        QDesktopServices::openUrl(outputDirUrl);
                    });

    // Clear helios++ output
    QObject::connect(_ui->pushButton_3, &QPushButton::clicked, this, [this]()
                    {
                        _ui->textBrowser_2->clear();
                        _outputDir = _settings->value("DIRS/HeliosBaseDir").toString() + "/output";
                    });
// clang-format on
}

MainWindow::~MainWindow()
{
    // save settings on close
    writeHeliosBaseDirToSettings();
    writeLastSurveyToSettings();
    auto args = _ui->plainTextEdit->toPlainText().split(QRegExp("[\\s\n]+"));
    _settings->beginWriteArray("ARGS");
    for (int i = 0; i < args.size(); i++)
    {
        if (args.at(i).isEmpty())
        {
            continue;
        }
        _settings->setArrayIndex(i);
        _settings->setValue("arg", args.at(i));
    }
    _settings->endArray();
    _process.close();
    delete _ui;
}

void MainWindow::writeHeliosBaseDirToSettings()
{
    _settings->setValue("DIRS/HeliosBaseDir", _ui->lineEdit->text());
    _process.setWorkingDirectory(_settings->value("DIRS/HeliosBaseDir").toString());
}

void MainWindow::writeLastSurveyToSettings()
{
    _settings->setValue("DIRS/LastSurvey", _ui->lineEdit_2->text());
}

void MainWindow::startHeliospp()
{

    // Read helios base directory from lineEdit
    auto heliosBaseDir = _ui->lineEdit->text();
    // Read survey from lineEdit_2 and optional arguments from plainTextEdit
    auto options = QStringList() << _ui->lineEdit_2->text() << _ui->plainTextEdit->toPlainText().split(QRegExp("[ \n]"));

    // clear output
    if (!(options.contains("--help") || options.contains("-h") || options.contains("--version")))
    {
        _ui->textBrowser_2->clear();
    }
#ifdef _WIN32
    if (!(options.contains("--help") || options.contains("-h") || options.contains("--version")))
    {
        _ui->textBrowser_2->append(heliosBaseDir + ">run/helios.exe " + options.join(" "));
    }
    _process.start(_process.workingDirectory() + "/run/helios.exe", options);
#else
    if (!(options.contains("--help") || options.contains("-h") || options.contains("--version")))
    {
        _ui->textBrowser_2->append(qgetenv("USERNAME") + "@" + QSysInfo::machineHostName() + ":" + heliosBaseDir + "$ run/helios " + options.join(" "));
    }
    _process.start("run/helios", options);
#endif
}

void MainWindow::redirectStdout()
{
    // when '-h' or '--help': redirect output to HelpDialog
    if (_process.arguments().contains("-h") || _process.arguments().contains("--help"))
    {
        auto msg = QString(_process.readAll());
        HelpDialog help(msg, this);
        help.exec();
    }
    // when '--version': show version info in MessageBox
    else if (_process.arguments().contains("--version"))
    {
        auto msg = QString(_process.readAll());
        QMessageBox::information(this, "HELIOS++ version info", msg);
        return;
    }
    else
    {
        // Append new output to QTextBrowser
        _ui->textBrowser_2->append(_process.readAll());
    }
}

void MainWindow::exitHeliospp()
{
    if (!(_process.arguments().contains("--help") || _process.arguments().contains("-h") || _process.arguments().contains("--version")))
    {
        auto exitCode = _process.exitCode();
        if (exitCode == 0)
        {
            _ui->textBrowser_2->append("Helios++ exited successfully\n");
            // Extract output directory from Helios++ output
            QString relOutDir;
            for (auto& line : _ui->textBrowser_2->toPlainText().split("\n"))
            {
                if (line.startsWith("Output directory: \"output//"))
                {
                    relOutDir = line.split("//").at(1).left(line.split("//").at(1).length() - 1);
                    _outputDir = _settings->value("DIRS/HeliosBaseDir").toString() + "/output/" + relOutDir;
                    break;
                }
            }
        }
        else
        {
            _ui->textBrowser_2->append("Helios++ terminated with exit code " + QString::number(_process.exitCode()) + "\n");
        }
        _ui->textBrowser_2->verticalScrollBar()->setValue(_ui->textBrowser_2->verticalScrollBar()->maximum());
    }
    _process.close();
}
