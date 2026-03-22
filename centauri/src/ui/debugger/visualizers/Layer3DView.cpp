#include "Layer3DView.h"
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QBuffer>
#include <QByteArray>
#include <QRegularExpression>
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
#include <QReadWriteLock>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QDataStream>
#include <QDebug>
#include <QElapsedTimer>
#include <QSet>
#include <QHash>
#include <QMetaType>
#include <QMetaObject>
#include <QPainterPath>
#include <QRegion>
#include <QBitmap>
#include <QMatrix>
#include <QMatrix4x4>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QQuaternion>
#include <QEasingCurve>
#include <QtMath>
#include <algorithm>
#include <cmath>

#ifdef USE_CUDA
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#endif

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

Layer3DView::Layer3DView(QWidget *parent)
    : QOpenGLWidget(parent)
    , sizeX_(0)
    , sizeY_(0)
    , sizeZ_(0)
    , totalVoxels(0)
    , visibleVoxelCount(0)
    , maxVisibleVoxels(100000)
    , renderMode(RenderMode::GPU_OpenGL)
    , viewMode(LayerViewMode::Volume)
    , gpuAvailable(false)
    , highPerformanceMode(false)
    , showSlice(false)
    , opacityThreshold(0.1)
    , currentPalette(ColorPalette::Viridis)
    , isRotating(false)
    , isPanning(false)
    , selectedVoxel(0, 0, 0)
    , hoverX(-1)
    , hoverY(-1)
    , hoverZ(-1)
    , isHovering(false)
    , minValue(0.0)
    , maxValue(1.0)
    , meanValue(0.0)
    , currentFPS(0)
    , frameCount(0)
    , shaderProgram(nullptr)
    , volumeTexture(nullptr)
    , fbo(nullptr)
    #ifdef USE_CUDA
    , cudaResource(nullptr)
    , cudaAvailable(false)
    #endif
{
    setupUI();
    setupContextMenu();
    setupToolbar();
    
    // Таймер для подсчёта FPS
    fpsTimer = new QTimer(this);
    connect(fpsTimer, &QTimer::timeout, this, &Layer3DView::updateFPS);
    fpsTimer->start(1000);
    
    LOG_DEBUG("Layer3DView created");
}

Layer3DView::~Layer3DView() {
    cleanupOpenGL();
    LOG_DEBUG("Layer3DView destroyed");
}

// ============================================================================
// Инициализация OpenGL
// ============================================================================

void Layer3DView::initializeGL() {
    initializeOpenGLFunctions();
    
    // Проверка доступности GPU
    gpuInfo = QString(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    gpuAvailable = true;
    
    LOG_INFO("GPU: " + gpuInfo.toStdString());
    
    // Инициализация шейдеров
    initShaders();
    
    // Инициализация буферов
    initBuffers();
    
    // Настройка OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
    
    // Инициализация CUDA (если доступно)
    #ifdef USE_CUDA
    cudaGLSetGLDevice(0);
    cudaAvailable = true;
    LOG_INFO("CUDA available for rendering");
    #endif
    
    updateCamera();
    generatePalette();
    
    LOG_INFO("OpenGL initialized successfully");
}

void Layer3DView::initShaders() {
    // Volume rendering shader
    shaderProgram = new QOpenGLShaderProgram(this);
    
    // Vertex shader
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color;
        layout(location = 2) in float opacity;
        
        out vec3 fragColor;
        out float fragOpacity;
        out vec3 fragPos;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            fragPos = position;
            fragColor = color;
            fragOpacity = opacity;
            gl_Position = projection * view * model * vec4(position, 1.0);
        }
    )";
    
    // Fragment shader
    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec3 fragColor;
        in float fragOpacity;
        in vec3 fragPos;
        
        out vec4 outColor;
        
        void main() {
            outColor = vec4(fragColor, fragOpacity);
        }
    )";
    
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    shaderProgram->link();
    
    LOG_INFO("Shaders compiled successfully");
}

