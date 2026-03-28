#include "BuildReportDialog.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QRegularExpression>

namespace centauri::ui {

BuildReportDialog::BuildReportDialog(QWidget* parent)
    : QDialog(parent)
    , m_elapsedTime(0)
    , m_isBuilding(false)
{
    setWindowTitle("Отчёт о сборке");
    setMinimumSize(600, 400);
    resize(800, 500);
    
    setupUI();
    
    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this, &BuildReportDialog::updateProgress);
}

BuildReportDialog::~BuildReportDialog() = default;

void BuildReportDialog::setupUI() {
    auto* layout = new QVBoxLayout(this);
    
    // Статус бар
    auto* statusLayout = new QHBoxLayout();
    m_statusLabel = new QLabel("Готов к сборке");
    m_statusLabel->setStyleSheet("font-weight: bold;");
    statusLayout->addWidget(m_statusLabel);
    
    m_timeLabel = new QLabel("00:00");
    m_timeLabel->setMinimumWidth(80);
    statusLayout->addWidget(m_timeLabel);
    
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 0); // Бесконечная прогресс-бар
    m_progressBar->setVisible(false);
    statusLayout->addWidget(m_progressBar);
    
    statusLayout->addStretch();
    
    m_stopButton = new QPushButton("Остановить");
    m_stopButton->setEnabled(false);
    connect(m_stopButton, &QPushButton::clicked, this, &BuildReportDialog::onStopClicked);
    statusLayout->addWidget(m_stopButton);
    
    layout->addLayout(statusLayout);
    
    // Текстовое поле вывода
    m_outputText = new QTextEdit();
    m_outputText->setReadOnly(true);
    m_outputText->setFont(QFont("Consolas", 10));
    layout->addWidget(m_outputText);
    
    // Кнопки управления
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_saveButton = new QPushButton("Сохранить");
    connect(m_saveButton, &QPushButton::clicked, this, &BuildReportDialog::onSaveClicked);
    buttonLayout->addWidget(m_saveButton);
    
    m_clearButton = new QPushButton("Очистить");
    connect(m_clearButton, &QPushButton::clicked, this, &BuildReportDialog::onClearClicked);
    buttonLayout->addWidget(m_clearButton);
    
    auto* closeButton = new QPushButton("Закрыть");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(closeButton);
    
    layout->addLayout(buttonLayout);
}

void BuildReportDialog::appendMessage(const QString& message, const QString& type) {
    m_messages.append(message);
    
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString formattedMessage = QString("[%1] %2").arg(timestamp, message);
    
    QColor color;
    if (type == "error") {
        color = Qt::red;
    } else if (type == "warning") {
        color = Qt::darkYellow;
    } else {
        color = Qt::black;
    }
    
    m_outputText->setTextColor(color);
    m_outputText->append(formattedMessage);
    m_outputText->scrollToBottom();
}

void BuildReportDialog::startBuild(const QString& projectPath) {
    clear();
    m_isBuilding = true;
    m_elapsedTime = 0;
    
    m_statusLabel->setText("Сборка: " + projectPath);
    m_statusLabel->setStyleSheet("font-weight: bold; color: blue;");
    m_progressBar->setVisible(true);
    m_stopButton->setEnabled(true);
    
    appendMessage("Начало сборки проекта: " + projectPath, "info");
    appendMessage("------------------------------------------------", "info");
    
    m_progressTimer->start(1000);
    emit buildStarted();
}

void BuildReportDialog::finishBuild(bool success, int duration) {
    m_isBuilding = false;
    m_progressTimer->stop();
    m_progressBar->setVisible(false);
    m_stopButton->setEnabled(false);
    
    appendMessage("------------------------------------------------", "info");
    if (success) {
        appendMessage(QString("Сборка успешно завершена за %1 сек.").arg(duration / 1000.0, 0, 'f', 2), "info");
        m_statusLabel->setText("Сборка завершена успешно");
        m_statusLabel->setStyleSheet("font-weight: bold; color: green;");
    } else {
        appendMessage(QString("Сборка завершена с ошибками за %1 сек.").arg(duration / 1000.0, 0, 'f', 2), "error");
        m_statusLabel->setText("Сборка завершена с ошибками");
        m_statusLabel->setStyleSheet("font-weight: bold; color: red;");
    }
    
    emit buildFinished(success);
}

void BuildReportDialog::stopBuild() {
    if (!m_isBuilding) return;
    
    m_isBuilding = false;
    m_progressTimer->stop();
    m_progressBar->setVisible(false);
    m_stopButton->setEnabled(false);
    
    appendMessage("Сборка остановлена пользователем", "warning");
    m_statusLabel->setText("Сборка остановлена");
    m_statusLabel->setStyleSheet("font-weight: bold; color: orange;");
    
    emit buildStopped();
}

void BuildReportDialog::clear() {
    m_outputText->clear();
    m_messages.clear();
    m_elapsedTime = 0;
    m_timeLabel->setText("00:00");
    m_statusLabel->setText("Готов к сборке");
    m_statusLabel->setStyleSheet("font-weight: bold;");
}

void BuildReportDialog::onStopClicked() {
    stopBuild();
}

void BuildReportDialog::onSaveClicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Сохранить отчёт о сборке",
        "build_report.txt",
        "Текстовые файлы (*.txt);;Все файлы (*)"
    );
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "Отчёт о сборке - " << QDateTime::currentDateTime().toString() << "\n\n";
        for (const auto& msg : m_messages) {
            out << msg << "\n";
        }
        file.close();
        emit saveRequested();
    }
}

void BuildReportDialog::onClearClicked() {
    clear();
}

void BuildReportDialog::updateProgress() {
    m_elapsedTime++;
    int minutes = m_elapsedTime / 60;
    int seconds = m_elapsedTime % 60;
    m_timeLabel->setText(QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')));
}

void BuildReportDialog::highlightMessage(QTextDocument* doc, const QString& message, const QString& type) {
    Q_UNUSED(doc)
    Q_UNUSED(message)
    Q_UNUSED(type)
    // Реализация подсветки синтаксиса может быть добавлена при необходимости
}

} // namespace centauri::ui
