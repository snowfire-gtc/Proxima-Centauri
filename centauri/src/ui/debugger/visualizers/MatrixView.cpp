#include "MatrixView.h"
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
#include <QSemaphore>
#include <QReadWriteLock>
#include <QAtomicInt>
#include <QAtomicPointer>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QExplicitlySharedDataPointer>
#include <QImplicitlySharedDataPointer>
#include <QDataStream>
#include <QDebug>
#include <QElapsedTimer>
#include <QQueue>
#include <QSet>
#include <QHash>
#include <QMultiHash>
#include <QMultiMap>
#include <QPair>
#include <QTuple>
#include <QTypeInfo>
#include <QMetaType>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaEnum>
#include <QMetaMethod>
#include <QMetaClassInfo>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QScriptable>
#include <QScriptContext>
#include <QScriptContextInfo>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextListFormat>
#include <QTextTableFormat>
#include <QTextFrameFormat>
#include <QTextImageFormat>
#include <QTextObject>
#include <QTextObjectInterface>
#include <QAbstractTextDocumentLayout>
#include <QTextDocumentFragment>
#include <QTextDocumentWriter>
#include <QPdfWriter>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrintPreviewWidget>
#include <QPageSetupDialog>
#include <QFontInfo>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QStyle>
#include <QStyleFactory>
#include <QStylePainter>
#include <QStyleOption>
#include <QVariantAnimation>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QPauseAnimation>
#include <QEasingCurve>
#include <QGraphicsEffect>
#include <QGraphicsBlurEffect>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QGraphicsColorizeEffect>

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

MatrixView::MatrixView(QWidget *parent)
    : QWidget(parent)
    , rowCount(0)
    , colCount(0)
    , viewMode(MatrixViewMode::Table)
    , currentPalette(ColorPalette::Viridis)
    , zoomLevel(1.0)
    , minZoom(0.1)
    , maxZoom(10.0)
    , hoverRow(-1)
    , hoverCol(-1)
    , isHovering(false)
    , showValues(true)
    , showRowLabels(true)
    , showColLabels(true)
    , showGrid(true)
    , valueZoomThreshold(2.0)
    , numberFormat("%.4f")
    , minValue(0.0)
    , maxValue(1.0)
    , meanValue(0.0)
    , sumValue(0.0)
    , cellWidth(60)
    , cellHeight(30)
    , headerWidth(50)
    , headerHeight(30)
    , interactive(true)
    , editable(false)
    , isPanning(false)
    , isZooming(false)
    , overlayAlpha(0.5)
    , pointSize(5) {
    
    setupUI();
    setupContextMenu();
    setupToolbar();
    generatePalette();
    
    LOG_DEBUG("MatrixView created");
}

MatrixView::~MatrixView() {
    LOG_DEBUG("MatrixView destroyed");
}

// ============================================================================
// Настройка UI
// ============================================================================

void MatrixView::setupUI() {
    setMinimumSize(400, 300);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    
    // Включение double buffering для плавной отрисовки
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    
    // Стиль
    setStyleSheet(
        "QWidget { "
        "  background-color: #1e1e1e; "
        "}"
    );
}

