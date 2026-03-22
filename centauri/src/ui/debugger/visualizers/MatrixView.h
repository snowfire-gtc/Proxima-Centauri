#ifndef CENTAURI_MATRIXVIEW_H
#define CENTAURI_MATRIXVIEW_H

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
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QVariantAnimation>
#include <QAbstractAnimation>
#include <QAnimationGroup>
#include <QPauseAnimation>
#include <QEasingCurve>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QVariantAnimation>
#include <QAbstractAnimation>
#include <QAnimationGroup>
#include <QPauseAnimation>
#include <QEasingCurve>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QVariantAnimation>
#include <QAbstractAnimation>
#include <QAnimationGroup>
#include <QPauseAnimation>
#include <QEasingCurve>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QVariantAnimation>
#include <QAbstractAnimation>
#include <QAnimationGroup>
#include <QPauseAnimation>
#include <QEasingCurve>
#include "runtime/Runtime.h"
#include "utils/Logger.h"
#include "utils/CollectionParser.h"

namespace proxima {

/**
 * @brief Режимы отображения матрицы
 * 
 * Согласно требованию ide.txt пункт 10
 */
enum class MatrixViewMode {
    Table,      // Табличное представление
    Image,      // Изображение (тепловая карта)
    Heatmap     // Тепловая карта с палитрой
};

/**
 * @brief Цветовые палитры для визуализации
 */
enum class ColorPalette {
    Viridis,
    Plasma,
    Inferno,
    Magma,
    Grayscale,
    Rainbow,
    Jet,
    Cool,
    Hot,
    Bone,
    Copper,
    Pink,
    Spring,
    Summer,
    Autumn,
    Winter,
    Custom
};

/**
 * @brief Структура ячейки матрицы
 */
struct MatrixCell {
    int row;            // Номер строки (1-based)
    int col;            // Номер столбца (1-based)
    double value;       // Значение
    QString formatted;  // Форматированное значение
    bool isSelected;    // Выбрана ли ячейка
    bool isHovered;     // Наведён ли курсор
    QColor bgColor;     // Цвет фона
    QColor fgColor;     // Цвет текста
    QRect rect;         // Прямоугольник ячейки
    
    MatrixCell() : row(0), col(0), value(0.0), isSelected(false), 
                   isHovered(false) {}
};

/**
 * @brief Структура диапазона выделения
 */
struct SelectionRange {
    int startRow;   // Начальная строка (1-based)
    int startCol;   // Начальный столбец (1-based)
    int endRow;     // Конечная строка (1-based)
    int endCol;     // Конечный столбец (1-based)
    
    SelectionRange() : startRow(0), startCol(0), endRow(0), endCol(0) {}
    
    bool isValid() const {
        return startRow > 0 && startCol > 0 && 
               endRow > 0 && endCol > 0 &&
               startRow <= endRow && startCol <= endCol;
    }
    
    int rowCount() const {
        return endRow - startRow + 1;
    }
    
    int colCount() const {
        return endCol - startCol + 1;
    }
};

/**
 * @brief Класс визуализатора матриц для IDE Centauri
 * 
 * Согласно требованиям:
 * - ide.txt пункт 10 - инструменты визуализации состояний объектов
 * - ide.txt пункт 10 - для matrix - таблица или изображение с палитрой
 * - language.txt пункт 11.1 - индексация с 1
 */
class MatrixView : public QWidget {
    Q_OBJECT
    
public:
    explicit MatrixView(QWidget *parent = nullptr);
    ~MatrixView();
    
    // ========================================================================
    // Данные матрицы
    // ========================================================================
    
    /**
     * @brief Установка данных матрицы
     * @param data Двумерный вектор значений
     */
    void setData(const QVector<QVector<double>>& data);
    
    /**
     * @brief Установка данных матрицы из RuntimeValue
     * @param value Значение типа matrix
     */
    void setDataFromRuntime(const RuntimeValue& value);
    
    /**
     * @brief Получение данных матрицы
     * @return Двумерный вектор значений
     */
    QVector<QVector<double>> getData() const { return matrixData; }
    
