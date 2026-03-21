#include "BuildDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QDateTime>
#include <QFileDialog>
#include <QTextStream>
#include "utils/Logger.h"

namespace proxima {

BuildDialog::BuildDialog(Project* project, QWidget *parent)
    : QDialog(parent)
    , project(project)
    , buildMode(RunMode::Release)
    , isBuilding(false)
    , buildStartTime(0) {
    
    setWindowTitle(tr("Build Project"));
    setMinimumSize(700, 500);
    setModal(true);
    
    setupUI();
    loadProjectSettings();
}

BuildDialog::~BuildDialog() {
    saveProjectSettings();
}

void BuildDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Configuration group
    setupConfigurationGroup();
    mainLayout->addWidget(configurationGroup);
    
    // Output group
    setupOutputGroup();
    mainLayout->addWidget(outputGroup, 1);
    
    // Button group
    setupButtonGroup();
    mainLayout->addWidget(buttonGroup);
}

void BuildDialog::setupConfigurationGroup() {
    configurationGroup = new QGroupBox(tr("Build Configuration"), this);
    QFormLayout* layout = new QFormLayout(configurationGroup);
    
    // Build mode
    QComboBox* modeCombo = new QComboBox(this);
    modeCombo->addItem("Release (Optimized)", RunMode::Release);
    modeCombo->addItem("Debug (With Symbols)", RunMode::Debug);
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [modeCombo](int index) {
        buildMode = static_cast<RunMode>(modeCombo->itemData(index).toInt());
    });
    layout->addRow(tr("Build Mode:"), modeCombo);
    
    // Optimization level
    optimizationCombo = new QComboBox(this);
    optimizationCombo->addItem("O0 - No Optimization", 0);
    optimizationCombo->addItem("O1 - Basic Optimization", 1);
    optimizationCombo->addItem("O2 - Standard Optimization", 2);
    optimizationCombo->addItem("O3 - Aggressive Optimization", 3);
    optimizationCombo->setCurrentIndex(2);
    layout->addRow(tr("Optimization:"), optimizationCombo);
    
    // Debug symbols
    debugSymbolsCheck = new QCheckBox(this);
    debugSymbolsCheck->setChecked(true);
    layout->addRow(tr("Debug Symbols:"), debugSymbolsCheck);
    
    // CUDA support
    cudaCheck = new QCheckBox(this);
    cudaCheck->setChecked(false);
    layout->addRow(tr("CUDA Support:"), cudaCheck);
    
    // AVX2 support
    avx2Check = new QCheckBox(this);
    avx2Check->setChecked(true);
    layout->addRow(tr("AVX2 Support:"), avx2Check);
    
    // SSE4 support
    sse4Check = new QCheckBox(this);
    sse4Check->setChecked(true);
    layout->addRow(tr("SSE4 Support:"), sse4Check);
    
    // Max memory
    maxMemorySpin = new QSpinBox(this);
    maxMemorySpin->setRange(128, 65536);
    maxMemorySpin->setValue(4096);
    maxMemorySpin->setSuffix(" MB");
    layout->addRow(tr("Max Memory:"), maxMemorySpin);
    
    // Verbose level
    verboseCombo = new QComboBox(this);
    verboseCombo->addItem("0 - Errors Only", 0);
    verboseCombo->addItem("1 - Warnings", 1);
    verboseCombo->addItem("2 - Info", 2);
    verboseCombo->addItem("3 - Debug", 3);
    verboseCombo->addItem("4 - Trace", 4);
    verboseCombo->setCurrentIndex(2);
    layout->addRow(tr("Verbose Level:"), verboseCombo);
    
    // Parallel build
    parallelBuildCheck = new QCheckBox(this);
    parallelBuildCheck->setChecked(true);
    layout->addRow(tr("Parallel Build:"), parallelBuildCheck);
    
    // Cache
    cacheCheck = new QCheckBox(this);
    cacheCheck->setChecked(true);
    layout->addRow(tr("Enable Cache:"), cacheCheck);
}

void BuildDialog::setupOutputGroup() {
    outputGroup = new QGroupBox(tr("Build Output"), this);
    QVBoxLayout* layout = new QVBoxLayout(outputGroup);
    
    // Progress bar
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setFormat("%p% - %v/%m");
    layout->addWidget(progressBar);
    
    // Status label
    statusLabel = new QLabel(tr("Ready"), this);
    layout->addWidget(statusLabel);
    
    // Time label
    timeLabel = new QLabel(tr("Time: 00:00"), this);
    layout->addWidget(timeLabel);
    
    // Output text
    outputText = new QTextEdit(this);
    outputText->setReadOnly(true);
    outputText->setFont(QFont("Consolas", 9));
    layout->addWidget(outputText, 1);
    
    // Log buttons
    QHBoxLayout* logButtonLayout = new QHBoxLayout();
    
    showLogButton = new QPushButton(tr("Show Log"), this);
    connect(showLogButton, &QPushButton::clicked, this, &BuildDialog::onShowLog);
    logButtonLayout->addWidget(showLogButton);
    
    saveLogButton = new QPushButton(tr("Save Log"), this);
    connect(saveLogButton, &QPushButton::clicked, this, &BuildDialog::onSaveLog);
    logButtonLayout->addWidget(saveLogButton);
    
    logButtonLayout->addStretch();
    layout->addLayout(logButtonLayout);
}

