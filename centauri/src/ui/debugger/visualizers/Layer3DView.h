#ifndef CENTAURI_LAYER3DVIEW_H
#define CENTAURI_LAYER3DVIEW_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QToolBar>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QToolTip>
#include <QClipboard>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QProgressDialog>
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <QIcon>
#include <QFont>
#include <QFontMetrics>
#include <QRect>
#include <QPoint>
#include <QSize>
#include <QColor>
#include <QPalette>
#include <QStyle>
#include <QStyleOption>
#include <QMatrix>
#include <QTransform>
#include <QGradient>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QConicalGradient>
#include <QBrush>
#include <QPen>
#include <QRegion>
#include <QBitmap>
#include <QMaskedPixmap>
#include <QSvgRenderer>
#include <QSvgGenerator>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrintPreviewWidget>
#include <QPageSetupDialog>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextList>
#include <QTextListFormat>
#include <QTextFrame>
#include <QTextFrameFormat>
#include <QAbstractTextDocumentLayout>
#include <QPlainTextDocumentLayout>
#include <QSyntaxHighlighter>
#include <QDesktopServices>
#include <QUrl>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QGesture>
#include <QGestureEvent>
#include <QPanGesture>
#include <QPinchGesture>
#include <QSwipeGesture>
#include <QTapGesture>
#include <QTapAndHoldGesture>
#include <QGraphicsEffect>
#include <QGraphicsBlurEffect>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QGraphicsColorizeEffect>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QAbstractAnimation>
#include <QAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QPauseAnimation>
#include <QEasingCurve>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <QOpenGLFramebufferObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include "runtime/Runtime.h"
#include "utils/Logger.h"

#ifdef USE_CUDA
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#endif

namespace proxima {

/**
 * @brief Режимы рендеринга 3D
 */
enum class RenderMode {
    CPU,            // CPU рендеринг
    GPU_OpenGL,     // GPU через OpenGL
    GPU_CUDA        // GPU через CUDA (если доступно)
};

/**
 * @brief Режимы отображения слоя
 */
enum class LayerViewMode {
    Volume,         // Объёмное отображение
    Slice_XY,       // Срез XY
    Slice_XZ,       // Срез XZ
    Slice_YZ,       // Срез YZ
    Wireframe,      // Каркас
    Points,         // Точки
    Surfaces        // Поверхности
};

/**
 * @brief Структура вокселя
 */
struct Voxel {
    int x;              // Координата X (1-based)
    int y;              // Координата Y (1-based)
    int z;              // Координата Z (1-based)
    double value;       // Значение
    QColor color;       // Цвет
    float opacity;      // Прозрачность
    bool isVisible;     // Видимый ли
    bool isSelected;    // Выбранный ли
    bool isHovered;     // Наведён ли курсор
    QPoint screenPos;   // Позиция на экране
    
    Voxel() : x(0), y(0), z(0), value(0.0), opacity(1.0),
              isVisible(true), isSelected(false), isHovered(false) {}
};

/**
 * @brief Структура среза
 */
struct Slice {
    int plane;          // 0=XY, 1=XZ, 2=YZ
    int position;       // Позиция среза (1-based)
    QVector<QVector<double>> data;  // Данные среза
    
    Slice() : plane(0), position(1) {}
};

/**
 * @brief Структура камеры
 */
struct Camera {
    QVector3D position;     // Позиция камеры
    QVector3D target;       // Цель (куда смотрит)
    QVector3D up;           // Вектор вверх
    float fov;              // Поле зрения
    float aspect;           // Соотношение сторон
    float nearPlane;        // Ближняя плоскость
    float farPlane;         // Дальняя плоскость
    
    Camera() : position(0, 0, 5), target(0, 0, 0), up(0, 1, 0),
               fov(45.0f), aspect(1.0f), nearPlane(0.1f), farPlane(100.0f) {}
};

/**
 * @brief Класс 3D визуализатора слоёв для IDE Centauri
 * 
 * Согласно требованиям:
 * - ide.txt пункт 10 - инструменты визуализации состояний объектов
 * - ide.txt пункт 10 - для layer - 3D визуализация с прозрачностью вокселей
 * - language.txt пункт 11.1 - индексация с 1
 */
class Layer3DView : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
    
public:
    explicit Layer3DView(QWidget *parent = nullptr);
    ~Layer3DView();
    
