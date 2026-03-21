#ifndef CENTAURI_FOLDINGSETTINGSDIALOG_H
#define CENTAURI_FOLDINGSETTINGSDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QSpinBox>
#include <QColorDialog>
#include <QPushButton>
#include "editor/CodeFoldingManager.h"

namespace proxima {

class FoldingSettingsDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit FoldingSettingsDialog(QWidget *parent = nullptr);
    ~FoldingSettingsDialog();
    
    void loadSettings();
    void saveSettings();
    
private slots:
    void onApply();
    void onOk();
    void onCancel();
    void onReset();
    void onFoldColorChanged();
    void onHoverColorChanged();
    void onPreviewAll();
    void onFoldAll();
    void onUnfoldAll();
    
private:
    void setupUI();
    void setupGeneralGroup();
    void setupAutoFoldGroup();
    void setupAppearanceGroup();
    void setupPreviewGroup();
    
    // General
    QCheckBox* enableFoldingCheck;
    QCheckBox* preserveStateCheck;
    QSpinBox* maxPreviewSpin;
    
    // Auto-fold
    QCheckBox* autoFoldRegionsCheck;
    QCheckBox* autoFoldMethodsCheck;
    QCheckBox* autoFoldClassesCheck;
    QCheckBox* autoFoldCommentsCheck;
    
    // Appearance
    QPushButton* foldColorButton;
    QPushButton* hoverColorButton;
    QColor foldColor;
    QColor hoverColor;
    
    // Preview
    QTextEdit* previewText;
    QPushButton* previewAllButton;
    QPushButton* foldAllButton;
    QPushButton* unfoldAllButton;
    
    FoldConfig currentConfig;
};

} // namespace proxima

#endif // CENTAURI_FOLDINGSETTINGSDIALOG_H