    /**
     * @brief Получение количества строк
     * @return Количество строк (1-based для отображения)
     */
    int rows() const { return rowCount; }
    
    /**
     * @brief Получение количества столбцов
     * @return Количество столбцов (1-based для отображения)
     */
    int cols() const { return colCount; }
    
    /**
     * @brief Получение значения ячейки
     * @param row Номер строки (1-based)
     * @param col Номер столбца (1-based)
     * @return Значение ячейки
     */
    double getValue(int row, int col) const;
    
    /**
     * @brief Установка значения ячейки
     * @param row Номер строки (1-based)
     * @param col Номер столбец (1-based)
     * @param value Новое значение
     */
    void setValue(int row, int col, double value);
    
    // ========================================================================
    // Режимы отображения
    // ========================================================================
    
    /**
     * @brief Установка режима отображения
     * @param mode Режим отображения
     */
    void setViewMode(MatrixViewMode mode);
    
    /**
     * @brief Получение режима отображения
     * @return Текущий режим
     */
    MatrixViewMode getViewMode() const { return viewMode; }
    
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
    
    /**
     * @brief Установка пользовательской палитры
     * @param colors Вектор цветов
     */
    void setCustomPalette(const QVector<QColor>& colors);
    
    /**
     * @brief Получение цветов палитры
     * @return Вектор цветов
     */
    QVector<QColor> getPaletteColors() const { return paletteColors; }
    
    // ========================================================================
    // Масштабирование и навигация
    // ========================================================================
    
    /**
     * @brief Установка уровня zoom
     * @param level Уровень масштабирования
     */
    void setZoomLevel(double level);
    
    /**
     * @brief Получение уровня zoom
     * @return Текущий уровень
     */
    double getZoomLevel() const { return zoomLevel; }
    
    /**
     * @brief Увеличение масштаба
     */
    void zoomIn();
    
    /**
     * @brief Уменьшение масштаба
     */
    void zoomOut();
    
    /**
     * @brief Сброс масштаба к значению по умолчанию
     */
    void resetZoom();
    
    /**
     * @brief Подгонка масштаба под размер виджета
     */
    void fitToWidget();
    
    /**
     * @brief Прокрутка к ячейке
     * @param row Номер строки (1-based)
     * @param col Номер столбца (1-based)
     */
    void scrollToCell(int row, int col);
    
    /**
     * @brief Центрирование на ячейке
     * @param row Номер строки (1-based)
     * @param col Номер столбца (1-based)
     */
    void centerOnCell(int row, int col);
    
    // ========================================================================
    // Выделение
    // ========================================================================
    
    /**
     * @brief Выделение ячейки
     * @param row Номер строки (1-based)
     * @param col Номер столбца (1-based)
     */
    void selectCell(int row, int col);
    
    /**
     * @brief Выделение диапазона
     * @param startRow Начальная строка (1-based)
     * @param startCol Начальный столбец (1-based)
     * @param endRow Конечная строка (1-based)
     * @param endCol Конечный столбец (1-based)
     */
    void selectRange(int startRow, int startCol, int endRow, int endCol);
    
    /**
     * @brief Получение выделенного диапазона
     * @return Диапазон выделения
     */
    SelectionRange getSelection() const { return selection; }
    
    /**
     * @brief Очистка выделения
     */
    void clearSelection();
    
    /**
     * @brief Проверка наличия выделения
     * @return true если есть выделение
     */
    bool hasSelection() const { return selection.isValid(); }
    
    /**
     * @brief Получение данных выделенного диапазона
     * @return Двумерный вектор значений
     */
    QVector<QVector<double>> getSelectedData() const;
    
    /**
     * @brief Копирование выделенных данных в буфер
     */
    void copySelection();
    
    /**
     * @brief Вставка данных из буфера
     */
    void pasteSelection();
    
    // ========================================================================
    // Наложения
    // ========================================================================
    
    /**
     * @brief Наложение другой матрицы
     * @param overlay Матрица для наложения
     * @param alpha Прозрачность (0.0-1.0)
     */
    void setOverlay(const QVector<QVector<double>>& overlay, double alpha = 0.5);
    
