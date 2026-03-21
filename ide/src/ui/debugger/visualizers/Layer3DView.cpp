#include "Layer3DView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QToolTip>
#include <cmath>

namespace proxima {

Layer3DView::Layer3DView(QWidget *parent)
    : QWidget(parent)
    , currentSlice(0)
    , sliceAxis(2)
    , viewMode(1)
    , autoOpacity(true)
    , minOpacity(0.1)
    , maxOpacity(1.0)
    , colorMin(0)
    , colorMax(1)
    , selectionActive(false)
    , isSelecting(false)
    , hoverX(-1)
    , hoverY(-1)
    , hoverZ(-1)
    , isRotating(false)
    , isPanning(false)
    , showAxes(true)
    , showGrid(true)
    , showValues(false)
    , showColorBar(true)
    , renderingQuality(1)
    , enableLighting(true)
    , enableShadows(false) {
    
    transform.rotationX = 30;
    transform.rotationY = 45;
    transform.rotationZ = 0;
    transform.zoom = 1.0;
    transform.panX = 0;
    transform.panY = 0;
    
    setupUI();
    setupToolbar();
    setupContextMenu();
    setupSliceControl();
    generatePalette();
}

Layer3DView::~Layer3DView() {}

void Layer3DView::setupUI() {
    setMinimumSize(600, 400);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(toolbar);
    layout->addWidget(sliceControlWidget);
}

void Layer3DView::setupToolbar() {
    toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));
    
    wireframeAction = toolbar->addAction(QIcon(":/icons/wireframe.svg"), "Wireframe");
    wireframeAction->setCheckable(true);
    connect(wireframeAction, &QAction::triggered, this, [this]() {
        viewMode = 0;
        wireframeAction->setChecked(true);
        solidAction->setChecked(false);
        volumeAction->setChecked(false);
        update();
    });
    
    solidAction = toolbar->addAction(QIcon(":/icons/solid.svg"), "Solid");
    solidAction->setCheckable(true);
    solidAction->setChecked(true);
    connect(solidAction, &QAction::triggered, this, [this]() {
        viewMode = 1;
        wireframeAction->setChecked(false);
        solidAction->setChecked(true);
        volumeAction->setChecked(false);
        update();
    });
    
    volumeAction = toolbar->addAction(QIcon(":/icons/volume.svg"), "Volume");
    volumeAction->setCheckable(true);
    connect(volumeAction, &QAction::triggered, this, [this]() {
        viewMode = 2;
        wireframeAction->setChecked(false);
        solidAction->setChecked(false);
        volumeAction->setChecked(true);
        update();
    });
    
    toolbar->addSeparator();
    
    rotateLeftAction = toolbar->addAction(QIcon(":/icons/rotate-left.svg"), "Rotate Left", this, &Layer3DView::onRotateLeft);
    rotateRightAction = toolbar->addAction(QIcon(":/icons/rotate-right.svg"), "Rotate Right", this, &Layer3DView::onRotateRight);
    rotateUpAction = toolbar->addAction(QIcon(":/icons/rotate-up.svg"), "Rotate Up", this, &Layer3DView::onRotateUp);
    rotateDownAction = toolbar->addAction(QIcon(":/icons/rotate-down.svg"), "Rotate Down", this, &Layer3DView::onRotateDown);
    
    toolbar->addSeparator();
    
    zoomInAction = toolbar->addAction(QIcon(":/icons/zoom-in.svg"), "Zoom In", this, &Layer3DView::onZoomIn);
    zoomOutAction = toolbar->addAction(QIcon(":/icons/zoom-out.svg"), "Zoom Out", this, &Layer3DView::onZoomOut);
    resetViewAction = toolbar->addAction(QIcon(":/icons/reset.svg"), "Reset View", this, &Layer3DView::onResetView);
    
    toolbar->addSeparator();
    
    saveSliceAction = toolbar->addAction(QIcon(":/icons/save.svg"), "Save Slice", this, &Layer3DView::onSaveSlice);
    saveVolumeAction = toolbar->addAction(QIcon(":/icons/image.svg"), "Save Volume", this, &Layer3DView::onSaveVolume);
    exportCSVAction = toolbar->addAction(QIcon(":/icons/export.svg"), "Export CSV", this, &Layer3DView::onExportCSV);
    
    toolbar->addSeparator();
    
    copySliceAction = toolbar->addAction(QIcon(":/icons/copy.svg"), "Copy Slice", this, &Layer3DView::onCopySlice);
    copyVolumeAction = toolbar->addAction(QIcon(":/icons/copy-all.svg"), "Copy Volume", this, &Layer3DView::onCopyVolume);
}