void Layer3DView::initBuffers() {
    vertexBuffer.create();
    indexBuffer.create();
    vao.create();
    
    vao.bind();
    vertexBuffer.bind();
    indexBuffer.bind();
    vao.release();
    
    LOG_INFO("Buffers initialized");
}

void Layer3DView::initTextures() {
    if (volumeTexture) {
        delete volumeTexture;
    }
    
    volumeTexture = new QOpenGLTexture(QOpenGLTexture::Target3D);
    volumeTexture->setFormat(QOpenGLTexture::RGBA32F);
    volumeTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
    volumeTexture->setMinificationFilter(QOpenGLTexture::Linear);
    volumeTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    
    LOG_INFO("3D texture initialized");
}

void Layer3DView::cleanupOpenGL() {
    if (shaderProgram) {
        delete shaderProgram;
        shaderProgram = nullptr;
    }
    
    if (volumeTexture) {
        delete volumeTexture;
        volumeTexture = nullptr;
    }
    
    if (fbo) {
        delete fbo;
        fbo = nullptr;
    }
    
    #ifdef USE_CUDA
    if (cudaResource) {
        cudaGraphicsUnregisterResource(cudaResource);
        cudaResource = nullptr;
    }
    #endif
    
    LOG_INFO("OpenGL resources cleaned up");
}

// ============================================================================
// Данные слоя
// ============================================================================

void Layer3DView::setData(const QVector<QVector<QVector<double>>>& data) {
    layerData = data;
    
    if (data.isEmpty()) {
        sizeX_ = 0;
        sizeY_ = 0;
        sizeZ_ = 0;
        totalVoxels = 0;
        return;
    }
    
    sizeZ_ = data.size();
    sizeY_ = data[0].size();
    sizeX_ = data[0][0].size();
    totalVoxels = sizeX_ * sizeY_ * sizeZ_;
    
    // Инициализация вокселей (1-based индексация)
    voxels.clear();
    for (int z = 0; z < sizeZ_; z++) {
        for (int y = 0; y < sizeY_; y++) {
            for (int x = 0; x < sizeX_; x++) {
                Voxel voxel;
                voxel.x = x + 1;  // 1-based
                voxel.y = y + 1;  // 1-based
                voxel.z = z + 1;  // 1-based
                voxel.value = data[z][y][x];
                voxels.append(voxel);
            }
        }
    }
    
    calculateStatistics();
    generatePalette();
    updateVoxelVisibility();
    updateSliceData();
    
    // Обновление текстуры
    if (volumeTexture && gpuAvailable) {
        initTextures();
    }
    
    update();
    
    LOG_INFO("Layer data set: " + QString::number(sizeX_) + "x" + 
             QString::number(sizeY_) + "x" + QString::number(sizeZ_) + 
             " (" + QString::number(totalVoxels) + " voxels)");
}

void Layer3DView::setDataFromRuntime(const RuntimeValue& value) {
    if (value.type != RuntimeValue::Type::Array) {
        LOG_WARNING("Invalid runtime value type for layer");
        return;
    }
    
    QVector<QVector<QVector<double>>> data;
    
    for (const auto& z : value.arrayValue) {
        QVector<QVector<double>> layer;
        for (const auto& y : z.arrayValue) {
            QVector<double> row;
            for (const auto& x : y.arrayValue) {
                row.append(x.numberValue);
            }
            layer.append(row);
        }
        data.append(layer);
    }
    
    setData(data);
}

double Layer3DView::getVoxelValue(int x, int y, int z) const {
    // Проверка границ (1-based индексация)
    if (x < 1 || x > sizeX_ || y < 1 || y > sizeY_ || z < 1 || z > sizeZ_) {
        return 0.0;
    }
    
    // Конвертация в 0-based для внутреннего хранения
    return layerData[z - 1][y - 1][x - 1];
}

