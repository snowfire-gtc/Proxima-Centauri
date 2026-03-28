#pragma once

#include <QDialog>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QStringList>

namespace centauri::ui {

/**
 * @brief Диалог управления сниппетами кода
 * 
 * Позволяет создавать, редактировать, удалять и организовывать
 * сниппеты кода для быстрой вставки в редакторе.
 */
class SnippetManagerDialog : public QDialog {
    Q_OBJECT

public:
    explicit SnippetManagerDialog(QWidget* parent = nullptr);
    ~SnippetManagerDialog() override;

    /**
     * @brief Загрузить сниппеты из файла
     * @param filePath Путь к файлу со сниппетами
     * @return true если успешно
     */
    bool loadSnippets(const QString& filePath = QString());

    /**
     * @brief Сохранить сниппеты в файл
     * @param filePath Путь к файлу
     * @return true если успешно
     */
    bool saveSnippets(const QString& filePath = QString());

    /**
     * @brief Добавить новый сниппет
     * @param name Название сниппета
     * @param content Содержимое сниппета
     * @param category Категория
     * @param shortcut Горячие клавиши
     */
    void addSnippet(const QString& name, const QString& content,
                    const QString& category = "General",
                    const QString& shortcut = "");

    /**
     * @brief Получить выбранный сниппет
     * @return Содержимое выбранного сниппета
     */
    QString getSelectedSnippet() const;

    /**
     * @brief Получить количество сниппетов
     */
    int getSnippetCount() const { return m_snippetList->count(); }

signals:
    void snippetSelected(const QString& name, const QString& content);
    void snippetInserted(const QString& content);
    void snippetsLoaded();
    void snippetsSaved();

private slots:
    void onNewClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onImportClicked();
    void onExportClicked();
    void onInsertClicked();
    void onSelectionChanged();
    void onSearchTextChanged(const QString& text);
    void onCategoryChanged(const QString& category);
    void updateSnippetList();

private:
    void setupUI();
    void populateCategories();
    void clearInputs();
    bool validateInputs();
    int findSnippetByName(const QString& name);

    // Список сниппетов
    QListWidget* m_snippetList;
    
    // Поиск и фильтрация
    QLineEdit* m_searchEdit;
    QComboBox* m_categoryCombo;
    
    // Редактирование
    QLineEdit* m_nameEdit;
    QTextEdit* m_contentEdit;
    QComboBox* m_categoryEdit;
    QLineEdit* m_shortcutEdit;
    
    // Кнопки
    QPushButton* m_newButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_importButton;
    QPushButton* m_exportButton;
    QPushButton* m_insertButton;
    QPushButton* m_closeButton;
    
    // Данные
    struct Snippet {
        QString name;
        QString content;
        QString category;
        QString shortcut;
    };
    
    QList<Snippet> m_snippets;
    QString m_currentFile;
};

} // namespace centauri::ui