void Layer3DView::setupContextMenu() {
    contextMenu = new QMenu(this);
    contextMenu->addAction(wireframeAction);
    contextMenu->addAction(solidAction);
    contextMenu->addAction(volumeAction);
    contextMenu->addSeparator();
    contextMenu->addAction(zoomInAction);
    contextMenu->addAction(zoomOutAction);
    contextMenu->addAction(resetViewAction);
    contextMenu->addSeparator();
    contextMenu->addAction(saveSliceAction);
    contextMenu->addAction(saveVolumeAction);
    contextMenu->addAction(exportCSVAction);
}

void Layer3DView::setupSliceControl() {
    sliceControlWidget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(sliceControlWidget);
    layout->setContentsMargins(5, 5, 5, 5);
    
    QLabel* label = new QLabel("Slice:", this);
    layout->addWidget(label);
    
    sliceSlider = new QSlider(Qt::Horizontal, this);
    sliceSlider->setMinimum(0);
    sliceSlider->setMaximum(0);
    sliceSlider->setTickPosition(QSlider::TicksBelow);
    connect(sliceSlider, &QSlider::valueChanged, this, &Layer3DView::onSliceSliderChanged);
    layout->addWidget(sliceSlider, 1);
    
    sliceLabel = new QLabel("0", this);
    sliceLabel->setMinimumWidth(30);
    layout->addWidget(sliceLabel);
}

void Layer3DView::setData(const QVector<QVector<QVector<double>>>& data) {
    layerData = data;
    
    if (layerData.isEmpty()) return;
    
    // Initialize voxel data
    voxelData.clear();
    for (int z = 0; z < layerData.size(); z++) {
        QVector<QVector<Voxel>> slice;
        for (int y = 0; y < layerData[z].size(); y++) {
            QVector<Voxel> row;
            for (int x = 0; x < layerData[z][y].size(); x++) {
                Voxel voxel;
                voxel.value = layerData[z][y][x];
                voxel.opacity = valueToOpacity(voxel.value);
                voxel.color = valueToColor(voxel.value);
                voxel.isVisible = (voxel.opacity > 0.01);
                row.append(voxel);
            }
            slice.append(row);
        }
        voxelData.append(slice);
    }
    
    // Update slice slider
    sliceSlider->setMaximum(layerData.size() - 1);
    currentSlice = layerData.size() / 2;
    sliceSlider->setValue(currentSlice);
    sliceLabel->setText(QString::number(currentSlice));
    
    calculateStatistics();
    calculateVoxelVisibility();
    update();
    emit dataModified();
}

void Layer3DView::clearData() {
    layerData.clear();
    voxelData.clear();
    selectionActive = false;
    hoverX = hoverY = hoverZ = -1;
    sliceSlider->setMaximum(0);
    sliceLabel->setText("0");
    update();
}

void Layer3DView::setCurrentSlice(int slice) {
    if (slice < 0 || slice >= layerData.size()) return;
    
    currentSlice = slice;
    sliceSlider->setValue(slice);
    sliceLabel->setText(QString::number(slice));
    emit sliceChanged(slice);
    update();
}

void Layer3DView::setSliceAxis(int axis) {
    if (axis < 0 || axis > 2) return;
    sliceAxis = axis;
    update();
}

