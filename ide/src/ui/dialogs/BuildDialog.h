#ifndef CENTAURI_BUILDDIALOG_H
#define CENTAURI_BUILDDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include "core/Project.h"

namespace proxima {

class BuildDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit BuildDialog(Project* project, QWidget *parent = nullptr);
    ~BuildDialog();
    
    void setBuildMode(RunMode mode);
    RunMode getBuildMode() const { return buildMode; }
    
    // Build configuration
    void setOptimizationLevel(int level);
    int getOptimizationLevel() const;
    
    void setDebugSymbols(bool enable);
    bool getDebugSymbols() const;
    
    void setEnableCUDA(bool enable);
    bool getEnableCUDA() const;
    
    void setEnableAVX2(bool enable);
    bool getEnableAVX2() const;
    
    void setMaxMemory(size_t mb);
    size_t getMaxMemory() const;
    
    void setVerboseLevel(int level);
    int getVerboseLevel() const;
    
signals:
    void buildStarted();
    void buildProgress(int percent, const QString& message);
    void buildCompleted(bool success, const QString& outputPath);
    void buildCancelled();
    
private slots:
    void onStartBuild();
    void onCancelBuild();
    void onBuildOutput(const QString& output);
    void onBuildError(const QString& error);
    void onBuildProgress(int percent, const QString& message);
    void onBuildFinished(bool success, const QString& outputPath);
    void onShowLog();
    void onSaveLog();
    
private:
    void setupUI();
    void setupConfigurationGroup();
    void setupOutputGroup();
    void setupButtonGroup();
    void loadProjectSettings();
    void saveProjectSettings();
    void updateProgress(int percent, const QString& message);
    void appendOutput(const QString& text, bool isError = false);
    
    Project* project;
    RunMode buildMode;
    
    // Configuration widgets
    QComboBox* optimizationCombo;
    QCheckBox* debugSymbolsCheck;
    QCheckBox* cudaCheck;
    QCheckBox* avx2Check;
    QCheckBox* sse4Check;
    QSpinBox* maxMemorySpin;
    QComboBox* verboseCombo;
    QCheckBox* parallelBuildCheck;
    QCheckBox* cacheCheck;
    
    // Output widgets
    QTextEdit* outputText;
    QProgressBar* progressBar;
    QLabel* statusLabel;
    QLabel* timeLabel;
    
    // Buttons
    QPushButton* startButton;
    QPushButton* cancelButton;
    QPushButton* closeButton;
    QPushButton* showLogButton;
    QPushButton* saveLogButton;
    
    // Build state
    bool isBuilding;
    qint64 buildStartTime;
    QString buildLog;
};

} // namespace proxima

#endif // CENTAURI_BUILDDIALOG_H