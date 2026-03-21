#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>
#include "utils/Logger.h"

namespace proxima {

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , settings(&Settings::getInstance()) {
    
    setWindowTitle(tr("Centauri IDE Settings"));
    setMinimumSize(700, 500);
    
    setupUI();
    loadSettings();
}

SettingsDialog::~SettingsDialog() {}

void SettingsDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);
    
    setupAppearanceTab();
    setupEditorTab();
    setupDebuggerTab();
    setupBuildTab();
    setupLLMTab();
    setupGitTab();
    setupCollaborationTab();
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton* applyButton = new QPushButton(tr("Apply"), this);
    connect(applyButton, &QPushButton::clicked, this, &SettingsDialog::onApply);
    buttonLayout->addWidget(applyButton);
    
    QPushButton* okButton = new QPushButton(tr("OK"), this);
    connect(okButton, &QPushButton::clicked, this, &SettingsDialog::onOk);
    buttonLayout->addWidget(okButton);
    
    QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
    connect(cancelButton, &QPushButton::clicked, this, &SettingsDialog::onCancel);
    buttonLayout->addWidget(cancelButton);
    
    QPushButton* resetButton = new QPushButton(tr("Reset"), this);
    connect(resetButton, &QPushButton::clicked, this, &SettingsDialog::onReset);
    buttonLayout->addWidget(resetButton);
    
    mainLayout->addLayout(buttonLayout);
}

void SettingsDialog::setupAppearanceTab() {
    QWidget* tab = new QWidget();
    QFormLayout* layout = new QFormLayout(tab);
    
    themeCombo = new QComboBox();
    themeCombo->addItem("Light", static_cast<int>(Theme::Light));
    themeCombo->addItem("Dark", static_cast<int>(Theme::Dark));
    themeCombo->addItem("System", static_cast<int>(Theme::System));
    connect(themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onThemeChanged);
    layout->addRow(tr("Theme:"), themeCombo);
    
    backgroundColorButton = new QPushButton(tr("Background Color"));
    connect(backgroundColorButton, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(backgroundColor, this, tr("Background Color"));
        if (color.isValid()) {
            backgroundColor = color;
        }
    });
    layout->addRow(tr("Background:"), backgroundColorButton);
    
    textColorButton = new QPushButton(tr("Text Color"));
    connect(textColorButton, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(textColor, this, tr("Text Color"));
        if (color.isValid()) {
            textColor = color;
        }
    });
    layout->addRow(tr("Text:"), textColorButton);
    
    tabWidget->addTab(tab, tr("Appearance"));
}

void SettingsDialog::setupEditorTab() {
    QWidget* tab = new QWidget();
    QFormLayout* layout = new QFormLayout(tab);
    
    fontCombo = new QFontComboBox();
    fontCombo->setCurrentFont(settings->getEditorSettings().font);
    connect(fontCombo, &QFontComboBox::currentFontChanged, this, &SettingsDialog::onFontChanged);
    layout->addRow(tr("Font:"), fontCombo);
    
    fontSizeSpin = new QSpinBox();
    fontSizeSpin->setRange(8, 72);
    fontSizeSpin->setValue(settings->getEditorSettings().fontSize);
    layout->addRow(tr("Font Size:"), fontSizeSpin);
    
    tabWidthSpin = new QSpinBox();
    tabWidthSpin->setRange(1, 16);
    tabWidthSpin->setValue(settings->getEditorSettings().tabWidth);
    layout->addRow(tr("Tab Width:"), tabWidthSpin);
    
    useSpacesCheck = new QCheckBox();
    useSpacesCheck->setChecked(settings->getEditorSettings().useSpaces);
    layout->addRow(tr("Use Spaces:"), useSpacesCheck);
    
    lineNumbersCheck = new QCheckBox();
    lineNumbersCheck->setChecked(settings->getEditorSettings().showLineNumbers);
    layout->addRow(tr("Line Numbers:"), lineNumbersCheck);
    
    wordWrapCheck = new QCheckBox();
    wordWrapCheck->setChecked(settings->getEditorSettings().wordWrap);
    layout->addRow(tr("Word Wrap:"), wordWrapCheck);
    
    autoIndentCheck = new QCheckBox();
    autoIndentCheck->setChecked(settings->getEditorSettings().autoIndent);
    layout->addRow(tr("Auto Indent:"), autoIndentCheck);
    
    highlightLineCheck = new QCheckBox();
    highlightLineCheck->setChecked(settings->getEditorSettings().highlightCurrentLine);
    layout->addRow(tr("Highlight Current Line:"), highlightLineCheck);
    
    codeFoldingCheck = new QCheckBox();
    codeFoldingCheck->setChecked(settings->getEditorSettings().codeFolding);
    layout->addRow(tr("Code Folding:"), codeFoldingCheck);
    
    tabWidget->addTab(tab, tr("Editor"));
}

