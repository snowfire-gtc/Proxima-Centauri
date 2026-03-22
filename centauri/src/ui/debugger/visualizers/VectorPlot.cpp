#include "VectorPlot.h"
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QBuffer>
#include <QByteArray>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QReadWriteLock>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QDataStream>
#include <QDebug>
#include <QElapsedTimer>
#include <QSet>
#include <QHash>
#include <QMultiHash>
#include <QMultiMap>
#include <QMetaType>
#include <QMetaObject>
#include <QPainterPath>
#include <QRegion>
#include <QBitmap>
#include <QMatrix>
#include <QPolygon>
#include <QPolygonF>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QQuaternion>
#include <QEasingCurve>

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

VectorPlot::VectorPlot(QWidget *parent)
    : QWidget(parent)
    , pointCount(0)
    , plotMode(PlotMode::Line)
    , showPoints(false)
    , showExtremums(true)
    , showGrid(true)
    , showXAxis(true)
    , showYAxis(true)
    , showLegend(false)
    , lineColor(QColor(78, 201, 176))  // #4EC9B0
    , lineWidth(2)
    , pointColor(QColor(255, 255, 255))
    , pointSize(5)
    , gridColor(QColor(60, 60, 60))
    , backgroundColor(QColor(30, 30, 30))
    , selectionColor(QColor(0, 122, 204, 100))
    , zoomLevel(1.0)
    , minZoom(0.1)
    , maxZoom(10.0)
    , hoverIndex(-1)
    , isHovering(false)
    , peakThreshold(0.1)
    , valleyThreshold(0.1)
    , autoScale(true)
    , yMin(0.0)
    , yMax(1.0)
    , minValue(0.0)
    , maxValue(0.0)
    , meanValue(0.0)
    , sumValue(0.0)
    , stdDev(0.0)
    , interactive(true)
    , editable(false)
    , isPanning(false)
    , isSelecting(false)
    , selectionEnabled(true) {
    
    setupUI();
    setupContextMenu();
    setupToolbar();
    
    LOG_DEBUG("VectorPlot created");
}

VectorPlot::~VectorPlot() {
    LOG_DEBUG("VectorPlot destroyed");
}

// ============================================================================
// Настройка UI
// ============================================================================

void VectorPlot::setupUI() {
    setMinimumSize(400, 300);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    
    // Включение double buffering
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    
    // Стиль
    setStyleSheet(
        "QWidget { "
        "  background-color: #1e1e1e; "
        "}"
    );
}

