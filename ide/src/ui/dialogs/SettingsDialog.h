#ifndef CENTAURI_SETTINGSDIALOG_H
#define CENTAURI_SETTINGSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QFontComboBox>
#include <QColorDialog>
#include "app/Settings.h"

namespace proxima {

class SettingsDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();
    
    void loadSettings();
    void saveSettings();
    
private slots:
    void onApply();
    void onOk();
    void onCancel();
    void onReset();
    void onThemeChanged(int index);
    void onFontChanged();
    void onColorSelected();
    
private:
    void setupUI();
    void setupEditorTab();
    void setupDebuggerTab();
    void setupBuildTab();
    void setupLLMTab();
    void setupGitTab();
    void setupCollaborationTab();
    void setupAppearanceTab();
    
    QTabWidget* tabWidget;
    
    // Editor settings
    QFontComboBox* fontCombo;
    QSpinBox* fontSizeSpin;
    QSpinBox* tabWidthSpin;
    QCheckBox* useSpacesCheck;
    QCheckBox* lineNumbersCheck;
    QCheckBox* wordWrapCheck;
    QCheckBox* autoIndentCheck;
    QCheckBox* highlightLineCheck;
    QCheckBox* codeFoldingCheck;
    
    // Debugger settings
    QSpinBox* verboseLevelSpin;
    QCheckBox* autoBreakCheck;
    QCheckBox* tooltipsCheck;
    QCheckBox* inspectVarsCheck;
    QCheckBox* watchExprCheck;
    
    // Build settings
    QComboBox* optimizationCombo;
    QCheckBox* debugSymbolsCheck;
    QCheckBox* parallelBuildCheck;
    QCheckBox* cacheCheck;
    QCheckBox* cudaCheck;
    QCheckBox* avx2Check;
    QCheckBox* sse4Check;
    QSpinBox* maxMemorySpin;
    
    // LLM settings
    QLineEdit* serverUrlEdit;
    QLineEdit* modelEdit;
    QSpinBox* maxTokensSpin;
    QDoubleSpinBox* temperatureSpin;
    QCheckBox* autoSuggestCheck;
    QSpinBox* timeoutSpin;
    
    // Git settings
    QLineEdit* userNameEdit;
    QLineEdit* userEmailEdit;
    QCheckBox* autoCommitCheck;
    QCheckBox* autoPushCheck;
    
    // Collaboration settings
    QCheckBox* collaborationEnableCheck;
    QLineEdit* hostServerEdit;
    QSpinBox* portSpin;
    QCheckBox* autoSyncCheck;
    QCheckBox* showCursorsCheck;
    QLineEdit* usernameEdit;
    
    // Appearance settings
    QComboBox* themeCombo;
    QPushButton* backgroundColorButton;
    QPushButton* textColorButton;
    QColor backgroundColor;
    QColor textColor;
    
    Settings* settings;
};

} // namespace proxima

#endif // CENTAURI_SETTINGSDIALOG_H