    // ========================================================================
    // Данные слоя
    // ========================================================================
    
    /**
     * @brief Установка данных слоя
     * @param data Трёхмерный вектор значений
     */
    void setData(const QVector<QVector<QVector<double>>>& data);
    
    /**
     * @brief Установка данных из RuntimeValue
     * @param value Значение типа layer
     */
    void setDataFromRuntime(const RuntimeValue& value);
    
    /**
     * @brief Получение данных слоя
     * @return Трёхмерный вектор значений
     */
    QVector<QVector<QVector<double>>> getData() const { return layerData; }
    
    /**
     * @brief Получение размера по X
     * @return Размер X (1-based для отображения)
     */
    int sizeX() const { return sizeX_; }
    
    /**
     * @brief Получение размера по Y
     * @return Размер Y (1-based для отображения)
     */
    int sizeY() const { return sizeY_; }
    
    /**
     * @brief Получение размера по Z
     * @return Размер Z (1-based для отображения)
     */
    int sizeZ() const { return sizeZ_; }
    
    /**
     * @brief Получение значения вокселя
     * @param x Координата X (1-based)
     * @param y Координата Y (1-based)
     * @param z Координата Z (1-based)
     * @return Значение вокселя
     */
    double getVoxelValue(int x, int y, int z) const;
    
    /**
     * @brief Установка значения вокселя
     * @param x Координата X (1-based)
     * @param y Координата Y (1-based)
     * @param z Координата Z (1-based)
     * @param value Новое значение
     */
    void setVoxelValue(int x, int y, int z, double value);
    
    // ========================================================================
    // Режимы рендеринга
    // ========================================================================
    
    /**
     * @brief Установка режима рендеринга
     * @param mode Режим рендеринга
     */
    void setRenderMode(RenderMode mode);
    
    /**
     * @brief Получение режима рендеринга
     * @return Текущий режим
     */
    RenderMode getRenderMode() const { return renderMode; }
    
    /**
     * @brief Проверка доступности GPU
     * @return true если GPU доступен
     */
    bool isGPUAvailable() const { return gpuAvailable; }
    
    /**
     * @brief Получение информации о GPU
     * @return Информация о GPU
     */
    QString getGPUInfo() const { return gpuInfo; }
    
    // ========================================================================
    // Режимы отображения
    // ========================================================================
    
    /**
     * @brief Установка режима отображения
     * @param mode Режим отображения
     */
    void setViewMode(LayerViewMode mode);
    
    /**
     * @brief Получение режима отображения
     * @return Текущий режим
     */
    LayerViewMode getViewMode() const { return viewMode; }
    
    // ========================================================================
    // Срезы
    // ========================================================================
    
    /**
     * @brief Установка текущего среза
     * @param plane Плоскость среза (0=XY, 1=XZ, 2=YZ)
     * @param position Позиция среза (1-based)
     */
    void setCurrentSlice(int plane, int position);
    
    /**
     * @brief Получение текущего среза
     * @return Текущий срез
     */
    Slice getCurrentSlice() const { return currentSlice; }
    
    /**
     * @brief Включение отображения срезов
     * @param show true для отображения
     */
    void setShowSlice(bool show);
    
    /**
     * @brief Проверка отображения срезов
     * @return true если срезы отображаются
     */
    bool getShowSlice() const { return showSlice; }
    
    // ========================================================================
    // Прозрачность и цвет
    // ========================================================================
    
    /**
     * @brief Установка порога прозрачности
     * @param threshold Порог (0.0-1.0)
     */
    void setOpacityThreshold(double threshold);
    
    /**
     * @brief Получение порога прозрачности
     * @return Порог
     */
    double getOpacityThreshold() const { return opacityThreshold; }
    
    /**
     * @brief Установка цветовой палитры
     * @param palette Цветовая палитра
     */
    void setColorPalette(ColorPalette palette);
    
    /**
     * @brief Получение цветовой палитры
     * @return Текущая палитра
     */
    ColorPalette getColorPalette() const { return currentPalette; }
    
    // ========================================================================
    // Камера и навигация
    // ========================================================================
    
    /**
     * @brief Установка позиции камеры
     * @param position Позиция камеры
     */
    void setCameraPosition(const QVector3D& position);
    