void VectorPlot::setupContextMenu() {
    contextMenu = new QMenu(this);
    
    // Действия копирования/вставки
    copyAction = contextMenu->addAction("Копировать данные", this, &VectorPlot::copySelection);
    copyAction->setShortcut(QKeySequence::Copy);
    
    pasteAction = contextMenu->addAction("Вставить данные", this, &VectorPlot::pasteData);
    pasteAction->setShortcut(QKeySequence::Paste);
    
    selectAllAction = contextMenu->addAction("Выделить всё", this, [this]() {
        if (pointCount > 0) {
            selectRange(1, pointCount);  // 1-based индексация
        }
    });
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    
    contextMenu->addSeparator();
    
    // Действия сохранения
    saveImageAction = contextMenu->addAction("Сохранить как изображение...", this, [this]() {
        QString path = QFileDialog::getSaveFileName(this, "Сохранить как изображение", "", 
                                                    "PNG Files (*.png);;JPG Files (*.jpg);;SVG Files (*.svg);;All Files (*)");
        if (!path.isEmpty()) {
            QFileInfo fi(path);
            saveAsImage(path, fi.suffix().toUpper());
        }
    });
    
    saveCSVAction = contextMenu->addAction("Сохранить данные как CSV...", this, [this]() {
        QString path = QFileDialog::getSaveFileName(this, "Сохранить как CSV", "", 
                                                    "CSV Files (*.csv);;All Files (*)");
        if (!path.isEmpty()) {
            saveAsCSV(path);
        }
    });
    
    loadCSVAction = contextMenu->addAction("Загрузить данные из CSV...", this, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "Загрузить из CSV", "", 
                                                    "CSV Files (*.csv);;All Files (*)");
        if (!path.isEmpty()) {
            loadFromCSV(path);
        }
    });
    
    saveJSONAction = contextMenu->addAction("Сохранить данные как JSON...", this, [this]() {
        QString path = QFileDialog::getSaveFileName(this, "Сохранить как JSON", "", 
                                                    "JSON Files (*.json);;All Files (*)");
        if (!path.isEmpty()) {
            saveAsJSON(path);
        }
    });
    
    loadJSONAction = contextMenu->addAction("Загрузить данные из JSON...", this, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "Загрузить из JSON", "", 
                                                    "JSON Files (*.json);;All Files (*)");
        if (!path.isEmpty()) {
            loadFromJSON(path);
        }
    });
    
    contextMenu->addSeparator();
    
    // Копирование как изображение
    copyImageAction = contextMenu->addAction("Копировать как изображение", this, &VectorPlot::copyAsImage);
    
    // Действия печати
    printAction = contextMenu->addAction("Печать...", this, &VectorPlot::print);
    printAction->setShortcut(QKeySequence::Print);
    
    contextMenu->addSeparator();
    
    // Действия масштабирования
    zoomInAction = contextMenu->addAction("Увеличить", this, &VectorPlot::zoomIn);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    
    zoomOutAction = contextMenu->addAction("Уменьшить", this, &VectorPlot::zoomOut);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    
    resetZoomAction = contextMenu->addAction("Сбросить масштаб", this, &VectorPlot::resetZoom);
    resetZoomAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    
    fitToWidgetAction = contextMenu->addAction("Подогнать под размер", this, &VectorPlot::fitToWidget);
    
    contextMenu->addSeparator();
    
    // Режимы отображения
    lineModeAction = contextMenu->addAction("Линейный", this, [this]() {
        setPlotMode(PlotMode::Line);
    });
    lineModeAction->setCheckable(true);
    
    scatterModeAction = contextMenu->addAction("Точечный", this, [this]() {
        setPlotMode(PlotMode::Scatter);
    });
    scatterModeAction->setCheckable(true);
    
    splineModeAction = contextMenu->addAction("Сплайн", this, [this]() {
        setPlotMode(PlotMode::Spline);
    });
    splineModeAction->setCheckable(true);
    
    areaModeAction = contextMenu->addAction("Областной", this, [this]() {
        setPlotMode(PlotMode::Area);
    });
    areaModeAction->setCheckable(true);
    
    barModeAction = contextMenu->addAction("Столбчатый", this, [this]() {
        setPlotMode(PlotMode::Bar);
    });
    barModeAction->setCheckable(true);
    
    stepModeAction = contextMenu->addAction("Ступенчатый", this, [this]() {
        setPlotMode(PlotMode::Step);
    });
    stepModeAction->setCheckable(true);
    
    contextMenu->addSeparator();
    
    // Отображение точек
    showPointsAction = contextMenu->addAction("Показывать точки", this, [this](bool checked) {
        setShowPoints(checked);
    });
    showPointsAction->setCheckable(true);
    showPointsAction->setChecked(showPoints);
    
    // Отображение сетки
    showGridAction = contextMenu->addAction("Показывать сетку", this, [this](bool checked) {
        showGrid = checked;
        update();
    });
    showGridAction->setCheckable(true);
    showGridAction->setChecked(showGrid);
    
    // Отображение пиков/впадин
    showExtremumsAction = contextMenu->addAction("Показывать пики/впадины", this, [this](bool checked) {
        setShowExtremums(checked);
    });
    showExtremumsAction->setCheckable(true);
    showExtremumsAction->setChecked(showExtremums);
    
    contextMenu->addSeparator();
    
    // Поиск пиков и впадин
    findPeaksAction = contextMenu->addAction("Найти пики", this, &VectorPlot::findPeaks);
    findValleysAction = contextMenu->addAction("Найти впадины", this, &VectorPlot::findValleys);
    clearExtremumsAction = contextMenu->addAction("Очистить пики/впадины", this, &VectorPlot::clearExtremums);
    
    contextMenu->addSeparator();
    
    // Цвет линии
    colorMenu = contextMenu->addMenu("Цвет линии");
    colorGroup = new QActionGroup(this);
    
    QStringList colorNames = {"Бирюзовый", "Синий", "Зелёный", "Красный", 
                              "Оранжевый", "Фиолетовый", "Белый", "Жёлтый"};
    QList<QColor> colors = {
        QColor(78, 201, 176),  // Бирюзовый
        QColor(56, 139, 253),  // Синий
        QColor(106, 153, 85),  // Зелёный
        QColor(244, 71, 71),   // Красный
        QColor(255, 152, 0),   // Оранжевый
        QColor(156, 39, 176),  // Фиолетовый
        QColor(255, 255, 255), // Белый
        QColor(255, 230, 0)    // Жёлтый
    };
    
    for (int i = 0; i < colorNames.size(); i++) {
        QAction* action = colorMenu->addAction(colorNames[i]);
        action->setCheckable(true);
        action->setActionGroup(colorGroup);
        action->setData(colors[i]);
        
        connect(action, &QAction::triggered, this, [this, colors, i]() {
            setLineColor(colors[i]);
        });
        
        if (colors[i] == lineColor) {
            action->setChecked(true);
        }
    }
    
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        contextMenu->exec(mapToGlobal(pos));
    });
}

void VectorPlot::setupToolbar() {
    toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));
    toolbar->setVisible(false);  // Скрыта по умолчанию
}

// ============================================================================
// Данные графика
// ============================================================================

void VectorPlot::setData(const QVector<double>& newData) {
    data = newData;
    pointCount = data.size();
    
    // Инициализация точек данных (1-based индексация)
    points.clear();
    for (int i = 0; i < pointCount; i++) {
        DataPoint point;
        point.index = i + 1;  // 1-based индекс
        point.value = data[i];
        points.append(point);
    }
    
    calculateStatistics();
    calculatePointPositions();
    update();
    
    LOG_INFO("Vector data set: " + QString::number(pointCount) + " points");
}

