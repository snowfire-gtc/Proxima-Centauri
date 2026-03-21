#include "LLMAssistantDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include "utils/Logger.h"

namespace proxima {

LLMAssistantDialog::LLMAssistantDialog(QWidget *parent)
    : QDialog(parent)
    , selectionStart(0)
    , selectionEnd(0)
    , isProcessing(false)
    , llmService(new LLMService(this)) {
    
    setWindowTitle(tr("LLM Code Assistant"));
    setMinimumSize(1000, 700);
    setModal(true);
    
    connect(llmService, &LLMService::suggestionsReady, this, &LLMAssistantDialog::onLLMResponse);
    connect(llmService, &LLMService::errorOccurred, this, &LLMAssistantDialog::onLLMError);
    connect(llmService, &LLMService::processingStarted, this, &LLMAssistantDialog::onProcessingStarted);
    connect(llmService, &LLMService::processingFinished, this, &LLMAssistantDialog::onProcessingFinished);
    
    setupUI();
}

LLMAssistantDialog::~LLMAssistantDialog() {}

void LLMAssistantDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Prompt section
    setupPromptGroup();
    mainLayout->addWidget(promptGroup);
    
    // Main content splitter
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // Suggestions section
    setupSuggestionsGroup();
    mainSplitter->addWidget(suggestionsGroup);
    
    // Preview section
    setupPreviewGroup();
    mainSplitter->addWidget(previewGroup);
    
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 2);
    
    mainLayout->addWidget(mainSplitter, 1);
    
    // Progress section
    QWidget* progressWidget = new QWidget(this);
    QHBoxLayout* progressLayout = new QHBoxLayout(progressWidget);
    progressLayout->setContentsMargins(0, 0, 0, 0);
    
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 0);
    progressBar->setVisible(false);
    progressLayout->addWidget(progressBar);
    
    statusLabel = new QLabel(tr("Ready"), this);
    progressLayout->addWidget(statusLabel);
    
    progressLayout->addStretch();
    
    mainLayout->addWidget(progressWidget);
    
    // Button section
    setupButtonGroup();
    mainLayout->addWidget(buttonGroup);
}

void LLMAssistantDialog::setupPromptGroup() {
    promptGroup = new QGroupBox(tr("Request"), this);
    QVBoxLayout* layout = new QVBoxLayout(promptGroup);
    
    // Prompt template
    QHBoxLayout* templateLayout = new QHBoxLayout();
    templateLayout->addWidget(new QLabel(tr("Template:"), this));
    
    promptTemplateCombo = new QComboBox(this);
    promptTemplateCombo->addItem("Optimize Code", "optimize");
    promptTemplateCombo->addItem("Fix Bugs", "fix_bugs");
    promptTemplateCombo->addItem("Add Comments", "add_comments");
    promptTemplateCombo->addItem("Refactor", "refactor");
    promptTemplateCombo->addItem("Add Tests", "add_tests");
    promptTemplateCombo->addItem("Custom", "custom");
    connect(promptTemplateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LLMAssistantDialog::onPromptChanged);
    templateLayout->addWidget(promptTemplateCombo);
    templateLayout->addStretch();
    
    layout->addLayout(templateLayout);
    
    // Prompt text
    promptEdit = new QTextEdit(this);
    promptEdit->setPlaceholderText(tr("Describe what you want the AI to do with the selected code..."));
    promptEdit->setMinimumHeight(80);
    connect(promptEdit, &QTextEdit::textChanged, this, &LLMAssistantDialog::onPromptChanged);
    layout->addWidget(promptEdit);
    
    // Options
    QHBoxLayout* optionsLayout = new QHBoxLayout();
    
    includeContextCheck = new QCheckBox(tr("Include surrounding context"), this);
    includeContextCheck->setChecked(true);
    optionsLayout->addWidget(includeContextCheck);
    
    includeCommentsCheck = new QCheckBox(tr("Include existing comments"), this);
    includeCommentsCheck->setChecked(true);
    optionsLayout->addWidget(includeCommentsCheck);
    
    optionsLayout->addStretch();
    layout->addLayout(optionsLayout);
    
    // Request buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    requestButton = new QPushButton(tr("Request Assistance"), this);
    requestButton->setIcon(QIcon(":/icons/llm.svg"));
    connect(requestButton, &QPushButton::clicked, this, &LLMAssistantDialog::onRequestAssistance);
    buttonLayout->addWidget(requestButton);
    
    cancelButton = new QPushButton(tr("Cancel"), this);
    cancelButton->setEnabled(false);
    connect(cancelButton, &QPushButton::clicked, this, &LLMAssistantDialog::onCancelRequest);
    buttonLayout->addWidget(cancelButton);
    
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);
}

