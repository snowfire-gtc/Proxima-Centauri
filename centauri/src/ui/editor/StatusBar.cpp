#include "StatusBar.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>
#include <QSysInfo>
#include <QFile>
#include <QDir>

#ifdef Q_OS_LINUX
#include <unistd.h>
#include <fstream>
#include <sstream>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

#ifdef Q_OS_MAC
#include <sys/sysctl.h>
#include <mach/mach.h>
#endif

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

StatusBar::StatusBar(QWidget *parent)
    : QStatusBar(parent)
    , currentLine(1)
    , currentColumn(1)
    , executionLine(0)
    , fileSaved(true)
    , cpuUsagePercent(0.0)
    , memoryUsageMB(0.0)
    , diskUsagePercent(0.0)
    , windowActive(true)
    , llmFreeContext(0)
    , llmTotalContext(0)
    , llmStatus(LLMStatus::Idle)
    , updateTimer(nullptr)
{
    setupUI();
    
    // Настройка таймера для обновления времени и ресурсов
    updateTimer = new QTimer(this);
    updateTimer->setInterval(1000); // Обновление каждую секунду
    connect(updateTimer, &QTimer::timeout, this, [this]() {
        if (windowActive) {
            setCurrentTime(QTime::currentTime());
            refreshSystemResources();
        }
    });
    updateTimer->start();
}

StatusBar::~StatusBar() {
    if (updateTimer) {
        updateTimer->stop();
    }
}

// ============================================================================
// Инициализация UI
// ============================================================================

void StatusBar::setupUI() {
    createCursorPositionLabel();
    createExecutionStatusLabel();
    createSaveStatusLabel();
    createTimeLabel();
    createSystemResourcesLabel();
    createLLMStatusLabel();
    
    // Начальное обновление отображения
    updateDisplay();
}

void StatusBar::createCursorPositionLabel() {
    cursorPositionLabel = new QLabel(this);
    cursorPositionLabel->setMinimumWidth(120);
    cursorPositionLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    addPermanentWidget(cursorPositionLabel, 0);
}

void StatusBar::createExecutionStatusLabel() {
    executionStatusLabel = new QLabel(this);
    executionStatusLabel->setMinimumWidth(100);
    executionStatusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    addPermanentWidget(executionStatusLabel, 0);
}

void StatusBar::createSaveStatusLabel() {
    saveStatusLabel = new QLabel(this);
    saveStatusLabel->setMinimumWidth(80);
    saveStatusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    addPermanentWidget(saveStatusLabel, 0);
}

void StatusBar::createTimeLabel() {
    timeLabel = new QLabel(this);
    timeLabel->setMinimumWidth(100);
    timeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    addPermanentWidget(timeLabel, 0);
}

void StatusBar::createSystemResourcesLabel() {
    systemResourcesLabel = new QLabel(this);
    systemResourcesLabel->setMinimumWidth(150);
    systemResourcesLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    addPermanentWidget(systemResourcesLabel, 0);
}

void StatusBar::createLLMStatusLabel() {
    llmStatusLabel = new QLabel(this);
    llmStatusLabel->setMinimumWidth(180);
    llmStatusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    addPermanentWidget(llmStatusLabel, 1);
}

// ============================================================================
// Позиция курсора
// ============================================================================

void StatusBar::setCursorPosition(int line, int column) {
    currentLine = line;
    currentColumn = column;
    cursorPositionLabel->setText(QString("Line: %1, Col: %2").arg(line).arg(column));
}

// ============================================================================
// Статус выполнения
// ============================================================================

void StatusBar::setExecutionStatus(IDEMode mode) {
    QString statusText;
    switch (mode) {
        case IDEMode::Edit:
            statusText = "Edit";
            break;
        case IDEMode::Runtime:
            statusText = "Running";
            break;
        case IDEMode::Pause:
            statusText = "Paused";
            break;
        default:
            statusText = "Unknown";
            break;
    }
    executionStatusLabel->setText(statusText);
}

void StatusBar::setExecutionPosition(const QString& file, int line) {
    executionFile = file;
    executionLine = line;
    // Можно добавить дополнительную информацию о позиции выполнения
}

QString StatusBar::getExecutionStatusText() const {
    return executionStatusLabel->text();
}

// ============================================================================
// Статус сохранения
// ============================================================================

void StatusBar::setSaveStatus(bool saved) {
    fileSaved = saved;
    saveStatusLabel->setText(saved ? "Saved" : "Modified");
    saveStatusLabel->setStyleSheet(saved ? "color: green;" : "color: orange;");
}

// ============================================================================
// Системное время
// ============================================================================

void StatusBar::setCurrentTime(const QTime& time) {
    currentTime = time;
    if (windowActive) {
        timeLabel->setText(time.toString("hh:mm:ss"));
    } else {
        timeLabel->clear();
    }
}

// ============================================================================
// Ресурсы системы
// ============================================================================

void StatusBar::setSystemResources(double cpuUsage, double memoryUsage, double diskUsage) {
    cpuUsagePercent = cpuUsage;
    memoryUsageMB = memoryUsage;
    diskUsagePercent = diskUsage;
    
    if (windowActive) {
        systemResourcesLabel->setText(formatResourcesString());
    } else {
        systemResourcesLabel->clear();
    }
}