    /**
     * @brief Получение позиции камеры
     * @return Позиция камеры
     */
    QVector3D getCameraPosition() const { return camera.position; }
    
    /**
     * @brief Вращение камеры
     * @param dx Смещение по X
     * @param dy Смещение по Y
     */
    void rotateCamera(int dx, int dy);
    
    /**
     * @brief Приближение/отдаление камеры
     * @param delta Смещение
     */
    void zoomCamera(float delta);
    
    /**
     * @brief Сброс камеры
     */
    void resetCamera();
    
    // ========================================================================
    // Выделение
    // ========================================================================
    
    /**
     * @brief Выделение вокселя
     * @param x Координата X (1-based)
     * @param y Координата Y (1-based)
     * @param z Координата Z (1-based)
     */
    void selectVoxel(int x, int y, int z);
    
    /**
     * @brief Очистка выделения
     */
    void clearSelection();
    
    /**
     * @brief Получение выделенного вокселя
     * @return Координаты вокселя
     */
    QVector3D getSelectedVoxel() const { return selectedVoxel; }
    
    /**
     * @brief Проверка наличия выделения
     * @return true если есть выделение
     */
    bool hasSelection() const { return selectedVoxel.x() > 0; }
    
    // ========================================================================
    // Статистика
    // ========================================================================
    
    /**
     * @brief Получение минимального значения
     * @return Минимальное значение
     */
    double getMinValue() const { return minValue; }
    
    /**
     * @brief Получение максимального значения
     * @return Максимальное значение
     */
    double getMaxValue() const { return maxValue; }
    
    /**
     * @brief Получение среднего значения
     * @return Среднее значение
     */
    double getMeanValue() const { return meanValue; }
    
    /**
     * @brief Получение количества видимых вокселей
     * @return Количество видимых вокселей
     */
    int getVisibleVoxelCount() const { return visibleVoxelCount; }
    
    /**
     * @brief Получение FPS
     * @return FPS
     */
    int getFPS() const { return currentFPS; }
    
    // ========================================================================
    // Экспорт/Импорт
    // ========================================================================
    
    /**
     * @brief Сохранение как изображение
     * @param path Путь к файлу
     * @param format Формат (PNG, JPG)
     * @return true если успешно
     */
    bool saveAsImage(const QString& path, const QString& format = "PNG");
    
    /**
     * @brief Сохранение данных в CSV
     * @param path Путь к файлу
     * @return true если успешно
     */
    bool saveAsCSV(const QString& path);
    
    /**
     * @brief Загрузка данных из CSV
     * @param path Путь к файлу
     * @return true если успешно
     */
    bool loadFromCSV(const QString& path);
    
    /**
     * @brief Печать вида
     */
    void print();
    
    /**
     * @brief Копирование в буфер как изображение
     */
    void copyAsImage();
    
    // ========================================================================
    // Производительность
    // ========================================================================
    
    /**
     * @brief Включение режима высокой производительности
     * @param enable true для включения
     */
    void setHighPerformanceMode(bool enable);
    
    /**
     * @brief Проверка режима высокой производительности
     * @return true если включён
     */
    bool isHighPerformanceMode() const { return highPerformanceMode; }
    
    /**
     * @brief Установка максимального количества отображаемых вокселей
     * @param count Максимальное количество
     */
    void setMaxVisibleVoxels(int count);
    
    /**
     * @brief Получение максимального количества отображаемых вокселей
     * @return Максимальное количество
     */
    int getMaxVisibleVoxels() const { return maxVisibleVoxels; }
    
    // ========================================================================
    // Интерактивность
    // ========================================================================
    
    /**
     * @brief Включение интерактивного режима
     * @param enable true для включения
     */
    void setInteractive(bool enable);
    
    /**
     * @brief Проверка интерактивного режима
     * @return true если интерактивный
     */
    bool isInteractive() const { return interactive; }
    
    /**
     * @brief Включение редактирования значений
     * @param enable true для включения
     */
    void setEditable(bool enable);
    
