#include <QButtonGroup>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QThread>

#include "../ui/ui_launcher.h"
#include "basedir.h"
#include "help.h"
#include "launcher.h"

Launcher::Launcher(const std::string& version, QSettings* settings, QWidget* parent)
    : QMainWindow(parent),
      _ui(new Ui::Launcher),
      _settings(settings),
      _process(new QProcess(this)),
      _outputDir(_settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString() + "/output")
{
    _ui->setupUi(this);
    this->setWindowIcon(QIcon(":/heliospp.png"));
    this->setWindowTitle("HELIOS++ launcher version " + QString::fromStdString(version));
    _ui->settingsWidget->layout()->setAlignment(Qt::AlignTop);
    _ui->optionsTabs->setCurrentIndex(_settings->value(QStringLiteral("MISC/CurrentTab")).toInt());
#ifdef _WIN32
    _ui->assetsEdit->setPlaceholderText(_ui->assetsEdit->placeholderText().replace("/", "\\"));
    _ui->outputEdit->setPlaceholderText(_ui->outputEdit->placeholderText().replace("/", "\\"));
#endif

    // Some options are only available in default mode, not in helios.py mode (according to the "--help" outputs of both)
    QObject::connect(_ui->defaultModeButton, &QRadioButton::toggled, this, [this]()
                     {
                         if (_ui->heliospyModeButton->isChecked() && _ui->versionButton->isChecked())
                         {
                             _ui->runOptionButton->setChecked(true);
                         } });
    QObject::connect(_ui->heliospyModeButton, &QRadioButton::toggled, this, [this]()
                     { _ui->lasScaleSpinbox->setEnabled(!_ui->heliospyModeButton->isChecked() && _ui->lasScaleCheckbox->isChecked()); });
    QObject::connect(_ui->lasScaleCheckbox, &QCheckBox::toggled, this, [this]()
                     { _ui->lasScaleSpinbox->setEnabled(!_ui->heliospyModeButton->isChecked() && _ui->lasScaleCheckbox->isChecked()); });
    QObject::connect(_ui->heliospyModeButton, &QRadioButton::toggled, this, [this]()
                     { _ui->strategyWidget->setEnabled(!_ui->heliospyModeButton->isChecked() && _ui->strategyCheckbox->isChecked()); });
    QObject::connect(_ui->strategyCheckbox, &QCheckBox::toggled, this, [this]()
                     { _ui->strategyWidget->setEnabled(!_ui->heliospyModeButton->isChecked() && _ui->strategyCheckbox->isChecked()); });
    QObject::connect(_ui->heliospyModeButton, &QRadioButton::toggled, this, [this]()
                     { _ui->chunkSizeWidget->setEnabled(!_ui->heliospyModeButton->isChecked() && _ui->chunkSizeCheckbox->isChecked()); });
    QObject::connect(_ui->chunkSizeCheckbox, &QCheckBox::toggled, this, [this]()
                     { _ui->chunkSizeWidget->setEnabled(!_ui->heliospyModeButton->isChecked() && _ui->chunkSizeCheckbox->isChecked()); });
    QObject::connect(_ui->heliospyModeButton, &QRadioButton::toggled, this, [this]()
                     { _ui->warehouseFactorWidget->setEnabled(!_ui->heliospyModeButton->isChecked() && _ui->warehouseFactorCheckbox->isChecked()); });
    QObject::connect(_ui->warehouseFactorCheckbox, &QCheckBox::toggled, this, [this]()
                     { _ui->warehouseFactorWidget->setEnabled(!_ui->heliospyModeButton->isChecked() && _ui->warehouseFactorCheckbox->isChecked()); });
    QObject::connect(_ui->heliospyModeButton, &QRadioButton::toggled, this, [this]()
                     { _ui->gpsStartTimeEdit->setEnabled(!_ui->heliospyModeButton->isChecked() && _ui->gpsStartTimeCheckbox->isChecked()); });
    QObject::connect(_ui->gpsStartTimeCheckbox, &QCheckBox::toggled, this, [this]()
                     { _ui->gpsStartTimeEdit->setEnabled(!_ui->heliospyModeButton->isChecked() && _ui->gpsStartTimeCheckbox->isChecked()); });

    // restore helios base directory, last survey.xml, execution mode and optional arguments from settings
    _ui->heliosBaseDirLineEdit->setText(_settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString());
    _ui->surveyPathLineEdit->setText(_settings->value(QStringLiteral("DIRS/LastSurvey")).toString());
#ifdef _WIN32
    _ui->heliosBaseDirLineEdit->setText(_ui->heliosBaseDirLineEdit->text().replace("/", "\\"));
    _ui->surveyPathLineEdit->setText(_ui->surveyPathLineEdit->text().replace("/", "\\"));
#endif
    _ui->defaultModeButton->setChecked(_settings->value(QStringLiteral("MISC/ExecMode")).toString() == QStringLiteral("default"));
    _ui->heliospyModeButton->setChecked(_settings->value(QStringLiteral("MISC/ExecMode")).toString() == QStringLiteral("helios.py"));

    _ui->runOptionButton->setChecked(_settings->value(QStringLiteral("ARGS/General")) == "");
    _ui->helpButton->setChecked(_settings->value(QStringLiteral("ARGS/General")) == "--help");
    _ui->versionButton->setChecked(_settings->value(QStringLiteral("ARGS/General")) == "--version");
    _ui->testButton->setChecked(_settings->value(QStringLiteral("ARGS/General")) == "--test");

    _ui->asciiButton->setChecked(_settings->value(QStringLiteral("ARGS/Output")) == "");
    _ui->lasButton->setChecked(_settings->value(QStringLiteral("ARGS/Output")) == "--lasOutput");
    _ui->las10Button->setChecked(_settings->value(QStringLiteral("ARGS/Output")) == "--las10");

    _ui->zipOutputButton->setChecked(_settings->value(QStringLiteral("ARGS/ZipOutput")).toBool());
    if (_settings->value(QStringLiteral("ARGS/LasScale")).toString() == QStringLiteral("default"))
    {
        _ui->lasScaleCheckbox->setChecked(false);
    }
    else
    {
        _ui->lasScaleCheckbox->setChecked(true);
        _ui->lasScaleSpinbox->setValue(_settings->value(QStringLiteral("ARGS/LasScale")).toFloat());
    }
    if (_settings->value(QStringLiteral("ARGS/Parallelization")).toString() == QStringLiteral("default"))
    {
        _ui->strategyCheckbox->setChecked(false);
    }
    else
    {
        _ui->strategyCheckbox->setChecked(true);
        if (_settings->value(QStringLiteral("ARGS/Parallelization")).toInt() == 0)
        {
            _ui->staticDynamicChunkButton->setChecked(true);
            _ui->warehousButton->setChecked(false);
        }
        else if (_settings->value(QStringLiteral("ARGS/Parallelization")).toInt() == 1)
        {
            _ui->staticDynamicChunkButton->setChecked(false);
            _ui->warehousButton->setChecked(true);
        }
    }
    if (_settings->value(QStringLiteral("ARGS/nthreads")).toString() == QStringLiteral("default"))
    {
        _ui->nthreadCheckbox->setChecked(false);
    }
    else
    {
        _ui->nthreadCheckbox->setChecked(true);
        _ui->nthreadSlider->setValue(_settings->value(QStringLiteral("ARGS/nthreads")).toInt());
        _ui->nthreadSpinbox->setValue(_settings->value(QStringLiteral("ARGS/nthreads")).toInt());
    }
    if (_settings->value(QStringLiteral("ARGS/ChunkSize")).toString() == QStringLiteral("default"))
    {
        _ui->chunkSizeCheckbox->setChecked(false);
    }
    else
    {
        _ui->chunkSizeCheckbox->setChecked(true);
        _ui->chunkSizeSpinbox->setValue(_settings->value(QStringLiteral("ARGS/ChunkSize")).toInt());
    }
    if (_settings->value(QStringLiteral("ARGS/WarehouseFactor")).toString() == QStringLiteral("default"))
    {
        _ui->warehouseFactorCheckbox->setChecked(false);
    }
    else
    {
        _ui->warehouseFactorCheckbox->setChecked(true);
        _ui->warehouseFactorSpinbox->setValue(_settings->value(QStringLiteral("ARGS/WarehouseFactor")).toInt());
    }
    _ui->splitByChannelCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/SplitByChannel")).toBool());
    _ui->writeWaveformCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/WriteWaveform")).toBool());
    _ui->calcEchoWidthCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/CalcEchoWidth")).toBool());
    _ui->fullwaveNoiseCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/FullwaveNoise")).toBool());
    _ui->fixedIncidenceAngleCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/FixedIncidenceAngle")).toBool());
    _ui->disablePlatformNoiseCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/DisablePlatformNoise")).toBool());
    _ui->disableLegNoiseCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/DisableLegNoise")).toBool());
    if (_settings->value(QStringLiteral("ARGS/Seed")).toString() == QStringLiteral("default"))
    {
        _ui->seedCheckbox->setChecked(false);
    }
    else
    {
        _ui->seedCheckbox->setChecked(true);
        _ui->seedEdit->setText(_settings->value(QStringLiteral("ARGS/Seed")).toString());
    }
    if (_settings->value(QStringLiteral("ARGS/GpsStartTime")).toString() == QStringLiteral("default"))
    {
        _ui->gpsStartTimeCheckbox->setChecked(false);
    }
    else
    {
        _ui->gpsStartTimeCheckbox->setChecked(true);
        _ui->gpsStartTimeEdit->setText(_settings->value(QStringLiteral("ARGS/GpsStartTime")).toString());
    }
    _ui->logFileCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/LogFile")).toBool());
    _ui->logFileOnlyCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/LogFileOnly")).toBool());
    _ui->silentCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/Silent")).toBool());
    _ui->quietCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/Quiet")).toBool());
    _ui->vtCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/Vt")).toBool());
    _ui->vCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/V")).toBool());
    _ui->vvCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/VV")).toBool());
    _ui->rebuildSceneCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/RebuildScene")).toBool());
    if (_settings->value(QStringLiteral("ARGS/KDTree")).toString() == QStringLiteral("default"))
    {
        _ui->kdtTypeCheckbox->setChecked(false);
    }
    else
    {
        _ui->kdtTypeCheckbox->setChecked(true);
        if (_settings->value(QStringLiteral("ARGS/KDTree")).toInt() == 1)
        {
            _ui->medianBalancingButton->setChecked(true);
            _ui->SAHButton->setChecked(false);
            _ui->SAHbestAxisButton->setChecked(false);
            _ui->fastSAHButton->setChecked(false);
        }
        else if (_settings->value(QStringLiteral("ARGS/KDTree")).toInt() == 2)
        {
            _ui->medianBalancingButton->setChecked(false);
            _ui->SAHButton->setChecked(true);
            _ui->SAHbestAxisButton->setChecked(false);
            _ui->fastSAHButton->setChecked(false);
        }
        else if (_settings->value(QStringLiteral("ARGS/KDTree")).toInt() == 3)
        {
            _ui->medianBalancingButton->setChecked(false);
            _ui->SAHButton->setChecked(false);
            _ui->SAHbestAxisButton->setChecked(true);
            _ui->fastSAHButton->setChecked(false);
        }
        else if (_settings->value(QStringLiteral("ARGS/KDTree")).toInt() == 4)
        {
            _ui->medianBalancingButton->setChecked(false);
            _ui->SAHButton->setChecked(false);
            _ui->SAHbestAxisButton->setChecked(false);
            _ui->fastSAHButton->setChecked(true);
        }
    }
    if (_settings->value(QStringLiteral("ARGS/KDTreeThreads")).toString() == QStringLiteral("default"))
    {
        _ui->kdtThreadsCheckbox->setChecked(false);
    }
    else
    {
        _ui->kdtThreadsCheckbox->setChecked(true);
        _ui->kdtThreadsSpinbox->setValue(_settings->value(QStringLiteral("ARGS/KDTreeThreads")).toInt());
    }
    if (_settings->value(QStringLiteral("ARGS/KDTreeGeomThreads")).toString() == QStringLiteral("default"))
    {
        _ui->kdtGeomThreadsCheckbox->setChecked(false);
    }
    else
    {
        _ui->kdtGeomThreadsCheckbox->setChecked(true);
        _ui->kdtGeomThreadsSpinbox->setValue(_settings->value(QStringLiteral("ARGS/KDTreeGeomThreads")).toInt());
    }
    if (_settings->value(QStringLiteral("ARGS/SAHnodes")).toString() == QStringLiteral("default"))
    {
        _ui->SAHnodesCheckbox->setChecked(false);
    }
    else
    {
        _ui->SAHnodesCheckbox->setChecked(true);
        _ui->SAHnodesSpinbox->setValue(_settings->value(QStringLiteral("ARGS/SAHnodes")).toInt());
    }
    _ui->unzipCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/Unzip")).toBool());
    _ui->unzipInputEdit->setText(_settings->value(QStringLiteral("ARGS/UnzipInput")).toString());
    _ui->unzipOutputEdit->setText(_settings->value(QStringLiteral("ARGS/UnzipOutput")).toString());
#ifdef _WIN32
    _ui->unzipInputEdit->setText(_ui->unzipInputEdit->text().replace("/", "\\"));
    _ui->unzipOutputEdit->setText(_ui->unzipOutputEdit->text().replace("/", "\\"));
#endif
    _ui->assetsCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/AssetsPathFlag")).toBool());
    _ui->assetsEdit->setText(_settings->value(QStringLiteral("ARGS/AssetsPath")).toString());
    _ui->outputCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/OutputPathFlag")).toBool());
    _ui->outputEdit->setText(_settings->value(QStringLiteral("ARGS/OutputPath")).toString());
    _ui->liveTrajectoryPlotCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/LiveTrajectoryPlot")).toBool());
    _ui->polyscopeCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/Polyscope")).toBool());
    _ui->open3dCheckbox->setChecked(_settings->value(QStringLiteral("ARGS/Open3D")).toBool());

    _ui->argsEditor->setPlainText(_settings->value(QStringLiteral("ARGS/ArgsEditorString")).toString());

    _ui->sourceGuiButton->setChecked(_settings->value(QStringLiteral("MISC/ArgsSource")).toString() == QStringLiteral("GUI"));
    _ui->sourceArgEditorButton->setChecked(_settings->value(QStringLiteral("MISC/ArgsSource")).toString() == QStringLiteral("ArgsEditor"));

    // disable options when unchecked
    _ui->heliospyBox->setDisabled(_ui->defaultModeButton->isChecked());
    _ui->lasScaleSpinbox->setDisabled(!_ui->lasScaleCheckbox->isChecked());
    _ui->strategyWidget->setDisabled(!_ui->strategyCheckbox->isChecked());
    _ui->nthreadWidget->setDisabled(!_ui->nthreadCheckbox->isChecked());
    _ui->chunkSizeWidget->setDisabled(!_ui->chunkSizeCheckbox->isChecked());
    _ui->warehouseFactorWidget->setDisabled(!_ui->warehouseFactorCheckbox->isChecked());
    _ui->seedEdit->setDisabled(!_ui->seedCheckbox->isChecked());
    _ui->gpsStartTimeEdit->setDisabled(!_ui->gpsStartTimeCheckbox->isChecked());
    _ui->kdtTypeWidget->setDisabled(!_ui->kdtTypeCheckbox->isChecked());
    _ui->kdtThreadsWidget->setDisabled(!_ui->kdtThreadsCheckbox->isChecked());
    _ui->kdtGeomThreadsWidget->setDisabled(!_ui->kdtGeomThreadsCheckbox->isChecked());
    _ui->SAHnodesWidget->setDisabled(!_ui->SAHnodesCheckbox->isChecked());
    _ui->unzipInputLabel->setDisabled(!_ui->unzipCheckbox->isChecked());
    _ui->unzipOutputLabel->setDisabled(!_ui->unzipCheckbox->isChecked());
    _ui->unzipInputEdit->setDisabled(!_ui->unzipCheckbox->isChecked());
    _ui->unzipOutputEdit->setDisabled(!_ui->unzipCheckbox->isChecked());
    _ui->unzipInputBrowseButton->setDisabled(!_ui->unzipCheckbox->isChecked());
    _ui->unzipOutputBrowseButton->setDisabled(!_ui->unzipCheckbox->isChecked());
    _ui->assetsEdit->setDisabled(!_ui->assetsCheckbox->isChecked());
    _ui->assetsBrowseButton->setDisabled(!_ui->assetsCheckbox->isChecked());
    _ui->outputEdit->setDisabled(!_ui->outputCheckbox->isChecked());
    _ui->outputBrowseButton->setDisabled(!_ui->outputCheckbox->isChecked());

    // fill command browser
    _ui->outputBrowser->setWordWrapMode(QTextOption::WrapAnywhere);
    _ui->cmdBrowser->setWordWrapMode(QTextOption::WrapAnywhere);
    this->updateCmd();

    // set up process
    _process.setWorkingDirectory(_settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString());
    _process.setProcessChannelMode(QProcess::MergedChannels);

    // set thread settings maximum to number of available threads
    auto numThreads = QThread::idealThreadCount();
    _ui->nthreadSlider->setMaximum(numThreads);
    _ui->nThreadMaxLabel->setText(QString::number(numThreads));
    _ui->nthreadSpinbox->setMaximum(numThreads);
    _ui->kdtThreadsSlider->setMaximum(numThreads);
    _ui->kdtThreadsMaxLabel->setText(QString::number(numThreads));
    _ui->kdtThreadsSpinbox->setMaximum(numThreads);
    _ui->kdtGeomThreadsSlider->setMaximum(numThreads);
    _ui->kdtGeomThreadsMaxLabel->setText(QString::number(numThreads));
    _ui->kdtGeomThreadsSpinbox->setMaximum(numThreads);

    // clang-format off

    // when changing options, write to settings in real time
    QObject::connect(_ui->heliosBaseDirLineEdit, &QLineEdit::textChanged, this, &Launcher::writeHeliosBaseDirToSettings);
    QObject::connect(_ui->surveyPathLineEdit, &QLineEdit::textChanged, this, &Launcher::writeLastSurveyToSettings);
    QObject::connect(_ui->defaultModeButton, &QRadioButton::toggled, this, &Launcher::writeExecModeToSettings);
    QObject::connect(_ui->heliospyModeButton, &QRadioButton::toggled, this, &Launcher::writeExecModeToSettings);
    QObject::connect(_ui->runOptionButton, &QRadioButton::toggled, this, &Launcher::writeGeneralToSettings);
    QObject::connect(_ui->helpButton, &QRadioButton::toggled, this, &Launcher::writeGeneralToSettings);
    QObject::connect(_ui->versionButton, &QRadioButton::toggled, this, &Launcher::writeGeneralToSettings);
    QObject::connect(_ui->testButton, &QRadioButton::toggled, this, &Launcher::writeGeneralToSettings);
    QObject::connect(_ui->asciiButton, &QRadioButton::toggled, this, &Launcher::writeOutputToSettings);
    QObject::connect(_ui->lasButton, &QRadioButton::toggled, this, &Launcher::writeOutputToSettings);
    QObject::connect(_ui->las10Button, &QRadioButton::toggled, this, &Launcher::writeOutputToSettings);
    QObject::connect(_ui->zipOutputButton, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/ZipOutput"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->lasScaleCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/LasScale"), checked ? QString::number(_ui->lasScaleSpinbox->value()) : QStringLiteral("default"));
                        this->updateCmd();
                    });
    QObject::connect(_ui->lasScaleSpinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](float value)
                    {
                        _settings->setValue(QStringLiteral("ARGS/LasScale"), QString::number(value));
                        this->updateCmd();
                    });
    QObject::connect(_ui->strategyCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        if (checked)
                        {
                            _settings->setValue(QStringLiteral("ARGS/Parallelization"), _ui->staticDynamicChunkButton->isChecked() ? "0" : "1");
                        }
                        else
                        {
                            _settings->setValue(QStringLiteral("ARGS/Parallelization"), "default");
                        }
                        this->updateCmd();
                    });
    QObject::connect(_ui->staticDynamicChunkButton, &QRadioButton::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/Parallelization"), checked ? "0" : "1");
                        this->updateCmd();
                    });
    QObject::connect(_ui->warehousButton, &QRadioButton::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/Parallelization"), checked ? "1" : "0");
                        this->updateCmd();
                    });
    QObject::connect(_ui->nthreadCheckbox, &QCheckBox::toggled, this, &Launcher::writeNThreadsToSettings);
    QObject::connect(_ui->nthreadSlider, &QSlider::valueChanged, this, &Launcher::writeNThreadsToSettings);
    QObject::connect(_ui->nthreadSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Launcher::writeNThreadsToSettings);
    QObject::connect(_ui->chunkSizeCheckbox, &QCheckBox::toggled, this, &Launcher::writeChunkSizeToSettings);
    QObject::connect(_ui->chunkSizeSlider, &QSlider::valueChanged, this, &Launcher::writeChunkSizeToSettings);
    QObject::connect(_ui->chunkSizeSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Launcher::writeChunkSizeToSettings);
    QObject::connect(_ui->warehouseFactorCheckbox, &QCheckBox::toggled, this, &Launcher::writeWarehouseFactorToSettings);
    QObject::connect(_ui->warehouseFactorSlider, &QSlider::valueChanged, this, &Launcher::writeWarehouseFactorToSettings);
    QObject::connect(_ui->warehouseFactorSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Launcher::writeWarehouseFactorToSettings);
    QObject::connect(_ui->splitByChannelCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/SplitByChannel"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->writeWaveformCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/WriteWaveform"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->calcEchoWidthCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/CalcEchoWidth"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->fullwaveNoiseCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/FullwaveNoise"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->fixedIncidenceAngleCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/FixedIncidenceAngle"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->disablePlatformNoiseCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/DisablePlatformNoise"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->disableLegNoiseCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/DisableLegNoise"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->seedCheckbox, &QCheckBox::toggled, this, &Launcher::writeSeedToSettings);
    QObject::connect(_ui->seedEdit, &QLineEdit::textChanged, this, &Launcher::writeSeedToSettings);
    QObject::connect(_ui->gpsStartTimeCheckbox, &QCheckBox::toggled, this, &Launcher::writeGpsStartTimeToSettings);
    QObject::connect(_ui->gpsStartTimeEdit, &QLineEdit::textChanged, this, &Launcher::writeGpsStartTimeToSettings);
    QObject::connect(_ui->logFileCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/LogFile"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->logFileOnlyCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/LogFileOnly"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->silentCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/Silent"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->quietCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/Quiet"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->vtCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/Vt"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->vCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/V"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->vvCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/VV"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->rebuildSceneCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/RebuildScene"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->kdtTypeCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        if (checked)
                        {
                            if (_ui->medianBalancingButton->isChecked())
                            {
                                _settings->setValue(QStringLiteral("ARGS/KDTree"), 1);
                            }
                            else if (_ui->SAHButton->isChecked())
                            {
                                _settings->setValue(QStringLiteral("ARGS/KDTree"), 2);
                            }
                            else if (_ui->SAHbestAxisButton->isChecked())
                            {
                                _settings->setValue(QStringLiteral("ARGS/KDTree"), 3);
                            }
                            else if (_ui->fastSAHButton->isChecked())
                            {
                                _settings->setValue(QStringLiteral("ARGS/KDTree"), 4);
                            }
                        }
                        else
                        {
                            _settings->setValue(QStringLiteral("ARGS/KDTree"), "default");
                        }
                        this->updateCmd();
                    });
    QObject::connect(_ui->medianBalancingButton, &QRadioButton::toggled, this, [this](bool checked)
                    {
                        if (checked)
                        {
                            _settings->setValue(QStringLiteral("ARGS/KDTree"), 1);
                        }
                        this->updateCmd();
                    });
    QObject::connect(_ui->SAHButton, &QRadioButton::toggled, this, [this](bool checked)
                    {
                        if (checked)
                        {
                            _settings->setValue(QStringLiteral("ARGS/KDTree"), 2);
                        }
                        this->updateCmd();
                    });
    QObject::connect(_ui->SAHbestAxisButton, &QRadioButton::toggled, this, [this](bool checked)
                    {
                        if (checked)
                        {
                            _settings->setValue(QStringLiteral("ARGS/KDTree"), 3);
                        }
                        this->updateCmd();
                    });
    QObject::connect(_ui->fastSAHButton, &QRadioButton::toggled, this, [this](bool checked)
                    {
                        if (checked)
                        {
                            _settings->setValue(QStringLiteral("ARGS/KDTree"), 4);
                        }
                        this->updateCmd();
                    });
    QObject::connect(_ui->kdtThreadsCheckbox, &QCheckBox::toggled, this, &Launcher::writeKDTreeThreadsToSettings);
    QObject::connect(_ui->kdtThreadsSlider, &QSlider::valueChanged, this, &Launcher::writeKDTreeThreadsToSettings);
    QObject::connect(_ui->kdtThreadsSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Launcher::writeKDTreeThreadsToSettings);
    QObject::connect(_ui->kdtGeomThreadsCheckbox, &QCheckBox::toggled, this, &Launcher::writeKDTreeGeomThreadsToSettings);
    QObject::connect(_ui->kdtGeomThreadsSlider, &QSlider::valueChanged, this, &Launcher::writeKDTreeGeomThreadsToSettings);
    QObject::connect(_ui->kdtGeomThreadsSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Launcher::writeKDTreeGeomThreadsToSettings);
    QObject::connect(_ui->SAHnodesCheckbox, &QCheckBox::toggled, this, &Launcher::writeSAHnodesToSettings);
    QObject::connect(_ui->SAHnodesSlider, &QSlider::valueChanged, this, &Launcher::writeSAHnodesToSettings);
    QObject::connect(_ui->SAHnodesSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Launcher::writeSAHnodesToSettings);
    QObject::connect(_ui->unzipCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/Unzip"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->unzipInputEdit, &QLineEdit::textChanged, this, [this]()
                    {
#ifdef _WIN32
                        _ui->unzipInputEdit->setText(_ui->unzipInputEdit->text().replace("/", "\\"));
#endif
                        if (_ui->unzipInputEdit->text().startsWith(_ui->heliosBaseDirLineEdit->text()))
                        {
                            _ui->unzipInputEdit->setText(_ui->unzipInputEdit->text().replace(_ui->heliosBaseDirLineEdit->text(), QStringLiteral(".")));
                        }
                        _settings->setValue(QStringLiteral("ARGS/UnzipInput"), _ui->unzipInputEdit->text());
                        this->updateCmd();
                    });
    QObject::connect(_ui->unzipOutputEdit, &QLineEdit::textChanged, this, [this]()
                    {
#ifdef _WIN32
                        _ui->unzipOutputEdit->setText(_ui->unzipOutputEdit->text().replace("/", "\\"));
#endif
                        if (_ui->unzipOutputEdit->text().startsWith(_ui->heliosBaseDirLineEdit->text()))
                        {
                            _ui->unzipOutputEdit->setText(_ui->unzipOutputEdit->text().replace(_ui->heliosBaseDirLineEdit->text(), QStringLiteral(".")));
                        }
                        _settings->setValue(QStringLiteral("ARGS/UnzipOutput"), _ui->unzipOutputEdit->text());
                        this->updateCmd();
                    });
    QObject::connect(_ui->assetsCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/AssetsPathFlag"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->assetsEdit, &QLineEdit::textChanged, this, [this]()
                    {
#ifdef _WIN32
                        _ui->assetsEdit->setText(_ui->assetsEdit->text().replace("/", "\\"));
#endif
                        if (_ui->assetsEdit->text().startsWith(_ui->heliosBaseDirLineEdit->text()))
                        {
                            _ui->assetsEdit->setText(_ui->assetsEdit->text().replace(_ui->heliosBaseDirLineEdit->text(), QStringLiteral(".")));
                        }
                        _settings->setValue(QStringLiteral("ARGS/AssetsPath"), _ui->assetsEdit->text());
                        this->updateCmd();
                    });
    QObject::connect(_ui->outputCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/OutputPathFlag"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->outputEdit, &QLineEdit::textChanged, this, [this]()
                    {
#ifdef _WIN32
                        _ui->outputEdit->setText(_ui->outputEdit->text().replace("/", "\\"));
#endif
                        if (_ui->outputEdit->text().startsWith(_ui->heliosBaseDirLineEdit->text()))
                        {
                            _ui->outputEdit->setText(_ui->outputEdit->text().replace(_ui->heliosBaseDirLineEdit->text(), QStringLiteral(".")));
                        }
                        _settings->setValue(QStringLiteral("ARGS/OutputPath"), _ui->outputEdit->text());
                        this->updateCmd();
                    });
    QObject::connect(_ui->liveTrajectoryPlotCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/LiveTrajectoryPlot"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->polyscopeCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/Polyscope"), checked);
                        this->updateCmd();
                    });
    QObject::connect(_ui->open3dCheckbox, &QCheckBox::toggled, this, [this](bool checked)
                    {
                        _settings->setValue(QStringLiteral("ARGS/Open3D"), checked);
                        this->updateCmd();
                    });

    QObject::connect(_ui->sourceGuiButton, &QRadioButton::toggled, this, [this](bool checked)
                    {
                        if (checked)
                        {
                            _settings->setValue(QStringLiteral("MISC/ArgsSource"), "GUI");
                        }
                        this->updateCmd();
                    });
    QObject::connect(_ui->sourceArgEditorButton, &QRadioButton::toggled, this, [this](bool checked)
                    {
                        if (checked)
                        {
                            _settings->setValue(QStringLiteral("MISC/ArgsSource"), "ArgsEditor");
                        }
                        this->updateCmd();
                    });

    // update command browser in real time when helios base directory, survey.xml or arguments change
    QObject::connect(_ui->heliosBaseDirLineEdit, &QLineEdit::textChanged, this, &Launcher::updateCmd);
    QObject::connect(_ui->surveyPathLineEdit, &QLineEdit::textChanged, this, &Launcher::updateCmd);
    QObject::connect(_ui->argsEditor, &QPlainTextEdit::textChanged, this, [this]
                    {
                        _settings->setValue(QStringLiteral("ARGS/ArgsEditorString"), _ui->argsEditor->toPlainText());
                        this->updateCmd();
                    });

    QObject::connect(_ui->optionsTabs, &QTabWidget::currentChanged, this, [this](int index)
                    {
                        _settings->setValue(QStringLiteral("MISC/CurrentTab"), index);
                    });

    // Button functions

    // Buttons: Open File Dialogs to select helios base directory and survey.xml
    QObject::connect(_ui->heliosBaseDirButton, &QPushButton::clicked, this, [this]()
                    {
                        const QString heliosBaseDir = QFileDialog::getExistingDirectory(this, tr("Select HELIOS++ base directory"), _settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString());
                        if (!heliosBaseDir.isEmpty())
                        {
                            _ui->heliosBaseDirLineEdit->setText(heliosBaseDir);
                            this->writeHeliosBaseDirToSettings();
                        }
                    });
    QObject::connect(_ui->surveyPathButton, &QPushButton::clicked, this, [this]()
                    {
                        const QString survey = QFileDialog::getOpenFileName(this, tr("Select survey XML file"), _settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString() + "/data/surveys", tr("XML files (*.xml)"));
                        if (!survey.isEmpty())
                        {
                            _ui->surveyPathLineEdit->setText(survey);
                            this->writeLastSurveyToSettings();
                        }
                    });

    // browse buttons @ path arguments section
    QObject::connect(_ui->unzipInputBrowseButton, &QPushButton::clicked, this, [this]()
                    {
                        QString last = _settings->value(QStringLiteral("ARGS/UnzipInput")).toString();
                        if (last.isEmpty())
                        {
                            if (QDir(_outputDir).exists())
                            {
                                last = _outputDir;
                            }
                            else
                            {
                                last = _settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString();
                            }
                        }
                        else if (last.startsWith(QStringLiteral(".")))
                        {
                            last.remove(0, 1);
                            last = _settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString() + last;
                        }
                        if (!QFile(last).exists())
                        {
                            last = _settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString();
                        }
                        QString unzipInputFile = QFileDialog::getOpenFileName(this, tr("Select file to decompress"), last);
                        if (!unzipInputFile.isEmpty())
                        {
                            if (unzipInputFile.startsWith(_settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString()))
                            {
                                unzipInputFile.replace(_settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString(), QStringLiteral("."));
                            }
                            _ui->unzipInputEdit->setText(unzipInputFile);
                            _settings->setValue(QStringLiteral("ARGS/UnzipInput"), _ui->unzipInputEdit->text());
                        }
                        this->updateCmd();
                    });
        QObject::connect(_ui->unzipOutputBrowseButton, &QPushButton::clicked, this, [this]()
                    {
                        QString last = _settings->value(QStringLiteral("ARGS/UnzipOutput")).toString();
                        if (last.isEmpty())
                        {
                            if (_ui->unzipInputEdit->text().isEmpty())
                            {
                                last = _settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString();
                            }
                            else
                            {
                                last = QFileInfo(_ui->unzipInputEdit->text()).path();
                            }
                        }
                        if (last.startsWith(QStringLiteral(".")))
                        {
                            last.remove(0, 1);
                            last = _settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString() + last;
                        }
                        QString unzipOutputFile = QFileDialog::getSaveFileName(this, tr("Specify target folder for decompression"), last);
                        if (!unzipOutputFile.isEmpty())
                        {
                            if (unzipOutputFile.startsWith(_settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString()))
                            {
                                unzipOutputFile.replace(_settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString(), QStringLiteral("."));
                            }
                            _ui->unzipOutputEdit->setText(unzipOutputFile);
                            _settings->setValue(QStringLiteral("ARGS/UnzipOutput"), _ui->unzipOutputEdit->text());
                        }
                        this->updateCmd();
                    });
        QObject::connect(_ui->assetsBrowseButton, &QPushButton::clicked, this, [this]()
                    {
                        QString last = _settings->value(QStringLiteral("ARGS/AssetsPath")).toString();
                        if (last.isEmpty())
                        {
                            last = _settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString() + "/assets/";
                        }
                        else if (last.startsWith(QStringLiteral(".")))
                        {
                            last.remove(0, 1);
                            last = _settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString() + last;
                        }
                        if (!QDir(last).exists())
                        {
                            last = _settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString();
                        }
                        QString assetsPath = QFileDialog::getExistingDirectory(this, tr("Select assets path"), last);
                        if (!assetsPath.isEmpty())
                        {
                            if (assetsPath.startsWith(_settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString()))
                            {
                                assetsPath.replace(_settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString(), QStringLiteral("."));
                            }
                            _ui->assetsEdit->setText(assetsPath);
                            _settings->setValue(QStringLiteral("ARGS/AssetsPath"), _ui->assetsEdit->text());
                        }
                        this->updateCmd();
                    });
        QObject::connect(_ui->outputBrowseButton, &QPushButton::clicked, this, [this]()
                    {
                        QString last = _settings->value(QStringLiteral("ARGS/OutputPath")).toString();
                        if (last.isEmpty())
                        {
                            last = _settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString() + "/output/";
                        }
                        else if (last.startsWith(QStringLiteral(".")))
                        {
                            last.remove(0, 1);
                            last = _settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString() + last;
                        }
                        if (!QDir(last).exists())
                        {
                            last = _settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString();
                        }
                        QString outputPath = QFileDialog::getExistingDirectory(this, tr("Select output path"), last);
                        if (!outputPath.isEmpty())
                        {
                            if (outputPath.startsWith(_settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString()))
                            {
                                outputPath.replace(_settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString(), QStringLiteral("."));
                            }
                            _ui->outputEdit->setText(outputPath);
                            _settings->setValue(QStringLiteral("ARGS/OutputPath"), _ui->outputEdit->text());
                        }
                        this->updateCmd();
                    });

    // Help button displays HELIOS++ main help
    QObject::connect(_ui->showHelpButton, &QPushButton::clicked, this, [this]()
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
                        env.insert(QStringLiteral("LD_LIBRARY_PATH"), _process.workingDirectory() + "/run");
                        _process.setProcessEnvironment(env);
                        if (_ui->defaultModeButton->isChecked() && !_ui->heliospyModeButton->isChecked())
                        {
                            _process.start(QStringLiteral("run/helios"), QStringList() << QStringLiteral("--help"));
                        }
                        else if (!_ui->defaultModeButton->isChecked() && _ui->heliospyModeButton->isChecked())
                        {
                            _process.start(QStringLiteral("python3"), QStringList() << QStringLiteral("run/helios.py") << QStringLiteral("--help"));
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
                        _outputDir = _settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString() + "/output";
                    });
    // clang-format on
}