void Layer3DView::setViewMode(int mode) {
    viewMode = mode;
    update();
}

void Layer3DView::setRotation(double x, double y, double z) {
    transform.rotationX = x;
    transform.rotationY = y;
    transform.rotationZ = z;
    update();
    emit viewChanged();
}

void Layer3DView::setZoom(double zoom) {
    transform.zoom = qBound(0.1, zoom, 10.0);
    update();
    emit viewChanged();
}

void Layer3DView::resetView() {
    transform.rotationX = 30;
    transform.rotationY = 45;
    transform.rotationZ = 0;
    transform.zoom = 1.0;
    transform.panX = 0;
    transform.panY = 0;
    update();
    emit viewChanged();
}

void Layer3DView::setOpacityFunction(const QVector<QPair<double, double>>& function) {
    opacityFunction = function;
    calculateVoxelVisibility();
    update();
}

void Layer3DView::setAutoOpacity(bool enable) {
    autoOpacity = enable;
    calculateVoxelVisibility();
    update();
}

void Layer3DView::setMinOpacity(double min) {
    minOpacity = min;
    calculateVoxelVisibility();
    update();
}

void Layer3DView::setMaxOpacity(double max) {
    maxOpacity = max;
    calculateVoxelVisibility();
    update();
}

double Layer3DView::valueToOpacity(double value) const {
    if (autoOpacity) {
        // Linear mapping based on data range
        if (statistics.isEmpty()) return 0.5;
        double min = statistics["min"];
        double max = statistics["max"];
        if (max == min) return 0.5;
        double normalized = (value - min) / (max - min);
        return minOpacity + normalized * (maxOpacity - minOpacity);
    } else {
        // Use opacity function
        for (int i = 0; i < opacityFunction.size() - 1; i++) {
            if (value >= opacityFunction[i].first && value <= opacityFunction[i+1].first) {
                double t = (value - opacityFunction[i].first) / 
                          (opacityFunction[i+1].first - opacityFunction[i].first);
                return opacityFunction[i].second + t * (opacityFunction[i+1].second - opacityFunction[i].second);
            }
        }
        return 0.5;
    }
}

void Layer3DView::setColorPalette(const QVector<QColor>& colors) {
    paletteColors = colors;
    update();
}

void Layer3DView::setColorRange(double min, double max) {
    colorMin = min;
    colorMax = max;
    update();
}

QColor Layer3DView::valueToColor(double value) const {
    if (paletteColors.isEmpty()) return Qt::gray;
    
    double normalized = 0.5;
    if (colorMax != colorMin) {
        normalized = (value - colorMin) / (colorMax - colorMin);
        normalized = qBound(0.0, normalized, 1.0);
    }
    
    int index = normalized * (paletteColors.size() - 1);
    return paletteColors[index];
}

void Layer3DView::selectVoxel(int x, int y, int z) {
    selectionX1 = selectionX2 = x;
    selectionY1 = selectionY2 = y;
    selectionZ1 = selectionZ2 = z;
    selectionActive = true;
    update();
}

void Layer3DView::selectRange(int x1, int y1, int z1, int x2, int y2, int z2) {
    selectionX1 = qMin(x1, x2);
    selectionX2 = qMax(x1, x2);
    selectionY1 = qMin(y1, y2);
    selectionY2 = qMax(y1, y2);
    selectionZ1 = qMin(z1, z2);
    selectionZ2 = qMax(z1, z2);
    selectionActive = true;
    update();
}

void Layer3DView::clearSelection() {
    selectionActive = false;
    update();
}

QVector<QVector<QVector<double>>> Layer3DView::getSelectedData() const {
    if (!selectionActive) return QVector<QVector<QVector<double>>>();
    
    QVector<QVector<QVector<double>>> selected;
    for (int z = selectionZ1; z <= selectionZ2 && z < layerData.size(); z++) {
        QVector<QVector<double>> slice;
        for (int y = selectionY1; y <= selectionY2 && y < layerData[z].size(); y++) {
            QVector<double> row;
            for (int x = selectionX1; x <= selectionX2 && x < layerData[z][y].size(); x++) {
                row.append(layerData[z][y][x]);
            }
            slice.append(row);
        }
        selected.append(slice);
    }
    return selected;
}

