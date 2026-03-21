#include "VectorPlot.h"
#include <QVBoxLayout>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QToolTip>
#include <cmath>

namespace proxima {

VectorPlot::VectorPlot(QWidget *parent)
    : QWidget(parent)
    , selectionStart(-1)
    , selectionEnd(-1)
    , isSelecting(false)
    , hoverIndex(-1)
    , isPanning(false)
    , isLogScaleX(false)
    , isLogScaleY(false)
    , lineWidth(2)
    , showGrid(true)
    , showPoints(false)
    , showLabels(true)
    , showStatistics(false)
    , lineColor(QColor(100, 150, 255))
    , gridColor(QColor(50, 50, 50))
    , backgroundColor(QColor(30, 30, 30)) {
    
    setupUI();
    setupToolbar();
    setupContextMenu();
}

VectorPlot::~VectorPlot() {}

void VectorPlot::setupUI() {
    setMinimumSize(400, 300);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(toolbar);
}

void VectorPlot::setupToolbar() {
    toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));
    
    zoomInAction = toolbar->addAction(QIcon(":/icons/zoom-in.svg"), "Zoom In", this, &VectorPlot::onZoomIn);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    
    zoomOutAction = toolbar->addAction(QIcon(":/icons/zoom-out.svg"), "Zoom Out", this, &VectorPlot::onZoomOut);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    
    zoomFitAction = toolbar->addAction(QIcon(":/icons/zoom-fit.svg"), "Zoom to Fit", this, &VectorPlot::onZoomToFit);
    zoomFitAction->setShortcut(Qt::Key_F);
    
    toolbar->addSeparator();
    
    findPeaksAction = toolbar->addAction(QIcon(":/icons/peaks.svg"), "Find Peaks", this, &VectorPlot::onFindPeaks);
    findValleysAction = toolbar->addAction(QIcon(":/icons/valleys.svg"), "Find Valleys", this, &VectorPlot::onFindValleys);
    
    toolbar->addSeparator();
    
    copyDataAction = toolbar->addAction(QIcon(":/icons/copy.svg"), "Copy Data", this, &VectorPlot::onCopyData);
    saveCSVAction = toolbar->addAction(QIcon(":/icons/save.svg"), "Save CSV", this, &VectorPlot::onSaveCSV);
    saveImageAction = toolbar->addAction(QIcon(":/icons/image.svg"), "Save Image", this, &VectorPlot::onSaveImage);
}

void VectorPlot::setupContextMenu() {
    contextMenu = new QMenu(this);
    contextMenu->addAction(zoomInAction);
    contextMenu->addAction(zoomOutAction);
    contextMenu->addAction(zoomFitAction);
    contextMenu->addSeparator();
    contextMenu->addAction(findPeaksAction);
    contextMenu->addAction(findValleysAction);
    contextMenu->addSeparator();
    contextMenu->addAction(copyDataAction);
    contextMenu->addAction(saveCSVAction);
    contextMenu->addAction(saveImageAction);
    contextMenu->addSeparator();
    clearSelectionAction = contextMenu->addAction("Clear Selection", this, &VectorPlot::onClearSelection);
}

void VectorPlot::setData(const QVector<double>& data) {
    currentData = data;
    calculateViewRange();
    calculateStatistics();
    update();
    emit dataModified();
}

void VectorPlot::addDataset(const QVector<double>& data, const QString& name, const QColor& color) {
    datasets[name] = data;
    datasetColors[name] = color;
    datasetVisibility[name] = true;
    calculateViewRange();
    update();
}

void VectorPlot::clearData() {
    currentData.clear();
    datasets.clear();
    selectionStart = -1;
    selectionEnd = -1;
    hoverIndex = -1;
    update();
}

void VectorPlot::zoomIn() {
    double centerX = (viewRange.minX + viewRange.maxX) / 2.0;
    double centerY = (viewRange.minY + viewRange.maxY) / 2.0;
    double width = viewRange.maxX - viewRange.minX;
    double height = viewRange.maxY - viewRange.minY;
    
    viewRange.minX = centerX - width * 0.4;
    viewRange.maxX = centerX + width * 0.4;
    viewRange.minY = centerY - height * 0.4;
    viewRange.maxY = centerY + height * 0.4;
    
    update();
    emit viewChanged();
}