void VectorPlot::setDataFromRuntime(const RuntimeValue& value) {
    if (value.type != RuntimeValue::Type::Array) {
        LOG_WARNING("Invalid runtime value type for vector");
        return;
    }
    
    QVector<double> data;
    for (const auto& item : value.arrayValue) {
        data.append(item.numberValue);
    }
    
    setData(data);
}

double VectorPlot::getValue(int index) const {
    // Проверка границ (1-based индексация)
    if (index < 1 || index > pointCount) {
        return 0.0;
    }
    
    // Конвертация в 0-based для внутреннего хранения
    return data[index - 1];
}

void VectorPlot::setValue(int index, double value) {
    if (!editable) return;
    
    // Проверка границ (1-based индексация)
    if (index < 1 || index > pointCount) {
        return;
    }
    
    // Конвертация в 0-based для внутреннего хранения
    data[index - 1] = value;
    points[index - 1].value = value;
    
    calculateStatistics();
    calculatePointPositions();
    update();
    
    emit dataModified();
}

// ============================================================================
// Режимы отображения
// ============================================================================

void VectorPlot::setPlotMode(PlotMode mode) {
    if (plotMode != mode) {
        plotMode = mode;
        update();
        emit modeChanged(mode);
        
        // Обновление чекбоксов в меню
        lineModeAction->setChecked(mode == PlotMode::Line);
        scatterModeAction->setChecked(mode == PlotMode::Scatter);
        splineModeAction->setChecked(mode == PlotMode::Spline);
        areaModeAction->setChecked(mode == PlotMode::Area);
        barModeAction->setChecked(mode == PlotMode::Bar);
        stepModeAction->setChecked(mode == PlotMode::Step);
    }
}

void VectorPlot::setLineColor(const QColor& color) {
    lineColor = color;
    update();
}

void VectorPlot::setLineWidth(int width) {
    lineWidth = qMax(1, width);
    update();
}

void VectorPlot::setShowPoints(bool show) {
    showPoints = show;
    showPointsAction->setChecked(show);
    update();
}

void VectorPlot::setPointSize(int size) {
    pointSize = qMax(1, size);
    update();
}

// ============================================================================
// Масштабирование
// ============================================================================

void VectorPlot::setZoomLevel(double level) {
    zoomLevel = qBound(minZoom, level, maxZoom);
    updateTransform();
    calculatePointPositions();
    update();
    emit zoomChanged(zoomLevel);
}

void VectorPlot::zoomIn() {
    setZoomLevel(zoomLevel * 1.2);
}

void VectorPlot::zoomOut() {
    setZoomLevel(zoomLevel / 1.2);
}

void VectorPlot::resetZoom() {
    setZoomLevel(1.0);
    panOffset = QPointF(0, 0);
}

void VectorPlot::fitToWidget() {
    if (pointCount == 0) return;
    
    double availableWidth = width() - 80;  // Место для осей
    double availableHeight = height() - 60;
    
    double zoomX = availableWidth / pointCount;
    double zoomY = availableHeight / (maxValue - minValue);
    
    setZoomLevel(qMin(zoomX, zoomY) / 10.0);
    panOffset = QPointF(40, 30);  // Отступ для осей
}

void VectorPlot::scrollToIndex(int index) {
    ensurePointVisible(index);
}

void VectorPlot::centerOnIndex(int index) {
    QPoint pointPos = indexToScreen(index);
    panOffset = QPointF(width() / 2 - pointPos.x(), height() / 2 - pointPos.y());
    update();
}

// ============================================================================
// Анализ данных
// ============================================================================

void VectorPlot::findPeaks() {
    detectPeaks();
    update();
    LOG_INFO("Peaks detected: " + QString::number(peaks.size()));
}

void VectorPlot::findValleys() {
    detectValleys();
    update();
    LOG_INFO("Valleys detected: " + QString::number(valleys.size()));
}

void VectorPlot::clearExtremums() {
    peaks.clear();
    valleys.clear();
    for (auto& point : points) {
        point.isPeak = false;
        point.isValley = false;
    }
    update();
}

void VectorPlot::setShowExtremums(bool show) {
    showExtremums = show;
    showExtremumsAction->setChecked(show);
    update();
}

// ============================================================================
// Выделение
// ============================================================================

void VectorPlot::selectRange(int startIndex, int endIndex) {
    // Проверка границ (1-based индексация)
    startIndex = qBound(1, startIndex, pointCount);
    endIndex = qBound(1, endIndex, pointCount);
    
    // Нормализация диапазона
    if (startIndex > endIndex) {
        qSwap(startIndex, endIndex);
    }
    
    selection.startIndex = startIndex;
    selection.endIndex = endIndex;
    
    // Вычисление статистики выделения
    selection.minValue = data[startIndex - 1];
    selection.maxValue = data[startIndex - 1];
    selection.sumValue = 0.0;
    
    for (int i = startIndex - 1; i < endIndex; i++) {
        double value = data[i];
        if (value < selection.minValue) selection.minValue = value;
        if (value > selection.maxValue) selection.maxValue = value;
        selection.sumValue += value;
    }
    
    selection.meanValue = selection.sumValue / selection.count();
    
    // Обновление состояния точек
    for (auto& point : points) {
        point.isSelected = (point.index >= startIndex && point.index <= endIndex);
    }
    
    update();
    emit selectionChanged(selection);
}