void Layer3DView::saveSliceAsImage(const QString& path, int slice) {
    QPixmap pixmap(size());
    render(&pixmap);
    pixmap.save(path);
}

void Layer3DView::saveVolumeAsImage(const QString& path) {
    // Save current view as image
    QPixmap pixmap(size());
    render(&pixmap);
    pixmap.save(path);
}

void Layer3DView::exportDataAsCSV(const QString& path) {
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "x,y,z,value\n";
        for (int z = 0; z < layerData.size(); z++) {
            for (int y = 0; y < layerData[z].size(); y++) {
                for (int x = 0; x < layerData[z][y].size(); x++) {
                    out << x << "," << y << "," << z << "," 
                        << layerData[z][y][x] << "\n";
                }
            }
        }
        file.close();
    }
}

void Layer3DView::copySliceData(int slice) {
    if (slice < 0 || slice >= layerData.size()) return;
    
    QString csvData;
    for (int y = 0; y < layerData[slice].size(); y++) {
        for (int x = 0; x < layerData[slice][y].size(); x++) {
            csvData += QString::number(layerData[slice][y][x]);
            if (x < layerData[slice][y].size() - 1) csvData += ",";
        }
        csvData += "\n";
    }
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(csvData);
}

void Layer3DView::copyVolumeData() {
    QString csvData = "x,y,z,value\n";
    for (int z = 0; z < layerData.size(); z++) {
        for (int y = 0; y < layerData[z].size(); y++) {
            for (int x = 0; x < layerData[z][y].size(); x++) {
                csvData += QString("%1,%2,%3,%4\n")
                    .arg(x).arg(y).arg(z).arg(layerData[z][y][x]);
            }
        }
    }
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(csvData);
}

void Layer3DView::setShowAxes(bool show) {
    showAxes = show;
    update();
}

void Layer3DView::setShowGrid(bool show) {
    showGrid = show;
    update();
}

void Layer3DView::setShowValues(bool show) {
    showValues = show;
    update();
}

void Layer3DView::setShowColorBar(bool show) {
    showColorBar = show;
    update();
}

void Layer3DView::setRenderingQuality(int quality) {
    renderingQuality = qBound(0, quality, 2);
    update();
}

void Layer3DView::enableLighting(bool enable) {
    enableLighting = enable;
    update();
}

void Layer3DView::enableShadows(bool enable) {
    enableShadows = enable;
    update();
}

void Layer3DView::calculateStatistics() {
    statistics.clear();
    
    if (layerData.isEmpty()) return;
    
    double sum = 0;
    double min = layerData[0][0][0];
    double max = layerData[0][0][0];
    int count = 0;
    
    for (int z = 0; z < layerData.size(); z++) {
        for (int y = 0; y < layerData[z].size(); y++) {
            for (int x = 0; x < layerData[z][y].size(); x++) {
                double value = layerData[z][y][x];
                sum += value;
                if (value < min) min = value;
                if (value > max) max = value;
                count++;
            }
        }
    }
    
    statistics["count"] = count;
    statistics["sum"] = sum;
    statistics["mean"] = sum / count;
    statistics["min"] = min;
    statistics["max"] = max;
    statistics["range"] = max - min;
    
    // Update color range
    colorMin = min;
    colorMax = max;
}