void Layer3DView::setVoxelValue(int x, int y, int z, double value) {
    if (!isEditable()) return;
    
    // Проверка границ (1-based индексация)
    if (x < 1 || x > sizeX_ || y < 1 || y > sizeY_ || z < 1 || z > sizeZ_) {
        return;
    }
    
    // Конвертация в 0-based для внутреннего хранения
    layerData[z - 1][y - 1][x - 1] = value;
    
    // Обновление вокселя
    for (auto& voxel : voxels) {
        if (voxel.x == x && voxel.y == y && voxel.z == z) {
            voxel.value = value;
            break;
        }
    }
    
    calculateStatistics();
    update();
    
    emit dataModified();
}

// ============================================================================
// Режимы рендеринга
// ============================================================================

void Layer3DView::setRenderMode(RenderMode mode) {
    if (renderMode != mode) {
        renderMode = mode;
        update();
        emit renderModeChanged(mode);
        
        LOG_INFO("Render mode changed to: " + QString::number(static_cast<int>(mode)));
    }
}

// ============================================================================
// Режимы отображения
// ============================================================================

void Layer3DView::setViewMode(LayerViewMode mode) {
    if (viewMode != mode) {
        viewMode = mode;
        update();
        
        LOG_INFO("View mode changed to: " + QString::number(static_cast<int>(mode)));
    }
}

// ============================================================================
// Срезы
// ============================================================================

void Layer3DView::setCurrentSlice(int plane, int position) {
    currentSlice.plane = plane;
    currentSlice.position = qBound(1, position, plane == 0 ? sizeZ_ : (plane == 1 ? sizeY_ : sizeX_));
    
    updateSliceData();
    update();
    
    emit sliceChanged(plane, position);
}

void Layer3DView::setShowSlice(bool show) {
    showSlice = show;
    showSliceAction->setChecked(show);
    update();
}

void Layer3DView::updateSliceData() {
    // Обновление данных текущего среза
    currentSlice.data.clear();
    
    switch (currentSlice.plane) {
        case 0: // XY
            if (currentSlice.position - 1 < sizeZ_) {
                currentSlice.data = layerData[currentSlice.position - 1];
            }
            break;
        case 1: // XZ
            for (int z = 0; z < sizeZ_; z++) {
                QVector<double> row;
                for (int x = 0; x < sizeX_; x++) {
                    row.append(layerData[z][currentSlice.position - 1][x]);
                }
                currentSlice.data.append(row);
            }
            break;
        case 2: // YZ
            for (int z = 0; z < sizeZ_; z++) {
                QVector<double> row;
                for (int y = 0; y < sizeY_; y++) {
                    row.append(layerData[z][y][currentSlice.position - 1]);
                }
                currentSlice.data.append(row);
            }
            break;
    }
}

// ============================================================================
// Прозрачность и цвет
// ============================================================================

void Layer3DView::setOpacityThreshold(double threshold) {
    opacityThreshold = qBound(0.0, threshold, 1.0);
    updateVoxelVisibility();
    update();
}

void Layer3DView::setColorPalette(ColorPalette palette) {
    if (currentPalette != palette) {
        currentPalette = palette;
        generatePalette();
        update();
    }
}

void Layer3DView::generatePalette() {
    const int paletteSize = 256;
    
    // Генерация палитры (аналогично MatrixView)
    switch (currentPalette) {
        case ColorPalette::Viridis:
            paletteColors.clear();
            for (int i = 0; i < paletteSize; i++) {
                double t = static_cast<double>(i) / (paletteSize - 1);
                int r = static_cast<int>(68 + t * 187);
                int g = static_cast<int>(1 + t * 200);
                int b = static_cast<int>(84 + t * 92);
                paletteColors.append(QColor(r, g, b));
            }
            break;
        // ... другие палитры ...
        default:
            break;
    }
}