PlotSelection VectorPlot::getSelection() const {
    return selection;
}

void VectorPlot::clearSelection() {
    selection = PlotSelection();
    for (auto& point : points) {
        point.isSelected = false;
    }
    update();
    emit selectionChanged(selection);
}

bool VectorPlot::hasSelection() const {
    return selection.isValid();
}

QVector<double> VectorPlot::getSelectedData() const {
    if (!hasSelection()) {
        return QVector<double>();
    }
    
    QVector<double> selected;
    for (int i = selection.startIndex - 1; i < selection.endIndex; i++) {
        selected.append(data[i]);
    }
    return selected;
}

void VectorPlot::copySelection() {
    if (!hasSelection()) return;
    
    QVector<double> selectedData = getSelectedData();
    
    QString csvData;
    for (int i = 0; i < selectedData.size(); i++) {
        csvData += QString::number(selectedData[i]);
        if (i < selectedData.size() - 1) {
            csvData += ",";
        }
    }
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(csvData);
    
    LOG_INFO("Selection copied to clipboard");
}

void VectorPlot::pasteData() {
    if (!editable) return;
    
    QClipboard* clipboard = QApplication::clipboard();
    QString text = clipboard->text();
    
    // Парсинг CSV данных
    QStringList values = text.split(",", Qt::SkipEmptyParts);
    if (values.isEmpty()) return;
    
    // В полной реализации - вставка данных
    LOG_INFO("Paste operation requested: " + QString::number(values.size()) + " values");
}

// ============================================================================
// Оси и сетка
// ============================================================================

void VectorPlot::setShowXAxis(bool show) {
    showXAxis = show;
    update();
}

void VectorPlot::setShowYAxis(bool show) {
    showYAxis = show;
    update();
}

void VectorPlot::setShowGrid(bool show) {
    showGrid = show;
    showGridAction->setChecked(show);
    update();
}

void VectorPlot::setShowLegend(bool show) {
    showLegend = show;
    update();
}

void VectorPlot::setTitle(const QString& newTitle) {
    title = newTitle;
    update();
}

void VectorPlot::setXAxisLabel(const QString& label) {
    xAxisLabel = label;
    update();
}

void VectorPlot::setYAxisLabel(const QString& label) {
    yAxisLabel = label;
    update();
}

// ============================================================================
// Статистика
// ============================================================================

void VectorPlot::calculateStatistics() {
    if (pointCount == 0) {
        minValue = 0.0;
        maxValue = 0.0;
        meanValue = 0.0;
        sumValue = 0.0;
        stdDev = 0.0;
        return;
    }
    
    minValue = data[0];
    maxValue = data[0];
    sumValue = 0.0;
    
    for (double value : data) {
        if (value < minValue) minValue = value;
        if (value > maxValue) maxValue = value;
        sumValue += value;
    }
    
    meanValue = sumValue / pointCount;
    
    // Стандартное отклонение
    double variance = 0.0;
    for (double value : data) {
        variance += (value - meanValue) * (value - meanValue);
    }
    stdDev = std::sqrt(variance / pointCount);
    
    // Авто-масштабирование оси Y
    if (autoScale) {
        yMin = minValue - (maxValue - minValue) * 0.1;
        yMax = maxValue + (maxValue - minValue) * 0.1;
    }
}

void VectorPlot::detectPeaks() {
    peaks.clear();
    
    for (int i = 1; i < pointCount - 1; i++) {
        // 1-based индексация для пользователя
        if (data[i] > data[i - 1] && data[i] > data[i + 1]) {
            // Проверка порога
            double leftDiff = data[i] - data[i - 1];
            double rightDiff = data[i] - data[i + 1];
            
            if (leftDiff > peakThreshold && rightDiff > peakThreshold) {
                Extremum peak;
                peak.index = i + 1;  // 1-based
                peak.value = data[i];
                peak.isPeak = true;
                peak.label = QString("Peak %1").arg(peaks.size() + 1);
                peaks.append(peak);
                
                points[i].isPeak = true;
            }
        }
    }
}

void VectorPlot::detectValleys() {
    valleys.clear();
    
    for (int i = 1; i < pointCount - 1; i++) {
        // 1-based индексация для пользователя
        if (data[i] < data[i - 1] && data[i] < data[i + 1]) {
            // Проверка порога
            double leftDiff = data[i - 1] - data[i];
            double rightDiff = data[i + 1] - data[i];
            
            if (leftDiff > valleyThreshold && rightDiff > valleyThreshold) {
                Extremum valley;
                valley.index = i + 1;  // 1-based
                valley.value = data[i];
                valley.isPeak = false;
                valley.label = QString("Valley %1").arg(valleys.size() + 1);
                valleys.append(valley);
                
                points[i].isValley = true;
            }
        }
    }
}

