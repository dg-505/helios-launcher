#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>

#include "../ui/ui_launcher.h"
#include "basedir.h"
#include "help.h"
#include "launcher.h"

Launcher::Launcher(const std::string& version, QSettings* settings, QWidget* parent)
    : QMainWindow(parent),
      _ui(new Ui::Launcher),
      _settings(settings),
      _process(new QProcess(this)),
      _outputDir(_settings->value("DIRS/HeliosBaseDir").toString() + "/output")
{
    _ui->setupUi(this);
    this->setWindowIcon(QIcon(":/heliospp.png"));
    this->setWindowTitle("HELIOS++ launcher version " + QString::fromStdString(version));

    // restore helios base directory, last survey.xml, execution mode and optional arguments from settings
    _ui->heliosBaseDirLineEdit->setText(_settings->value("DIRS/HeliosBaseDir").toString());
    _ui->surveyPathLineEdit->setText(_settings->value("DIRS/LastSurvey").toString());
#ifdef _WIN32
    _ui->heliosBaseDirLineEdit->setText(_ui->heliosBaseDirLineEdit->text().replace("/", "\\"));
    _ui->surveyPathLineEdit->setText(_ui->surveyPathLineEdit->text().replace("/", "\\"));
#endif
    _ui->defaultModeButton->setChecked(_settings->value("MODE/ExecMode").toString() == "default");
    _ui->heliospyModeButton->setChecked(_settings->value("MODE/ExecMode").toString() == "helios.py");
    QObject::connect(_ui->defaultModeButton, &QRadioButton::toggled, this, &Launcher::writeExecModeToSettings);
    QObject::connect(_ui->heliospyModeButton, &QRadioButton::toggled, this, &Launcher::writeExecModeToSettings);
    auto numArgs = _settings->beginReadArray("ARGS");
    for (int i = 0; i < numArgs; i++)
    {
        _settings->setArrayIndex(i);
        if (_settings->value("arg").toString().isEmpty())
        {
            continue;
        }
        _ui->argsEditor->insertPlainText(_settings->value("arg").toString() + " ");
        _ui->argsEditor->moveCursor(QTextCursor::End);
    }
    _settings->endArray();

    // fill command browser
    _ui->cmdBrowser->setWordWrapMode(QTextOption::WrapAnywhere);
    this->updateCmd();

    // update command browser in real time when helios base directory, survey.xml or arguments change
    QObject::connect(_ui->heliosBaseDirLineEdit, &QLineEdit::textChanged, this, &Launcher::updateCmd);
    QObject::connect(_ui->surveyPathLineEdit, &QLineEdit::textChanged, this, &Launcher::updateCmd);
    QObject::connect(_ui->argsEditor, &QPlainTextEdit::textChanged, this, &Launcher::updateCmd);

    // set up process
    _process.setWorkingDirectory(_settings->value("DIRS/HeliosBaseDir").toString());
    _process.setProcessChannelMode(QProcess::MergedChannels);

// clang-format off
    // when changing the helios base directory, survey.xml or optional arguments, instantly save new values to settings
    QObject::connect(_ui->heliosBaseDirLineEdit, &QLineEdit::editingFinished, this, &Launcher::writeHeliosBaseDirToSettings);
    QObject::connect(_ui->surveyPathLineEdit, &QLineEdit::editingFinished, this, &Launcher::writeLastSurveyToSettings);

    // Buttons: Open File Dialogs to select helios base directory and survey.xml
    QObject::connect(_ui->heliosBaseDirButton, &QPushButton::clicked, this, [this]()
                    {
                        const QString heliosBaseDir = QFileDialog::getExistingDirectory(this, tr("Select HELIOS++ base directory"), _settings->value("DIRS/HeliosBaseDir").toString());
                        if (!heliosBaseDir.isEmpty())
                        {
                            _ui->heliosBaseDirLineEdit->setText(heliosBaseDir);
                            this->writeHeliosBaseDirToSettings();
                        }
                    });
    QObject::connect(_ui->surveyPathButton, &QPushButton::clicked, this, [this]()
                    {
                        const QString survey = QFileDialog::getOpenFileName(this, tr("Select survey XML file"), _settings->value("DIRS/HeliosBaseDir").toString() + "/data/surveys", tr("XML files (*.xml)"));
                        if (!survey.isEmpty())
                        {
                            _ui->surveyPathLineEdit->setText(survey);
                            this->writeLastSurveyToSettings();
                        }
                    });

    // Help button displays HELIOS++ main help
    QObject::connect(_ui->helpButton, &QPushButton::clicked, this, [this]()
                    {
#ifdef _WIN32
                        if (_ui->defaultModeButton->isChecked() && !_ui->heliospyModeButton->isChecked())
                        {
                            _process.start(_process.workingDirectory() + "/run/helios.exe", QStringList() << "--help");
                        }
                        else if (!_ui->defaultModeButton->isChecked() && _ui->heliospyModeButton->isChecked())
                        {
                            _process.start("python", QStringList() << _process.workingDirectory() + "/run/helios.py" << "--help");
                        }
                        else
                        {
                            QCoreApplication::exit(EXIT_FAILURE);
                        }
#else
                        // Set LD_LIBRARY_PATH to <heliosBaseDir>/run
                        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
                        env.insert("LD_LIBRARY_PATH", _process.workingDirectory() + "/run");
                        _process.setProcessEnvironment(env);
                        if (_ui->defaultModeButton->isChecked() && !_ui->heliospyModeButton->isChecked())
                        {
                            _process.start("run/helios", QStringList() << "--help");
                        }
                        else if (!_ui->defaultModeButton->isChecked() && _ui->heliospyModeButton->isChecked())
                        {
                            _process.start("python3", QStringList() << "run/helios.py" << "--help");
                        }
                        else
                        {
                            QCoreApplication::exit(EXIT_FAILURE);
                        }
#endif
                    });

    // Redirect console output of HELIOS++ to QTextBrowser
    QObject::connect(_ui->runButton, &QPushButton::clicked, this, &Launcher::startHeliospp);
    QObject::connect(&_process, &QProcess::readyReadStandardOutput, this, &Launcher::redirectStdout);
    QObject::connect(&_process, &QProcess::readyReadStandardError, this, &Launcher::redirectStderr);
    QObject::connect(&_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Launcher::exitHeliospp);

    // Kill running HELIOS++
    QObject::connect(_ui->cancelButton, &QPushButton::clicked, this, [this]()
                    {
                        if (_process.state() == QProcess::Running)
                        {
                           _process.kill();
                        }
                    });

    // Open HELIOS++ output directory in file explorer
    QObject::connect(_ui->openOutputDirButton, &QPushButton::clicked, this, [this]()
                    {
                        const QUrl outputDirUrl = QUrl::fromLocalFile(_outputDir);
                        QDesktopServices::openUrl(outputDirUrl);
                    });

    // Clear HELIOS++ output
    QObject::connect(_ui->clearButton, &QPushButton::clicked, this, [this]()
                    {
                        _ui->outputBrowser->clear();
                        _outputDir = _settings->value("DIRS/HeliosBaseDir").toString() + "/output";
                    });
// clang-format on
}