void StatusBar::refreshSystemResources() {
#ifdef Q_OS_LINUX
    // Чтение информации о CPU из /proc/stat
    std::ifstream statFile("/proc/stat");
    if (statFile.isGood()) {
        std::string line;
        std::getline(statFile, line);
        std::istringstream iss(line);
        std::string cpu;
        unsigned long long user, nice, system, idle, iowait, irq, softirq;
        iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq;
        
        unsigned long long total = user + nice + system + idle + iowait + irq + softirq;
        unsigned long long idleTotal = idle + iowait;
        
        static unsigned long long prevTotal = total;
        static unsigned long long prevIdle = idleTotal;
        
        unsigned long long diffTotal = total - prevTotal;
        unsigned long long diffIdle = idleTotal - prevIdle;
        
        if (diffTotal > 0) {
            cpuUsagePercent = 100.0 * (diffTotal - diffIdle) / diffTotal;
        }
        
        prevTotal = total;
        prevIdle = idleTotal;
    }
    
    // Чтение информации о памяти из /proc/meminfo
    std::ifstream meminfoFile("/proc/meminfo");
    if (meminfoFile.isGood()) {
        std::string line;
        unsigned long long memTotal = 0, memAvailable = 0;
        
        while (std::getline(meminfoFile, line)) {
            std::istringstream iss(line);
            std::string key;
            unsigned long long value;
            iss >> key >> value;
            
            if (key == "MemTotal:") {
                memTotal = value;
            } else if (key == "MemAvailable:") {
                memAvailable = value;
            }
        }
        
        if (memTotal > 0) {
            memoryUsageMB = (memTotal - memAvailable) / 1024.0; // Конвертация в МБ
        }
    }
    
    // Чтение информации о диске (упрощённо - корневой раздел)
    struct statfs stat;
    if (statfs("/", &stat) == 0) {
        unsigned long long total = stat.f_blocks * stat.f_bsize;
        unsigned long long free = stat.f_bfree * stat.f_bsize;
        if (total > 0) {
            diskUsagePercent = 100.0 * (total - free) / total;
        }
    }
#elif defined(Q_OS_WIN)
    // Windows implementation
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        memoryUsageMB = (memStatus.ullTotalPhys - memStatus.ullAvailPhys) / (1024.0 * 1024.0);
        diskUsagePercent = memStatus.dwMemoryLoad;
    }
    
    // CPU usage would require performance counters
    cpuUsagePercent = 0.0; // Placeholder
#elif defined(Q_OS_MAC)
    // macOS implementation
    vm_size_t pageSize = sysconf(_SC_PAGESIZE);
    vm_statistics64_data_t vmStats;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    
    if (host_statistics64(mach_host_self(), HOST_VM_INFO64, 
                          (host_info64_t)&vmStats, &count) == KERN_SUCCESS) {
        unsigned long long total = vmStats.page_count * pageSize;
        unsigned long long free = vmStats.free_count * pageSize;
        memoryUsageMB = (total - free) / (1024.0 * 1024.0);
    }
    
    cpuUsagePercent = 0.0; // Placeholder
    diskUsagePercent = 0.0; // Placeholder
#endif
    
    if (windowActive) {
        systemResourcesLabel->setText(formatResourcesString());
    }
}

QString StatusBar::formatResourcesString() const {
    return QString("CPU: %1% | RAM: %2 MB | Disk: %3%")
        .arg(cpuUsagePercent, 0, 'f', 1)
        .arg(memoryUsageMB, 0, 'f', 0)
        .arg(diskUsagePercent, 0, 'f', 1);
}

// ============================================================================
// LLM статус
// ============================================================================

void StatusBar::setLLMStatus(const QString& modelName, 
                            int freeContext, 
                            int totalContext,
                            LLMStatus status) {
    llmModelName = modelName;
    llmFreeContext = freeContext;
    llmTotalContext = totalContext;
    llmStatus = status;
    
    llmStatusLabel->setText(formatLLMStatusString());
    llmStatusLabel->setStyleSheet(QString("color: %1;").arg(getLLMStatusColor(status)));
}

QString StatusBar::getLLMStatusText() const {
    switch (llmStatus) {
        case LLMStatus::Idle:
            return "idle";
        case LLMStatus::Generating:
            return "gen";
        case LLMStatus::Fixing:
            return "fix";
        case LLMStatus::Documenting:
            return "doc";
        case LLMStatus::Error:
            return "error";
        default:
            return "unknown";
    }
}

QString StatusBar::formatLLMStatusString() const {
    if (llmModelName.isEmpty()) {
        return "LLM: Not connected";
    }
    
    QString statusStr = getLLMStatusText();
    int usedContext = llmTotalContext - llmFreeContext;
    
    return QString("LLM: %1 [%2] Context: %3/%4")
        .arg(llmModelName)
        .arg(statusStr)
        .arg(usedContext)
        .arg(llmTotalContext);
}

QString StatusBar::getLLMStatusColor(LLMStatus status) {
    switch (status) {
        case LLMStatus::Idle:
            return "#808080"; // Gray
        case LLMStatus::Generating:
            return "#00AA00"; // Green
        case LLMStatus::Fixing:
            return "#FFA500"; // Orange
        case LLMStatus::Documenting:
            return "#0000FF"; // Blue
        case LLMStatus::Error:
            return "#FF0000"; // Red
        default:
            return "#000000"; // Black
    }
}

// ============================================================================
// Обновление отображения
// ============================================================================

void StatusBar::updateDisplay() {
    setCursorPosition(currentLine, currentColumn);
    setSaveStatus(fileSaved);
    
    if (windowActive) {
        setCurrentTime(QTime::currentTime());
        systemResourcesLabel->setText(formatResourcesString());
        llmStatusLabel->setText(formatLLMStatusString());
    } else {
        timeLabel->clear();
        systemResourcesLabel->clear();
    }
}

void StatusBar::setActiveWindow(bool active) {
    windowActive = active;
    if (!active) {
        timeLabel->clear();
        systemResourcesLabel->clear();
    } else {
        setCurrentTime(QTime::currentTime());
        systemResourcesLabel->setText(formatResourcesString());
    }
}

} // namespace proxima