void VectorPlot::zoomOut() {
    double centerX = (viewRange.minX + viewRange.maxX) / 2.0;
    double centerY = (viewRange.minY + viewRange.maxY) / 2.0;
    double width = viewRange.maxX - viewRange.minX;
    double height = viewRange.maxY - viewRange.minY;
    
    viewRange.minX = centerX - width * 0.6;
    viewRange.maxX = centerX + width * 0.6;
    viewRange.minY = centerY - height * 0.6;
    viewRange.maxY = centerY + height * 0.6;
    
    update();
    emit viewChanged();
}

void VectorPlot::zoomToFit() {
    calculateViewRange();
    update();
    emit viewChanged();
}

void VectorPlot::resetView() {
    zoomToFit();
}

void VectorPlot::selectRange(int start, int end) {
    selectionStart = qMin(start, end);
    selectionEnd = qMax(start, end);
    update();
    emit rangeSelected(selectionStart, selectionEnd);
}

QVector<double> VectorPlot::getSelectedData() const {
    if (!hasSelection()) return QVector<double>();
    
    QVector<double> selected;
    for (int i = selectionStart; i <= selectionEnd && i < currentData.size(); i++) {
        selected.append(currentData[i]);
    }
    return selected;
}

void VectorPlot::clearSelection() {
    selectionStart = -1;
    selectionEnd = -1;
    update();
}

void VectorPlot::findPeaks() {
    peakIndices.clear();
    
    for (int i = 1; i < currentData.size() - 1; i++) {
        if (currentData[i] > currentData[i-1] && currentData[i] > currentData[i+1]) {
            peakIndices.append(i);
        }
    }
    
    update();
}

void VectorPlot::findValleys() {
    valleyIndices.clear();
    
    for (int i = 1; i < currentData.size() - 1; i++) {
        if (currentData[i] < currentData[i-1] && currentData[i] < currentData[i+1]) {
            valleyIndices.append(i);
        }
    }
    
    update();
}

void VectorPlot::calculateStatistics() {
    statistics.clear();
    
    if (currentData.isEmpty()) return;
    
    double sum = 0;
    double min = currentData[0];
    double max = currentData[0];
    
    for (double value : currentData) {
        sum += value;
        if (value < min) min = value;
        if (value > max) max = value;
    }
    
    statistics["count"] = currentData.size();
    statistics["sum"] = sum;
    statistics["mean"] = sum / currentData.size();
    statistics["min"] = min;
    statistics["max"] = max;
    statistics["range"] = max - min;
    
    // Standard deviation
    double variance = 0;
    double mean = statistics["mean"];
    for (double value : currentData) {
        variance += (value - mean) * (value - mean);
    }
    statistics["stddev"] = std::sqrt(variance / currentData.size());
}

void VectorPlot::setHoverPoint(int index) {
    hoverIndex = index;
    update();
    
    if (index >= 0 && index < currentData.size()) {
        emit pointHovered(index, currentData[index]);
        
        // Show tooltip
        QString tooltip = QString("Index: %1\nValue: %2")
            .arg(index).arg(formatValue(currentData[index]));
        QToolTip::showText(QCursor::pos(), tooltip, this);
    }
}

double VectorPlot::getPointValue(int index) const {
    if (index >= 0 && index < currentData.size()) {
        return currentData[index];
    }
    return 0;
}

QPoint VectorPlot::getPointPosition(int index) const {
    if (index < 0 || index >= currentData.size()) return QPoint();
    
    double value = currentData[index];
    return dataToPoint(index, value).toPoint();
}

void VectorPlot::copyData() {
    QString csvData;
    for (int i = 0; i < currentData.size(); i++) {
        csvData += QString("%1,%2\n").arg(i).arg(currentData[i]);
    }
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(csvData);
}

void VectorPlot::saveAsCSV(const QString& path) {
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "index,value\n";
        for (int i = 0; i < currentData.size(); i++) {
            out << i << "," << currentData[i] << "\n";
        }
        file.close();
    }
}