void MatrixView::setupContextMenu() {
    contextMenu = new QMenu(this);
    
    // Действия копирования/вставки
    copyAction = contextMenu->addAction("Копировать", this, &MatrixView::copySelection);
    copyAction->setShortcut(QKeySequence::Copy);
    
    pasteAction = contextMenu->addAction("Вставить", this, &MatrixView::pasteSelection);
    pasteAction->setShortcut(QKeySequence::Paste);
    
    selectAllAction = contextMenu->addAction("Выделить всё", this, [this]() {
        selectRange(1, 1, rowCount, colCount);
    });
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    
    contextMenu->addSeparator();
    
    // Действия обрезки
    cropAction = contextMenu->addAction("Обрезать по выделению", this, &MatrixView::cropToSelection);
    
    contextMenu->addSeparator();
    
    // Действия сохранения
    saveImageAction = contextMenu->addAction("Сохранить как изображение...", this, [this]() {
        QString path = QFileDialog::getSaveFileName(this, "Сохранить как изображение", "", 
                                                    "PNG Files (*.png);;JPG Files (*.jpg);;All Files (*)");
        if (!path.isEmpty()) {
            saveAsImage(path);
        }
    });
    
    saveCSVAction = contextMenu->addAction("Сохранить как CSV...", this, [this]() {
        QString path = QFileDialog::getSaveFileName(this, "Сохранить как CSV", "", 
                                                    "CSV Files (*.csv);;All Files (*)");
        if (!path.isEmpty()) {
            saveAsCSV(path);
        }
    });
    
    loadCSVAction = contextMenu->addAction("Загрузить из CSV...", this, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "Загрузить из CSV", "", 
                                                    "CSV Files (*.csv);;All Files (*)");
        if (!path.isEmpty()) {
            loadFromCSV(path);
        }
    });
    
    contextMenu->addSeparator();
    
    // Действия печати
    printAction = contextMenu->addAction("Печать...", this, &MatrixView::print);
    printAction->setShortcut(QKeySequence::Print);
    
    contextMenu->addSeparator();
    
    // Действия масштабирования
    zoomInAction = contextMenu->addAction("Увеличить", this, &MatrixView::zoomIn);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    
    zoomOutAction = contextMenu->addAction("Уменьшить", this, &MatrixView::zoomOut);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    
    resetZoomAction = contextMenu->addAction("Сбросить масштаб", this, &MatrixView::resetZoom);
    resetZoomAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    
    fitToWidgetAction = contextMenu->addAction("Подогнать под размер", this, &MatrixView::fitToWidget);
    
    contextMenu->addSeparator();
    
    // Режимы отображения
    tableModeAction = contextMenu->addAction("Таблица", this, [this]() {
        setViewMode(MatrixViewMode::Table);
    });
    tableModeAction->setCheckable(true);
    
    imageModeAction = contextMenu->addAction("Изображение", this, [this]() {
        setViewMode(MatrixViewMode::Image);
    });
    imageModeAction->setCheckable(true);
    
    heatmapModeAction = contextMenu->addAction("Тепловая карта", this, [this]() {
        setViewMode(MatrixViewMode::Heatmap);
    });
    heatmapModeAction->setCheckable(true);
    
    contextMenu->addSeparator();
    
    // Отображение значений
    showValuesAction = contextMenu->addAction("Показывать значения", this, [this](bool checked) {
        setShowValues(checked);
    });
    showValuesAction->setCheckable(true);
    showValuesAction->setChecked(showValues);
    
    showGridAction = contextMenu->addAction("Показывать сетку", this, [this](bool checked) {
        showGrid = checked;
        update();
    });
    showGridAction->setCheckable(true);
    showGridAction->setChecked(showGrid);
    
    showRowLabelsAction = contextMenu->addAction("Заголовки строк", this, [this](bool checked) {
        setShowRowLabels(checked);
    });
    showRowLabelsAction->setCheckable(true);
    showRowLabelsAction->setChecked(showRowLabels);
    
    showColLabelsAction = contextMenu->addAction("Заголовки столбцов", this, [this](bool checked) {
        setShowColLabels(checked);
    });
    showColLabelsAction->setCheckable(true);
    showColLabelsAction->setChecked(showColLabels);
    
    contextMenu->addSeparator();
    
    // Палитры
    paletteMenu = contextMenu->addMenu("Палитра");
    paletteGroup = new QActionGroup(this);
    
    QStringList paletteNames = {"Viridis", "Plasma", "Inferno", "Magma", "Grayscale", 
                                "Rainbow", "Jet", "Cool", "Hot", "Bone", "Copper", 
                                "Pink", "Spring", "Summer", "Autumn", "Winter"};
    
    for (const QString& name : paletteNames) {
        QAction* action = paletteMenu->addAction(name);
        action->setCheckable(true);
        action->setActionGroup(paletteGroup);
        
        ColorPalette palette = static_cast<ColorPalette>(paletteNames.indexOf(name));
        connect(action, &QAction::triggered, this, [this, palette]() {
            setColorPalette(palette);
        });
        
        if (palette == currentPalette) {
            action->setChecked(true);
        }
    }
    
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        contextMenu->exec(mapToGlobal(pos));
    });
}

void MatrixView::setupToolbar() {
    toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));
    toolbar->setVisible(false);  // Скрыта по умолчанию
}

// ============================================================================
// Данные матрицы
// ============================================================================

void MatrixView::setData(const QVector<QVector<double>>& data) {
    matrixData = data;
    rowCount = data.size();
    colCount = rowCount > 0 ? data[0].size() : 0;
    
    // Инициализация заголовков (1-based для отображения)
    rowLabels.clear();
    colLabels.clear();
    
    for (int i = 1; i <= rowCount; i++) {
        rowLabels.append(QString::number(i));
    }
    for (int j = 1; j <= colCount; j++) {
        colLabels.append(QString::number(j));
    }
    
    calculateStatistics();
    generatePalette();
    updateCellRects();
    update();
    
    LOG_INFO("Matrix data set: " + QString::number(rowCount) + "x" + 
             QString::number(colCount));
}

void MatrixView::setDataFromRuntime(const RuntimeValue& value) {
    if (value.type != RuntimeValue::Type::Array) {
        LOG_WARNING("Invalid runtime value type for matrix");
        return;
    }
    
    QVector<QVector<double>> data;
    
    for (const auto& row : value.arrayValue) {
        QVector<double> rowData;
        for (const auto& cell : row.arrayValue) {
            rowData.append(cell.numberValue);
        }
        data.append(rowData);
    }
    
    setData(data);
}

