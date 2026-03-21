#ifndef CENTAURI_SNIPPETMANAGERDIALOG_H
#define CENTAURI_SNIPPETMANAGERDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include "editor/SnippetManager.h"

namespace proxima {

class SnippetManagerDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit SnippetManagerDialog(QWidget *parent = nullptr);
    ~SnippetManagerDialog();
    
    void loadSnippets();
    void saveSnippets();
    
private slots:
    void onSnippetSelected(int index);
    void onAddSnippet();
    void onDeleteSnippet();
    void onSave();
    void onImport();
    void onExport();
    void onPreview();
    void onTriggerChanged(const QString& text);
    void onContentChanged();
    
private:
    void setupUI();
    void updatePreview();
    void updateSnippetFromUI();
    void loadSnippetToUI(const SnippetDefinition& snippet);
    void clearUI();
    
    // Snippet list
    QListWidget* snippetList;
    
    // Snippet editor
    QLineEdit* triggerEdit;
    QLineEdit* nameEdit;
    QTextEdit* contentEdit;
    QTextEdit* descriptionEdit;
    QTextEdit* previewEdit;
    
    // Options
    QCheckBox* enabledCheck;
    
    // Buttons
    QPushButton* addButton;
    QPushButton* deleteButton;
    QPushButton* saveButton;
    QPushButton* importButton;
    QPushButton* exportButton;
    QPushButton* previewButton;
    QPushButton* closeButton;
    
    QVector<SnippetDefinition> snippets;
    int currentIndex;
    bool isModified;
};

} // namespace proxima

#endif // CENTAURI_SNIPPETMANAGERDIALOG_H