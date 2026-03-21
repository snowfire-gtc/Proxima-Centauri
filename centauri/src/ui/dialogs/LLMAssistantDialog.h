#ifndef CENTAURI_LLMASSISTANTDIALOG_H
#define CENTAURI_LLMASSISTANTDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QListWidget>
#include <QSplitter>
#include <QProgressBar>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include "services/llm/LLMService.h"

namespace proxima {

struct CodeModification {
    int id;
    int startLine;
    int endLine;
    QString originalCode;
    QString suggestedCode;
    QString explanation;
    double confidence;
    bool accepted;
};

class LLMAssistantDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit LLMAssistantDialog(QWidget *parent = nullptr);
    ~LLMAssistantDialog();
    
    // Request configuration
    void setFile(const QString& file);
    void setSelection(int startLine, int endLine, const QString& code);
    void setPrompt(const QString& prompt);
    
    // Results
    QVector<CodeModification> getModifications() const { return modifications; }
    QVector<CodeModification> getAcceptedModifications() const;
    
signals:
    void requestSent(const QString& prompt);
    void responseReceived(const QVector<CodeModification>& modifications);
    void modificationsApplied(const QVector<int>& acceptedIds);
    
private slots:
    void onRequestAssistance();
    void onCancelRequest();
    void onModificationSelected(int index);
    void onAcceptModification();
    void onRejectModification();
    void onAcceptAll();
    void onRejectAll();
    void onApplySelected();
    void onPromptChanged();
    void onLLMResponse(const QVector<CodeSuggestion>& suggestions);
    void onLLMError(const QString& error);
    void onProcessingStarted();
    void onProcessingFinished();
    
private:
    void setupUI();
    void setupPromptGroup();
    void setupSuggestionsGroup();
    void setupPreviewGroup();
    void setupButtonGroup();
    void populateSuggestions();
    void showModificationPreview(const CodeModification& mod);
    void updateModificationStatus(int index, bool accepted);
    QString buildPrompt() const;
    
    // Prompt section
    QTextEdit* promptEdit;
    QComboBox* promptTemplateCombo;
    QCheckBox* includeContextCheck;
    QCheckBox* includeCommentsCheck;
    QPushButton* requestButton;
    QPushButton* cancelButton;
    
    // Suggestions section
    QListWidget* suggestionsList;
    QLabel* confidenceLabel;
    QLabel* explanationLabel;
    
    // Preview section
    QTextEdit* originalPreview;
    QTextEdit* suggestedPreview;
    QSplitter* previewSplitter;
    
    // Progress
    QProgressBar* progressBar;
    QLabel* statusLabel;
    
    // Buttons
    QPushButton* acceptButton;
    QPushButton* rejectButton;
    QPushButton* acceptAllButton;
    QPushButton* rejectAllButton;
    QPushButton* applyButton;
    QPushButton* closeButton;
    
    // State
    QString currentFile;
    int selectionStart;
    int selectionEnd;
    QString selectedCode;
    QVector<CodeModification> modifications;
    bool isProcessing;
    
    // LLM Service
    LLMService* llmService;
};

} // namespace proxima

#endif // CENTAURI_LLMASSISTANTDIALOG_H