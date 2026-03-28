#pragma once

#include <QDialog>
#include <QTreeWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QStringList>

namespace centauri::ui {

/**
 * @brief Диалог отчёта о совместимости кода с различными версиями и платформами
 * 
 * Анализирует код на предмет совместимости с разными версиями языка,
 * операционными системами и архитектурами процессоров.
 */
class CompatibilityReportDialog : public QDialog {
    Q_OBJECT

public:
    explicit CompatibilityReportDialog(QWidget* parent = nullptr);
    ~CompatibilityReportDialog() override;

    /**
     * @brief Запустить анализ совместимости
     * @param filePath Путь к файлу для анализа
     * @param targets Список целевых платформ
     */
    void analyzeCompatibility(const QString& filePath, const QStringList& targets);

    /**
     * @brief Добавить проблему совместимости
     * @param category Категория проблемы
     * @param severity Уровень серьёзности (error, warning, info)
     * @param message Описание проблемы
     * @param line Номер строки
     */
    void addIssue(const QString& category, const QString& severity, 
                  const QString& message, int line = -1);

    /**
     * @brief Очистить отчёт
     */
    void clear();

    /**
     * @brief Экспортировать отчёт в файл
     * @param fileName Путь к файлу
     * @return true если успешно
     */
    bool exportReport(const QString& fileName);

    /**
     * @brief Получить количество проблем
     */
    int getErrorCount() const { return m_errorCount; }
    int getWarningCount() const { return m_warningCount; }
    int getInfoCount() const { return m_infoCount; }

signals:
    void analysisStarted();
    void analysisFinished();
    void issueSelected(int line);

private slots:
    void onItemClicked(QTreeWidgetItem* item);
    void onExportClicked();
    void onCloseClicked();
    void updateProgress();

private:
    void setupUI();
    void categorizeIssues();
    QTreeWidgetItem* createIssueItem(const QString& category, const QString& severity,
                                     const QString& message, int line);

    QTreeWidget* m_issuesTree;
    QTextEdit* m_detailsText;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QLabel* m_summaryLabel;
    QPushButton* m_exportButton;
    QPushButton* m_closeButton;
    
    QStringList m_targets;
    QString m_currentFile;
    int m_errorCount;
    int m_warningCount;
    int m_infoCount;
    bool m_isAnalyzing;
};

} // namespace centauri::ui