void LLMAssistantDialog::setupSuggestionsGroup() {
    suggestionsGroup = new QGroupBox(tr("Suggestions"), this);
    QVBoxLayout* layout = new QVBoxLayout(suggestionsGroup);
    
    suggestionsList = new QListWidget(this);
    connect(suggestionsList, &QListWidget::currentRowChanged, this, &LLMAssistantDialog::onModificationSelected);
    layout->addWidget(suggestionsList);
    
    // Confidence indicator
    QHBoxLayout* confidenceLayout = new QHBoxLayout();
    confidenceLayout->addWidget(new QLabel(tr("Confidence:"), this));
    confidenceLabel = new QLabel(this);
    confidenceLayout->addWidget(confidenceLabel);
    confidenceLayout->addStretch();
    layout->addLayout(confidenceLayout);
    
    // Explanation
    QHBoxLayout* explanationLayout = new QHBoxLayout();
    explanationLayout->addWidget(new QLabel(tr("Explanation:"), this));
    explanationLabel = new QLabel(this);
    explanationLabel->setWordWrap(true);
    explanationLayout->addWidget(explanationLabel);
    layout->addLayout(explanationLayout);
}

void LLMAssistantDialog::setupPreviewGroup() {
    previewGroup = new QGroupBox(tr("Preview"), this);
    QVBoxLayout* layout = new QVBoxLayout(previewGroup);
    
    previewSplitter = new QSplitter(Qt::Vertical, this);
    
    originalPreview = new QTextEdit(this);
    originalPreview->setReadOnly(true);
    originalPreview->setPlaceholderText(tr("Original code will appear here"));
    originalPreview->setFont(QFont("Consolas", 10));
    
    suggestedPreview = new QTextEdit(this);
    suggestedPreview->setReadOnly(true);
    suggestedPreview->setPlaceholderText(tr("Suggested code will appear here"));
    suggestedPreview->setFont(QFont("Consolas", 10));
    
    previewSplitter->addWidget(originalPreview);
    previewSplitter->addWidget(suggestedPreview);
    previewSplitter->setStretchFactor(0, 1);
    previewSplitter->setStretchFactor(1, 1);
    
    layout->addWidget(previewSplitter);
}

void LLMAssistantDialog::setupButtonGroup() {
    buttonGroup = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(buttonGroup);
    layout->setContentsMargins(0, 0, 0, 0);
    
    acceptButton = new QPushButton(tr("Accept"), this);
    connect(acceptButton, &QPushButton::clicked, this, &LLMAssistantDialog::onAcceptModification);
    acceptButton->setEnabled(false);
    layout->addWidget(acceptButton);
    
    rejectButton = new QPushButton(tr("Reject"), this);
    connect(rejectButton, &QPushButton::clicked, this, &LLMAssistantDialog::onRejectModification);
    rejectButton->setEnabled(false);
    layout->addWidget(rejectButton);
    
    acceptAllButton = new QPushButton(tr("Accept All"), this);
    connect(acceptAllButton, &QPushButton::clicked, this, &LLMAssistantDialog::onAcceptAll);
    acceptAllButton->setEnabled(false);
    layout->addWidget(acceptAllButton);
    
    rejectAllButton = new QPushButton(tr("Reject All"), this);
    connect(rejectAllButton, &QPushButton::clicked, this, &LLMAssistantDialog::onRejectAll);
    rejectAllButton->setEnabled(false);
    layout->addWidget(rejectAllButton);
    
    layout->addStretch();
    
    applyButton = new QPushButton(tr("Apply Selected"), this);
    applyButton->setIcon(QIcon(":/icons/apply.svg"));
    connect(applyButton, &QPushButton::clicked, this, &LLMAssistantDialog::onApplySelected);
    applyButton->setEnabled(false);
    layout->addWidget(applyButton);
    
    closeButton = new QPushButton(tr("Close"), this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeButton);
}

void LLMAssistantDialog::setFile(const QString& file) {
    currentFile = file;
}