void Layer3DView::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Background
    painter.fillRect(rect(), QColor(20, 20, 30));
    
    // Apply transform
    applyTransform(painter);
    
    // Draw based on view mode
    switch (viewMode) {
        case 0:
            drawWireframe(painter);
            break;
        case 1:
            drawSlice(painter);
            break;
        case 2:
            drawVolume(painter);
            break;
    }
    
    // Draw axes
    if (showAxes) {
        drawAxes(painter);
    }
    
    // Draw grid
    if (showGrid) {
        drawGrid(painter);
    }
    
    // Draw selection
    if (selectionActive) {
        drawSelection(painter);
    }
    
    // Draw hover voxel
    if (hoverX >= 0) {
        drawHoverVoxel(painter);
    }
    
    // Draw color bar
    if (showColorBar) {
        drawColorBar(painter);
    }
}

void Layer3DView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (event->modifiers() & Qt::ControlModifier) {
            // Start rotation
            isRotating = true;
            lastRotatePos = event->pos();
        } else if (event->modifiers() & Qt::ShiftModifier) {
            // Start selection
            isSelecting = true;
            selectionStartPos = event->pos();
            unprojectPointToPoint(event->pos(), selectionX1, selectionY1, selectionZ1);
        } else {
            // Start panning
            isPanning = true;
            lastPanPos = event->pos();
        }
    }
}

void Layer3DView::mouseMoveEvent(QMouseEvent *event) {
    if (isRotating) {
        QPoint delta = event->pos() - lastRotatePos;
        transform.rotationY += delta.x() * 0.5;
        transform.rotationX += delta.y() * 0.5;
        lastRotatePos = event->pos();
        update();
        emit viewChanged();
    }
    
    if (isPanning) {
        QPoint delta = event->pos() - lastPanPos;
        transform.panX += delta.x();
        transform.panY += delta.y();
        lastPanPos = event->pos();
        update();
        emit viewChanged();
    }
    
    if (isSelecting) {
        int x2, y2, z2;
        unprojectPointToPoint(event->pos(), x2, y2, z2);
        update();
    }
    
    // Update hover voxel
    updateHoverVoxel(event->pos());
}

void Layer3DView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (isSelecting) {
            isSelecting = false;
            int x2, y2, z2;
            unprojectPointToPoint(event->pos(), x2, y2, z2);
            selectRange(selectionX1, selectionY1, selectionZ1, x2, y2, z2);
        }
        isRotating = false;
        isPanning = false;
    }
}

void Layer3DView::wheelEvent(QWheelEvent *event) {
    if (event->angleDelta().y() > 0) {
        setZoom(transform.zoom * 1.1);
    } else {
        setZoom(transform.zoom / 1.1);
    }
}

void Layer3DView::contextMenuEvent(QContextMenuEvent *event) {
    contextMenu->exec(event->globalPos());
}

void Layer3DView::applyTransform(QPainter& painter) {
    painter.translate(width() / 2 + transform.panX, height() / 2 + transform.panY);
    painter.scale(transform.zoom, transform.zoom);
    painter.rotate(transform.rotationX, 1, 0, 0);
    painter.rotate(transform.rotationY, 0, 1, 0);
    painter.rotate(transform.rotationZ, 0, 0, 1);
}

void Layer3DView::drawSlice(QPainter& painter) {
    if (layerData.isEmpty()) return;
    
    int slice = currentSlice;
    if (sliceAxis == 0) slice = hoverX >= 0 ? hoverX : currentSlice;
    else if (sliceAxis == 1) slice = hoverY >= 0 ? hoverY : currentSlice;
    
    // Draw slice based on axis
    for (int y = 0; y < layerData[slice].size(); y++) {
        for (int x = 0; x < layerData[slice][y].size(); x++) {
            double value = layerData[slice][y][x];
            QColor color = valueToColor(value);
            painter.fillRect(x * 10, y * 10, 10, 10, color);
            
            if (showValues) {
                painter.setPen(QColor(255, 255, 255));
                painter.drawText(QRect(x * 10, y * 10, 10, 10), 
                               Qt::AlignCenter, QString::number(value, 'f', 2));
            }
        }
    }
}