    /**
     * @brief Проверка режима редактирования
     * @return true если редактируемый
     */
    bool isEditable() const { return editable; }
    
signals:
    void voxelClicked(int x, int y, int z, double value);
    void voxelDoubleClicked(int x, int y, int z, double value);
    void voxelHovered(int x, int y, int z, double value);
    void selectionChanged(int x, int y, int z);
    void dataModified();
    void cameraChanged();
    void sliceChanged(int plane, int position);
    void fpsChanged(int fps);
    void renderModeChanged(RenderMode mode);
    
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    
private:
    // ========================================================================
    // Инициализация OpenGL
    // ========================================================================
    
    void initOpenGL();
    void initShaders();
    void initBuffers();
    void initTextures();
    void cleanupOpenGL();
    
    // ========================================================================
    // Методы рендеринга
    // ========================================================================
    
    void renderVolume();
    void renderSlice();
    void renderWireframe();
    void renderPoints();
    void renderSurfaces();
    void renderCPUBased();
    void renderGPUBased();
    void renderCUDABased();
    
    // ========================================================================
    // Методы камеры
    // ========================================================================
    
    void updateCamera();
    QMatrix4x4 getViewMatrix() const;
    QMatrix4x4 getProjectionMatrix() const;
    QMatrix4x4 getModelMatrix() const;
    
    // ========================================================================
    // Методы данных
    // ========================================================================
    
    void calculateStatistics();
    void generatePalette();
    QColor valueToColor(double value) const;
    float valueToOpacity(double value) const;
    void updateVoxelVisibility();
    void updateSliceData();
    
    // ========================================================================
    // Методы навигации
    // ========================================================================
    
    QVector3D screenToWorld(const QPoint& screenPos) const;
    bool pickVoxel(const QPoint& screenPos, int& x, int& y, int& z) const;
    void ensureVoxelVisible(int x, int y, int z);
    
    // ========================================================================
    // Методы утилит
    // ========================================================================
    
    void setupUI();
    void setupContextMenu();
    void setupToolbar();
    QString formatValue(double value) const;
    QString formatCoordinate(int x, int y, int z) const;
    void updateFPS();
    
    // ========================================================================
    // Переменные
    // ========================================================================
    
    // Данные слоя
    QVector<QVector<QVector<double>>> layerData;
    int sizeX_, sizeY_, sizeZ_;
    int totalVoxels;
    
    // Воксели
    QVector<Voxel> voxels;
    int visibleVoxelCount;
    int maxVisibleVoxels;
    
    // Режимы
    RenderMode renderMode;
    LayerViewMode viewMode;
    bool gpuAvailable;
    QString gpuInfo;
    bool highPerformanceMode;
    
    // Срезы
    Slice currentSlice;
    bool showSlice;
    
    // Прозрачность и цвет
    double opacityThreshold;
    ColorPalette currentPalette;
    QVector<QColor> paletteColors;
    
    // Камера
    Camera camera;
    bool isRotating;
    bool isPanning;
    QPoint lastMousePos;
    
    // Выделение
    QVector3D selectedVoxel;
    int hoverX, hoverY, hoverZ;
    bool isHovering;
    
    // Статистика
    double minValue, maxValue, meanValue;
    int currentFPS;
    QTimer* fpsTimer;
    int frameCount;
    
    // OpenGL
    QOpenGLShaderProgram* shaderProgram;
    QOpenGLBuffer vertexBuffer;
    QOpenGLBuffer indexBuffer;
    QOpenGLVertexArrayObject vao;
    QOpenGLTexture* volumeTexture;
    QOpenGLFramebufferObject* fbo;
    
    // CUDA
    #ifdef USE_CUDA
    cudaGraphicsResource* cudaResource;
    bool cudaAvailable;
    #endif
    
    // Контекстное меню
    QMenu* contextMenu;
    QToolBar* toolbar;
    
    // Действия
    QAction* saveImageAction;
    QAction* saveCSVAction;
    QAction* loadCSVAction;
    QAction* printAction;
    QAction* copyImageAction;
    QAction* volumeModeAction;
    QAction* sliceXYModeAction;
    QAction* sliceXZModeAction;
    QAction* sliceYZModeAction;
    QAction* wireframeModeAction;
    QAction* pointsModeAction;
    QAction* surfacesModeAction;
    QAction* showSliceAction;
    QAction* resetCameraAction;
    QAction* highPerfModeAction;
};

} // namespace proxima

#endif // CENTAURI_LAYER3DVIEW_H