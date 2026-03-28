#include "CompatibilityReportDialog.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMap>

namespace centauri::ui {

CompatibilityReportDialog::CompatibilityReportDialog(QWidget* parent)
    : QDialog(parent)
    , m_errorCount(0)
    , m_warningCount(0)
    , m_infoCount(0)
    , m_isAnalyzing(false)
{
    setWindowTitle("Отчёт о совместимости");
    setMinimumSize(700, 500);
    resize(800, 600);
    
    setupUI();
}

CompatibilityReportDialog::~CompatibilityReportDialog() = default;

void CompatibilityReportDialog::setupUI() {
    auto* layout = new QVBoxLayout(this);
    
    // Статус бар
    auto* statusLayout = new QHBoxLayout();
    m_statusLabel = new QLabel("Готов к анализу");
    m_statusLabel->setStyleSheet("font-weight: bold;");
    statusLayout->addWidget(m_statusLabel);
    
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    statusLayout->addWidget(m_progressBar);
    
    statusLayout->addStretch();
    layout->addLayout(statusLayout);
    
    // Дерево проблем
    m_issuesTree = new QTreeWidget();
    m_issuesTree->setHeaderLabels(QStringList() << "Категория" << "Проблема" << "Строка" << "Платформа");
    m_issuesTree->setColumnWidth(0, 150);
    m_issuesTree->setColumnWidth(1, 400);
    m_issuesTree->setColumnWidth(2, 80);
    m_issuesTree->setColumnWidth(3, 150);
    m_issuesTree->setAlternatingRowColors(true);
    connect(m_issuesTree, &QTreeWidget::itemClicked, 
            this, &CompatibilityReportDialog::onItemClicked);
    layout->addWidget(m_issuesTree);
    
    // Детали
    auto* detailsLayout = new QVBoxLayout();
    auto* detailsLabel = new QLabel("Детали:");
    detailsLabel->setFont(QFont("", 10, QFont::Bold));
    detailsLayout->addWidget(detailsLabel);
    
    m_detailsText = new QTextEdit();
    m_detailsText->setReadOnly(true);
    m_detailsText->setMaximumHeight(120);
    detailsLayout->addWidget(m_detailsText);
    layout->addLayout(detailsLayout);
    
    // Сводка и кнопки
    auto* bottomLayout = new QHBoxLayout();
    
    m_summaryLabel = new QLabel("Ошибок: 0 | Предупреждений: 0 | Информации: 0");
    m_summaryLabel->setStyleSheet("font-weight: bold;");
    bottomLayout->addWidget(m_summaryLabel);
    
    bottomLayout->addStretch();
    
    m_exportButton = new QPushButton("Экспорт");
    connect(m_exportButton, &QPushButton::clicked, this, &CompatibilityReportDialog::onExportClicked);
    bottomLayout->addWidget(m_exportButton);
    
    m_closeButton = new QPushButton("Закрыть");
    connect(m_closeButton, &QPushButton::clicked, this, &CompatibilityReportDialog::onCloseClicked);
    bottomLayout->addWidget(m_closeButton);
    
    layout->addLayout(bottomLayout);
}

void CompatibilityReportDialog::analyzeCompatibility(const QString& filePath, const QStringList& targets) {
    clear();
    m_currentFile = filePath;
    m_targets = targets;
    m_isAnalyzing = true;
    
    m_statusLabel->setText("Анализ: " + filePath);
    m_statusLabel->setStyleSheet("font-weight: bold; color: blue;");
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    
    emit analysisStarted();
    
    // Имитация анализа (в реальной реализации здесь будет статический анализ)
    for (const auto& target : targets) {
        // Пример добавления проблем для демонстрации
        if (target.contains("Windows")) {
            addIssue("API", "warning", "Используется POSIX-специфичная функция", 42);
        }
        if (target.contains("Linux")) {
            addIssue("Paths", "info", "Пути используют обратные слеши", 15);
        }
        if (target.contains("macOS")) {
            addIssue("Frameworks", "info", "Требуется проверка наличия фреймворка", 78);
        }
    }
    
    m_progressBar->setValue(100);
    m_isAnalyzing = false;
    
    m_statusLabel->setText("Анализ завершён");
    m_statusLabel->setStyleSheet("font-weight: bold; color: green;");
    
    updateSummary();
    categorizeIssues();
    
    emit analysisFinished();
}

void CompatibilityReportDialog::addIssue(const QString& category, const QString& severity,
                                         const QString& message, int line) {
    auto* item = createIssueItem(category, severity, message, line);
    m_issuesTree->addTopLevelItem(item);
    
    if (severity == "error") {
        m_errorCount++;
        item->setForeground(0, Qt::red);
    } else if (severity == "warning") {
        m_warningCount++;
        item->setForeground(0, Qt::darkYellow);
    } else {
        m_infoCount++;
        item->setForeground(0, Qt::blue);
    }
    
    // Добавляем информацию о платформах
    for (const auto& target : m_targets) {
        auto* child = new QTreeWidgetItem();
        child->setText(0, "");
        child->setText(1, message);
        child->setText(2, line > 0 ? QString::number(line) : "-");
        child->setText(3, target);
        item->addChild(child);
    }
}

void CompatibilityReportDialog::clear() {
    m_issuesTree->clear();
    m_detailsText->clear();
    m_errorCount = 0;
    m_warningCount = 0;
    m_infoCount = 0;
    m_currentFile.clear();
    m_targets.clear();
    
    m_statusLabel->setText("Готов к анализу");
    m_statusLabel->setStyleSheet("font-weight: bold;");
    m_progressBar->setVisible(false);
    m_progressBar->setValue(0);
    
    updateSummary();
}

bool CompatibilityReportDialog::exportReport(const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out << "Отчёт о совместимости - " << QDateTime::currentDateTime().toString() << "\n";
    out << "Файл: " << m_currentFile << "\n";
    out << "Целевые платформы: " << m_targets.join(", ") << "\n\n";
    
    out << "Сводка:\n";
    out << "  Ошибок: " << m_errorCount << "\n";
    out << "  Предупреждений: " << m_warningCount << "\n";
    out << "  Информации: " << m_infoCount << "\n\n";
    
    out << "Проблемы:\n";
    for (int i = 0; i < m_issuesTree->topLevelItemCount(); ++i) {
        auto* item = m_issuesTree->topLevelItem(i);
        out << "[" << item->text(0) << "] " << item->text(1) 
            << " (строка " << item->text(2) << ")\n";
        
        for (int j = 0; j < item->childCount(); ++j) {
            auto* child = item->child(j);
            out << "  - " << child->text(3) << ": " << child->text(1) << "\n";
        }
    }
    
    file.close();
    return true;
}

void CompatibilityReportDialog::onItemClicked(QTreeWidgetItem* item) {
    if (!item) return;
    
    QString details;
    details += "Категория: " + item->text(0) + "\n";
    details += "Проблема: " + item->text(1) + "\n";
    details += "Строка: " + item->text(2) + "\n";
    details += "Платформа: " + item->text(3) + "\n\n";
    
    if (item->text(0).contains("API")) {
        details += "Рекомендация: Используйте кроссплатформенные абстракции или условную компиляцию.\n";
    } else if (item->text(0).contains("Paths")) {
        details += "Рекомендация: Используйте QDir для работы с путями.\n";
    } else if (item->text(0).contains("Frameworks")) {
        details += "Рекомендация: Проверьте наличие фреймворка перед использованием.\n";
    }
    
    m_detailsText->setText(details);
    
    int line = item->text(2).toInt();
    if (line > 0) {
        emit issueSelected(line);
    }
}

void CompatibilityReportDialog::onExportClicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Экспорт отчёта о совместимости",
        "compatibility_report.txt",
        "Текстовые файлы (*.txt);;Все файлы (*)"
    );
    
    if (fileName.isEmpty()) return;
    
    if (exportReport(fileName)) {
        m_statusLabel->setText("Отчёт экспортирован: " + fileName);
    } else {
        m_statusLabel->setText("Ошибка экспорта");
    }
}