void Layer3DView::drawVolume(QPainter& painter) {
    // Simplified volume rendering
    // Full implementation would use ray casting
    
    // Draw visible voxels
    for (int z = 0; z < voxelData.size(); z++) {
        for (int y = 0; y < voxelData[z].size(); y++) {
            for (int x = 0; x < voxelData[z][y].size(); x++) {
                if (!voxelData[z][y][x].isVisible) continue;
                
                QPoint point;
                project3DToPoint(x, y, z, point);
                
                QColor color = voxelData[z][y][x].color;
                color.setAlphaF(voxelData[z][y][x].opacity);
                
                painter.fillRect(point.x(), point.y(), 8, 8, color);
            }
        }
    }
}

void Layer3DView::drawWireframe(QPainter& painter) {
    // Draw wireframe box
    painter.setPen(QColor(100, 100, 100));
    
    int w = layerData.isEmpty() ? 100 : layerData[0][0].size() * 10;
    int h = layerData.isEmpty() ? 100 : layerData[0].size() * 10;
    int d = layerData.isEmpty() ? 100 : layerData.size() * 10;
    
    // Front face
    painter.drawRect(0, 0, w, h);
    
    // Back face (offset)
    int offset = 50;
    painter.drawRect(offset, -offset, w, h);
    
    // Connecting lines
    painter.drawLine(0, 0, offset, -offset);
    painter.drawLine(w, 0, w + offset, -offset);
    painter.drawLine(0, h, offset, h - offset);
    painter.drawLine(w, h, w + offset, h - offset);
}

void Layer3DView::drawAxes(QPainter& painter) {
    painter.setPen(QPen(QColor(200, 200, 200), 2));
    
    int axisLength = 100;
    
    // X axis (red)
    painter.setPen(QColor(255, 100, 100));
    painter.drawLine(0, 0, axisLength, 0);
    painter.drawText(axisLength + 5, 0, "X");
    
    // Y axis (green)
    painter.setPen(QColor(100, 255, 100));
    painter.drawLine(0, 0, 0, -axisLength);
    painter.drawText(5, -axisLength - 5, "Y");
    
    // Z axis (blue)
    painter.setPen(QColor(100, 100, 255));
    painter.drawLine(0, 0, 50, -50);
    painter.drawText(55, -55, "Z");
}

void Layer3DView::drawColorBar(QPainter& painter) {
    int barWidth = 20;
    int barHeight = 200;
    int x = width() - barWidth - 20;
    int y = (height() - barHeight) / 2;
    
    // Draw gradient
    for (int i = 0; i < barHeight; i++) {
        double t = 1.0 - (double)i / barHeight;
        QColor color = valueToColor(colorMin + t * (colorMax - colorMin));
        painter.fillRect(x, y + i, barWidth, 1, color);
    }
    
    // Draw border
    painter.setPen(QColor(200, 200, 200));
    painter.drawRect(x, y, barWidth, barHeight);
    
    // Draw labels
    painter.setPen(QColor(200, 200, 200));
    painter.drawText(x + barWidth + 5, y + 15, QString::number(colorMax, 'f', 2));
    painter.drawText(x + barWidth + 5, y + barHeight - 5, QString::number(colorMin, 'f', 2));
}

void Layer3DView::project3DToPoint(double x, double y, double z, QPoint& point) const {
    // Simple orthographic projection
    point.setX(x * 10 + z * 5);
    point.setY(-y * 10 + z * 5);
}

void Layer3DView::unprojectPointToPoint(const QPoint& point, int& x, int& y, int& z) const {
    // Simplified unprojection
    x = point.x() / 10;
    y = -point.y() / 10;
    z = currentSlice;
    
    x = qBound(0, x, cols() - 1);
    y = qBound(0, y, rows() - 1);
    z = qBound(0, z, depth() - 1);
}

