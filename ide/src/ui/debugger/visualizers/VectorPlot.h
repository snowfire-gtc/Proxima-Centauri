#ifndef CENTAURI_VECTORPLOT_H
#define CENTAURI_VECTORPLOT_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QVector>
#include <QMap>
#include <QMenu>
#include <QToolBar>

namespace proxima {

struct PlotPoint {
    double x;
    double y;
    bool isSelected;
    bool isPeak;
    bool isValley;
};

struct PlotRange {
    double minX;
    double maxX;
    double minY;
    double maxY;
};

class VectorPlot : public QWidget {
    Q_OBJECT
    
public:
    explicit VectorPlot(QWidget *parent = nullptr);
    ~VectorPlot();
    
    // Data management
    void setData(const QVector<double>& data);
    void addData(const QVector<double>& data, const QString& name);
    void clearData();
    QVector<double> getData() const { return currentData; }
    
    // Multiple datasets
    void addDataset(const QVector<double>& data, const QString& name, const QColor& color);
    void removeDataset(const QString& name);
    void toggleDataset(const QString& name, bool visible);
    QMap<QString, QVector<double>> getAllDatasets() const { return datasets; }
    
    // Zoom and pan
    void zoomIn();
    void zoomOut();
    void zoomToFit();
    void zoomToSelection();
    void panLeft();
    void panRight();
    void panUp();
    void panDown();
    void resetView();
    
    // Selection
    void selectRange(int start, int end);
    QVector<double> getSelectedData() const;
    void clearSelection();
    bool hasSelection() const { return selectionStart >= 0; }
    
    // Analysis tools
    void findPeaks();
    void findValleys();
    void calculateStatistics();
    QMap<QString, double> getStatistics() const { return statistics; }
    
    // Point inspection
    void setHoverPoint(int index);
    int getHoverPoint() const { return hoverIndex; }
    double getPointValue(int index) const;
    QPoint getPointPosition(int index) const;
    
    // Export
    void copyData();
    void saveAsCSV(const QString& path);
    void saveAsImage(const QString& path);
    
    // Configuration
    void setLineColor(const QColor& color);
    void setLineWidth(int width);
    void setShowGrid(bool show);
    void setShowPoints(bool show);
    void setShowLabels(bool show);
    void setShowStatistics(bool show);
    void setGridColor(const QColor& color);
    void setBackgroundColor(const QColor& color);
    
    // Get current view
    PlotRange getViewRange() const { return viewRange; }
    int getDataSize() const { return currentData.size(); }
    
signals:
    void pointHovered(int index, double value);
    void rangeSelected(int start, int end);
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
    void onZoomIn();
    void onZoomOut();
    void onZoomToFit();
    void onCopyData();
    void onSaveCSV();
    void onSaveImage();
    void onFindPeaks();
    void onFindValleys();
    void onClearSelection();
    
private:
    void setupUI();
    void setupToolbar();
    void setupContextMenu();
    void calculateViewRange();
    QPointF dataToPoint(int index, double value) const;
    int pointToDataIndex(const QPoint& pos) const;
    double pointToDataValue(const QPoint& pos) const;
    void drawGrid(QPainter& painter);
    void drawAxes(QPainter& painter);
    void drawData(QPainter& painter);
    void drawSelection(QPainter& painter);
    void drawPoints(QPainter& painter);
    void drawLabels(QPainter& painter);
    void drawStatistics(QPainter& painter);
    void drawHoverPoint(QPainter& painter);
    void updateHoverPoint(const QPoint& pos);
    QString formatValue(double value) const;
    QString formatIndex(int index) const;
    
    // Data
    QVector<double> currentData;
    QMap<QString, QVector<double>> datasets;
    QMap<QString, QColor> datasetColors;
    QMap<QString, bool> datasetVisibility;
    
    // View
    PlotRange viewRange;
    PlotRange dataRange;
    bool isLogScaleX;
    bool isLogScaleY;
    
    // Selection
    int selectionStart;
    int selectionEnd;
    bool isSelecting;
    QPoint selectionStartPos;
    
    // Interaction
    int hoverIndex;
    bool isPanning;
    QPoint lastPanPos;
    
    // Analysis
    QMap<QString, double> statistics;
    QVector<int> peakIndices;
    QVector<int> valleyIndices;
    
    // Appearance
    QColor lineColor;
    int lineWidth;
    bool showGrid;
    bool showPoints;
    bool showLabels;
    bool showStatistics;
    QColor gridColor;
    QColor backgroundColor;
    QFont labelFont;
    
    // UI
    QToolBar* toolbar;
    QMenu* contextMenu;
    QAction* zoomInAction;
    QAction* zoomOutAction;
    QAction* zoomFitAction;
    QAction* copyDataAction;
    QAction* saveCSVAction;
    QAction* saveImageAction;
    QAction* findPeaksAction;
    QAction* findValleysAction;
    QAction* clearSelectionAction;
};

} // namespace proxima

#endif // CENTAURI_VECTORPLOT_H