void CompatibilityReportDialog::onCloseClicked() {
    accept();
}

void CompatibilityReportDialog::updateProgress() {
    // Может использоваться для обновления прогресса во время анализа
}

void CompatibilityReportDialog::updateSummary() {
    m_summaryLabel->setText(
        QString("Ошибок: %1 | Предупреждений: %2 | Информации: %3")
            .arg(m_errorCount).arg(m_warningCount).arg(m_infoCount)
    );
}

void CompatibilityReportDialog::categorizeIssues() {
    // Группировка проблем по категориям (может быть расширена)
    QMap<QString, QList<QTreeWidgetItem*>> categorized;
    
    for (int i = 0; i < m_issuesTree->topLevelItemCount(); ++i) {
        auto* item = m_issuesTree->topLevelItem(i);
        categorized[item->text(0)].append(item);
    }
    
    // Сортировка категорий по количеству проблем
    m_issuesTree->sortItems(0, Qt::DescendingOrder);
}

QTreeWidgetItem* CompatibilityReportDialog::createIssueItem(const QString& category, 
                                                            const QString& severity,
                                                            const QString& message, 
                                                            int line) {
    auto* item = new QTreeWidgetItem();
    item->setText(0, category);
    item->setText(1, message);
    item->setText(2, line > 0 ? QString::number(line) : "-");
    item->setText(3, m_targets.first());
    
    if (severity == "error") {
        item->setIcon(0, QIcon::fromTheme("dialog-error"));
    } else if (severity == "warning") {
        item->setIcon(0, QIcon::fromTheme("dialog-warning"));
    } else {
        item->setIcon(0, QIcon::fromTheme("dialog-information"));
    }
    
    return item;
}

} // namespace centauri::ui
