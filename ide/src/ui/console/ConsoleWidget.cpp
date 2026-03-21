#include "ConsoleWidget.h"
#include <QDateTime>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QInputDialog>

namespace proxima {

ConsoleWidget::ConsoleWidget(QWidget *parent)
    : QWidget(parent)
    , verboseLevel(2)
    , autoScroll(true)
    , maxLines(10000)
    , currentTabType(ConsoleTabType::Build) {
    
    setupUI();
    createTabs();
    setupConnections();
}

ConsoleWidget::~ConsoleWidget() {}

void ConsoleWidget::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    tabWidget = new QTabWidget(this);
    tabWidget->setDocumentMode(true);
    tabWidget->setTabsClosable(false);
    tabWidget->setMovable(false);
    
    layout->addWidget(tabWidget);
    
    // Context menu
    contextMenu = new QMenu(this);
    clearAction = contextMenu->addAction("Clear");
    saveAction = contextMenu->addAction("Save...");
    copyAction = contextMenu->addAction("Copy");
    contextMenu->addSeparator();
    filterAction = contextMenu->addAction("Filter...");
    
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void ConsoleWidget::createTabs() {
    // Build tab
    ConsoleTab* buildTab = new ConsoleTab(ConsoleTabType::Build, this);
    buildTab->setPlaceholderText("Build process output...");
    tabs[ConsoleTabType::Build] = buildTab;
    tabWidget->addTab(buildTab, QIcon(":/icons/build.svg"), "Build");
    
    // Errors tab
    ConsoleTab* errorsTab = new ConsoleTab(ConsoleTabType::Errors, this);
    errorsTab->setPlaceholderText("Errors will appear here...");
    tabs[ConsoleTabType::Errors] = errorsTab;
    tabWidget->addTab(errorsTab, QIcon(":/icons/error.svg"), "Errors");
    
    // Output tab
    ConsoleTab* outputTab = new ConsoleTab(ConsoleTabType::Output, this);
    outputTab->setPlaceholderText("Program output...");
    tabs[ConsoleTabType::Output] = outputTab;
    tabWidget->addTab(outputTab, QIcon(":/icons/output.svg"), "Output");
    
    // Call Stack tab
    ConsoleTab* callStackTab = new ConsoleTab(ConsoleTabType::CallStack, this);
    callStackTab->setPlaceholderText("Call stack (available during debug)...");
    tabs[ConsoleTabType::CallStack] = callStackTab;
    tabWidget->addTab(callStackTab, QIcon(":/icons/stack.svg"), "Call Stack");
    
    // Debug tab
    ConsoleTab* debugTab = new ConsoleTab(ConsoleTabType::Debug, this);
    debugTab->setPlaceholderText("Debug messages...");
    tabs[ConsoleTabType::Debug] = debugTab;
    tabWidget->addTab(debugTab, QIcon(":/icons/debug.svg"), "Debug");
}

void ConsoleWidget::setupConnections() {
    connect(tabWidget, &QTabWidget::currentChanged, this, &ConsoleWidget::onTabChanged);
    connect(this, &ConsoleWidget::customContextMenuRequested, this, &ConsoleWidget::onContextMenu);
    
    connect(clearAction, &QAction::triggered, this, &ConsoleWidget::onClearTab);
    connect(saveAction, &QAction::triggered, this, &ConsoleWidget::onSaveTab);
    connect(copyAction, &QAction::triggered, this, &ConsoleWidget::onCopyTab);
    connect(filterAction, &QAction::triggered, this, [this]() {
        bool ok;
        QString filter = QInputDialog::getText(this, "Filter", "Enter filter:", 
                                               QLineEdit::Normal, "", &ok);
        if (ok) {
            onFilterChanged(filter);
        }
    });
}

ConsoleTab* ConsoleWidget::getTab(ConsoleTabType type) const {
    return tabs.value(type, nullptr);
}

void ConsoleWidget::clearTab(ConsoleTabType type) {
    ConsoleTab* tab = tabs.value(type);
    if (tab) {
        tab->clear();
    }
}

void ConsoleWidget::showTab(ConsoleTabType type) {
    int index = tabWidget->indexOf(tabs.value(type));
    if (index >= 0) {
        tabWidget->setCurrentIndex(index);
    }
}

void ConsoleWidget::addBuildMessage(const QString& message, MessageType type) {
    ConsoleTab* tab = tabs[ConsoleTabType::Build];
    if (!tab) return;
    
    QString timestamp = formatTimestamp();
    QString formattedMessage;
    
    switch (type) {
        case MessageType::Error:
            formattedMessage = QString("<font color='red'>[ERROR]</font> %1 %2")
                .arg(timestamp).arg(message);
            break;
        case MessageType::Warning:
            formattedMessage = QString("<font color='orange'>[WARN]</font> %1 %2")
                .arg(timestamp).arg(message);
            break;
        case MessageType::Success:
            formattedMessage = QString("<font color='green'>[OK]</font> %1 %2")
                .arg(timestamp).arg(message);
            break;
        default:
            formattedMessage = QString("<font color='gray'>[INFO]</font> %1 %2")
                .arg(timestamp).arg(message);
            break;
    }
    
    tab->appendHtml(formattedMessage);
    
    if (autoScroll) {
        tab->scrollToBottom();
    }
}

void ConsoleWidget::addErrorMessage(const QString& message, const QString& file, int line) {
    ConsoleTab* tab = tabs[ConsoleTabType::Errors];
    if (!tab) return;
    
    QString timestamp = formatTimestamp();
    QString formattedMessage;
    
    if (!file.isEmpty()) {
        formattedMessage = QString("<font color='red'>[ERROR]</font> %1 %2:%3 - %4")
            .arg(timestamp).arg(file).arg(line).arg(message);
    } else {
        formattedMessage = QString("<font color='red'>[ERROR]</font> %1 %2")
            .arg(timestamp).arg(message);
    }
    
    tab->appendHtml(formattedMessage);
    
    // Also show in build tab
    addBuildMessage(message, MessageType::Error);
    
    // Switch to errors tab
    showTab(ConsoleTabType::Errors);
}

void ConsoleWidget::addOutputMessage(const QString& message) {
    ConsoleTab* tab = tabs[ConsoleTabType::Output];
    if (!tab) return;
    
    tab->appendPlainText(message);
    
    if (autoScroll) {
        tab->scrollToBottom();
    }
}

void ConsoleWidget::addDebugMessage(const QString& message) {
    if (verboseLevel < 3) return; // Only show if verbose enough
    
    ConsoleTab* tab = tabs[ConsoleTabType::Debug];
    if (!tab) return;
    
    QString timestamp = formatTimestamp();
    QString formattedMessage = QString("<font color='blue'>[DEBUG]</font> %1 %2")
        .arg(timestamp).arg(message);
    
    tab->appendHtml(formattedMessage);
    
    if (autoScroll) {
        tab->scrollToBottom();
    }
}

void ConsoleWidget::addCallStack(const QVector<StackFrame>& frames) {
    ConsoleTab* tab = tabs[ConsoleTabType::CallStack];
    if (!tab) return;
    
    tab->clear();
    
    QString html = "<html><body>";
    html += "<h3>Call Stack</h3>";
    html += "<table border='1' cellpadding='5'>";
    html += "<tr><th>#</th><th>Function</th><th>File</th><th>Line</th></tr>";
    
    for (int i = 0; i < frames.size(); i++) {
        const StackFrame& frame = frames[i];
        html += QString("<tr onclick='selectFrame(%1)'>")
            .arg(i);
        html += QString("<td>%1</td>").arg(i);
        html += QString("<td>%1</td>").arg(frame.function);
        html += QString("<td>%1</td>").arg(frame.filename);
        html += QString("<td>%1</td>").arg(frame.line);
        html += "</tr>";
    }
    
    html += "</table></body></html>";
    
    tab->appendHtml(html);
}

void ConsoleWidget::setVerboseLevel(int level) {
    verboseLevel = level;
    emit verboseLevelChanged(level);
}

void ConsoleWidget::setAutoScroll(bool enable) {
    autoScroll = enable;
}

void ConsoleWidget::setMaxLines(int count) {
    maxLines = count;
    
    for (auto tab : tabs) {
        tab->setMaxLines(count);
    }
}

void ConsoleWidget::saveOutput(const QString& path) {
    ConsoleTab* tab = tabs[currentTabType];
    if (!tab) return;
    
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << tab->toPlainText();
        file.close();
    }
}