void SettingsDialog::setupDebuggerTab() {
    QWidget* tab = new QWidget();
    QFormLayout* layout = new QFormLayout(tab);
    
    verboseLevelSpin = new QSpinBox();
    verboseLevelSpin->setRange(0, 5);
    verboseLevelSpin->setValue(settings->getDebuggerSettings().verboseLevel);
    layout->addRow(tr("Verbose Level:"), verboseLevelSpin);
    
    autoBreakCheck = new QCheckBox();
    autoBreakCheck->setChecked(settings->getDebuggerSettings().autoBreakOnError);
    layout->addRow(tr("Auto Break on Error:"), autoBreakCheck);
    
    tooltipsCheck = new QCheckBox();
    tooltipsCheck->setChecked(settings->getDebuggerSettings().showTooltips);
    layout->addRow(tr("Show Tooltips:"), tooltipsCheck);
    
    inspectVarsCheck = new QCheckBox();
    inspectVarsCheck->setChecked(settings->getDebuggerSettings().inspectVariables);
    layout->addRow(tr("Inspect Variables:"), inspectVarsCheck);
    
    watchExprCheck = new QCheckBox();
    watchExprCheck->setChecked(settings->getDebuggerSettings().watchExpressions);
    layout->addRow(tr("Watch Expressions:"), watchExprCheck);
    
    tabWidget->addTab(tab, tr("Debugger"));
}

void SettingsDialog::setupBuildTab() {
    QWidget* tab = new QWidget();
    QFormLayout* layout = new QFormLayout(tab);
    
    optimizationCombo = new QComboBox();
    optimizationCombo->addItem("O0 (None)", 0);
    optimizationCombo->addItem("O1 (Basic)", 1);
    optimizationCombo->addItem("O2 (Standard)", 2);
    optimizationCombo->addItem("O3 (Aggressive)", 3);
    optimizationCombo->setCurrentIndex(settings->getBuildSettings().optimizationLevel);
    layout->addRow(tr("Optimization:"), optimizationCombo);
    
    debugSymbolsCheck = new QCheckBox();
    debugSymbolsCheck->setChecked(settings->getBuildSettings().debugSymbols);
    layout->addRow(tr("Debug Symbols:"), debugSymbolsCheck);
    
    parallelBuildCheck = new QCheckBox();
    parallelBuildCheck->setChecked(settings->getBuildSettings().parallelCompilation);
    layout->addRow(tr("Parallel Build:"), parallelBuildCheck);
    
    cacheCheck = new QCheckBox();
    cacheCheck->setChecked(settings->getBuildSettings().cacheEnabled);
    layout->addRow(tr("Cache Enabled:"), cacheCheck);
    
    cudaCheck = new QCheckBox();
    cudaCheck->setChecked(settings->getBuildSettings().enableCUDA);
    layout->addRow(tr("CUDA Support:"), cudaCheck);
    
    avx2Check = new QCheckBox();
    avx2Check->setChecked(settings->getBuildSettings().enableAVX2);
    layout->addRow(tr("AVX2 Support:"), avx2Check);
    
    sse4Check = new QCheckBox();
    sse4Check->setChecked(settings->getBuildSettings().enableSSE4);
    layout->addRow(tr("SSE4 Support:"), sse4Check);
    
    maxMemorySpin = new QSpinBox();
    maxMemorySpin->setRange(128, 65536);
    maxMemorySpin->setValue(settings->getBuildSettings().maxMemory / 1024 / 1024);
    maxMemorySpin->setSuffix(" MB");
    layout->addRow(tr("Max Memory:"), maxMemorySpin);
    
    tabWidget->addTab(tab, tr("Build"));
}

void SettingsDialog::setupLLMTab() {
    QWidget* tab = new QWidget();
    QFormLayout* layout = new QFormLayout(tab);
    
    serverUrlEdit = new QLineEdit();
    serverUrlEdit->setText(settings->getLLMSettings().serverURL);
    layout->addRow(tr("Server URL:"), serverUrlEdit);
    
    modelEdit = new QLineEdit();
    modelEdit->setText(settings->getLLMSettings().model);
    layout->addRow(tr("Model:"), modelEdit);
    
    maxTokensSpin = new QSpinBox();
    maxTokensSpin->setRange(256, 8192);
    maxTokensSpin->setValue(settings->getLLMSettings().maxTokens);
    layout->addRow(tr("Max Tokens:"), maxTokensSpin);
    
    temperatureSpin = new QDoubleSpinBox();
    temperatureSpin->setRange(0.0, 2.0);
    temperatureSpin->setSingleStep(0.1);
    temperatureSpin->setValue(settings->getLLMSettings().temperature);
    layout->addRow(tr("Temperature:"), temperatureSpin);
    
    autoSuggestCheck = new QCheckBox();
    autoSuggestCheck->setChecked(settings->getLLMSettings().autoSuggest);
    layout->addRow(tr("Auto Suggest:"), autoSuggestCheck);
    
    timeoutSpin = new QSpinBox();
    timeoutSpin->setRange(1000, 120000);
    timeoutSpin->setSingleStep(1000);
    timeoutSpin->setValue(settings->getLLMSettings().timeout);
    timeoutSpin->setSuffix(" ms");
    layout->addRow(tr("Timeout:"), timeoutSpin);
    
    tabWidget->addTab(tab, tr("LLM"));
}