// ============================================================================
// Экспорт/Импорт
// ============================================================================

bool VectorPlot::saveAsImage(const QString& path, const QString& format) {
    QImage image(size(), QImage::Format_ARGB32);
    image.fill(backgroundColor);
    
    QPainter painter(&image);
    paintEvent(nullptr);
    painter.end();
    
    return image.save(path, format.toStdString().c_str());
}

bool VectorPlot::saveAsCSV(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "Index,Value\n";
    
    for (int i = 0; i < pointCount; i++) {
        out << (i + 1) << "," << data[i] << "\n";  // 1-based индексация
    }
    
    file.close();
    return true;
}

bool VectorPlot::loadFromCSV(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    
    QVector<double> loadedData;
    
    // Пропуск заголовка
    QString headerLine = in.readLine();
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        
        QStringList parts = line.split(",");
        if (parts.size() >= 2) {
            bool ok;
            double value = parts[1].toDouble(&ok);
            if (ok) {
                loadedData.append(value);
            }
        }
    }
    
    file.close();
    
    if (!loadedData.isEmpty()) {
        setData(loadedData);
        return true;
    }
    
    return false;
}

bool VectorPlot::saveAsJSON(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonArray array;
    for (int i = 0; i < pointCount; i++) {
        QJsonObject point;
        point["index"] = i + 1;  // 1-based
        point["value"] = data[i];
        array.append(point);
    }
    
    QJsonObject root;
    root["data"] = array;
    root["count"] = pointCount;
    root["title"] = title;
    
    QJsonDocument doc(root);
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool VectorPlot::loadFromJSON(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }
    
    QJsonObject root = doc.object();
    QJsonArray array = root["data"].toArray();
    
    QVector<double> loadedData;
    for (const QJsonValue& value : array) {
        loadedData.append(value.toObject()["value"].toDouble());
    }
    
    if (!loadedData.isEmpty()) {
        setData(loadedData);
        if (root.contains("title")) {
            setTitle(root["title"].toString());
        }
        return true;
    }
    
    return false;
}

void VectorPlot::print() {
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        paintEvent(nullptr);
        painter.end();
    }
}

void VectorPlot::copyAsImage() {
    QImage image(size(), QImage::Format_ARGB32);
    image.fill(backgroundColor);
    
    QPainter painter(&image);
    paintEvent(nullptr);
    painter.end();
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setImage(image);
    
    LOG_INFO("Plot copied to clipboard as image");
}

// ============================================================================
// Интерактивность
// ============================================================================

void VectorPlot::setInteractive(bool enable) {
    interactive = enable;
}

void VectorPlot::setEditable(bool enable) {
    editable = enable;
}

void VectorPlot::setSelectionEnabled(bool enable) {
    selectionEnabled = enable;
}

// ============================================================================
// События отрисовки
// ============================================================================

void VectorPlot::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Очистка фона
    painter.fillRect(rect(), backgroundColor);
    
    // Применение трансформации
    painter.save();
    painter.translate(panOffset);
    painter.scale(zoomLevel, 1.0);  // Zoom только по X
    
    // Отрисовка графика
    drawPlot(painter);
    
    painter.restore();
    
    // Отрисовка осей (без трансформации)
    if (showXAxis) {
        drawXAxis(painter);
    }
    if (showYAxis) {
        drawYAxis(painter);
    }
    
    // Отрисовка сетки
    if (showGrid) {
        drawGrid(painter);
    }
    
    // Отрисовка легенды
    if (showLegend) {
        drawLegend(painter);
    }
    
    // Отрисовка выделения
    if (hasSelection()) {
        drawSelection(painter);
    }
    
    // Отрисовка наведения
    if (isHovering && hoverIndex > 0) {
        drawHoverPoint(painter);
    }
    
    // Отрисовка пиков/впадин
    if (showExtremums) {
        drawExtremums(painter);
    }
    
    // Заголовок
    if (!title.isEmpty()) {
        painter.setPen(QColor(200, 200, 200));
        painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
        painter.drawText(rect(), Qt::AlignHCenter | Qt::AlignTop, title);
    }
}

void VectorPlot::drawPlot(QPainter& painter) {
    switch (plotMode) {
        case PlotMode::Line:
            drawLinePlot(painter);
            break;
        case PlotMode::Scatter:
            drawScatterPlot(painter);
            break;
        case PlotMode::Spline:
            drawSplinePlot(painter);
            break;
        case PlotMode::Area:
            drawAreaPlot(painter);
            break;
        case PlotMode::Bar:
            drawBarPlot(painter);
            break;
        case PlotMode::Step:
            drawStepPlot(painter);
            break;
    }
}