    /**
     * @brief Наложение графика
     * @param points Точки графика
     * @param color Цвет графика
     */
    void setGraphOverlay(const QVector<QPoint>& points, const QColor& color = Qt::red);
    
    /**
     * @brief Наложение точек
     * @param points Точки для отображения
     * @param color Цвет точек
     * @param size Размер точек
     */
    void setPointOverlay(const QVector<QPoint>& points, const QColor& color = Qt::blue, int size = 5);
    
    /**
     * @brief Очистка всех наложений
     */
    void clearOverlays();
    
    // ========================================================================
    // Отображение значений
    // ========================================================================
    
    /**
     * @brief Включение отображения значений в ячейках
     * @param show true для отображения
     */
    void setShowValues(bool show);
    
    /**
     * @brief Проверка отображения значений
     * @return true если значения отображаются
     */
    bool getShowValues() const { return showValues; }
    
    /**
     * @brief Установка порога zoom для отображения значений
     * @param threshold Порог zoom
     */
    void setValueZoomThreshold(double threshold);
    
    /**
     * @brief Получение порога zoom для отображения значений
     * @return Порог zoom
     */
    double getValueZoomThreshold() const { return valueZoomThreshold; }
    
    /**
     * @brief Установка формата чисел
     * @param format Формат (например, "%.4f")
     */
    void setNumberFormat(const QString& format);
    
    /**
     * @brief Получение формата чисел
     * @return Формат
     */
    QString getNumberFormat() const { return numberFormat; }
    
    // ========================================================================
    // Заголовки
    // ========================================================================
    
    /**
     * @brief Включение отображения заголовков строк
     * @param show true для отображения
     */
    void setShowRowLabels(bool show);
    
    /**
     * @brief Включение отображения заголовков столбцов
     * @param show true для отображения
     */
    void setShowColLabels(bool show);
    
    /**
     * @brief Установка заголовков строк
     * @param labels Вектор заголовков
     */
    void setRowLabels(const QVector<QString>& labels);
    
    /**
     * @brief Установка заголовков столбцов
     * @param labels Вектор заголовков
     */
    void setColLabels(const QVector<QString>& labels);
    
    /**
     * @brief Получение заголовков строк
     * @return Вектор заголовков
     */
    QVector<QString> getRowLabels() const { return rowLabels; }
    
    /**
     * @brief Получение заголовков столбцов
     * @return Вектор заголовков
     */
    QVector<QString> getColLabels() const { return colLabels; }
    
    // ========================================================================
    // Обрезка матрицы
    // ========================================================================
    
    /**
     * @brief Обрезка матрицы по выделенному диапазону
     */
    void cropToSelection();
    
    /**
     * @brief Обрезка матрицы по заданному диапазону
     * @param startRow Начальная строка (1-based)
     * @param startCol Начальный столбец (1-based)
     * @param endRow Конечная строка (1-based)
     * @param endCol Конечный столбец (1-based)
     */
    void crop(int startRow, int startCol, int endRow, int endCol);
    
    // ========================================================================
    // Экспорт/Импорт
    // ========================================================================
    
    /**
     * @brief Сохранение как изображение
     * @param path Путь к файлу
     * @param format Формат изображения (PNG, JPG, etc.)
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
     * @brief Печать матрицы
     */
    void print();
    
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
     * @brief Получение суммы всех значений
     * @return Сумма
     */
    double getSumValue() const { return sumValue; }
    
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
    void cellClicked(int row, int col, double value);
    void cellDoubleClicked(int row, int col, double value);
    void cellHovered(int row, int col, double value);
    void selectionChanged(const SelectionRange& range);
    void dataModified();
    void zoomChanged(double level);
    void modeChanged(MatrixViewMode mode);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    
private:
    // ========================================================================
    // Методы отрисовки
    // ========================================================================
    