QColor Layer3DView::valueToColor(double value) const {
    if (paletteColors.isEmpty()) {
        return Qt::gray;
    }
    
    double normalized = (maxValue != minValue) ? 
                       (value - minValue) / (maxValue - minValue) : 0.5;
    normalized = qBound(0.0, normalized, 1.0);
    
    int index = static_cast<int>(normalized * (paletteColors.size() - 1));
    index = qBound(0, index, paletteColors.size() - 1);
    
    return paletteColors[index];
}

float Layer3DView::valueToOpacity(double value) const {
    double normalized = (maxValue != minValue) ? 
                       (value - minValue) / (maxValue - minValue) : 0.5;
    
    if (normalized < opacityThreshold) {
        return 0.0f;
    }
    
    return static_cast<float>(normalized);
}

void Layer3DView::updateVoxelVisibility() {
    visibleVoxelCount = 0;
    
    for (auto& voxel : voxels) {
        voxel.isVisible = (valueToOpacity(voxel.value) > 0.0f);
        if (voxel.isVisible) {
            visibleVoxelCount++;
        }
    }
    
    LOG_DEBUG("Visible voxels: " + QString::number(visibleVoxelCount) + 
              " / " + QString::number(totalVoxels));
}

// ============================================================================
// Камера и навигация
// ============================================================================

void Layer3DView::setCameraPosition(const QVector3D& position) {
    camera.position = position;
    updateCamera();
    update();
    emit cameraChanged();
}

void Layer3DView::rotateCamera(int dx, int dy) {
    float yaw = dx * 0.5f;
    float pitch = dy * 0.5f;
    
    QMatrix4x4 rotation;
    rotation.rotate(yaw, 0, 1, 0);
    rotation.rotate(pitch, 1, 0, 0);
    
    QVector3D direction = camera.position - camera.target;
    direction = rotation * direction;
    
    camera.position = camera.target + direction;
    updateCamera();
    update();
}

void Layer3DView::zoomCamera(float delta) {
    QVector3D direction = camera.position - camera.target;
    float length = direction.length();
    
    length += delta * 0.1f;
    length = qMax(1.0f, qMin(length, 50.0f));
    
    direction.normalize();
    camera.position = camera.target + direction * length;
    
    updateCamera();
    update();
}

void Layer3DView::resetCamera() {
    camera = Camera();
    camera.position = QVector3D(sizeX_/2.0f, sizeY_/2.0f, sizeZ_ * 2.0f);
    camera.target = QVector3D(sizeX_/2.0f, sizeY_/2.0f, sizeZ_/2.0f);
    updateCamera();
    update();
}

void Layer3DView::updateCamera() {
    camera.aspect = static_cast<float>(width()) / static_cast<float>(height());
}

QMatrix4x4 Layer3DView::getViewMatrix() const {
    QMatrix4x4 view;
    view.lookAt(camera.position, camera.target, camera.up);
    return view;
}

QMatrix4x4 Layer3DView::getProjectionMatrix() const {
    QMatrix4x4 projection;
    projection.perspective(camera.fov, camera.aspect, camera.nearPlane, camera.farPlane);
    return projection;
}

QMatrix4x4 Layer3DView::getModelMatrix() const {
    QMatrix4x4 model;
    model.translate(-sizeX_/2.0f, -sizeY_/2.0f, -sizeZ_/2.0f);
    return model;
}

// ============================================================================
// Выделение
// ============================================================================

void Layer3DView::selectVoxel(int x, int y, int z) {
    selectedVoxel = QVector3D(x, y, z);
    
    for (auto& voxel : voxels) {
        voxel.isSelected = (voxel.x == x && voxel.y == y && voxel.z == z);
    }
    
    update();
    emit selectionChanged(x, y, z);
}

void Layer3DView::clearSelection() {
    selectedVoxel = QVector3D(0, 0, 0);
    
    for (auto& voxel : voxels) {
        voxel.isSelected = false;
    }
    
    update();
    emit selectionChanged(0, 0, 0);
}