void BuildDialog::setupButtonGroup() {
    buttonGroup = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(buttonGroup);
    layout->setContentsMargins(0, 0, 0, 0);
    
    startButton = new QPushButton(tr("Start Build"), this);
    startButton->setIcon(QIcon(":/icons/build.svg"));
    connect(startButton, &QPushButton::clicked, this, &BuildDialog::onStartBuild);
    layout->addWidget(startButton);
    
    cancelButton = new QPushButton(tr("Cancel"), this);
    cancelButton->setIcon(QIcon(":/icons/cancel.svg"));
    cancelButton->setEnabled(false);
    connect(cancelButton, &QPushButton::clicked, this, &BuildDialog::onCancelBuild);
    layout->addWidget(cancelButton);
    
    closeButton = new QPushButton(tr("Close"), this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    closeButton->setEnabled(false);
    layout->addWidget(closeButton);
    
    layout->addStretch();
}

void BuildDialog::loadProjectSettings() {
    if (!project) return;
    
    Config config = project->getBuildConfig();
    
    optimizationCombo->setCurrentIndex(config.optimizationLevel);
    debugSymbolsCheck->setChecked(config.debugSymbols);
    cudaCheck->setChecked(config.enableCUDA);
    avx2Check->setChecked(config.enableAVX2);
    sse4Check->setChecked(config.enableSSE4);
    maxMemorySpin->setValue(config.maxMemory / 1024 / 1024);
}

void BuildDialog::saveProjectSettings() {
    if (!project) return;
    
    Config config = project->getBuildConfig();
    config.optimizationLevel = optimizationCombo->currentIndex();
    config.debugSymbols = debugSymbolsCheck->isChecked();
    config.enableCUDA = cudaCheck->isChecked();
    config.enableAVX2 = avx2Check->isChecked();
    config.enableSSE4 = sse4Check->isChecked();
    config.maxMemory = maxMemorySpin->value() * 1024 * 1024;
    
    project->setBuildConfig(config);
}

void BuildDialog::setBuildMode(RunMode mode) {
    buildMode = mode;
    
    for (int i = 0; i < optimizationCombo->count(); i++) {
        if (optimizationCombo->itemData(i).toInt() == mode) {
            optimizationCombo->setCurrentIndex(i);
            break;
        }
    }
}

void BuildDialog::setOptimizationLevel(int level) {
    optimizationCombo->setCurrentIndex(level);
}

int BuildDialog::getOptimizationLevel() const {
    return optimizationCombo->currentIndex();
}

void BuildDialog::setDebugSymbols(bool enable) {
    debugSymbolsCheck->setChecked(enable);
}

bool BuildDialog::getDebugSymbols() const {
    return debugSymbolsCheck->isChecked();
}

void BuildDialog::setEnableCUDA(bool enable) {
    cudaCheck->setChecked(enable);
}

bool BuildDialog::getEnableCUDA() const {
    return cudaCheck->isChecked();
}

void BuildDialog::setEnableAVX2(bool enable) {
    avx2Check->setChecked(enable);
}

bool BuildDialog::getEnableAVX2() const {
    return avx2Check->isChecked();
}

void BuildDialog::setMaxMemory(size_t mb) {
    maxMemorySpin->setValue(mb);
}

size_t BuildDialog::getMaxMemory() const {
    return maxMemorySpin->value();
}

void BuildDialog::setVerboseLevel(int level) {
    verboseCombo->setCurrentIndex(level);
}

int BuildDialog::getVerboseLevel() const {
    return verboseCombo->currentData().toInt();
}

void BuildDialog::onStartBuild() {
    if (isBuilding) return;
    
    isBuilding = true;
    buildStartTime = QDateTime::currentMSecsSinceEpoch();
    buildLog.clear();
    
    startButton->setEnabled(false);
    cancelButton->setEnabled(true);
    closeButton->setEnabled(false);
    outputText->clear();
    progressBar->setValue(0);
    statusLabel->setText(tr("Building..."));
    
    emit buildStarted();
    
    appendOutput("========================================\n");
    appendOutput("Build Started: " + QDateTime::currentDateTime().toString() + "\n");
    appendOutput("========================================\n\n");
    
    // Update time label every second
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - buildStartTime;
        int seconds = elapsed / 1000;
        int minutes = seconds / 60;
        seconds = seconds % 60;
        timeLabel->setText(QString("Time: %1:%2")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0')));
    });
    timer->start(1000);
    
    // Simulate build process (would connect to actual compiler)
    // In production, this would call CompilerConnector
    QTimer::singleShot(100, this, [this, timer]() {
        // Start actual build
        appendOutput("Configuring build...\n");
        updateProgress(10, "Configuring");
        
        QTimer::singleShot(500, this, [this, timer]() {
            appendOutput("Compiling modules...\n");
            updateProgress(30, "Compiling");
            
            QTimer::singleShot(1000, this, [this, timer]() {
                appendOutput("Linking...\n");
                updateProgress(70, "Linking");
                
                QTimer::singleShot(500, this, [this, timer]() {
                    appendOutput("Generating documentation...\n");
                    updateProgress(90, "Generating docs");
                    
                    QTimer::singleShot(500, this, [this, timer]() {
                        updateProgress(100, "Complete");
                        statusLabel->setText(tr("Build completed successfully"));
                        appendOutput("\n========================================\n");
                        appendOutput("Build Completed Successfully\n");
                        appendOutput("========================================\n");
                        
                        timer->stop();
                        timer->deleteLater();
                        
                        isBuilding = false;
                        startButton->setEnabled(false);
                        cancelButton->setEnabled(false);
                        closeButton->setEnabled(true);
                        
                        emit buildCompleted(true, project->getPath() + "/build/output");
                    });
                });
            });
        });
    });
}