void ConsoleWidget::copyOutput() {
    ConsoleTab* tab = tabs[currentTabType];
    if (!tab) return;
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(tab->toPlainText());
}

void ConsoleWidget::clearAll() {
    for (auto tab : tabs) {
        tab->clear();
    }
}

void ConsoleWidget::onTabChanged(int index) {
    ConsoleTab* tab = qobject_cast<ConsoleTab*>(tabWidget->widget(index));
    if (tab) {
        currentTabType = tab->getTabType();
    }
}

void ConsoleWidget::onContextMenu(const QPoint& pos) {
    contextMenu->exec(mapToGlobal(pos));
}

void ConsoleWidget::onClearTab() {
    clearTab(currentTabType);
}

void ConsoleWidget::onSaveTab() {
    QString path = QFileDialog::getSaveFileName(this, "Save Output",
                         "", "Text Files (*.txt);;All Files (*)");
    if (!path.isEmpty()) {
        saveOutput(path);
    }
}

void ConsoleWidget::onCopyTab() {
    copyOutput();
}

void ConsoleWidget::onFilterChanged(const QString& filter) {
    ConsoleTab* tab = tabs[currentTabType];
    if (tab) {
        tab->setFilter(filter);
    }
}

void ConsoleWidget::styleMessage(QPlainTextEdit* editor, MessageType type) {
    // Apply styling based on message type
    QTextCharFormat format;
    
    switch (type) {
        case MessageType::Error:
            format.setForeground(QColor(255, 100, 100));
            break;
        case MessageType::Warning:
            format.setForeground(QColor(255, 200, 100));
            break;
        case MessageType::Success:
            format.setForeground(QColor(100, 255, 100));
            break;
        default:
            format.setForeground(QColor(200, 200, 200));
            break;
    }
    
    // Apply format to current selection
    QTextCursor cursor = editor->textCursor();
    cursor.mergeCharFormat(format);
}

QString ConsoleWidget::formatTimestamp() const {
    return QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
}

} // namespace proxima