double MatrixView::getValue(int row, int col) const {
    // Проверка границ (1-based индексация)
    if (row < 1 || row > rowCount || col < 1 || col > colCount) {
        return 0.0;
    }
    
    // Конвертация в 0-based для внутреннего хранения
    return matrixData[row - 1][col - 1];
}

void MatrixView::setValue(int row, int col, double value) {
    if (!editable) return;
    
    // Проверка границ (1-based индексация)
    if (row < 1 || row > rowCount || col < 1 || col > colCount) {
        return;
    }
    
    // Конвертация в 0-based для внутреннего хранения
    matrixData[row - 1][col - 1] = value;
    
    calculateStatistics();
    update();
    
    emit dataModified();
}

// ============================================================================
// Режимы отображения
// ============================================================================

void MatrixView::setViewMode(MatrixViewMode mode) {
    if (viewMode != mode) {
        viewMode = mode;
        update();
        emit modeChanged(mode);
        
        // Обновление чекбоксов в меню
        tableModeAction->setChecked(mode == MatrixViewMode::Table);
        imageModeAction->setChecked(mode == MatrixViewMode::Image);
        heatmapModeAction->setChecked(mode == MatrixViewMode::Heatmap);
    }
}

void MatrixView::setColorPalette(ColorPalette palette) {
    if (currentPalette != palette) {
        currentPalette = palette;
        generatePalette();
        update();
        
        // Обновление чекбокса в меню
        for (QAction* action : paletteGroup->actions()) {
            if (action->text() == QString::number(static_cast<int>(palette))) {
                action->setChecked(true);
                break;
            }
        }
    }
}

void MatrixView::setCustomPalette(const QVector<QColor>& colors) {
    customPalette = colors;
    if (currentPalette == ColorPalette::Custom) {
        paletteColors = colors;
        update();
    }
}

// ============================================================================
// Масштабирование
// ============================================================================

void MatrixView::setZoomLevel(double level) {
    zoomLevel = qBound(minZoom, level, maxZoom);
    updateTransform();
    update();
    emit zoomChanged(zoomLevel);
}

void MatrixView::zoomIn() {
    setZoomLevel(zoomLevel * 1.2);
}

void MatrixView::zoomOut() {
    setZoomLevel(zoomLevel / 1.2);
}

void MatrixView::resetZoom() {
    setZoomLevel(1.0);
    panOffset = QPointF(0, 0);
}

void MatrixView::fitToWidget() {
    if (rowCount == 0 || colCount == 0) return;
    
    double availableWidth = width() - headerWidth;
    double availableHeight = height() - headerHeight;
    
    double zoomX = availableWidth / (colCount * cellWidth);
    double zoomY = availableHeight / (rowCount * cellHeight);
    
    setZoomLevel(qMin(zoomX, zoomY));
    panOffset = QPointF(0, 0);
}

void MatrixView::scrollToCell(int row, int col) {
    // Прокрутка к ячейке
    ensureCellVisible(row, col);
}

void MatrixView::centerOnCell(int row, int col) {
    // Центрирование на ячейке
    QPoint cellPos = cellToScreen(row, col);
    panOffset = QPointF(width() / 2 - cellPos.x(), height() / 2 - cellPos.y());
    update();
}

// ============================================================================
// Выделение
// ============================================================================

void MatrixView::selectCell(int row, int col) {
    selectRange(row, col, row, col);
}

void MatrixView::selectRange(int startRow, int startCol, int endRow, int endCol) {
    // Проверка границ (1-based индексация)
    startRow = qBound(1, startRow, rowCount);
    startCol = qBound(1, startCol, colCount);
    endRow = qBound(1, endRow, rowCount);
    endCol = qBound(1, endCol, colCount);
    
    // Нормализация диапазона
    if (startRow > endRow) qSwap(startRow, endRow);
    if (startCol > endCol) qSwap(startCol, endCol);
    
    selection.startRow = startRow;
    selection.startCol = startCol;
    selection.endRow = endRow;
    selection.endCol = endCol;
    
    update();
    emit selectionChanged(selection);
}

SelectionRange MatrixView::getSelection() const {
    return selection;
}

void MatrixView::clearSelection() {
    selection = SelectionRange();
    update();
    emit selectionChanged(selection);
}

bool MatrixView::hasSelection() const {
    return selection.isValid();
}

QVector<QVector<double>> MatrixView::getSelectedData() const {
    if (!selection.isValid()) {
        return QVector<QVector<double>>();
    }
    
    QVector<QVector<double>> selected;
    
    for (int i = selection.startRow; i <= selection.endRow; i++) {
        QVector<double> row;
        for (int j = selection.startCol; j <= selection.endCol; j++) {
            row.append(getValue(i, j));
        }
        selected.append(row);
    }
    
    return selected;
}