void VectorPlot::drawLinePlot(QPainter& painter) {
    if (pointCount < 2) return;
    
    painter.setPen(QPen(lineColor, lineWidth));
    painter.setBrush(Qt::NoBrush);
    
    QPainterPath path;
    path.moveTo(indexToScreen(1));  // 1-based
    
    for (int i = 2; i <= pointCount; i++) {  // 1-based индексация
        path.lineTo(indexToScreen(i));
    }
    
    painter.drawPath(path);
    
    // Отрисовка точек
    if (showPoints) {
        for (const auto& point : points) {
            drawPoint(painter, point);
        }
    }
}

void VectorPlot::drawScatterPlot(QPainter& painter) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(pointColor));
    
    for (const auto& point : points) {
        QPoint pos = indexToScreen(point.index);
        painter.drawEllipse(pos, pointSize, pointSize);
    }
}

void VectorPlot::drawSplinePlot(QPainter& painter) {
    // В полной реализации - отрисовка сплайна
    drawLinePlot(painter);
}

void VectorPlot::drawAreaPlot(QPainter& painter) {
    if (pointCount < 2) return;
    
    // Область под графиком
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, QColor(lineColor.red(), lineColor.green(), lineColor.blue(), 100));
    gradient.setColorAt(1, QColor(lineColor.red(), lineColor.green(), lineColor.blue(), 0));
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(gradient);
    
    QPainterPath path;
    path.moveTo(indexToScreen(1));
    
    for (int i = 2; i <= pointCount; i++) {
        path.lineTo(indexToScreen(i));
    }
    
    path.lineTo(indexToScreen(pointCount), height());
    path.lineTo(indexToScreen(1), height());
    path.closeSubpath();
    
    painter.drawPath(path);
    
    // Линия графика
    drawLinePlot(painter);
}

void VectorPlot::drawBarPlot(QPainter& painter) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(lineColor));
    
    int barWidth = qMax(1, static_cast<int>(cellWidth * zoomLevel * 0.8));
    
    for (int i = 1; i <= pointCount; i++) {  // 1-based
        QPoint pos = indexToScreen(i);
        int barHeight = static_cast<int>((points[i-1].value - yMin) / (yMax - yMin) * height());
        
        painter.drawRect(pos.x() - barWidth/2, height() - barHeight, barWidth, barHeight);
    }
}

void VectorPlot::drawStepPlot(QPainter& painter) {
    if (pointCount < 2) return;
    
    painter.setPen(QPen(lineColor, lineWidth));
    painter.setBrush(Qt::NoBrush);
    
    QPainterPath path;
    path.moveTo(indexToScreen(1));
    
    for (int i = 1; i < pointCount; i++) {
        QPoint current = indexToScreen(i);
        QPoint next = indexToScreen(i + 1);
        
        path.lineTo(next.x(), current.y());
        path.lineTo(next);
    }
    
    painter.drawPath(path);
}

void VectorPlot::drawXAxis(QPainter& painter) {
    painter.setPen(QPen(QColor(150, 150, 150), 1));
    
    int yAxisY = height() - 30;
    painter.drawLine(40, yAxisY, width(), yAxisY);
    
    // Подписи
    painter.setPen(QColor(150, 150, 150));
    painter.setFont(QFont("Segoe UI", 9));
    
    int labelCount = qMin(10, pointCount);
    for (int i = 0; i <= labelCount; i++) {
        int index = i * pointCount / labelCount + 1;  // 1-based
        int x = indexToScreen(index).x();
        QString label = QString::number(index);
        
        painter.drawText(x - 20, yAxisY + 20, 40, 20, Qt::AlignCenter, label);
    }
    
    // Подпись оси
    if (!xAxisLabel.isEmpty()) {
        painter.drawText(width() / 2, yAxisY + 40, xAxisLabel);
    }
}

void VectorPlot::drawYAxis(QPainter& painter) {
    painter.setPen(QPen(QColor(150, 150, 150), 1));
    
    int xAxisX = 40;
    painter.drawLine(xAxisX, 30, xAxisX, height() - 30);
    
    // Подписи
    painter.setPen(QColor(150, 150, 150));
    painter.setFont(QFont("Segoe UI", 9));
    
    int labelCount = 5;
    for (int i = 0; i <= labelCount; i++) {
        double value = yMin + (yMax - yMin) * i / labelCount;
        int y = 30 + (height() - 60) * (labelCount - i) / labelCount;
        QString label = formatValue(value);
        
        painter.drawText(0, y - 10, xAxisX - 10, 20, Qt::AlignRight | Qt::AlignVCenter, label);
    }
    
    // Подпись оси
    if (!yAxisLabel.isEmpty()) {
        painter.save();
        painter.translate(15, height() / 2);
        painter.rotate(-90);
        painter.drawText(0, 0, yAxisLabel);
        painter.restore();
    }
}