void VectorPlot::saveAsImage(const QString& path) {
    QPixmap pixmap(size());
    render(&pixmap);
    pixmap.save(path);
}

void VectorPlot::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Background
    painter.fillRect(rect(), backgroundColor);
    
    // Calculate plot area (leave space for labels)
    QRect plotRect = rect().adjusted(50, 30, -10, -30);
    painter.setClipRect(plotRect);
    
    // Draw grid
    if (showGrid) {
        drawGrid(painter);
    }
    
    // Draw axes
    drawAxes(painter);
    
    // Draw data
    drawData(painter);
    
    // Draw selection
    if (hasSelection()) {
        drawSelection(painter);
    }
    
    // Draw points
    if (showPoints) {
        drawPoints(painter);
    }
    
    // Draw peaks and valleys
    drawPoints(painter);
    
    // Draw labels
    if (showLabels) {
        drawLabels(painter);
    }
    
    // Draw statistics
    if (showStatistics) {
        drawStatistics(painter);
    }
    
    // Draw hover point
    if (hoverIndex >= 0) {
        drawHoverPoint(painter);
    }
}

void VectorPlot::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (event->modifiers() & Qt::ShiftModifier) {
            // Start selection
            isSelecting = true;
            selectionStartPos = event->pos();
            selectionStart = pointToDataIndex(event->pos());
        } else {
            // Start panning
            isPanning = true;
            lastPanPos = event->pos();
        }
    }
}

void VectorPlot::mouseMoveEvent(QMouseEvent *event) {
    if (isPanning) {
        QPoint delta = event->pos() - lastPanPos;
        double dx = (delta.x() * (viewRange.maxX - viewRange.minX)) / width();
        double dy = (delta.y() * (viewRange.maxY - viewRange.minY)) / height();
        
        viewRange.minX -= dx;
        viewRange.maxX -= dx;
        viewRange.minY += dy;
        viewRange.maxY += dy;
        
        lastPanPos = event->pos();
        update();
        emit viewChanged();
    }
    
    if (isSelecting) {
        selectionEnd = pointToDataIndex(event->pos());
        update();
    }
    
    // Update hover point
    updateHoverPoint(event->pos());
}

void VectorPlot::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (isSelecting) {
            isSelecting = false;
            if (selectionEnd >= 0) {
                selectRange(selectionStart, selectionEnd);
            }
        }
        isPanning = false;
    }
}

void VectorPlot::wheelEvent(QWheelEvent *event) {
    if (event->angleDelta().y() > 0) {
        zoomIn();
    } else {
        zoomOut();
    }
}

void VectorPlot::contextMenuEvent(QContextMenuEvent *event) {
    contextMenu->exec(event->globalPos());
}

void VectorPlot::drawGrid(QPainter& painter) {
    painter.setPen(QPen(gridColor, 1, Qt::DashLine));
    
    // Vertical grid lines
    int numVerticalLines = 10;
    for (int i = 0; i <= numVerticalLines; i++) {
        double x = viewRange.minX + (viewRange.maxX - viewRange.minX) * i / numVerticalLines;
        QPointF p1 = dataToPoint(x, viewRange.minY);
        QPointF p2 = dataToPoint(x, viewRange.maxY);
        painter.drawLine(p1.toPoint(), p2.toPoint());
    }
    
    // Horizontal grid lines
    int numHorizontalLines = 5;
    for (int i = 0; i <= numHorizontalLines; i++) {
        double y = viewRange.minY + (viewRange.maxY - viewRange.minY) * i / numHorizontalLines;
        QPointF p1 = dataToPoint(viewRange.minX, y);
        QPointF p2 = dataToPoint(viewRange.maxX, y);
        painter.drawLine(p1.toPoint(), p2.toPoint());
    }
}