// ============================================================================
// Статистика
// ============================================================================

void Layer3DView::calculateStatistics() {
    if (voxels.isEmpty()) {
        minValue = 0.0;
        maxValue = 0.0;
        meanValue = 0.0;
        return;
    }
    
    minValue = voxels[0].value;
    maxValue = voxels[0].value;
    double sum = 0.0;
    
    for (const auto& voxel : voxels) {
        if (voxel.value < minValue) minValue = voxel.value;
        if (voxel.value > maxValue) maxValue = voxel.value;
        sum += voxel.value;
    }
    
    meanValue = sum / voxels.size();
}

int Layer3DView::getVisibleVoxelCount() const {
    return visibleVoxelCount;
}

int Layer3DView::getFPS() const {
    return currentFPS;
}

// ============================================================================
// Рендеринг
// ============================================================================

void Layer3DView::paintGL() {
    QElapsedTimer timer;
    timer.start();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (shaderProgram && gpuAvailable) {
        renderGPUBased();
    } else {
        renderCPUBased();
    }
    
    frameCount++;
    
    // Отрисовка информации
    if (highPerformanceMode) {
        renderWireframe();
    }
    
    LOG_DEBUG("Frame rendered in " + QString::number(timer.elapsed()) + "ms");
}

void Layer3DView::renderCPUBased() {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // CPU-based rendering для систем без GPU
    switch (viewMode) {
        case LayerViewMode::Volume:
            // Упрощённое объёмное отображение
            break;
        case LayerViewMode::Slice_XY:
        case LayerViewMode::Slice_XZ:
        case LayerViewMode::Slice_YZ:
            renderSlice();
            break;
        case LayerViewMode::Wireframe:
            renderWireframe();
            break;
        case LayerViewMode::Points:
            renderPoints();
            break;
        case LayerViewMode::Surfaces:
            renderSurfaces();
            break;
    }
    
    painter.end();
}

void Layer3DView::renderGPUBased() {
    shaderProgram->bind();
    
    shaderProgram->setUniformValue("model", getModelMatrix());
    shaderProgram->setUniformValue("view", getViewMatrix());
    shaderProgram->setUniformValue("projection", getProjectionMatrix());
    
    vao.bind();
    
    switch (viewMode) {
        case LayerViewMode::Volume:
            renderVolume();
            break;
        case LayerViewMode::Slice_XY:
        case LayerViewMode::Slice_XZ:
        case LayerViewMode::Slice_YZ:
            renderSlice();
            break;
        case LayerViewMode::Wireframe:
            renderWireframe();
            break;
        case LayerViewMode::Points:
            renderPoints();
            break;
        case LayerViewMode::Surfaces:
            renderSurfaces();
            break;
    }
    
    vao.release();
    shaderProgram->release();
}

void Layer3DView::renderVolume() {
    // Volume rendering через ray casting
    // В полной реализации - GPU ray casting shader
}

void Layer3DView::renderSlice() {
    // Отрисовка среза
    if (currentSlice.data.isEmpty()) return;
    
    QPainter painter(this);
    
    int sliceWidth = width() - 100;
    int sliceHeight = height() - 100;
    
    int dataWidth = currentSlice.plane == 2 ? sizeY_ : sizeX_;
    int dataHeight = currentSlice.plane == 1 ? sizeZ_ : 
                    (currentSlice.plane == 2 ? sizeZ_ : sizeY_);
    
    int cellWidth = sliceWidth / dataWidth;
    int cellHeight = sliceHeight / dataHeight;
    
    for (int y = 0; y < dataHeight; y++) {
        for (int x = 0; x < dataWidth; x++) {
            double value = currentSlice.data[y][x];
            QColor color = valueToColor(value);
            
            int screenX = 50 + x * cellWidth;
            int screenY = 50 + y * cellHeight;
            
            painter.fillRect(screenX, screenY, cellWidth, cellHeight, color);
        }
    }
    
    painter.end();
}