void SettingsDialog::setupGitTab() {
    QWidget* tab = new QWidget();
    QFormLayout* layout = new QFormLayout(tab);
    
    userNameEdit = new QLineEdit();
    userNameEdit->setText(settings->getGitSettings().userName);
    layout->addRow(tr("User Name:"), userNameEdit);
    
    userEmailEdit = new QLineEdit();
    userEmailEdit->setText(settings->getGitSettings().userEmail);
    layout->addRow(tr("User Email:"), userEmailEdit);
    
    autoCommitCheck = new QCheckBox();
    autoCommitCheck->setChecked(settings->getGitSettings().autoCommit);
    layout->addRow(tr("Auto Commit:"), autoCommitCheck);
    
    autoPushCheck = new QCheckBox();
    autoPushCheck->setChecked(settings->getGitSettings().autoPush);
    layout->addRow(tr("Auto Push:"), autoPushCheck);
    
    tabWidget->addTab(tab, tr("Git"));
}

void SettingsDialog::setupCollaborationTab() {
    QWidget* tab = new QWidget();
    QFormLayout* layout = new QFormLayout(tab);
    
    collaborationEnableCheck = new QCheckBox();
    collaborationEnableCheck->setChecked(settings->getCollaborationSettings().enabled);
    layout->addRow(tr("Enabled:"), collaborationEnableCheck);
    
    hostServerEdit = new QLineEdit();
    hostServerEdit->setText(settings->getCollaborationSettings().hostServer);
    layout->addRow(tr("Host Server:"), hostServerEdit);
    
    portSpin = new QSpinBox();
    portSpin->setRange(1, 65535);
    portSpin->setValue(settings->getCollaborationSettings().port);
    layout->addRow(tr("Port:"), portSpin);
    
    autoSyncCheck = new QCheckBox();
    autoSyncCheck->setChecked(settings->getCollaborationSettings().autoSync);
    layout->addRow(tr("Auto Sync:"), autoSyncCheck);
    
    showCursorsCheck = new QCheckBox();
    showCursorsCheck->setChecked(settings->getCollaborationSettings().showUserCursors);
    layout->addRow(tr("Show User Cursors:"), showCursorsCheck);
    
    usernameEdit = new QLineEdit();
    usernameEdit->setText(settings->getCollaborationSettings().username);
    layout->addRow(tr("Username:"), usernameEdit);
    
    tabWidget->addTab(tab, tr("Collaboration"));
}

void SettingsDialog::loadSettings() {
    EditorSettings editor = settings->getEditorSettings();
    fontCombo->setCurrentFont(editor.font);
    fontSizeSpin->setValue(editor.fontSize);
    tabWidthSpin->setValue(editor.tabWidth);
    useSpacesCheck->setChecked(editor.useSpaces);
    lineNumbersCheck->setChecked(editor.showLineNumbers);
    wordWrapCheck->setChecked(editor.wordWrap);
    autoIndentCheck->setChecked(editor.autoIndent);
    highlightLineCheck->setChecked(editor.highlightCurrentLine);
    codeFoldingCheck->setChecked(editor.codeFolding);
    
    DebuggerSettings debugger = settings->getDebuggerSettings();
    verboseLevelSpin->setValue(debugger.verboseLevel);
    autoBreakCheck->setChecked(debugger.autoBreakOnError);
    tooltipsCheck->setChecked(debugger.showTooltips);
    inspectVarsCheck->setChecked(debugger.inspectVariables);
    watchExprCheck->setChecked(debugger.watchExpressions);
    
    BuildSettings build = settings->getBuildSettings();
    optimizationCombo->setCurrentIndex(build.optimizationLevel);
    debugSymbolsCheck->setChecked(build.debugSymbols);
    parallelBuildCheck->setChecked(build.parallelCompilation);
    cacheCheck->setChecked(build.cacheEnabled);
    cudaCheck->setChecked(build.enableCUDA);
    avx2Check->setChecked(build.enableAVX2);
    sse4Check->setChecked(build.enableSSE4);
    maxMemorySpin->setValue(build.maxMemory / 1024 / 1024);
    
    LLMSettings llm = settings->getLLMSettings();
    serverUrlEdit->setText(llm.serverURL);
    modelEdit->setText(llm.model);
    maxTokensSpin->setValue(llm.maxTokens);
    temperatureSpin->setValue(llm.temperature);
    autoSuggestCheck->setChecked(llm.autoSuggest);
    timeoutSpin->setValue(llm.timeout);
    
    GitSettings git = settings->getGitSettings();
    userNameEdit->setText(git.userName);
    userEmailEdit->setText(git.userEmail);
    autoCommitCheck->setChecked(git.autoCommit);
    autoPushCheck->setChecked(git.autoPush);
    
    CollaborationSettings collab = settings->getCollaborationSettings();
    collaborationEnableCheck->setChecked(collab.enabled);
    hostServerEdit->setText(collab.hostServer);
    portSpin->setValue(collab.port);
    autoSyncCheck->setChecked(collab.autoSync);
    showCursorsCheck->setChecked(collab.showUserCursors);
    usernameEdit->setText(collab.username);
    
    themeCombo->setCurrentIndex(static_cast<int>(settings->getTheme()));
    backgroundColor = QColor(30, 30, 30);
    textColor = QColor(200, 200, 200);
}