void Layer3DView::updateHoverVoxel(const QPoint& pos) {
    int x, y, z;
    unprojectPointToPoint(pos, x, y, z);
    
    if (x != hoverX || y != hoverY || z != hoverZ) {
        hoverX = x;
        hoverY = y;
        hoverZ = z;
        
        if (hoverX >= 0 && hoverX < cols() &&
            hoverY >= 0 && hoverY < rows() &&
            hoverZ >= 0 && hoverZ < depth()) {
            double value = layerData[hoverZ][hoverY][hoverX];
            emit voxelHovered(hoverX, hoverY, hoverZ, value);
            
            QString tooltip = QString("X: %1\nY: %2\nZ: %3\nValue: %4")
                .arg(hoverX).arg(hoverY).arg(hoverZ).arg(formatValue(value));
            QToolTip::showText(QCursor::pos(), tooltip, this);
        }
        
        update();
    }
}

void Layer3DView::calculateVoxelVisibility() {
    for (int z = 0; z < voxelData.size(); z++) {
        for (int y = 0; y < voxelData[z].size(); y++) {
            for (int x = 0; x < voxelData[z][y].size(); x++) {
                double opacity = valueToOpacity(voxelData[z][y][x].value);
                voxelData[z][y][x].opacity = opacity;
                voxelData[z][y][x].isVisible = (opacity > 0.01);
            }
        }
    }
}

void Layer3DView::generatePalette() {
    // Viridis-like palette
    paletteColors.clear();
    paletteColors.append(QColor(68, 1, 84));
    paletteColors.append(QColor(72, 40, 120));
    paletteColors.append(QColor(62, 74, 137));
    paletteColors.append(QColor(49, 104, 142));
    paletteColors.append(QColor(38, 130, 142));
    paletteColors.append(QColor(31, 158, 137));
    paletteColors.append(QColor(53, 183, 121));
    paletteColors.append(QColor(109, 205, 89));
    paletteColors.append(QColor(180, 222, 44));
    paletteColors.append(QColor(253, 231, 37));
    
    colorMin = 0;
    colorMax = 1;
}

QString Layer3DView::formatValue(double value) const {
    if (std::abs(value) < 0.001 || std::abs(value) > 10000) {
        return QString::number(value, 'e', 3);
    }
    return QString::number(value, 'f', 4);
}

QString Layer3DView::formatCoordinate(int x, int y, int z) const {
    return QString("(%1, %2, %3)").arg(x).arg(y).arg(z);
}

// Slot implementations
void Layer3DView::onSliceSliderChanged(int value) {
    setCurrentSlice(value);
}

void Layer3DView::onViewModeChanged(int index) {
    setViewMode(index);
}

void Layer3DView::onZoomIn() { setZoom(transform.zoom * 1.2); }
void Layer3DView::onZoomOut() { setZoom(transform.zoom / 1.2); }
void Layer3DView::onResetView() { resetView(); }

void Layer3DView::onSaveSlice() {
    QString path = QFileDialog::getSaveFileName(this, "Save Slice", "", "PNG Files (*.png)");
    if (!path.isEmpty()) saveSliceAsImage(path, currentSlice);
}

void Layer3DView::onSaveVolume() {
    QString path = QFileDialog::getSaveFileName(this, "Save Volume", "", "PNG Files (*.png)");
    if (!path.isEmpty()) saveVolumeAsImage(path);
}

void Layer3DView::onExportCSV() {
    QString path = QFileDialog::getSaveFileName(this, "Export CSV", "", "CSV Files (*.csv)");
    if (!path.isEmpty()) exportDataAsCSV(path);
}

void Layer3DView::onCopySlice() { copySliceData(currentSlice); }
void Layer3DView::onCopyVolume() { copyVolumeData(); }

void Layer3DView::onRotateLeft() { setRotation(transform.rotationX, transform.rotationY - 15, transform.rotationZ); }
void Layer3DView::onRotateRight() { setRotation(transform.rotationX, transform.rotationY + 15, transform.rotationZ); }
void Layer3DView::onRotateUp() { setRotation(transform.rotationX - 15, transform.rotationY, transform.rotationZ); }
void Layer3DView::onRotateDown() { setRotation(transform.rotationX + 15, transform.rotationY, transform.rotationZ); }

} // namespace proxima