void Layer3DView::renderWireframe() {
    QPainter painter(this);
    painter.setPen(QPen(QColor(100, 100, 100), 1));
    
    // Отрисовка каркаса
    int boxSize = qMin({width(), height()}) / 3;
    int centerX = width() / 2;
    int centerY = height() / 2;
    
    painter.drawRect(centerX - boxSize/2, centerY - boxSize/2, boxSize, boxSize);
    
    painter.end();
}

void Layer3DView::renderPoints() {
    // Отрисовка точек
}

void Layer3DView::renderSurfaces() {
    // Отрисовка поверхностей (marching cubes)
}

// ============================================================================
// События мыши
// ============================================================================

void Layer3DView::mousePressEvent(QMouseEvent *event) {
    if (!interactive) {
        QOpenGLWidget::mousePressEvent(event);
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        isRotating = true;
        lastMousePos = event->pos();
    } else if (event->button() == Qt::MiddleButton) {
        isPanning = true;
        lastMousePos = event->pos();
    } else if (event->button() == Qt::RightButton) {
        // Выделение вокселя
        int x, y, z;
        if (pickVoxel(event->pos(), x, y, z)) {
            selectVoxel(x, y, z);
            emit voxelClicked(x, y, z, getVoxelValue(x, y, z));
        }
    }
    
    QOpenGLWidget::mousePressEvent(event);
}

void Layer3DView::mouseMoveEvent(QMouseEvent *event) {
    if (!interactive) {
        QOpenGLWidget::mouseMoveEvent(event);
        return;
    }
    
    if (isRotating) {
        QPoint delta = event->pos() - lastMousePos;
        rotateCamera(delta.x(), delta.y());
        lastMousePos = event->pos();
    } else if (isPanning) {
        QPoint delta = event->pos() - lastMousePos;
        camera.target += QVector3D(delta.x() * 0.01f, -delta.y() * 0.01f, 0);
        camera.position += QVector3D(delta.x() * 0.01f, -delta.y() * 0.01f, 0);
        lastMousePos = event->pos();
        update();
    } else {
        // Наведение на воксель
        int x, y, z;
        if (pickVoxel(event->pos(), x, y, z)) {
            hoverX = x;
            hoverY = y;
            hoverZ = z;
            isHovering = true;
            
            emit voxelHovered(x, y, z, getVoxelValue(x, y, z));
            
            // Tooltip
            QString tooltip = QString("Position: [%1, %2, %3]\nValue: %4")
                .arg(x).arg(y).arg(z)
                .arg(formatValue(getVoxelValue(x, y, z)));
            QToolTip::showText(event->globalPos(), tooltip, this);
        } else {
            isHovering = false;
            hoverX = -1;
            hoverY = -1;
            hoverZ = -1;
        }
        
        update();
    }
    
    QOpenGLWidget::mouseMoveEvent(event);
}

void Layer3DView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isRotating = false;
    } else if (event->button() == Qt::MiddleButton) {
        isPanning = false;
    }
    
    QOpenGLWidget::mouseReleaseEvent(event);
}

void Layer3DView::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        int x, y, z;
        if (pickVoxel(event->pos(), x, y, z)) {
            emit voxelDoubleClicked(x, y, z, getVoxelValue(x, y, z));
            
            // Если редактируемый - открытие диалога редактирования
            if (editable) {
                bool ok;
                double newValue = QInputDialog::getDouble(this, "Edit Voxel",
                    QString("Value at [%1,%2,%3]:").arg(x).arg(y).arg(z),
                    getVoxelValue(x, y, z), -1e10, 1e10, 6, &ok);
                
                if (ok) {
                    setVoxelValue(x, y, z, newValue);
                }
            }
        }
    }
    
    QOpenGLWidget::mouseDoubleClickEvent(event);
}