void SettingsDialog::saveSettings() {
    EditorSettings editor;
    editor.font = fontCombo->currentFont();
    editor.fontSize = fontSizeSpin->value();
    editor.tabWidth = tabWidthSpin->value();
    editor.useSpaces = useSpacesCheck->isChecked();
    editor.showLineNumbers = lineNumbersCheck->isChecked();
    editor.wordWrap = wordWrapCheck->isChecked();
    editor.autoIndent = autoIndentCheck->isChecked();
    editor.highlightCurrentLine = highlightLineCheck->isChecked();
    editor.codeFolding = codeFoldingCheck->isChecked();
    editor.theme = settings->getTheme();
    settings->setEditorSettings(editor);
    
    DebuggerSettings debugger;
    debugger.verboseLevel = verboseLevelSpin->value();
    debugger.autoBreakOnError = autoBreakCheck->isChecked();
    debugger.showTooltips = tooltipsCheck->isChecked();
    debugger.inspectVariables = inspectVarsCheck->isChecked();
    debugger.watchExpressions = watchExprCheck->isChecked();
    settings->setDebuggerSettings(debugger);
    
    BuildSettings build;
    build.optimizationLevel = optimizationCombo->currentIndex();
    build.debugSymbols = debugSymbolsCheck->isChecked();
    build.parallelCompilation = parallelBuildCheck->isChecked();
    build.cacheEnabled = cacheCheck->isChecked();
    build.enableCUDA = cudaCheck->isChecked();
    build.enableAVX2 = avx2Check->isChecked();
    build.enableSSE4 = sse4Check->isChecked();
    build.maxMemory = maxMemorySpin->value() * 1024 * 1024;
    settings->setBuildSettings(build);
    
    LLMSettings llm;
    llm.serverURL = serverUrlEdit->text();
    llm.model = modelEdit->text();
    llm.maxTokens = maxTokensSpin->value();
    llm.temperature = temperatureSpin->value();
    llm.autoSuggest = autoSuggestCheck->isChecked();
    llm.timeout = timeoutSpin->value();
    settings->setLLMSettings(llm);
    
    GitSettings git;
    git.userName = userNameEdit->text();
    git.userEmail = userEmailEdit->text();
    git.autoCommit = autoCommitCheck->isChecked();
    git.autoPush = autoPushCheck->isChecked();
    settings->setGitSettings(git);
    
    CollaborationSettings collab;
    collab.enabled = collaborationEnableCheck->isChecked();
    collab.hostServer = hostServerEdit->text();
    collab.port = portSpin->value();
    collab.autoSync = autoSyncCheck->isChecked();
    collab.showUserCursors = showCursorsCheck->isChecked();
    collab.username = usernameEdit->text();
    settings->setCollaborationSettings(collab);
    
    settings->setTheme(static_cast<Theme>(themeCombo->currentData().toInt()));
    
    settings->save();
}

void SettingsDialog::onApply() {
    saveSettings();
    LOG_INFO("Settings applied");
}

void SettingsDialog::onOk() {
    saveSettings();
    accept();
}

void SettingsDialog::onCancel() {
    reject();
}

void SettingsDialog::onReset() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Reset Settings"),
        tr("Reset all settings to defaults?"),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        settings->reset();
        loadSettings();
        LOG_INFO("Settings reset to defaults");
    }
}

void SettingsDialog::onThemeChanged(int index) {
    // Preview theme change
    LOG_DEBUG("Theme changed to index: " + QString::number(index));
}

void SettingsDialog::onFontChanged() {
    // Preview font change
    LOG_DEBUG("Font changed to: " + fontCombo->currentFont().family());
}

void SettingsDialog::onColorSelected() {
    // Color selected
}

} // namespace proxima