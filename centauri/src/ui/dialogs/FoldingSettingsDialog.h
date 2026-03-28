#pragma once

#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QGroupBox>

namespace centauri::ui {

/**
 * @brief Диалог настроек сворачивания кода (code folding)
 * 
 * Позволяет пользователю настроить поведение и внешний вид
 * функционала сворачивания блоков кода в редакторе.
 */
class FoldingSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit FoldingSettingsDialog(QWidget* parent = nullptr);
    ~FoldingSettingsDialog() override;

    /**
     * @brief Загрузить текущие настройки
     * @param settings Объект настроек (или путь к файлу настроек)
     */
    void loadSettings(const QString& settingsPath = QString());

    /**
     * @brief Сохранить текущие настройки
     * @param settingsPath Путь к файлу настроек
     * @return true если успешно
     */
    bool saveSettings(const QString& settingsPath = QString());

    /**
     * @brief Применить настройки к редактору
     */
    void applySettings();

    // Геттеры настроек
    bool isFoldingEnabled() const { return m_enableFolding->isChecked(); }
    bool isShowFoldingIcons() const { return m_showIcons->isChecked(); }
    bool isAutoFoldFunctions() const { return m_autoFoldFunctions->isChecked(); }
    bool isAutoFoldClasses() const { return m_autoFoldClasses->isChecked(); }
    bool isAutoFoldComments() const { return m_autoFoldComments->isChecked(); }
    int getMinFoldLines() const { return m_minLines->value(); }
    QString getFoldColor() const { return m_colorCombo->currentData().toString(); }

signals:
    void settingsChanged();
    void settingsApplied();

private slots:
    void onApplyClicked();
    void onSaveClicked();
    void onLoadClicked();
    void onResetClicked();
    void onPreviewChanged();
    void updatePreview();

private:
    void setupUI();
    void setupPreview();
    void populateColorCombo();

    // Основные настройки
    QCheckBox* m_enableFolding;
    QCheckBox* m_showIcons;
    QCheckBox* m_autoFoldFunctions;
    QCheckBox* m_autoFoldClasses;
    QCheckBox* m_autoFoldComments;
    
    // Дополнительные настройки
    QSpinBox* m_minLines;
    QComboBox* m_colorCombo;
    
    // Предпросмотр
    QTextEdit* m_previewText;
    
    // Кнопки
    QPushButton* m_applyButton;
    QPushButton* m_saveButton;
    QPushButton* m_loadButton;
    QPushButton* m_resetButton;
    QPushButton* m_closeButton;
    
    QString m_settingsPath;
};

} // namespace centauri::ui