void Layer3DView::wheelEvent(QWheelEvent *event) {
    if (!interactive) {
        QOpenGLWidget::wheelEvent(event);
        return;
    }
    
    if (event->modifiers() & Qt::ControlModifier) {
        zoomCamera(event->angleDelta().y());
        event->accept();
    } else {
        QOpenGLWidget::wheelEvent(event);
    }
}

void Layer3DView::keyPressEvent(QKeyEvent *event) {
    if (!interactive) {
        QOpenGLWidget::keyPressEvent(event);
        return;
    }
    
    switch (event->key()) {
        case Qt::Key_R:
            resetCamera();
            break;
        case Qt::Key_S:
            if (event->modifiers() & Qt::ControlModifier) {
                saveAsImage("screenshot.png");
            }
            break;
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            zoomCamera(-1.0f);
            break;
        case Qt::Key_Minus:
            zoomCamera(1.0f);
            break;
        default:
            QOpenGLWidget::keyPressEvent(event);
            break;
    }
}

void Layer3DView::contextMenuEvent(QContextMenuEvent *event) {
    if (interactive) {
        contextMenu->exec(event->globalPos());
    }
}

// ============================================================================
// Утилиты
// ============================================================================

QVector3D Layer3DView::screenToWorld(const QPoint& screenPos) const {
    // Конвертация экранных координат в мировые
    // В полной реализации - ray casting
    return QVector3D(0, 0, 0);
}

bool Layer3DView::pickVoxel(const QPoint& screenPos, int& x, int& y, int& z) const {
    // Выбор вокселя по координатам экрана
    // В полной реализации - GPU picking
    if (isHovering && hoverX > 0 && hoverY > 0 && hoverZ > 0) {
        x = hoverX;
        y = hoverY;
        z = hoverZ;
        return true;
    }
    return false;
}

void Layer3DView::ensureVoxelVisible(int x, int y, int z) {
    // Центрирование на вокселе
    camera.target = QVector3D(x, y, z);
    update();
}

QString Layer3DView::formatValue(double value) const {
    if (std::abs(value) < 0.001 || std::abs(value) > 10000) {
        return QString::number(value, 'e', 3);
    }
    return QString::number(value, 'f', 4);
}

QString Layer3DView::formatCoordinate(int x, int y, int z) const {
    return QString("[%1, %2, %3]").arg(x).arg(y).arg(z);
}

void Layer3DView::updateFPS() {
    currentFPS = frameCount;
    frameCount = 0;
    emit fpsChanged(currentFPS);
}

// ============================================================================
// Экспорт/Импорт
// ============================================================================

bool Layer3DView::saveAsImage(const QString& path, const QString& format) {
    QImage image(size(), QImage::Format_ARGB32);
    image.fill(backgroundColor);
    
    QPainter painter(&image);
    render(&painter);
    painter.end();
    
    return image.save(path, format.toStdString().c_str());
}

bool Layer3DView::saveAsCSV(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "X,Y,Z,Value\n";
    
    for (int z = 0; z < sizeZ_; z++) {
        for (int y = 0; y < sizeY_; y++) {
            for (int x = 0; x < sizeX_; x++) {
                out << (x + 1) << "," << (y + 1) << "," << (z + 1) << "," 
                    << layerData[z][y][x] << "\n";  // 1-based индексация
            }
        }
    }
    
    file.close();
    return true;
}

bool Layer3DView::loadFromCSV(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    // В полной реализации - загрузка данных
    file.close();
    return false;
}

void Layer3DView::print() {
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        render(&painter);
        painter.end();
    }
}

void Layer3DView::copyAsImage() {
    QImage image(size(), QImage::Format_ARGB32);
    image.fill(backgroundColor);
    
    QPainter painter(&image);
    render(&painter);
    painter.end();
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setImage(image);
    
    LOG_INFO("3D view copied to clipboard as image");
}

// ============================================================================
// Производительность
// ============================================================================