Launcher::~Launcher()
{
    // save settings on close
    this->writeHeliosBaseDirToSettings();
    this->writeLastSurveyToSettings();
    this->writeExecModeToSettings();
    auto args = _ui->argsEditor->toPlainText().split(QRegExp("[\\s\n]+"));
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

void Launcher::writeHeliosBaseDirToSettings()
{
#ifdef _WIN32
    _ui->heliosBaseDirLineEdit->setText(_ui->heliosBaseDirLineEdit->text().replace("/", "\\"));
#endif
    _settings->setValue("DIRS/HeliosBaseDir", _ui->heliosBaseDirLineEdit->text());
    _process.setWorkingDirectory(_ui->heliosBaseDirLineEdit->text());
}

void Launcher::writeLastSurveyToSettings()
{
#ifdef _WIN32
    _ui->surveyPathLineEdit->setText(_ui->surveyPathLineEdit->text().replace("/", "\\"));
#endif
    // when survey.xml is within helios base directory, use relative path
    if (_ui->surveyPathLineEdit->text().startsWith(_settings->value("DIRS/HeliosBaseDir").toString())
        && _ui->heliosBaseDirLineEdit->text() != ""
        && QDir(_ui->heliosBaseDirLineEdit->text()).exists())
    {
        _ui->surveyPathLineEdit->setText(_ui->surveyPathLineEdit->text().replace(_ui->heliosBaseDirLineEdit->text(), "."));
    }
    _settings->setValue("DIRS/LastSurvey", _ui->surveyPathLineEdit->text());
}

void Launcher::writeExecModeToSettings()
{
    if (_ui->defaultModeButton->isChecked())
    {
        _settings->setValue("MODE/ExecMode", "default");
    }
    else if (_ui->heliospyModeButton->isChecked())
    {
        _settings->setValue("MODE/ExecMode", "helios.py");
    }
    else
    {
        _settings->setValue("MODE/ExecMode", "");
    }
    this->updateCmd();
}

void Launcher::updateCmd()
{
    _ui->cmdBrowser->clear();
#ifdef _WIN32
    _ui->cmdBrowser->moveCursor(QTextCursor::End);
    _ui->cmdBrowser->insertPlainText(_ui->heliosBaseDirLineEdit->text().replace("/", "\\") + ">");
    if (_ui->defaultModeButton->isChecked() && !_ui->heliospyModeButton->isChecked())
    {
        _ui->cmdBrowser->insertPlainText("run\\helios ");
    }
    else if (!_ui->defaultModeButton->isChecked() && _ui->heliospyModeButton->isChecked())
    {
        _ui->cmdBrowser->insertPlainText("python run\\helios.py ");
    }
    _ui->cmdBrowser->insertPlainText(_ui->surveyPathLineEdit->text() + " " + _ui->argsEditor->toPlainText().replace("\n", " "));
#else
    QTextCharFormat fmt;
    fmt.setFontWeight(QFont::Bold);
    fmt.setForeground(Qt::green);
    _ui->cmdBrowser->moveCursor(QTextCursor::End);
    _ui->cmdBrowser->setCurrentCharFormat(fmt);
    _ui->cmdBrowser->insertPlainText(qgetenv("USERNAME") + "@" + QSysInfo::machineHostName());
    _ui->cmdBrowser->moveCursor(QTextCursor::End);
    fmt.setForeground(Qt::white);
    _ui->cmdBrowser->setCurrentCharFormat(fmt);
    _ui->cmdBrowser->insertPlainText(":");
    _ui->cmdBrowser->moveCursor(QTextCursor::End);
    fmt.setForeground(Qt::cyan);
    _ui->cmdBrowser->setCurrentCharFormat(fmt);
    _ui->cmdBrowser->insertPlainText(_ui->heliosBaseDirLineEdit->text());
    fmt.setForeground(Qt::white);
    fmt.setFontWeight(QFont::Normal);
    _ui->cmdBrowser->setCurrentCharFormat(fmt);
    if (_ui->defaultModeButton->isChecked() && !_ui->heliospyModeButton->isChecked())
    {
        _ui->cmdBrowser->insertPlainText("$ run/helios ");
    }
    else if (!_ui->defaultModeButton->isChecked() && _ui->heliospyModeButton->isChecked())
    {
        _ui->cmdBrowser->insertPlainText("$ python3 run/helios.py ");
    }
    else
    {
        _ui->cmdBrowser->insertPlainText("$ ");
    }
    _ui->cmdBrowser->insertPlainText(_ui->surveyPathLineEdit->text() + " " + _ui->argsEditor->toPlainText().replace("\n", " "));
#endif
}

void Launcher::startHeliospp()
{
    if (_settings->value("DIRS/HeliosBaseDir").isNull() || _settings->value("DIRS/HeliosBaseDir").toString() == "")
    {
        auto* baseDir = std::make_unique<BaseDir>(nullptr).release();
        if (baseDir->exec() == QDialog::Accepted)
        {
            _ui->heliosBaseDirLineEdit->setText(baseDir->getBaseDir());
            _settings->setValue("DIRS/HeliosBaseDir", baseDir->getBaseDir());
            _process.setWorkingDirectory(baseDir->getBaseDir());
        }
        else
        {
            return;
        }
    }
    // Read survey path from surveyPathLineEdit and optional arguments from argsEditor
    auto options = QStringList() << _ui->surveyPathLineEdit->text() << _ui->argsEditor->toPlainText().split(QRegExp("[ \n]"));
    options.removeAll("");

    // clear output
    if (!(options.contains("--help") || options.contains("-h") || options.contains("--version")))
    {
        _ui->outputBrowser->clear();
    }
#ifdef _WIN32
    if (_ui->defaultModeButton->isChecked() && !_ui->heliospyModeButton->isChecked())
    {
        _process.start(_process.workingDirectory() + "/run/helios.exe", options);
    }
    else if (!_ui->defaultModeButton->isChecked() && _ui->heliospyModeButton->isChecked())
    {
        _process.start("python", QStringList() << _process.workingDirectory() + "/run/helios.py" << options);
    }
    else
    {
        QCoreApplication::exit(EXIT_FAILURE);
    }
#else
    // Set LD_LIBRARY_PATH to <heliosBaseDir>/run
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LD_LIBRARY_PATH", _process.workingDirectory() + "/run");
    _process.setProcessEnvironment(env);
    if (_ui->defaultModeButton->isChecked() && !_ui->heliospyModeButton->isChecked())
    {
        _process.start("run/helios", options);
    }
    else if (!_ui->defaultModeButton->isChecked() && _ui->heliospyModeButton->isChecked())
    {
        _process.start("python3", QStringList() << "run/helios.py" << options);
    }
    else
    {
        QCoreApplication::exit(EXIT_FAILURE);
    }
#endif
}

void Launcher::redirectStdout()
{
    // when '-h' or '--help': redirect output to Help
    if (_process.arguments().contains("-h") || _process.arguments().contains("--help") || _process.arguments().isEmpty())
    {
        auto* const help = std::make_unique<Help>(_process.readAll(), this).release();
        help->show();
    }
    // when '--version': show version info in MessageBox
    else if (_process.arguments().contains("--version") && !_ui->heliospyModeButton->isChecked())
    {
        auto msg = QString(_process.readAll());
        QMessageBox::information(this, "HELIOS++ version info", msg);
        return;
    }
    else
    {
        // Append new output to QTextBrowser
        _ui->cmdBrowser->moveCursor(QTextCursor::End);
        QTextCharFormat fmt;
        fmt.setForeground(Qt::black);
        _ui->outputBrowser->setCurrentCharFormat(fmt);
        _ui->outputBrowser->insertPlainText(_process.readAllStandardOutput());
        _ui->cmdBrowser->moveCursor(QTextCursor::End);
        _ui->outputBrowser->update();
        // scroll down
        _ui->outputBrowser->verticalScrollBar()->setValue(_ui->outputBrowser->verticalScrollBar()->maximum());
    }
}

void Launcher::redirectStderr()
{
    _ui->cmdBrowser->moveCursor(QTextCursor::End);
    QTextCharFormat fmt;
    fmt.setForeground(Qt::red);
    _ui->outputBrowser->setCurrentCharFormat(fmt);
    _ui->outputBrowser->append(_process.readAllStandardError());
    _ui->cmdBrowser->moveCursor(QTextCursor::End);
    _ui->outputBrowser->update();
    // scroll down
    _ui->outputBrowser->verticalScrollBar()->setValue(_ui->outputBrowser->verticalScrollBar()->maximum());
}

void Launcher::exitHeliospp()
{
    if (!(_process.arguments().contains("--help") || _process.arguments().contains("-h") || _process.arguments().contains("--version")) && !_process.arguments().isEmpty())
    {
        auto exitCode = _process.exitCode();
        if (exitCode == 0)
        {
            _ui->outputBrowser->append("HELIOS++ exited successfully\n");
            // Extract output directory from HELIOS++ output
            QString relOutDir;
            for (auto& line : _ui->outputBrowser->toPlainText().split("\n"))
            {
                if (line.startsWith("Output directory: \"output/"))
                {
#ifdef _WIN32
                    relOutDir = line.split("/\\").at(1).left(line.split("/\\").at(1).length() - 1);
#else
                    relOutDir = line.split("//").at(1).left(line.split("//").at(1).length() - 1);
#endif
                    _outputDir = _settings->value("DIRS/HeliosBaseDir").toString() + "/output/" + relOutDir;
                    break;
                }
            }
        }
        else
        {
            QTextCharFormat fmt;
            fmt.setForeground(Qt::red);
            _ui->outputBrowser->setCurrentCharFormat(fmt);
            _ui->outputBrowser->append("HELIOS++ terminated with exit code " + QString::number(_process.exitCode()) + "\n");
            fmt.setForeground(Qt::black);
            _ui->outputBrowser->setCurrentCharFormat(fmt);
        }
        _ui->outputBrowser->verticalScrollBar()->setValue(_ui->outputBrowser->verticalScrollBar()->maximum());
    }
    _process.close();
}