void VectorPlot::drawGrid(QPainter& painter) {
    painter.setPen(QPen(gridColor, 1, Qt::DashLine));
    
    // Горизонтальные линии
    int labelCount = 5;
    for (int i = 0; i <= labelCount; i++) {
        int y = 30 + (height() - 60) * i / labelCount;
        painter.drawLine(40, y, width(), y);
    }
    
    // Вертикальные линии
    int vLabelCount = qMin(10, pointCount);
    for (int i = 0; i <= vLabelCount; i++) {
        int index = i * pointCount / vLabelCount + 1;
        int x = indexToScreen(index).x();
        painter.drawLine(x, 30, x, height() - 30);
    }
}

void VectorPlot::drawLegend(QPainter& painter) {
    // В полной реализации - отрисовка легенды
}

void VectorPlot::drawSelection(QPainter& painter) {
    if (!hasSelection()) return;
    
    int startX = indexToScreen(selection.startIndex).x();
    int endX = indexToScreen(selection.endIndex).x();
    
    painter.setPen(QPen(selectionColor, 2));
    painter.setBrush(QBrush(selectionColor));
    painter.drawRect(startX, 30, endX - startX, height() - 60);
}

void VectorPlot::drawHoverPoint(QPainter& painter) {
    if (hoverIndex <= 0 || hoverIndex > pointCount) return;
    
    QPoint pos = indexToScreen(hoverIndex);
    
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(pos, pointSize + 3, pointSize + 3);
    
    // Tooltip со значением
    QString tooltip = QString("Index: %1\nValue: %2")
        .arg(hoverIndex)  // 1-based
        .arg(formatValue(getValue(hoverIndex)));
    
    QToolTip::showText(mapToGlobal(pos), tooltip, this);
}

void VectorPlot::drawExtremums(QPainter& painter) {
    // Отрисовка пиков
    painter.setPen(QPen(QColor(255, 100, 100), 2));
    painter.setBrush(QBrush(QColor(255, 100, 100)));
    
    for (const auto& peak : peaks) {
        QPoint pos = indexToScreen(peak.index);
        painter.drawEllipse(pos, 6, 6);
        
        // Метка
        painter.setPen(QColor(255, 100, 100));
        painter.drawText(pos.x() + 10, pos.y() - 10, peak.label);
    }
    
    // Отрисовка впадин
    painter.setPen(QPen(QColor(100, 255, 100), 2));
    painter.setBrush(QBrush(QColor(100, 255, 100)));
    
    for (const auto& valley : valleys) {
        QPoint pos = indexToScreen(valley.index);
        painter.drawEllipse(pos, 6, 6);
        
        // Метка
        painter.setPen(QColor(100, 255, 100));
        painter.drawText(pos.x() + 10, pos.y() + 20, valley.label);
    }
}

void VectorPlot::drawPoint(QPainter& painter, const DataPoint& point) {
    QPoint pos = indexToScreen(point.index);
    
    if (point.isPeak) {
        painter.setPen(QPen(QColor(255, 100, 100), 2));
        painter.setBrush(QBrush(QColor(255, 100, 100)));
    } else if (point.isValley) {
        painter.setPen(QPen(QColor(100, 255, 100), 2));
        painter.setBrush(QBrush(QColor(100, 255, 100)));
    } else if (point.isSelected) {
        painter.setPen(QPen(QColor(0, 122, 204), 2));
        painter.setBrush(QBrush(QColor(0, 122, 204)));
    } else if (point.isHovered) {
        painter.setPen(QPen(Qt::white, 2));
        painter.setBrush(QBrush(Qt::white));
    } else {
        painter.setPen(QPen(pointColor, 1));
        painter.setBrush(QBrush(pointColor));
    }
    
    painter.drawEllipse(pos, pointSize, pointSize);
}

// ============================================================================
// События мыши
// ============================================================================

void VectorPlot::mousePressEvent(QMouseEvent *event) {
    if (!interactive) {
        QWidget::mousePressEvent(event);
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        int index = screenToIndex(event->pos());
        
        if (index > 0 && index <= pointCount) {
            if (event->modifiers() & Qt::ShiftModifier) {
                // Выделение диапазона
                if (hasSelection()) {
                    selectRange(selection.startIndex, index);
                } else {
                    selectRange(index, index);
                }
            } else if (selectionEnabled) {
                // Начало выделения
                isSelecting = true;
                selectionStart = event->pos();
                selectRange(index, index);
            }
            
            emit pointClicked(index, getValue(index));
        } else {
            // Начало перетаскивания
            isPanning = true;
            lastPanPos = event->pos();
        }
    }
    
    QWidget::mousePressEvent(event);
}

void VectorPlot::mouseMoveEvent(QMouseEvent *event) {
    if (!interactive) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    
    if (isPanning) {
        QPoint delta = event->pos() - lastPanPos;
        panOffset += QPointF(delta);
        lastPanPos = event->pos();
        update();
    } else if (isSelecting) {
        int endIndex = screenToIndex(event->pos());
        if (endIndex > 0 && endIndex <= pointCount) {
            selectRange(selection.startIndex, endIndex);
        }
    } else {
        // Обновление наведения
        int index = screenToIndex(event->pos());
        
        if (index > 0 && index <= pointCount) {
            hoverIndex = index;
            isHovering = true;
            
            for (auto& point : points) {
                point.isHovered = (point.index == index);
            }
            
            emit pointHovered(index, getValue(index));
        } else {
            isHovering = false;
            hoverIndex = -1;
            
            for (auto& point : points) {
                point.isHovered = false;
            }
        }
        
        update();
    }
    
    QWidget::mouseMoveEvent(event);
}