Launcher::~Launcher()
{
    // save settings on close
    this->writeHeliosBaseDirToSettings();
    this->writeLastSurveyToSettings();
    this->writeExecModeToSettings();
    this->writeGeneralToSettings();
    this->writeOutputToSettings();
    _settings->setValue(QStringLiteral("ARGS/ZipOutput"), _ui->zipOutputButton->isChecked());
    if (!_ui->lasScaleCheckbox->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/LasScale"), "default");
    }
    else
    {
        _settings->setValue(QStringLiteral("ARGS/LasScale"), _ui->lasScaleSpinbox->value());
    }
    if (!_ui->strategyCheckbox->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/Parallelization"), "default");
    }
    else
    {
        _settings->setValue(QStringLiteral("ARGS/Parallelization"), _ui->staticDynamicChunkButton->isChecked() ? "0" : "1");
    }
    this->writeNThreadsToSettings();
    this->writeChunkSizeToSettings();
    this->writeWarehouseFactorToSettings();
    _settings->setValue(QStringLiteral("ARGS/SplitByChannel"), _ui->splitByChannelCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/WriteWaveform"), _ui->writeWaveformCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/CalcEchoWidth"), _ui->calcEchoWidthCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/FullwaveNoise"), _ui->fullwaveNoiseCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/FixedIncidenceAngle"), _ui->fixedIncidenceAngleCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/DisablePlatformNoise"), _ui->disablePlatformNoiseCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/DisableLegNoise"), _ui->disableLegNoiseCheckbox->isChecked());
    this->writeSeedToSettings();
    this->writeGpsStartTimeToSettings();
    _settings->setValue(QStringLiteral("ARGS/LogFile"), _ui->logFileCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/LogFileOnly"), _ui->logFileOnlyCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/Silent"), _ui->silentCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/Quiet"), _ui->quietCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/Vt"), _ui->vtCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/V"), _ui->vCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/VV"), _ui->vvCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/RebuildScene"), _ui->rebuildSceneCheckbox->isChecked());
    if (!_ui->kdtTypeCheckbox->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/KDTree"), "default");
    }
    else
    {
        if (_ui->medianBalancingButton->isChecked())
        {
            _settings->setValue(QStringLiteral("ARGS/KDTree"), 1);
        }
        else if (_ui->SAHButton->isChecked())
        {
            _settings->setValue(QStringLiteral("ARGS/KDTree"), 2);
        }
        else if (_ui->SAHbestAxisButton->isChecked())
        {
            _settings->setValue(QStringLiteral("ARGS/KDTree"), 3);
        }
        else if (_ui->fastSAHButton->isChecked())
        {
            _settings->setValue(QStringLiteral("ARGS/KDTree"), 4);
        }
    }
    this->writeKDTreeThreadsToSettings();
    this->writeKDTreeGeomThreadsToSettings();
    this->writeSAHnodesToSettings();
    _settings->setValue(QStringLiteral("ARGS/Unzip"), _ui->unzipCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/UnzipInput"), _ui->unzipInputEdit->text());
    _settings->setValue(QStringLiteral("ARGS/UnzipOutput"), _ui->unzipOutputEdit->text());
    _settings->setValue(QStringLiteral("ARGS/AssetsPathFlag"), _ui->assetsCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/AssetsPath"), _ui->assetsEdit->text());
    _settings->setValue(QStringLiteral("ARGS/OutputPathFlag"), _ui->outputCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/OutputPath"), _ui->outputEdit->text());
    _settings->setValue(QStringLiteral("ARGS/LiveTrajectoryPlot"), _ui->liveTrajectoryPlotCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/Polyscope"), _ui->polyscopeCheckbox->isChecked());
    _settings->setValue(QStringLiteral("ARGS/Open3D"), _ui->open3dCheckbox->isChecked());
    _process.close();
    delete _ui;
}

void Launcher::writeHeliosBaseDirToSettings()
{
#ifdef _WIN32
    _ui->heliosBaseDirLineEdit->setText(_ui->heliosBaseDirLineEdit->text().replace("/", "\\"));
#endif
    _settings->setValue(QStringLiteral("DIRS/HeliosBaseDir"), _ui->heliosBaseDirLineEdit->text());
    _process.setWorkingDirectory(_ui->heliosBaseDirLineEdit->text());
}

void Launcher::writeLastSurveyToSettings()
{
#ifdef _WIN32
    _ui->surveyPathLineEdit->setText(_ui->surveyPathLineEdit->text().replace("/", "\\"));
#endif
    // when survey.xml is within helios base directory, use relative path
    if (_ui->surveyPathLineEdit->text().startsWith(_ui->heliosBaseDirLineEdit->text()) && _ui->heliosBaseDirLineEdit->text() != QLatin1String("") && QDir(_ui->heliosBaseDirLineEdit->text()).exists())
    {
        _ui->surveyPathLineEdit->setText(_ui->surveyPathLineEdit->text().replace(_ui->heliosBaseDirLineEdit->text(), QStringLiteral(".")));
    }
    _settings->setValue(QStringLiteral("DIRS/LastSurvey"), _ui->surveyPathLineEdit->text());
}

void Launcher::writeExecModeToSettings()
{
    if (_ui->defaultModeButton->isChecked())
    {
        _settings->setValue(QStringLiteral("MISC/ExecMode"), "default");
    }
    else if (_ui->heliospyModeButton->isChecked())
    {
        _settings->setValue(QStringLiteral("MISC/ExecMode"), "helios.py");
    }
    else
    {
        _settings->setValue(QStringLiteral("MISC/ExecMode"), "");
    }
    this->updateCmd();
}

void Launcher::writeGeneralToSettings()
{
    if (_ui->runOptionButton->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/General"), "");
    }
    else if (_ui->helpButton->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/General"), "--help");
    }
    else if (_ui->versionButton->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/General"), "--version");
    }
    else if (_ui->testButton->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/General"), "--test");
    }
    this->updateCmd();
}

void Launcher::writeOutputToSettings()
{
    if (_ui->asciiButton->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/Output"), "");
    }
    else if (_ui->lasButton->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/Output"), "--lasOutput");
    }
    else if (_ui->las10Button->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/Output"), "--las10");
    }
    this->updateCmd();
}

void Launcher::writeNThreadsToSettings()
{
    if (_ui->nthreadCheckbox->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/nthreads"), _ui->nthreadSpinbox->value());
    }
    else
    {
        _settings->setValue(QStringLiteral("ARGS/nthreads"), "default");
    }
    this->updateCmd();
}

void Launcher::writeChunkSizeToSettings()
{
    if (_ui->chunkSizeCheckbox->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/ChunkSize"), _ui->chunkSizeSpinbox->value());
    }
    else
    {
        _settings->setValue(QStringLiteral("ARGS/ChunkSize"), "default");
    }
    this->updateCmd();
}

void Launcher::writeWarehouseFactorToSettings()
{
    if (_ui->warehouseFactorCheckbox->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/WarehouseFactor"), _ui->warehouseFactorSpinbox->value());
    }
    else
    {
        _settings->setValue(QStringLiteral("ARGS/WarehouseFactor"), "default");
    }
    this->updateCmd();
}

void Launcher::writeSeedToSettings()
{
    if (_ui->seedCheckbox->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/Seed"), _ui->seedEdit->text());
    }
    else
    {
        _settings->setValue(QStringLiteral("ARGS/Seed"), "default");
    }
    this->updateCmd();
}

void Launcher::writeGpsStartTimeToSettings()
{
    if (_ui->gpsStartTimeCheckbox->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/GpsStartTime"), _ui->gpsStartTimeEdit->text());
    }
    else
    {
        _settings->setValue(QStringLiteral("ARGS/GpsStartTime"), "default");
    }
    this->updateCmd();
}

void Launcher::writeKDTreeThreadsToSettings()
{
    if (_ui->kdtThreadsCheckbox->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/KDTreeThreads"), _ui->kdtThreadsSpinbox->value());
    }
    else
    {
        _settings->setValue(QStringLiteral("ARGS/KDTreeThreads"), "default");
    }
    this->updateCmd();
}

void Launcher::writeKDTreeGeomThreadsToSettings()
{
    if (_ui->kdtGeomThreadsCheckbox->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/KDTreeGeomThreads"), _ui->kdtGeomThreadsSpinbox->value());
    }
    else
    {
        _settings->setValue(QStringLiteral("ARGS/KDTreeGeomThreads"), "default");
    }
    this->updateCmd();
}

void Launcher::writeSAHnodesToSettings()
{
    if (_ui->SAHnodesCheckbox->isChecked())
    {
        _settings->setValue(QStringLiteral("ARGS/SAHnodes"), _ui->SAHnodesSpinbox->value());
    }
    else
    {
        _settings->setValue(QStringLiteral("ARGS/SAHnodes"), "default");
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
        _ui->cmdBrowser->insertPlainText("run\\helios");
    }
    else if (!_ui->defaultModeButton->isChecked() && _ui->heliospyModeButton->isChecked())
    {
        _ui->cmdBrowser->insertPlainText("python run\\helios.py");
    }
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
    _ui->cmdBrowser->insertPlainText(QStringLiteral(":"));
    _ui->cmdBrowser->moveCursor(QTextCursor::End);
    fmt.setForeground(Qt::cyan);
    _ui->cmdBrowser->setCurrentCharFormat(fmt);
    _ui->cmdBrowser->insertPlainText(_ui->heliosBaseDirLineEdit->text());
    fmt.setForeground(Qt::white);
    fmt.setFontWeight(QFont::Normal);
    _ui->cmdBrowser->setCurrentCharFormat(fmt);
    if (_ui->defaultModeButton->isChecked() && !_ui->heliospyModeButton->isChecked())
    {
        _ui->cmdBrowser->insertPlainText(QStringLiteral("$ run/helios"));
    }
    else if (!_ui->defaultModeButton->isChecked() && _ui->heliospyModeButton->isChecked())
    {
        _ui->cmdBrowser->insertPlainText(QStringLiteral("$ python3 run/helios.py"));
    }
    else
    {
        _ui->cmdBrowser->insertPlainText(QStringLiteral("$"));
    }
#endif
    if (!_ui->surveyPathLineEdit->text().isEmpty())
    {
        _ui->cmdBrowser->insertPlainText(QStringLiteral(" "));
    }
    _ui->cmdBrowser->insertPlainText(_ui->surveyPathLineEdit->text());
    if (!_ui->sourceGuiButton->isChecked() && _ui->sourceArgEditorButton->isChecked())
    {
        _ui->cmdBrowser->insertPlainText(" " + _ui->argsEditor->toPlainText().replace(QStringLiteral("\n"), QStringLiteral(" ")));
    }
    else if (_ui->sourceGuiButton->isChecked() && !_ui->sourceArgEditorButton->isChecked())
    {
        if (!_ui->runOptionButton->isChecked())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" "));
        }
        _ui->cmdBrowser->insertPlainText(_settings->value(QStringLiteral("ARGS/General")).toString());
        if (_ui->unzipCheckbox->isChecked() && _ui->unzipCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(" --unzip " + _settings->value(QStringLiteral("ARGS/UnzipInput")).toString() + " " + _settings->value(QStringLiteral("ARGS/UnzipOutput")).toString());
        }
        if (_ui->assetsCheckbox->isChecked())
        {
            _ui->cmdBrowser->insertPlainText(" --assets " + _settings->value(QStringLiteral("ARGS/AssetsPath")).toString());
        }
        if (_ui->outputCheckbox->isChecked())
        {
            _ui->cmdBrowser->insertPlainText(" --output " + _settings->value(QStringLiteral("ARGS/OutputPath")).toString());
        }
        if (_ui->splitByChannelCheckbox->isChecked() && _ui->splitByChannelCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" --splitByChannel"));
        }
        if (_ui->writeWaveformCheckbox->isChecked())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" --writeWaveform"));
        }
        if (_ui->calcEchoWidthCheckbox->isChecked())
        {
            if (_settings->value(QStringLiteral("MISC/ExecMode")).toString() == QStringLiteral("default"))
            {
                _ui->cmdBrowser->insertPlainText(QStringLiteral(" --calcEchoWidth"));
            }
            else if (_settings->value(QStringLiteral("MISC/ExecMode")).toString() == QStringLiteral("helios.py"))
            {
                _ui->cmdBrowser->insertPlainText(QStringLiteral(" --calcEchowidth"));
            }
        }
        if (_ui->fullwaveNoiseCheckbox->isChecked())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" --fullwaveNoise"));
        }
        if (_ui->fixedIncidenceAngleCheckbox->isChecked() && _ui->fixedIncidenceAngleCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" --fixedIncidenceAngle"));
        }
        if (_ui->seedCheckbox->isChecked())
        {
            _ui->cmdBrowser->insertPlainText(" --seed " + _settings->value(QStringLiteral("ARGS/Seed")).toString());
        }
        if (_ui->gpsStartTimeCheckbox->isChecked() && _ui->gpsStartTimeCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(" --gpsStartTime " + _settings->value(QStringLiteral("ARGS/GpsStartTime")).toString());
        }
        if (!_ui->asciiButton->isChecked())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" "));
        }
        _ui->cmdBrowser->insertPlainText(_settings->value(QStringLiteral("ARGS/Output")).toString());
        if (_ui->zipOutputButton->isChecked())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" --zipOutput"));
        }
        if (_ui->lasScaleCheckbox->isChecked() && _ui->lasScaleCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(" --lasScale " + _settings->value(QStringLiteral("ARGS/LasScale")).toString());
        }
        if (_ui->strategyCheckbox->isChecked() && _ui->strategyCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(" --parallelization " + _settings->value(QStringLiteral("ARGS/Parallelization")).toString());
        }
        if (_ui->nthreadCheckbox->isChecked())
        {
            _ui->cmdBrowser->insertPlainText(" --nthreads " + _settings->value(QStringLiteral("ARGS/nthreads")).toString());
        }
        if (_ui->chunkSizeCheckbox->isChecked() && _ui->chunkSizeCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(" --chunkSize " + _settings->value(QStringLiteral("ARGS/ChunkSize")).toString());
        }
        if (_ui->warehouseFactorCheckbox->isChecked() && _ui->warehouseFactorCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(" --warehouseFactor " + _settings->value(QStringLiteral("ARGS/WarehouseFactor")).toString());
        }
        if (_ui->rebuildSceneCheckbox->isChecked())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" --rebuildScene"));
        }
        if (_ui->kdtTypeCheckbox->isChecked() && _ui->kdtTypeCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(" --kdt " + _settings->value(QStringLiteral("ARGS/KDTree")).toString());
        }
        if (_ui->kdtThreadsCheckbox->isChecked() && _ui->kdtThreadsCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(" --kdtJobs " + _settings->value(QStringLiteral("ARGS/KDTreeThreads")).toString());
        }
        if (_ui->kdtGeomThreadsCheckbox->isChecked() && _ui->kdtGeomThreadsCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(" --kdtGeomJobs " + _settings->value(QStringLiteral("ARGS/KDTreeGeomThreads")).toString());
        }
        if (_ui->SAHnodesCheckbox->isChecked() && _ui->SAHnodesCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(" --sahNodes " + _settings->value(QStringLiteral("ARGS/SAHnodes")).toString());
        }
        if (_ui->disablePlatformNoiseCheckbox->isChecked())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" --disablePlatformNoise"));
        }
        if (_ui->disableLegNoiseCheckbox->isChecked())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" --disableLegNoise"));
        }
        if (_ui->logFileCheckbox->isChecked() && _ui->logFileCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" --logFile"));
        }
        if (_ui->logFileOnlyCheckbox->isChecked() && _ui->logFileOnlyCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" --logFileOnly"));
        }
        if (_ui->silentCheckbox->isChecked())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" --silent"));
        }
        if (_ui->quietCheckbox->isChecked())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" --quiet"));
        }
        if (_ui->vtCheckbox->isChecked() && _ui->vtCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" -vt"));
        }
        if (_ui->vCheckbox->isChecked())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" -v"));
        }
        if (_ui->vvCheckbox->isChecked())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" -vv"));
        }
        if (_ui->liveTrajectoryPlotCheckbox->isChecked() && _ui->liveTrajectoryPlotCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" --livetrajectoryplot"));
        }
        if (_ui->polyscopeCheckbox->isChecked() && _ui->polyscopeCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" --polyscope"));
        }
        if (_ui->open3dCheckbox->isChecked() && _ui->open3dCheckbox->isEnabled())
        {
            _ui->cmdBrowser->insertPlainText(QStringLiteral(" --open3d"));
        }
    }
}