void LLMAssistantDialog::setSelection(int startLine, int endLine, const QString& code) {
    selectionStart = startLine;
    selectionEnd = endLine;
    selectedCode = code;
    originalPreview->setPlainText(code);
}

void LLMAssistantDialog::setPrompt(const QString& prompt) {
    promptEdit->setPlainText(prompt);
}

void LLMAssistantDialog::onRequestAssistance() {
    if (isProcessing) return;
    
    QString prompt = buildPrompt();
    if (prompt.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please enter a prompt"));
        return;
    }
    
    isProcessing = true;
    modifications.clear();
    suggestionsList->clear();
    suggestedPreview->clear();
    
    requestButton->setEnabled(false);
    cancelButton->setEnabled(true);
    acceptButton->setEnabled(false);
    rejectButton->setEnabled(false);
    acceptAllButton->setEnabled(false);
    rejectAllButton->setEnabled(false);
    applyButton->setEnabled(false);
    
    progressBar->setVisible(true);
    statusLabel->setText(tr("Sending request to LLM..."));
    
    emit requestSent(prompt);
    
    // Send request to LLM service
    llmService->requestSuggestions(currentFile, selectionStart, selectionEnd, selectedCode, prompt);
    
    LOG_INFO("LLM request sent: " + prompt.toStdString());
}

void LLMAssistantDialog::onCancelRequest() {
    if (!isProcessing) return;
    
    isProcessing = false;
    progressBar->setVisible(false);
    statusLabel->setText(tr("Request cancelled"));
    
    requestButton->setEnabled(true);
    cancelButton->setEnabled(false);
}

void LLMAssistantDialog::onModificationSelected(int index) {
    if (index < 0 || index >= modifications.size()) {
        acceptButton->setEnabled(false);
        rejectButton->setEnabled(false);
        return;
    }
    
    const CodeModification& mod = modifications[index];
    showModificationPreview(mod);
    
    confidenceLabel->setText(QString("%1%").arg(static_cast<int>(mod.confidence * 100)));
    explanationLabel->setText(mod.explanation);
    
    acceptButton->setEnabled(true);
    rejectButton->setEnabled(true);
    acceptAllButton->setEnabled(!modifications.isEmpty());
    rejectAllButton->setEnabled(!modifications.isEmpty());
    applyButton->setEnabled(true);
}

void LLMAssistantDialog::onAcceptModification() {
    int index = suggestionsList->currentRow();
    if (index < 0 || index >= modifications.size()) return;
    
    modifications[index].accepted = true;
    updateModificationStatus(index, true);
}

void LLMAssistantDialog::onRejectModification() {
    int index = suggestionsList->currentRow();
    if (index < 0 || index >= modifications.size()) return;
    
    modifications[index].accepted = false;
    updateModificationStatus(index, false);
}

void LLMAssistantDialog::onAcceptAll() {
    for (int i = 0; i < modifications.size(); i++) {
        modifications[i].accepted = true;
        updateModificationStatus(i, true);
    }
}

void LLMAssistantDialog::onRejectAll() {
    for (int i = 0; i < modifications.size(); i++) {
        modifications[i].accepted = false;
        updateModificationStatus(i, false);
    }
}

void LLMAssistantDialog::onApplySelected() {
    QVector<int> acceptedIds;
    for (int i = 0; i < modifications.size(); i++) {
        if (modifications[i].accepted) {
            acceptedIds.append(modifications[i].id);
        }
    }
    
    if (acceptedIds.isEmpty()) {
        QMessageBox::information(this, tr("Info"), tr("No modifications selected"));
        return;
    }
    
    emit modificationsApplied(acceptedIds);
    accept();
}

void LLMAssistantDialog::onPromptChanged() {
    // Enable/disable request button based on prompt content
    requestButton->setEnabled(!promptEdit->toPlainText().isEmpty());
}