void MatrixView::copySelection() {
    if (!hasSelection()) return;
    
    QVector<QVector<double>> data = getSelectedData();
    
    QString csvData;
    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data[i].size(); j++) {
            csvData += QString::number(data[i][j]);
            if (j < data[i].size() - 1) {
                csvData += ",";
            }
        }
        csvData += "\n";
    }
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(csvData);
    
    LOG_INFO("Selection copied to clipboard");
}

void MatrixView::pasteSelection() {
    if (!editable) return;
    
    QClipboard* clipboard = QApplication::clipboard();
    QString text = clipboard->text();
    
    // Парсинг CSV данных
    QStringList rows = text.split("\n", Qt::SkipEmptyParts);
    if (rows.isEmpty()) return;
    
    // В полной реализации - вставка данных
    LOG_INFO("Paste operation requested");
}

// ============================================================================
// Наложения
// ============================================================================

void MatrixView::setOverlay(const QVector<QVector<double>>& overlay, double alpha) {
    overlayData = overlay;
    overlayAlpha = alpha;
    update();
}

void MatrixView::setGraphOverlay(const QVector<QPoint>& points, const QColor& color) {
    graphPoints = points;
    graphColor = color;
    update();
}

void MatrixView::setPointOverlay(const QVector<QPoint>& points, const QColor& color, int size) {
    pointOverlay = points;
    pointColor = color;
    pointSize = size;
    update();
}

void MatrixView::clearOverlays() {
    overlayData.clear();
    graphPoints.clear();
    pointOverlay.clear();
    update();
}

// ============================================================================
// Отображение значений
// ============================================================================

void MatrixView::setShowValues(bool show) {
    showValues = show;
    showValuesAction->setChecked(show);
    update();
}

void MatrixView::setValueZoomThreshold(double threshold) {
    valueZoomThreshold = threshold;
    update();
}

void MatrixView::setNumberFormat(const QString& format) {
    numberFormat = format;
    update();
}

// ============================================================================
// Заголовки
// ============================================================================

void MatrixView::setShowRowLabels(bool show) {
    showRowLabels = show;
    showRowLabelsAction->setChecked(show);
    update();
}

void MatrixView::setShowColLabels(bool show) {
    showColLabels = show;
    showColLabelsAction->setChecked(show);
    update();
}

void MatrixView::setRowLabels(const QVector<QString>& labels) {
    rowLabels = labels;
    update();
}

void MatrixView::setColLabels(const QVector<QString>& labels) {
    colLabels = labels;
    update();
}

// ============================================================================
// Обрезка матрицы
// ============================================================================

void MatrixView::cropToSelection() {
    if (!hasSelection()) {
        QMessageBox::warning(this, "Warning", "No selection to crop");
        return;
    }
    
    crop(selection.startRow, selection.startCol, selection.endRow, selection.endCol);
}

void MatrixView::crop(int startRow, int startCol, int endRow, int endCol) {
    // Проверка границ (1-based индексация)
    if (startRow < 1 || startCol < 1 || endRow > rowCount || endCol > colCount) {
        QMessageBox::warning(this, "Warning", "Invalid crop range");
        return;
    }
    
    QVector<QVector<double>> cropped;
    
    for (int i = startRow - 1; i < endRow; i++) {
        QVector<double> row;
        for (int j = startCol - 1; j < endCol; j++) {
            row.append(matrixData[i][j]);
        }
        cropped.append(row);
    }
    
    setData(cropped);
    clearSelection();
    
    LOG_INFO("Matrix cropped to: " + QString::number(endRow - startRow + 1) + "x" + 
             QString::number(endCol - startCol + 1));
}

// ============================================================================
// Экспорт/Импорт
// ============================================================================

bool MatrixView::saveAsImage(const QString& path, const QString& format) {
    QImage image(size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    
    QPainter painter(&image);
    paintEvent(nullptr);
    painter.end();
    
    return image.save(path, format.toStdString().c_str());
}

bool MatrixView::saveAsCSV(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < colCount; j++) {
            out << matrixData[i][j];
            if (j < colCount - 1) {
                out << ",";
            }
        }
        out << "\n";
    }
    
    file.close();
    return true;
}

bool MatrixView::loadFromCSV(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    
    QVector<QVector<double>> data;
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        
        QStringList values = line.split(",");
        QVector<double> row;
        
        for (const QString& value : values) {
            bool ok;
            double num = value.toDouble(&ok);
            if (ok) {
                row.append(num);
            }
        }
        
        if (!row.isEmpty()) {
            data.append(row);
        }
    }
    
    file.close();
    
    if (!data.isEmpty()) {
        setData(data);
        return true;
    }
    
    return false;
}

void MatrixView::print() {
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        paintEvent(nullptr);
        painter.end();
    }
}

// ============================================================================
// Статистика
// ============================================================================