    void setupUI();
    void setupContextMenu();
    void setupToolbar();
    void calculateCellGeometry();
    void drawTable(QPainter& painter);
    void drawImage(QPainter& painter);
    void drawHeatmap(QPainter& painter);
    void drawRowLabels(QPainter& painter);
    void drawColLabels(QPainter& painter);
    void drawGrid(QPainter& painter);
    void drawSelection(QPainter& painter);
    void drawHoverCell(QPainter& painter);
    void drawOverlays(QPainter& painter);
    void drawValue(QPainter& painter, int row, int col, const QRect& rect);
    
    // ========================================================================
    // Методы навигации
    // ========================================================================
    
    void updateTransform();
    void ensureCellVisible(int row, int col);
    QPoint cellToScreen(int row, int col) const;
    QPoint screenToCell(const QPoint& screenPos) const;
    QRect getCellRect(int row, int col) const;
    
    // ========================================================================
    // Методы палитры
    // ========================================================================
    
    void generatePalette();
    QColor valueToColor(double value) const;
    QVector<QColor> generateViridisPalette(int size) const;
    QVector<QColor> generatePlasmaPalette(int size) const;
    QVector<QColor> generateInfernoPalette(int size) const;
    QVector<QColor> generateMagmaPalette(int size) const;
    QVector<QColor> generateGrayscalePalette(int size) const;
    QVector<QColor> generateRainbowPalette(int size) const;
    QVector<QColor> generateJetPalette(int size) const;
    
    // ========================================================================
    // Методы данных
    // ========================================================================
    
    void calculateStatistics();
    void normalizeData();
    QString formatValue(double value) const;
    void updateCellRects();
    
    // ========================================================================
    // Переменные
    // ========================================================================
    
    // Данные матрицы
    QVector<QVector<double>> matrixData;
    int rowCount;
    int colCount;
    
    // Режимы отображения
    MatrixViewMode viewMode;
    ColorPalette currentPalette;
    QVector<QColor> paletteColors;
    QVector<QColor> customPalette;
    
    // Масштабирование
    double zoomLevel;
    double minZoom;
    double maxZoom;
    QPointF panOffset;
    QTransform transform;
    
    // Выделение
    SelectionRange selection;
    bool isSelecting;
    QPoint selectionStart;
    
    // Наведение
    int hoverRow;
    int hoverCol;
    bool isHovering;
    
    // Отображение
    bool showValues;
    bool showRowLabels;
    bool showColLabels;
    bool showGrid;
    double valueZoomThreshold;
    QString numberFormat;
    
    // Заголовки
    QVector<QString> rowLabels;
    QVector<QString> colLabels;
    
    // Наложения
    QVector<QVector<double>> overlayData;
    double overlayAlpha;
    QVector<QPoint> graphPoints;
    QColor graphColor;
    QVector<QPoint> pointOverlay;
    QColor pointColor;
    int pointSize;
    
    // Статистика
    double minValue;
    double maxValue;
    double meanValue;
    double sumValue;
    
    // Геометрия
    int cellWidth;
    int cellHeight;
    int headerWidth;
    int headerHeight;
    
    // Интерактивность
    bool interactive;
    bool editable;
    bool isPanning;
    QPoint lastPanPos;
    bool isZooming;
    
    // Контекстное меню
    QMenu* contextMenu;
    QToolBar* toolbar;
    
    // Действия
    QAction* copyAction;
    QAction* pasteAction;
    QAction* selectAllAction;
    QAction* cropAction;
    QAction* saveImageAction;
    QAction* saveCSVAction;
    QAction* loadCSVAction;
    QAction* printAction;
    QAction* zoomInAction;
    QAction* zoomOutAction;
    QAction* resetZoomAction;
    QAction* fitToWidgetAction;
    QAction* tableModeAction;
    QAction* imageModeAction;
    QAction* heatmapModeAction;
    QAction* showValuesAction;
    QAction* showGridAction;
    QAction* showRowLabelsAction;
    QAction* showColLabelsAction;
    
    // Палитры
    QMenu* paletteMenu;
    QActionGroup* paletteGroup;
};

} // namespace proxima

#endif // CENTAURI_MATRIXVIEW_H