void LLMAssistantDialog::onLLMResponse(const QVector<CodeSuggestion>& suggestions) {
    isProcessing = false;
    progressBar->setVisible(false);
    
    // Convert suggestions to modifications
    modifications.clear();
    for (int i = 0; i < suggestions.size(); i++) {
        const CodeSuggestion& sugg = suggestions[i];
        CodeModification mod;
        mod.id = sugg.blockId;
        mod.startLine = sugg.startLine;
        mod.endLine = sugg.endLine;
        mod.originalCode = sugg.originalCode;
        mod.suggestedCode = sugg.suggestedCode;
        mod.explanation = sugg.explanation;
        mod.confidence = sugg.confidence;
        mod.accepted = false;
        modifications.append(mod);
    }
    
    populateSuggestions();
    
    statusLabel->setText(tr("Received %1 suggestions").arg(modifications.size()));
    
    requestButton->setEnabled(true);
    cancelButton->setEnabled(false);
    
    if (!modifications.isEmpty()) {
        acceptAllButton->setEnabled(true);
        rejectAllButton->setEnabled(true);
    }
    
    LOG_INFO("Received " + std::to_string(modifications.size()) + " LLM suggestions");
}

void LLMAssistantDialog::onLLMError(const QString& error) {
    isProcessing = false;
    progressBar->setVisible(false);
    statusLabel->setText(tr("Error: ") + error);
    
    requestButton->setEnabled(true);
    cancelButton->setEnabled(false);
    
    QMessageBox::critical(this, tr("LLM Error"), error);
    
    LOG_ERROR("LLM error: " + error.toStdString());
}

void LLMAssistantDialog::onProcessingStarted() {
    isProcessing = true;
    progressBar->setVisible(true);
    statusLabel->setText(tr("Processing..."));
}

void LLMAssistantDialog::onProcessingFinished() {
    isProcessing = false;
    progressBar->setVisible(false);
}

void LLMAssistantDialog::populateSuggestions() {
    suggestionsList->clear();
    
    for (int i = 0; i < modifications.size(); i++) {
        const CodeModification& mod = modifications[i];
        
        QListWidgetItem* item = new QListWidgetItem();
        
        QString text = QString("Modification %1 (Lines %2-%3)")
            .arg(i + 1)
            .arg(mod.startLine)
            .arg(mod.endLine);
        
        item->setText(text);
        
        // Add confidence indicator
        QString confidence = QString("%1%").arg(static_cast<int>(mod.confidence * 100));
        item->setData(Qt::UserRole, confidence);
        
        suggestionsList->addItem(item);
    }
    
    if (suggestionsList->count() > 0) {
        suggestionsList->setCurrentRow(0);
    }
}

void LLMAssistantDialog::showModificationPreview(const CodeModification& mod) {
    originalPreview->setPlainText(mod.originalCode);
    suggestedPreview->setPlainText(mod.suggestedCode);
}

void LLMAssistantDialog::updateModificationStatus(int index, bool accepted) {
    if (index < 0 || index >= suggestionsList->count()) return;
    
    QListWidgetItem* item = suggestionsList->item(index);
    if (item) {
        QString text = item->text();
        if (accepted) {
            item->setText(text + " ✓");
            item->setForeground(Qt::darkGreen);
        } else {
            item->setText(text.replace(" ✓", ""));
            item->setForeground(Qt::black);
        }
    }
}

QString LLMAssistantDialog::buildPrompt() const {
    QString prompt = promptEdit->toPlainText();
    
    if (prompt.isEmpty()) {
        return "";
    }
    
    // Add context if requested
    if (includeContextCheck->isChecked()) {
        prompt += "\n\nContext: This code is from file " + currentFile;
        prompt += ", lines " + QString::number(selectionStart) + "-" + QString::number(selectionEnd);
    }
    
    // Add template-specific instructions
    QString templateType = promptTemplateCombo->currentData().toString();
    if (templateType == "optimize") {
        prompt += "\n\nFocus on performance optimization. Consider using vectorization, parallel processing, or algorithmic improvements.";
    } else if (templateType == "fix_bugs") {
        prompt += "\n\nFocus on identifying and fixing potential bugs, edge cases, and error handling.";
    } else if (templateType == "add_comments") {
        prompt += "\n\nAdd clear, concise comments explaining the purpose and logic of the code.";
    } else if (templateType == "refactor") {
        prompt += "\n\nImprove code structure, readability, and maintainability while preserving functionality.";
    } else if (templateType == "add_tests") {
        prompt += "\n\nGenerate comprehensive test cases covering normal operation, edge cases, and error conditions.";
    }
    
    return prompt;
}

QVector<CodeModification> LLMAssistantDialog::getAcceptedModifications() const {
    QVector<CodeModification> accepted;
    for (const CodeModification& mod : modifications) {
        if (mod.accepted) {
            accepted.append(mod);
        }
    }
    return accepted;
}

} // namespace proxima