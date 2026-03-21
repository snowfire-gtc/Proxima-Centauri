#ifndef CENTAURI_MATRIXVIEW_H
#define CENTAURI_MATRIXVIEW_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QVector>
#include <QMap>
#include <QMenu>
#include <QToolBar>
#include <QComboBox>

namespace proxima {

enum class MatrixViewMode {
    Table,
    Image,
    Heatmap
};

enum class ColorPalette {
    Viridis,
    Plasma,
    Inferno,
    Magma,
    Grayscale,
    Rainbow,
    Jet
};

class MatrixView : public QWidget {
    Q_OBJECT
    
public:
    explicit MatrixView(QWidget *parent = nullptr);
    ~MatrixView();
    
    // Data management
    void setData(const QVector<QVector<double>>& data);
    void setData(int rows, int cols, const double* data);
    void clearData();
    QVector<QVector<double>> getData() const { return matrixData; }
    int rows() const { return matrixData.size(); }
    int cols() const { return matrixData.isEmpty() ? 0 : matrixData[0].size(); }
    
    // View modes
    void setViewMode(MatrixViewMode mode);
    MatrixViewMode getViewMode() const { return viewMode; }
    
    // Color palette
    void setColorPalette(ColorPalette palette);
    ColorPalette getColorPalette() const { return currentPalette; }
    void setCustomPalette(const QVector<QColor>& colors);
    QVector<QColor> getColorPaletteColors() const { return paletteColors; }
    
    // Zoom and pan
    void zoomIn();
    void zoomOut();
    void zoomToFit();
    void zoomToCell(int row, int col);
    void resetView();
    double getZoomLevel() const { return zoomLevel; }
    
    // Selection
    void selectCell(int row, int col);
    void selectRange(int startRow, int startCol, int endRow, int endCol);
    void clearSelection();
    bool hasSelection() const { return selectionActive; }
    QVector<QVector<double>> getSelectedData() const;
    
    // Cell inspection
    void setHoverCell(int row, int col);
    int getHoverRow() const { return hoverRow; }
    int getHoverCol() const { return hoverCol; }
    double getCellValue(int row, int col) const;
    QString getCellFormattedValue(int row, int col) const;
    
    // Overlay
    void setOverlayMatrix(const QVector<QVector<double>>& overlay);
    void clearOverlay();
    void setOverlayAlpha(double alpha);
    
    // Export
    void copyData();
    void copySelection();
    void saveAsCSV(const QString& path);
    void saveAsImage(const QString& path);
    void saveSelectionAsCSV(const QString& path);
    
    // Cropping
    void cropToSelection();
    void crop(int startRow, int startCol, int endRow, int endCol);
    
    // Configuration
    void setShowValues(bool show);
    void setShowGrid(bool show);
    void setShowRowLabels(bool show);
    void setShowColLabels(bool show);
    void setValuePrecision(int precision);
    void setMinValue(double min);
    void setMaxValue(double max);
    void setAutoRange(bool enable);
    
    // Statistics
    QMap<QString, double> getStatistics() const { return statistics; }
    void calculateStatistics();
    
signals:
    void cellHovered(int row, int col, double value);
    void cellClicked(int row, int col, double value);
    void rangeSelected(int startRow, int startCol, int endRow, int endCol);
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
    void onViewModeChanged(int index);
    void onPaletteChanged(int index);
    void onZoomIn();
    void onZoomOut();
    void onZoomToFit();
    void onCopyData();
    void onCopySelection();
    void onSaveCSV();
    void onSaveImage();
    void onCropToSelection();
    
private:
    void setupUI();
    void setupToolbar();
    void setupContextMenu();
    void calculateCellSize();
    QRect getCellRect(int row, int col) const;
    QPoint getCellFromPos(const QPoint& pos) const;
    QColor valueToColor(double value) const;
    void generatePalette();
    
    void drawTableMode(QPainter& painter);
    void drawImageMode(QPainter& painter);
    void drawHeatmapMode(QPainter& painter);
    void drawGrid(QPainter& painter);
    void drawLabels(QPainter& painter);
    void drawSelection(QPainter& painter);
    void drawOverlay(QPainter& painter);
    void drawHoverCell(QPainter& painter);
    void drawStatistics(QPainter& painter);
    
    // Data
    QVector<QVector<double>> matrixData;
    QVector<QVector<double>> overlayData;
    double overlayAlpha;
    
    // View
    MatrixViewMode viewMode;
    ColorPalette currentPalette;
    QVector<QColor> paletteColors;
    double zoomLevel;
    double panX;
    double panY;
    double cellWidth;
    double cellHeight;
    bool autoRange;
    double minValue;
    double maxValue;
    
    // Selection
    bool selectionActive;
    int selectionStartRow;
    int selectionStartCol;
    int selectionEndRow;
    int selectionEndCol;
    bool isSelecting;
    QPoint selectionStartPos;
    
    // Hover
    int hoverRow;
    int hoverCol;
    
    // Display options
    bool showValues;
    bool showGrid;
    bool showRowLabels;
    bool showColLabels;
    int valuePrecision;
    
    // Statistics
    QMap<QString, double> statistics;
    
    // UI
    QToolBar* toolbar;
    QComboBox* viewModeCombo;
    QComboBox* paletteCombo;
    QMenu* contextMenu;
};

} // namespace proxima

#endif // CENTAURI_MATRIXVIEW_H