void Layer3DView::setHighPerformanceMode(bool enable) {
    highPerformanceMode = enable;
    highPerfModeAction->setChecked(enable);
    
    if (enable) {
        maxVisibleVoxels = 10000;
        setViewMode(LayerViewMode::Wireframe);
    } else {
        maxVisibleVoxels = 100000;
    }
    
    update();
}

void Layer3DView::setMaxVisibleVoxels(int count) {
    maxVisibleVoxels = qMax(1000, count);
    updateVoxelVisibility();
    update();
}

// ============================================================================
// Интерактивность
// ============================================================================

void Layer3DView::setInteractive(bool enable) {
    interactive = enable;
}

void Layer3DView::setEditable(bool enable) {
    editable = enable;
}

// ============================================================================
// UI Setup
// ============================================================================

void Layer3DView::setupUI() {
    setMinimumSize(400, 300);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    
    interactive = true;
    editable = false;
}

void Layer3DView::setupContextMenu() {
    contextMenu = new QMenu(this);
    
    // Режимы отображения
    volumeModeAction = contextMenu->addAction("Объёмное", this, [this]() {
        setViewMode(LayerViewMode::Volume);
    });
    volumeModeAction->setCheckable(true);
    
    sliceXYModeAction = contextMenu->addAction("Срез XY", this, [this]() {
        setViewMode(LayerViewMode::Slice_XY);
    });
    sliceXYModeAction->setCheckable(true);
    
    sliceXZModeAction = contextMenu->addAction("Срез XZ", this, [this]() {
        setViewMode(LayerViewMode::Slice_XZ);
    });
    sliceXZModeAction->setCheckable(true);
    
    sliceYZModeAction = contextMenu->addAction("Срез YZ", this, [this]() {
        setViewMode(LayerViewMode::Slice_YZ);
    });
    sliceYZModeAction->setCheckable(true);
    
    wireframeModeAction = contextMenu->addAction("Каркас", this, [this]() {
        setViewMode(LayerViewMode::Wireframe);
    });
    wireframeModeAction->setCheckable(true);
    
    pointsModeAction = contextMenu->addAction("Точки", this, [this]() {
        setViewMode(LayerViewMode::Points);
    });
    pointsModeAction->setCheckable(true);
    
    surfacesModeAction = contextMenu->addAction("Поверхности", this, [this]() {
        setViewMode(LayerViewMode::Surfaces);
    });
    surfacesModeAction->setCheckable(true);
    
    contextMenu->addSeparator();
    
    // Действия
    showSliceAction = contextMenu->addAction("Показать срез", this, [this](bool checked) {
        setShowSlice(checked);
    });
    showSliceAction->setCheckable(true);
    
    resetCameraAction = contextMenu->addAction("Сбросить камеру", this, &Layer3DView::resetCamera);
    resetCameraAction->setShortcut(QKeySequence(Qt::Key_R));
    
    highPerfModeAction = contextMenu->addAction("Режим высокой производительности", this, [this](bool checked) {
        setHighPerformanceMode(checked);
    });
    highPerfModeAction->setCheckable(true);
    
    contextMenu->addSeparator();
    
    // Экспорт
    saveImageAction = contextMenu->addAction("Сохранить как изображение...", this, [this]() {
        QString path = QFileDialog::getSaveFileName(this, "Сохранить как изображение", "", 
                                                    "PNG Files (*.png);;JPG Files (*.jpg);;All Files (*)");
        if (!path.isEmpty()) {
            saveAsImage(path);
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
    
    contextMenu->addSeparator();
    
    copyImageAction = contextMenu->addAction("Копировать как изображение", this, &Layer3DView::copyAsImage);
    printAction = contextMenu->addAction("Печать...", this, &Layer3DView::print);
    printAction->setShortcut(QKeySequence::Print);
    
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        contextMenu->exec(mapToGlobal(pos));
    });
}

void Layer3DView::setupToolbar() {
    toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));
    toolbar->setVisible(false);
}

} // namespace proxima