void VectorPlot::drawData(QPainter& painter) {
    if (currentData.isEmpty()) return;
    
    painter.setPen(QPen(lineColor, lineWidth));
    
    QPainterPath path;
    bool first = true;
    
    for (int i = 0; i < currentData.size(); i++) {
        // Skip points outside view
        if (i < viewRange.minX || i > viewRange.maxX) continue;
        if (currentData[i] < viewRange.minY || currentData[i] > viewRange.maxY) continue;
        
        QPointF point = dataToPoint(i, currentData[i]);
        
        if (first) {
            path.moveTo(point);
            first = false;
        } else {
            path.lineTo(point);
        }
    }
    
    painter.drawPath(path);
}

void VectorPlot::drawSelection(QPainter& painter) {
    if (!hasSelection()) return;
    
    QPointF p1 = dataToPoint(selectionStart, viewRange.minY);
    QPointF p2 = dataToPoint(selectionEnd, viewRange.maxY);
    
    painter.fillRect(QRectF(p1, p2), QColor(100, 100, 255, 50));
}

void VectorPlot::drawHoverPoint(QPainter& painter) {
    if (hoverIndex < 0 || hoverIndex >= currentData.size()) return;
    
    QPointF point = dataToPoint(hoverIndex, currentData[hoverIndex]);
    
    // Draw circle
    painter.setBrush(QColor(255, 255, 255));
    painter.setPen(QPen(QColor(255, 0, 0), 2));
    painter.drawEllipse(point.toPoint(), 5, 5);
    
    // Draw value label
    QString label = formatValue(currentData[hoverIndex]);
    painter.setPen(QColor(255, 255, 255));
    painter.drawText(point.toPoint() + QPoint(10, -10), label);
}

QPointF VectorPlot::dataToPoint(int index, double value) const {
    double x = rect().left() + 50 + 
               (index - viewRange.minX) / (viewRange.maxX - viewRange.minX) * (width() - 60);
    double y = rect().bottom() - 30 - 
               (value - viewRange.minY) / (viewRange.maxY - viewRange.minY) * (height() - 60);
    
    return QPointF(x, y);
}

int VectorPlot::pointToDataIndex(const QPoint& pos) const {
    double xRatio = (pos.x() - 50.0) / (width() - 60);
    int index = viewRange.minX + xRatio * (viewRange.maxX - viewRange.minX);
    return qBound(0, index, currentData.size() - 1);
}

void VectorPlot::calculateViewRange() {
    if (currentData.isEmpty()) {
        viewRange = {0, 100, 0, 1};
        dataRange = viewRange;
        return;
    }
    
    double min = currentData[0];
    double max = currentData[0];
    
    for (double value : currentData) {
        if (value < min) min = value;
        if (value > max) max = value;
    }
    
    // Add padding
    double padding = (max - min) * 0.1;
    if (padding == 0) padding = 1;
    
    dataRange = {0, currentData.size() - 1, min - padding, max + padding};
    viewRange = dataRange;
}

void VectorPlot::updateHoverPoint(const QPoint& pos) {
    int index = pointToDataIndex(pos);
    if (index != hoverIndex) {
        setHoverPoint(index);
    }
}

QString VectorPlot::formatValue(double value) const {
    if (std::abs(value) < 0.001 || std::abs(value) > 10000) {
        return QString::number(value, 'e', 3);
    }
    return QString::number(value, 'f', 4);
}

QString VectorPlot::formatIndex(int index) const {
    return QString::number(index);
}

// Slot implementations
void VectorPlot::onZoomIn() { zoomIn(); }
void VectorPlot::onZoomOut() { zoomOut(); }
void VectorPlot::onZoomToFit() { zoomToFit(); }
void VectorPlot::onCopyData() { copyData(); }
void VectorPlot::onSaveCSV() {
    QString path = QFileDialog::getSaveFileName(this, "Save CSV", "", "CSV Files (*.csv)");
    if (!path.isEmpty()) saveAsCSV(path);
}
void VectorPlot::onSaveImage() {
    QString path = QFileDialog::getSaveFileName(this, "Save Image", "", "PNG Files (*.png)");
    if (!path.isEmpty()) saveAsImage(path);
}
void VectorPlot::onFindPeaks() { findPeaks(); }
void VectorPlot::onFindValleys() { findValleys(); }
void VectorPlot::onClearSelection() { clearSelection(); }

} // namespace proxima