void VectorPlot::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isPanning = false;
        isSelecting = false;
    }
    
    QWidget::mouseReleaseEvent(event);
}

void VectorPlot::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        int index = screenToIndex(event->pos());
        
        if (index > 0 && index <= pointCount) {
            emit pointDoubleClicked(index, getValue(index));
            
            // Если редактируемый - открытие диалога редактирования
            if (editable) {
                bool ok;
                double newValue = QInputDialog::getDouble(this, "Edit Point",
                    QString("Value at index %1:").arg(index),
                    getValue(index), -1e10, 1e10, 6, &ok);
                
                if (ok) {
                    setValue(index, newValue);
                }
            }
        }
    }
    
    QWidget::mouseDoubleClickEvent(event);
}

void VectorPlot::wheelEvent(QWheelEvent *event) {
    if (!interactive) {
        QWidget::wheelEvent(event);
        return;
    }
    
    if (event->modifiers() & Qt::ControlModifier) {
        // Zoom
        if (event->angleDelta().y() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
        event->accept();
    } else {
        // Pan
        panOffset += QPointF(event->angleDelta().x(), event->angleDelta().y());
        update();
        event->accept();
    }
}

void VectorPlot::keyPressEvent(QKeyEvent *event) {
    if (!interactive) {
        QWidget::keyPressEvent(event);
        return;
    }
    
    switch (event->key()) {
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            zoomIn();
            break;
        case Qt::Key_Minus:
            zoomOut();
            break;
        case Qt::Key_0:
            if (event->modifiers() & Qt::ControlModifier) {
                resetZoom();
            }
            break;
        case Qt::Key_C:
            if (event->modifiers() & Qt::ControlModifier) {
                copySelection();
            }
            break;
        case Qt::Key_V:
            if (event->modifiers() & Qt::ControlModifier) {
                pasteData();
            }
            break;
        case Qt::Key_A:
            if (event->modifiers() & Qt::ControlModifier) {
                selectRange(1, pointCount);
            }
            break;
        default:
            QWidget::keyPressEvent(event);
            break;
    }
}

void VectorPlot::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    calculatePointPositions();
}

void VectorPlot::contextMenuEvent(QContextMenuEvent *event) {
    if (interactive) {
        contextMenu->exec(event->globalPos());
    }
}

void VectorPlot::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void VectorPlot::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl& url : urls) {
        QString filePath = url.toLocalFile();
        if (filePath.endsWith(".csv")) {
            loadFromCSV(filePath);
        } else if (filePath.endsWith(".json")) {
            loadFromJSON(filePath);
        }
    }
    event->acceptProposedAction();
}

// ============================================================================
// Навигация
// ============================================================================

void VectorPlot::updateTransform() {
    transform.reset();
    transform.translate(panOffset.x(), panOffset.y());
    transform.scale(zoomLevel, 1.0);
}

void VectorPlot::ensurePointVisible(int index) {
    QPoint pointPos = indexToScreen(index);
    
    if (!rect().contains(pointPos)) {
        centerOnIndex(index);
    }
}

QPoint VectorPlot::indexToScreen(int index) const {
    // Конвертация 1-based индекса в экранные координаты
    if (index < 1 || index > pointCount) {
        return QPoint(0, 0);
    }
    
    double x = 40 + (index - 1) * cellWidth * zoomLevel + panOffset.x();
    double normalizedValue = (points[index - 1].value - yMin) / (yMax - yMin);
    double y = 30 + (height() - 60) * (1.0 - normalizedValue) + panOffset.y();
    
    return QPoint(static_cast<int>(x), static_cast<int>(y));
}

int VectorPlot::screenToIndex(const QPoint& screenPos) const {
    // Конвертация экранных координат в 1-based индекс
    double x = (screenPos.x() - 40 - panOffset.x()) / (cellWidth * zoomLevel);
    int index = static_cast<int>(x) + 1;
    return qBound(1, index, pointCount);
}

QRect VectorPlot::getPointRect(int index) const {
    QPoint pos = indexToScreen(index);
    int size = pointSize * zoomLevel;
    return QRect(pos.x() - size/2, pos.y() - size/2, size, size);
}

void VectorPlot::calculatePointPositions() {
    for (auto& point : points) {
        point.screenPos = indexToScreen(point.index);
    }
}

// ============================================================================
// Утилиты
// ============================================================================

QString VectorPlot::formatValue(double value) const {
    if (std::abs(value) < 0.001 || std::abs(value) > 10000) {
        return QString::number(value, 'e', 3);
    }
    return QString::number(value, 'f', 4);
}

QString VectorPlot::formatIndex(int index) const {
    return QString::number(index);
}

} // namespace proxima