void MatrixView::calculateStatistics() {
    if (rowCount == 0 || colCount == 0) {
        minValue = 0.0;
        maxValue = 0.0;
        meanValue = 0.0;
        sumValue = 0.0;
        return;
    }
    
    minValue = matrixData[0][0];
    maxValue = matrixData[0][0];
    sumValue = 0.0;
    
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < colCount; j++) {
            double value = matrixData[i][j];
            if (value < minValue) minValue = value;
            if (value > maxValue) maxValue = value;
            sumValue += value;
        }
    }
    
    meanValue = sumValue / (rowCount * colCount);
}

// ============================================================================
// Интерактивность
// ============================================================================

void MatrixView::setInteractive(bool enable) {
    interactive = enable;
}

void MatrixView::setEditable(bool enable) {
    editable = enable;
}

// ============================================================================
// События отрисовки
// ============================================================================

void MatrixView::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Очистка фона
    painter.fillRect(rect(), QColor(30, 30, 30));
    
    // Применение трансформации
    painter.save();
    painter.translate(panOffset);
    painter.scale(zoomLevel, zoomLevel);
    
    // Отрисовка в зависимости от режима
    switch (viewMode) {
        case MatrixViewMode::Table:
            drawTable(painter);
            break;
        case MatrixViewMode::Image:
            drawImage(painter);
            break;
        case MatrixViewMode::Heatmap:
            drawHeatmap(painter);
            break;
    }
    
    painter.restore();
    
    // Отрисовка заголовков (без трансформации)
    if (showRowLabels) {
        drawRowLabels(painter);
    }
    if (showColLabels) {
        drawColLabels(painter);
    }
    
    // Отрисовка наведений и выделений
    if (isHovering && hoverRow > 0 && hoverCol > 0) {
        drawHoverCell(painter);
    }
    if (hasSelection()) {
        drawSelection(painter);
    }
}

void MatrixView::drawTable(QPainter& painter) {
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < colCount; j++) {
            int row = i + 1;  // 1-based для отображения
            int col = j + 1;  // 1-based для отображения
            
            QRect cellRect = getCellRect(row, col);
            
            // Цвет фона
            QColor bgColor = (i + j) % 2 == 0 ? QColor(40, 40, 40) : QColor(45, 45, 45);
            
            // Проверка выделения
            if (hasSelection() && 
                row >= selection.startRow && row <= selection.endRow &&
                col >= selection.startCol && col <= selection.endCol) {
                bgColor = QColor(0, 122, 204, 100);
            }
            
            // Проверка наведения
            if (isHovering && row == hoverRow && col == hoverCol) {
                bgColor = bgColor.lighter(120);
            }
            
            painter.fillRect(cellRect, bgColor);
            
            // Сетка
            if (showGrid) {
                painter.setPen(QPen(QColor(60, 60, 60), 1));
                painter.drawRect(cellRect);
            }
            
            // Значение
            if (showValues && zoomLevel >= valueZoomThreshold) {
                drawValue(painter, row, col, cellRect);
            }
        }
    }
}

void MatrixView::drawImage(QPainter& painter) {
    if (rowCount == 0 || colCount == 0) return;
    
    int cellW = cellWidth * zoomLevel;
    int cellH = cellHeight * zoomLevel;
    
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < colCount; j++) {
            QRect cellRect(i * cellW, j * cellH, cellW, cellH);
            double value = matrixData[i][j];
            
            // Нормализация значения
            double normalized = (maxValue != minValue) ? 
                               (value - minValue) / (maxValue - minValue) : 0.5;
            
            // Цвет из палитры
            int colorIndex = qMin(static_cast<int>(normalized * (paletteColors.size() - 1)), 
                                 paletteColors.size() - 1);
            QColor color = paletteColors[colorIndex];
            
            painter.fillRect(cellRect, color);
        }
    }
    
    // Наложения
    drawOverlays(painter);
}

void MatrixView::drawHeatmap(QPainter& painter) {
    drawImage(painter);
    
    // Дополнительные эффекты для тепловой карты
    // В полной реализации - градиенты, сглаживание
}

void MatrixView::drawRowLabels(QPainter& painter) {
    if (!showRowLabels || rowCount == 0) return;
    
    painter.setPen(QColor(200, 200, 200));
    painter.setFont(font());
    
    int cellH = cellHeight * zoomLevel;
    
    for (int i = 0; i < rowCount; i++) {
        int y = headerHeight + i * cellH;
        QRect labelRect(0, y, headerWidth, cellH);
        
        painter.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, rowLabels[i]);
    }
}

void MatrixView::drawColLabels(QPainter& painter) {
    if (!showColLabels || colCount == 0) return;
    
    painter.setPen(QColor(200, 200, 200));
    painter.setFont(font());
    
    int cellW = cellWidth * zoomLevel;
    
    for (int j = 0; j < colCount; j++) {
        int x = headerWidth + j * cellW;
        QRect labelRect(x, 0, cellW, headerHeight);
        
        painter.drawText(labelRect, Qt::AlignCenter, colLabels[j]);
    }
}