void Launcher::startHeliospp()
{
    if (_settings->value(QStringLiteral("DIRS/HeliosBaseDir")).isNull() || _settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString() == QLatin1String(""))
    {
        auto* baseDir = std::make_unique<BaseDir>(nullptr).release();
        if (baseDir->exec() == QDialog::Accepted)
        {
            _ui->heliosBaseDirLineEdit->setText(baseDir->getBaseDir());
            _settings->setValue(QStringLiteral("DIRS/HeliosBaseDir"), baseDir->getBaseDir());
            _process.setWorkingDirectory(baseDir->getBaseDir());
        }
        else
        {
            return;
        }
    }
    auto cmd = _ui->cmdBrowser->toPlainText();
    QString exec;
    auto execMode = _settings->value(QStringLiteral("MISC/ExecMode")).toString();
#ifdef _WIN32
    if (execMode == "default")
    {
        exec = "run\\helios.exe";
    }
    else if (execMode == "helios.py")
    {
        exec = "python run\\helios.py";
    }
#else
    if (execMode == QStringLiteral("default"))
    {
        exec = QStringLiteral("run/helios");
    }
    else if (execMode == QStringLiteral("helios.py"))
    {
        exec = QStringLiteral("python3 run/helios.py");
    }
#endif
    // Read arguments from cmdBrowser
    auto options = cmd.mid(cmd.indexOf(exec) + exec.length()).split(QRegExp("[\\s\n]+"));
    options.removeAll(QLatin1String(""));

    // clear output
    if (!(options.contains(QStringLiteral("--help")) || options.contains(QStringLiteral("-h")) || options.contains(QStringLiteral("--version"))))
    {
        _ui->outputBrowser->clear();
    }

    QTextCharFormat fmt;
    fmt.setForeground(Qt::blue);
    _ui->outputBrowser->setCurrentCharFormat(fmt);
    _ui->outputBrowser->insertPlainText(QStringLiteral("HELIOS++ execution command: '"));
    fmt.setFontItalic(true);
    _ui->outputBrowser->setCurrentCharFormat(fmt);
    _ui->outputBrowser->insertPlainText(exec);
    if (!options.isEmpty())
    {
        _ui->outputBrowser->insertPlainText(" " + options.join(QStringLiteral(" ")));
    }
    fmt.setFontItalic(false);
    _ui->outputBrowser->setCurrentCharFormat(fmt);
    _ui->outputBrowser->insertPlainText(QStringLiteral("'\n\n"));
    fmt.setForeground(Qt::black);
    _ui->outputBrowser->setCurrentCharFormat(fmt);

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
    env.insert(QStringLiteral("LD_LIBRARY_PATH"), _process.workingDirectory() + "/run");
    _process.setProcessEnvironment(env);
    if (_ui->defaultModeButton->isChecked() && !_ui->heliospyModeButton->isChecked())
    {
        _process.start(QStringLiteral("run/helios"), options);
    }
    else if (!_ui->defaultModeButton->isChecked() && _ui->heliospyModeButton->isChecked())
    {
        _process.start(QStringLiteral("python3"), QStringList() << QStringLiteral("run/helios.py") << options);
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
    if (_process.arguments().contains(QStringLiteral("-h")) || _process.arguments().contains(QStringLiteral("--help")) || _process.arguments().isEmpty())
    {
        auto* const help = std::make_unique<Help>(_process.readAll(), this).release();
        help->show();
    }
    // when '--version': show version info in MessageBox
    else if (_process.arguments().contains(QStringLiteral("--version")) && !_ui->heliospyModeButton->isChecked())
    {
        auto msg = QString(_process.readAll());
        QMessageBox::information(this, QStringLiteral("HELIOS++ version info"), msg);
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
    if (!(_process.arguments().contains(QStringLiteral("--help")) || _process.arguments().contains(QStringLiteral("-h")) || _process.arguments().contains(QStringLiteral("--version"))) && !_process.arguments().isEmpty())
    {
        auto exitCode = _process.exitCode();
        if (exitCode == 0)
        {
            QTextCharFormat fmt;
            fmt.setForeground(Qt::darkGreen);
            _ui->outputBrowser->setCurrentCharFormat(fmt);
            _ui->outputBrowser->append(QStringLiteral("HELIOS++ exited successfully\n"));
            fmt.setForeground(Qt::black);
            _ui->outputBrowser->setCurrentCharFormat(fmt);
            // Extract output directory from HELIOS++ output
            QString relOutDir;
            for (auto& line : _ui->outputBrowser->toPlainText().split(QStringLiteral("\n")))
            {
                if (line.startsWith(QStringLiteral("Output directory: \"output/")))
                {
#ifdef _WIN32
                    relOutDir = line.split("/\\").at(1).left(line.split("/\\").at(1).length() - 1);
#else
                    relOutDir = line.split(QStringLiteral("//")).at(1).left(line.split(QStringLiteral("//")).at(1).length() - 1);
#endif
                    _outputDir = _settings->value(QStringLiteral("DIRS/HeliosBaseDir")).toString() + "/output/" + relOutDir;
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
