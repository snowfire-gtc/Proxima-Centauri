#ifndef CENTAURI_LAYER3DVIEW_H
#define CENTAURI_LAYER3DVIEW_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QVector>
#include <QMap>
#include <QMenu>
#include <QToolBar>
#include <QSlider>
#include <QLabel>

namespace proxima {

struct Voxel {
    double value;
    double opacity;
    QColor color;
    bool isVisible;
};

struct ViewTransform {
    double rotationX;
    double rotationY;
    double rotationZ;
    double zoom;
    double panX;
    double panY;
};

class Layer3DView : public QWidget {
    Q_OBJECT
    
public:
    explicit Layer3DView(QWidget *parent = nullptr);
    ~Layer3DView();
    
    // Data management
    void setData(const QVector<QVector<QVector<double>>>& data);
    void clearData();
    QVector<QVector<QVector<double>>> getData() const { return layerData; }
    int depth() const { return layerData.size(); }
    int rows() const { return layerData.isEmpty() ? 0 : layerData[0].size(); }
    int cols() const { return layerData.isEmpty() || layerData[0].isEmpty() ? 0 : layerData[0][0].size(); }
    
    // Slice control
    void setCurrentSlice(int slice);
    int getCurrentSlice() const { return currentSlice; }
    void setSliceAxis(int axis); // 0=X, 1=Y, 2=Z
    int getSliceAxis() const { return sliceAxis; }
    
    // View control
    void setViewMode(int mode); // 0=Wireframe, 1=Solid, 2=Volume
    int getViewMode() const { return viewMode; }
    void setRotation(double x, double y, double z);
    void setZoom(double zoom);
    void resetView();
    ViewTransform getViewTransform() const { return transform; }
    
    // Opacity mapping
    void setOpacityFunction(const QVector<QPair<double, double>>& function);
    void setAutoOpacity(bool enable);
    void setMinOpacity(double min);
    void setMaxOpacity(double max);
    double valueToOpacity(double value) const;
    
    // Color mapping
    void setColorPalette(const QVector<QColor>& colors);
    void setColorRange(double min, double max);
    QColor valueToColor(double value) const;
    
    // Selection
    void selectVoxel(int x, int y, int z);
    void selectRange(int x1, int y1, int z1, int x2, int y2, int z2);
    void clearSelection();
    bool hasSelection() const { return selectionActive; }
    QVector<QVector<QVector<double>>> getSelectedData() const;
    
    // Export
    void saveSliceAsImage(const QString& path, int slice);
    void saveVolumeAsImage(const QString& path);
    void exportDataAsCSV(const QString& path);
    void copySliceData(int slice);
    void copyVolumeData();
    
    // Rendering options
    void setShowAxes(bool show);
    void setShowGrid(bool show);
    void setShowValues(bool show);
    void setShowColorBar(bool show);
    void setRenderingQuality(int quality); // 0=Low, 1=Medium, 2=High
    void enableLighting(bool enable);
    void enableShadows(bool enable);
    
    // Statistics
    QMap<QString, double> getStatistics() const { return statistics; }
    void calculateStatistics();
    
signals:
    void voxelHovered(int x, int y, int z, double value);
    void voxelClicked(int x, int y, int z, double value);
    void sliceChanged(int slice);
    void viewChanged();
    void dataModified();
    
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    
private slots:
    void onSliceSliderChanged(int value);
    void onViewModeChanged(int index);
    void onZoomIn();
    void onZoomOut();
    void onResetView();
    void onSaveSlice();
    void onSaveVolume();
    void onExportCSV();
    void onCopySlice();
    void onCopyVolume();
    void onRotateLeft();
    void onRotateRight();
    void onRotateUp();
    void onRotateDown();
    
private:
    void setupUI();
    void setupToolbar();
    void setupContextMenu();
    void setupSliceControl();
    void calculateVoxelVisibility();
    void project3DToPoint(double x, double y, double z, QPoint& point) const;
    void unprojectPointToPoint(const QPoint& point, int& x, int& y, int& z) const;
    void drawVolume(QPainter& painter);
    void drawSlice(QPainter& painter);
    void drawWireframe(QPainter& painter);
    void drawAxes(QPainter& painter);
    void drawGrid(QPainter& painter);
    void drawColorBar(QPainter& painter);
    void drawSelection(QPainter& painter);
    void drawHoverVoxel(QPainter& painter);
    void updateHoverVoxel(const QPoint& pos);
    void applyTransform(QPainter& painter);
    void generatePalette();
    QString formatValue(double value) const;
    QString formatCoordinate(int x, int y, int z) const;
    
    // Data
    QVector<QVector<QVector<double>>> layerData;
    QVector<QVector<QVector<Voxel>>> voxelData;
    
    // Slice control
    int currentSlice;
    int sliceAxis; // 0=X, 1=Y, 2=Z
    QSlider* sliceSlider;
    QLabel* sliceLabel;
    
    // View
    int viewMode; // 0=Wireframe, 1=Solid, 2=Volume
    ViewTransform transform;
    bool autoRotate;
    
    // Opacity
    QVector<QPair<double, double>> opacityFunction;
    bool autoOpacity;
    double minOpacity;
    double maxOpacity;
    
    // Color
    QVector<QColor> paletteColors;
    double colorMin;
    double colorMax;
    
    // Selection
    bool selectionActive;
    int selectionX1, selectionY1, selectionZ1;
    int selectionX2, selectionY2, selectionZ2;
    bool isSelecting;
    QPoint selectionStartPos;
    
    // Hover
    int hoverX, hoverY, hoverZ;
    bool isRotating;
    QPoint lastRotatePos;
    bool isPanning;
    QPoint lastPanPos;
    
    // Rendering
    bool showAxes;
    bool showGrid;
    bool showValues;
    bool showColorBar;
    int renderingQuality;
    bool enableLighting;
    bool enableShadows;
    
    // Statistics
    QMap<QString, double> statistics;
    
    // UI
    QToolBar* toolbar;
    QMenu* contextMenu;
    QWidget* sliceControlWidget;
    
    // Actions
    QAction* wireframeAction;
    QAction* solidAction;
    QAction* volumeAction;
    QAction* zoomInAction;
    QAction* zoomOutAction;
    QAction* resetViewAction;
    QAction* saveSliceAction;
    QAction* saveVolumeAction;
    QAction* exportCSVAction;
    QAction* copySliceAction;
    QAction* copyVolumeAction;
    QAction* rotateLeftAction;
    QAction* rotateRightAction;
    QAction* rotateUpAction;
    QAction* rotateDownAction;
};

} // namespace proxima

#endif // CENTAURI_LAYER3DVIEW_H