void MatrixView::drawGrid(QPainter& painter) {
    if (!showGrid) return;
    
    painter.setPen(QPen(QColor(60, 60, 60), 1));
    
    int cellW = cellWidth * zoomLevel;
    int cellH = cellHeight * zoomLevel;
    
    for (int i = 0; i <= rowCount; i++) {
        int y = headerHeight + i * cellH;
        painter.drawLine(headerWidth, y, headerWidth + colCount * cellW, y);
    }
    
    for (int j = 0; j <= colCount; j++) {
        int x = headerWidth + j * cellW;
        painter.drawLine(x, headerHeight, x, headerHeight + rowCount * cellH);
    }
}

void MatrixView::drawSelection(QPainter& painter) {
    if (!hasSelection()) return;
    
    int cellW = cellWidth * zoomLevel;
    int cellH = cellHeight * zoomLevel;
    
    int x = headerWidth + (selection.startCol - 1) * cellW;
    int y = headerHeight + (selection.startRow - 1) * cellH;
    int w = selection.colCount() * cellW;
    int h = selection.rowCount() * cellH;
    
    painter.setPen(QPen(QColor(0, 122, 204), 2));
    painter.setBrush(QColor(0, 122, 204, 50));
    painter.drawRect(x, y, w, h);
}

void MatrixView::drawHoverCell(QPainter& painter) {
    if (hoverRow <= 0 || hoverCol <= 0) return;
    
    QRect cellRect = getCellRect(hoverRow, hoverCol);
    
    painter.setPen(QPen(QColor(255, 255, 255), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(cellRect);
    
    // Tooltip со значением
    QString tooltip = QString("[%1,%2]\nValue: %3")
        .arg(hoverRow)  // 1-based
        .arg(hoverCol)  // 1-based
        .arg(formatValue(getValue(hoverRow, hoverCol)));
    
    QToolTip::showText(mapToGlobal(cellRect.center()), tooltip, this);
}

void MatrixView::drawOverlays(QPainter& painter) {
    // Отрисовка наложений
    // В полной реализации - отрисовка overlayData, graphPoints, pointOverlay
}

void MatrixView::drawValue(QPainter& painter, int row, int col, const QRect& rect) {
    double value = getValue(row, col);
    QString text = formatValue(value);
    
    // Выбор цвета текста в зависимости от фона
    QColor textColor = Qt::white;
    if (hasSelection() && 
        row >= selection.startRow && row <= selection.endRow &&
        col >= selection.startCol && col <= selection.endCol) {
        textColor = Qt::yellow;
    }
    
    painter.setPen(textColor);
    painter.drawText(rect, Qt::AlignCenter, text);
}

// ============================================================================
// События мыши
// ============================================================================

void MatrixView::mousePressEvent(QMouseEvent *event) {
    if (!interactive) {
        QWidget::mousePressEvent(event);
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        QPoint cellPos = screenToCell(event->pos());
        
        if (cellPos.x() > 0 && cellPos.x() <= rowCount &&
            cellPos.y() > 0 && cellPos.y() <= colCount) {
            
            if (event->modifiers() & Qt::ShiftModifier) {
                // Выделение диапазона
                if (hasSelection()) {
                    selectRange(selection.startRow, selection.startCol, 
                               cellPos.x(), cellPos.y());
                } else {
                    selectCell(cellPos.x(), cellPos.y());
                }
            } else {
                // Выделение ячейки
                selectCell(cellPos.x(), cellPos.y());
                emit cellClicked(cellPos.x(), cellPos.y(), getValue(cellPos.x(), cellPos.y()));
            }
        } else {
            // Начало перетаскивания
            isPanning = true;
            lastPanPos = event->pos();
        }
    }
    
    QWidget::mousePressEvent(event);
}

void MatrixView::mouseMoveEvent(QMouseEvent *event) {
    if (!interactive) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    
    if (isPanning) {
        QPoint delta = event->pos() - lastPanPos;
        panOffset += QPointF(delta);
        lastPanPos = event->pos();
        update();
    } else {
        // Обновление наведения
        QPoint cellPos = screenToCell(event->pos());
        
        if (cellPos.x() > 0 && cellPos.x() <= rowCount &&
            cellPos.y() > 0 && cellPos.y() <= colCount) {
            hoverRow = cellPos.x();
            hoverCol = cellPos.y();
            isHovering = true;
            
            emit cellHovered(hoverRow, hoverCol, getValue(hoverRow, hoverCol));
        } else {
            isHovering = false;
            hoverRow = -1;
            hoverCol = -1;
        }
        
        update();
    }
    
    QWidget::mouseMoveEvent(event);
}

void MatrixView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isPanning = false;
    }
    
    QWidget::mouseReleaseEvent(event);
}