void BuildDialog::onCancelBuild() {
    if (!isBuilding) return;
    
    isBuilding = false;
    appendOutput("\nBuild cancelled by user\n");
    statusLabel->setText(tr("Build cancelled"));
    
    startButton->setEnabled(true);
    cancelButton->setEnabled(false);
    closeButton->setEnabled(true);
    
    emit buildCancelled();
}

void BuildDialog::onBuildOutput(const QString& output) {
    appendOutput(output);
    buildLog += output;
}

void BuildDialog::onBuildError(const QString& error) {
    appendOutput(error, true);
    buildLog += error;
}

void BuildDialog::onBuildProgress(int percent, const QString& message) {
    updateProgress(percent, message);
}

void BuildDialog::onBuildFinished(bool success, const QString& outputPath) {
    isBuilding = false;
    
    if (success) {
        statusLabel->setText(tr("Build completed successfully"));
        startButton->setEnabled(false);
        closeButton->setEnabled(true);
    } else {
        statusLabel->setText(tr("Build failed"));
        startButton->setEnabled(true);
        closeButton->setEnabled(true);
    }
    
    cancelButton->setEnabled(false);
}

void BuildDialog::onShowLog() {
    // Show full build log in separate window
    QDialog* logDialog = new QDialog(this);
    logDialog->setWindowTitle(tr("Build Log"));
    logDialog->setMinimumSize(800, 600);
    
    QVBoxLayout* layout = new QVBoxLayout(logDialog);
    QTextEdit* logText = new QTextEdit(logDialog);
    logText->setReadOnly(true);
    logText->setFont(QFont("Consolas", 9));
    logText->setPlainText(buildLog);
    layout->addWidget(logText);
    
    QPushButton* closeButton = new QPushButton(tr("Close"), logDialog);
    connect(closeButton, &QPushButton::clicked, logDialog, &QDialog::accept);
    layout->addWidget(closeButton);
    
    logDialog->exec();
    logDialog->deleteLater();
}

void BuildDialog::onSaveLog() {
    QString path = QFileDialog::getSaveFileName(this, tr("Save Build Log"),
                         "", tr("Text Files (*.txt);;All Files (*)"));
    
    if (!path.isEmpty()) {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << buildLog;
            file.close();
            LOG_INFO("Build log saved: " + path.toStdString());
        }
    }
}

void BuildDialog::updateProgress(int percent, const QString& message) {
    progressBar->setValue(percent);
    statusLabel->setText(message);
}

void BuildDialog::appendOutput(const QString& text, bool isError) {
    if (isError) {
        outputText->setTextColor(Qt::red);
    } else {
        outputText->setTextColor(Qt::black);
    }
    outputText->append(text);
    outputText->setTextColor(Qt::black);
    
    // Auto-scroll to bottom
    outputText->verticalScrollBar()->setValue(outputText->verticalScrollBar()->maximum());
}

} // namespace proxima