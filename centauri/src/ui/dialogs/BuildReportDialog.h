#pragma once

#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QStringList>

namespace centauri::ui {

/**
 * @brief Диалог отображения отчёта о сборке проекта
 * 
 * Отображает вывод компилятора, линковщика и других инструментов сборки
 * с подсветкой ошибок, предупреждений и информационных сообщений.
 * Предоставляет возможность остановки сборки и сохранения отчёта.
 */
class BuildReportDialog : public QDialog {
    Q_OBJECT

public:
    explicit BuildReportDialog(QWidget* parent = nullptr);
    ~BuildReportDialog() override;

    /**
     * @brief Добавить сообщение в отчёт
     * @param message Текст сообщения
     * @param type Тип сообщения (error, warning, info)
     */
    void appendMessage(const QString& message, const QString& type = "info");

    /**
     * @brief Начать новую сборку
     * @param projectPath Путь к проекту
     */
    void startBuild(const QString& projectPath);

    /**
     * @brief Завершить сборку
     * @param success Успешно ли завершена сборка
     * @param duration Длительность сборки в мс
     */
    void finishBuild(bool success, int duration);

    /**
     * @brief Остановить текущую сборку
     */
    void stopBuild();

    /**
     * @brief Очистить отчёт
     */
    void clear();

    /**
     * @brief Проверить, идёт ли сборка
     */
    bool isBuilding() const { return m_isBuilding; }

signals:
    void buildStarted();
    void buildFinished(bool success);
    void buildStopped();
    void saveRequested();

private slots:
    void onStopClicked();
    void onSaveClicked();
    void onClearClicked();
    void updateProgress();

private:
    void setupUI();
    void highlightMessage(QTextDocument* doc, const QString& message, const QString& type);

    QTextEdit* m_outputText;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QLabel* m_timeLabel;
    QPushButton* m_stopButton;
    QPushButton* m_saveButton;
    QPushButton* m_clearButton;
    
    QTimer* m_progressTimer;
    int m_elapsedTime;
    bool m_isBuilding;
    QStringList m_messages;
};

} // namespace centauri::ui