void MatrixView::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QPoint cellPos = screenToCell(event->pos());
        
        if (cellPos.x() > 0 && cellPos.x() <= rowCount &&
            cellPos.y() > 0 && cellPos.y() <= colCount) {
            
            emit cellDoubleClicked(cellPos.x(), cellPos.y(), getValue(cellPos.x(), cellPos.y()));
            
            // Если редактируемый - открытие диалога редактирования
            if (editable) {
                bool ok;
                double newValue = QInputDialog::getDouble(this, "Edit Cell",
                    QString("Value [%1,%2]:").arg(cellPos.x()).arg(cellPos.y()),
                    getValue(cellPos.x(), cellPos.y()), -1e10, 1e10, 6, &ok);
                
                if (ok) {
                    setValue(cellPos.x(), cellPos.y(), newValue);
                }
            }
        }
    }
    
    QWidget::mouseDoubleClickEvent(event);
}

void MatrixView::wheelEvent(QWheelEvent *event) {
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

void MatrixView::keyPressEvent(QKeyEvent *event) {
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
                pasteSelection();
            }
            break;
        case Qt::Key_A:
            if (event->modifiers() & Qt::ControlModifier) {
                selectRange(1, 1, rowCount, colCount);
            }
            break;
        default:
            QWidget::keyPressEvent(event);
            break;
    }
}

void MatrixView::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    updateCellRects();
}

void MatrixView::contextMenuEvent(QContextMenuEvent *event) {
    if (interactive) {
        contextMenu->exec(event->globalPos());
    }
}

void MatrixView::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MatrixView::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl& url : urls) {
        QString filePath = url.toLocalFile();
        if (filePath.endsWith(".csv")) {
            loadFromCSV(filePath);
        }
    }
    event->acceptProposedAction();
}

// ============================================================================
// Навигация
// ============================================================================

void MatrixView::updateTransform() {
    transform.reset();
    transform.translate(panOffset.x(), panOffset.y());
    transform.scale(zoomLevel, zoomLevel);
}

void MatrixView::ensureCellVisible(int row, int col) {
    // Прокрутка к ячейке
    QPoint cellPos = cellToScreen(row, col);
    
    if (!rect().contains(cellPos)) {
        // Центрирование
        centerOnCell(row, col);
    }
}

QPoint MatrixView::cellToScreen(int row, int col) const {
    // Конвертация 1-based в экранные координаты
    int x = headerWidth + (col - 1) * cellWidth * zoomLevel + panOffset.x();
    int y = headerHeight + (row - 1) * cellHeight * zoomLevel + panOffset.y();
    return QPoint(x, y);
}

QPoint MatrixView::screenToCell(const QPoint& screenPos) const {
    // Конвертация экранных координат в 1-based индексы ячеек
    int col = 1 + static_cast<int>((screenPos.x() - headerWidth - panOffset.x()) / 
                                    (cellWidth * zoomLevel));
    int row = 1 + static_cast<int>((screenPos.y() - headerHeight - panOffset.y()) / 
                                    (cellHeight * zoomLevel));
    return QPoint(row, col);
}

QRect MatrixView::getCellRect(int row, int col) const {
    // Получение прямоугольника ячейки (1-based индексы)
    int x = headerWidth + (col - 1) * cellWidth * zoomLevel;
    int y = headerHeight + (row - 1) * cellHeight * zoomLevel;
    int w = cellWidth * zoomLevel;
    int h = cellHeight * zoomLevel;
    return QRect(x, y, w, h);
}

void MatrixView::updateCellRects() {
    calculateCellGeometry();
    update();
}

void MatrixView::calculateCellGeometry() {
    // Расчёт геометрии ячеек на основе размера виджета
    if (rowCount > 0 && colCount > 0) {
        int availableWidth = width() - headerWidth;
        int availableHeight = height() - headerHeight;
        
        cellWidth = availableWidth / colCount;
        cellHeight = availableHeight / rowCount;
        
        // Минимальный размер ячейки
        cellWidth = qMax(cellWidth, 20);
        cellHeight = qMax(cellHeight, 15);
    }
}

// ============================================================================
// Палитры
// ============================================================================

void MatrixView::generatePalette() {
    const int paletteSize = 256;
    
    switch (currentPalette) {
        case ColorPalette::Viridis:
            paletteColors = generateViridisPalette(paletteSize);
            break;
        case ColorPalette::Plasma:
            paletteColors = generatePlasmaPalette(paletteSize);
            break;
        case ColorPalette::Inferno:
            paletteColors = generateInfernoPalette(paletteSize);
            break;
        case ColorPalette::Magma:
            paletteColors = generateMagmaPalette(paletteSize);
            break;
        case ColorPalette::Grayscale:
            paletteColors = generateGrayscalePalette(paletteSize);
            break;
        case ColorPalette::Rainbow:
            paletteColors = generateRainbowPalette(paletteSize);
            break;
        case ColorPalette::Jet:
            paletteColors = generateJetPalette(paletteSize);
            break;
        case ColorPalette::Custom:
            paletteColors = customPalette;
            break;
        default:
            paletteColors = generateViridisPalette(paletteSize);
            break;
    }
}

QColor MatrixView::valueToColor(double value) const {
    if (paletteColors.isEmpty()) {
        return Qt::gray;
    }
    
    // Нормализация значения
    double normalized = (maxValue != minValue) ? 
                       (value - minValue) / (maxValue - minValue) : 0.5;
    normalized = qBound(0.0, normalized, 1.0);
    
    // Индекс в палитре
    int index = static_cast<int>(normalized * (paletteColors.size() - 1));
    index = qBound(0, index, paletteColors.size() - 1);
    
    return paletteColors[index];
}

QVector<QColor> MatrixView::generateViridisPalette(int size) const {
    // Генерация палитры Viridis
    QVector<QColor> colors;
    for (int i = 0; i < size; i++) {
        double t = static_cast<double>(i) / (size - 1);
        int r = static_cast<int>(68 + t * 187);
        int g = static_cast<int>(1 + t * 200);
        int b = static_cast<int>(84 + t * 92);
        colors.append(QColor(r, g, b));
    }
    return colors;
}

QVector<QColor> MatrixView::generatePlasmaPalette(int size) const {
    // Генерация палитры Plasma
    QVector<QColor> colors;
    for (int i = 0; i < size; i++) {
        double t = static_cast<double>(i) / (size - 1);
        int r = static_cast<int>(13 + t * 240);
        int g = static_cast<int>(8 + t * 7);
        int b = static_cast<int>(135 + t * 120);
        colors.append(QColor(r, g, b));
    }
    return colors;
}

QVector<QColor> MatrixView::generateInfernoPalette(int size) const {
    // Генерация палитры Inferno
    QVector<QColor> colors;
    for (int i = 0; i < size; i++) {
        double t = static_cast<double>(i) / (size - 1);
        int r = static_cast<int>(t * 255);
        int g = static_cast<int>(t * 100);
        int b = static_cast<int>(t * 50);
        colors.append(QColor(r, g, b));
    }
    return colors;
}

QVector<QColor> MatrixView::generateMagmaPalette(int size) const {
    // Генерация палитры Magma
    QVector<QColor> colors;
    for (int i = 0; i < size; i++) {
        double t = static_cast<double>(i) / (size - 1);
        int r = static_cast<int>(t * 250);
        int g = static_cast<int>(t * 100);
        int b = static_cast<int>(100 + t * 155);
        colors.append(QColor(r, g, b));
    }
    return colors;
}

QVector<QColor> MatrixView::generateGrayscalePalette(int size) const {
    // Генерация палитры Grayscale
    QVector<QColor> colors;
    for (int i = 0; i < size; i++) {
        int v = static_cast<int>(i * 255.0 / (size - 1));
        colors.append(QColor(v, v, v));
    }
    return colors;
}

QVector<QColor> MatrixView::generateRainbowPalette(int size) const {
    // Генерация палитры Rainbow
    QVector<QColor> colors;
    for (int i = 0; i < size; i++) {
        double t = static_cast<double>(i) / (size - 1);
        int r = static_cast<int>(255 * qAbs(3 * t - 1.5));
        int g = static_cast<int>(255 * (1 - qAbs(3 * t - 2)));
        int b = static_cast<int>(255 * (1 - qAbs(3 * t - 1)));
        colors.append(QColor(r, g, b));
    }
    return colors;
}

QVector<QColor> MatrixView::generateJetPalette(int size) const {
    // Генерация палитры Jet
    QVector<QColor> colors;
    for (int i = 0; i < size; i++) {
        double t = static_cast<double>(i) / (size - 1);
        int r, g, b;
        
        if (t < 0.25) {
            r = 0;
            g = static_cast<int>(255 * (4 * t));
            b = static_cast<int>(255 * (1 + 4 * t));
        } else if (t < 0.5) {
            r = 0;
            g = 255;
            b = static_cast<int>(255 * (2 - 4 * t));
        } else if (t < 0.75) {
            r = static_cast<int>(255 * (4 * t - 2));
            g = 255;
            b = 0;
        } else {
            r = 255;
            g = static_cast<int>(255 * (4 - 4 * t));
            b = 0;
        }
        
        colors.append(QColor(r, g, b));
    }
    return colors;
}

// ============================================================================
// Утилиты
// ============================================================================

QString MatrixView::formatValue(double value) const {
    return QString(numberFormat).arg(value);
}

} // namespace proxima