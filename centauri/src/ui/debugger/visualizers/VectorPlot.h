#ifndef CENTAURI_VECTORPLOT_H
#define CENTAURI_VECTORPLOT_H

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
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLogValueAxis>
#include "runtime/Runtime.h"
#include "utils/Logger.h"

namespace proxima {

/**
 * @brief Режимы отображения графика
 */
enum class PlotMode {
    Line,           // Линейный график
    Scatter,        // Точечный график
    Spline,         // Сплайн
    Area,           // Областной график
    Bar,            // Столбчатый график
    Step            // Ступенчатый график
};

/**
 * @brief Структура точки данных
 */
struct DataPoint {
    int index;          // Индекс точки (1-based)
    double value;       // Значение
    QString label;      // Метка
    bool isPeak;        // Является ли пиком
    bool isValley;      // Является ли впадиной
    bool isSelected;    // Выбрана ли точка
    bool isHovered;     // Наведён ли курсор
    QPoint screenPos;   // Позиция на экране
    
    DataPoint() : index(0), value(0.0), isPeak(false), 
                  isValley(false), isSelected(false), isHovered(false) {}
};

/**
 * @brief Структура диапазона выделения
 */
struct PlotSelection {
    int startIndex;     // Начальный индекс (1-based)
    int endIndex;       // Конечный индекс (1-based)
    double minValue;    // Минимальное значение в диапазоне
    double maxValue;    // Максимальное значение в диапазоне
    double sumValue;    // Сумма значений
    double meanValue;   // Среднее значение
    
    PlotSelection() : startIndex(0), endIndex(0), 
                      minValue(0.0), maxValue(0.0), 
                      sumValue(0.0), meanValue(0.0) {}
    
    bool isValid() const {
        return startIndex > 0 && endIndex > 0 && startIndex <= endIndex;
    }
    
    int count() const {
        return endIndex - startIndex + 1;
    }
};

/**
 * @brief Структура пика/впадины
 */
struct Extremum {
    int index;          // Индекс (1-based)
    double value;       // Значение
    bool isPeak;        // true для пика, false для впадины
    QString label;      // Метка
    
    Extremum() : index(0), value(0.0), isPeak(false) {}
};

/**
 * @brief Класс визуализатора векторов для IDE Centauri
 * 
 * Согласно требованиям:
 * - ide.txt пункт 10 - инструменты визуализации состояний объектов
 * - ide.txt пункт 10 - для vector - график с zoom, pan, пики/впадины
 * - language.txt пункт 11.1 - индексация с 1
 */
class VectorPlot : public QWidget {
    Q_OBJECT
    
public:
    explicit VectorPlot(QWidget *parent = nullptr);
    ~VectorPlot();
    
    // ========================================================================
    // Данные графика
    // ========================================================================
    
    /**
     * @brief Установка данных вектора
     * @param data Вектор значений
     */
    void setData(const QVector<double>& data);
    
    /**
     * @brief Установка данных из RuntimeValue
     * @param value Значение типа vector
     */
    void setDataFromRuntime(const RuntimeValue& value);
    
    /**
     * @brief Получение данных вектора
     * @return Вектор значений
     */
    QVector<double> getData() const { return data; }
    
    /**
     * @brief Получение количества точек
     * @return Количество точек
     */
    int getCount() const { return data.size(); }
    
    /**
     * @brief Получение значения по индексу
     * @param index Индекс (1-based)
     * @return Значение
     */
    double getValue(int index) const;
    
    /**
     * @brief Установка значения по индексу
     * @param index Индекс (1-based)
     * @param value Новое значение
     */
    void setValue(int index, double value);
    
    // ========================================================================
    // Режимы отображения
    // ========================================================================
    
    /**
     * @brief Установка режима отображения
     * @param mode Режим отображения
     */
    void setPlotMode(PlotMode mode);
    
    /**
     * @brief Получение режима отображения
     * @return Текущий режим
     */
    PlotMode getPlotMode() const { return plotMode; }
    
    /**
     * @brief Установка цвета линии
     * @param color Цвет
     */
    void setLineColor(const QColor& color);
    
    /**
     * @brief Получение цвета линии
     * @return Цвет линии
     */
    QColor getLineColor() const { return lineColor; }
    
    /**
     * @brief Установка толщины линии
     * @param width Толщина
     */
    void setLineWidth(int width);
    
    /**
     * @brief Получение толщины линии
     * @return Толщина
     */
    int getLineWidth() const { return lineWidth; }
    
    /**
     * @brief Включение отображения точек
     * @param show true для отображения
     */
    void setShowPoints(bool show);
    
    /**
     * @brief Проверка отображения точек
     * @return true если точки отображаются
     */
    bool getShowPoints() const { return showPoints; }
    
    /**
     * @brief Установка размера точек
     * @param size Размер
     */
    void setPointSize(int size);
    
    /**
     * @brief Получение размера точек
     * @return Размер
     */
    int getPointSize() const { return pointSize; }
    
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
     * @brief Сброс масштаба
     */
    void resetZoom();
    
    /**
     * @brief Подгонка под размер виджета
     */
    void fitToWidget();
    
    /**
     * @brief Прокрутка к точке
     * @param index Индекс точки (1-based)
     */
    void scrollToIndex(int index);
    
    /**
     * @brief Центрирование на точке
     * @param index Индекс точки (1-based)
     */
    void centerOnIndex(int index);
    
    // ========================================================================
    // Анализ данных
    // ========================================================================
    
    /**
     * @brief Поиск пиков
     */
    void findPeaks();
    
    /**
     * @brief Поиск впадин
     */
    void findValleys();
    
    /**
     * @brief Получение пиков
     * @return Вектор пиков
     */
    QVector<Extremum> getPeaks() const { return peaks; }
    
    /**
     * @brief Получение впадин
     * @return Вектор впадин
     */
    QVector<Extremum> getValleys() const { return valleys; }
    
    /**
     * @brief Очистка пиков и впадин
     */
    void clearExtremums();
    
    /**
     * @brief Подсветка пиков и впадин
     * @param show true для отображения
     */
    void setShowExtremums(bool show);
    
    /**
     * @brief Проверка отображения пиков/впадин
     * @return true если отображаются
     */
    bool getShowExtremums() const { return showExtremums; }
    
    // ========================================================================
    // Выделение
    // ========================================================================
    
    /**
     * @brief Выделение диапазона
     * @param startIndex Начальный индекс (1-based)
     * @param endIndex Конечный индекс (1-based)
     */
    void selectRange(int startIndex, int endIndex);
    
    /**
     * @brief Получение выделенного диапазона
     * @return Диапазон выделения
     */
    PlotSelection getSelection() const { return selection; }
    
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
     * @return Вектор значений
     */
    QVector<double> getSelectedData() const;
    
    /**
     * @brief Копирование выделенных данных
     */
    void copySelection();
    
    /**
     * @brief Вставка данных
     */
    void pasteData();
    
    // ========================================================================
    // Оси и сетка
    // ========================================================================
    
    /**
     * @brief Включение отображения оси X
     * @param show true для отображения
     */
    void setShowXAxis(bool show);
    
    /**
     * @brief Включение отображения оси Y
     * @param show true для отображения
     */
    void setShowYAxis(bool show);
    
    /**
     * @brief Включение отображения сетки
     * @param show true для отображения
     */
    void setShowGrid(bool show);
    
    /**
     * @brief Включение отображения легенды
     * @param show true для отображения
     */
    void setShowLegend(bool show);
    
    /**
     * @brief Установка заголовка графика
     * @param title Заголовок
     */
    void setTitle(const QString& title);
    
    /**
     * @brief Получение заголовка
     * @return Заголовок
     */
    QString getTitle() const { return title; }
    
    /**
     * @brief Установка подписи оси X
     * @param label Подпись
     */
    void setXAxisLabel(const QString& label);
    
    /**
     * @brief Установка подписи оси Y
     * @param label Подпись
     */
    void setYAxisLabel(const QString& label);
    
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
    
    /**
     * @brief Получение стандартного отклонения
     * @return Стандартное отклонение
     */
    double getStdDev() const { return stdDev; }
    
    // ========================================================================
    // Экспорт/Импорт
    // ========================================================================
    
    /**
     * @brief Сохранение как изображение
     * @param path Путь к файлу
     * @param format Формат (PNG, JPG, SVG)
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
     * @brief Сохранение данных в JSON
     * @param path Путь к файлу
     * @return true если успешно
     */
    bool saveAsJSON(const QString& path);
    
    /**
     * @brief Загрузка данных из JSON
     * @param path Путь к файлу
     * @return true если успешно
     */
    bool loadFromJSON(const QString& path);
    
    /**
     * @brief Печать графика
     */
    void print();
    
    /**
     * @brief Копирование в буфер как изображение
     */
    void copyAsImage();
    
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
    
    /**
     * @brief Включение выделения диапазонов
     * @param enable true для включения
     */
    void setSelectionEnabled(bool enable);
    
    /**
     * @brief Проверка режима выделения
     * @return true если выделение включено
     */
    bool isSelectionEnabled() const { return selectionEnabled; }
    
signals:
    void pointClicked(int index, double value);
    void pointDoubleClicked(int index, double value);
    void pointHovered(int index, double value);
    void selectionChanged(const PlotSelection& range);
    void dataModified();
    void zoomChanged(double level);
    void modeChanged(PlotMode mode);
    
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
    void calculatePointPositions();
    void drawPlot(QPainter& painter);
    void drawLinePlot(QPainter& painter);
    void drawScatterPlot(QPainter& painter);
    void drawSplinePlot(QPainter& painter);
    void drawAreaPlot(QPainter& painter);
    void drawBarPlot(QPainter& painter);
    void drawStepPlot(QPainter& painter);
    void drawXAxis(QPainter& painter);
    void drawYAxis(QPainter& painter);
    void drawGrid(QPainter& painter);
    void drawLegend(QPainter& painter);
    void drawSelection(QPainter& painter);
    void drawHoverPoint(QPainter& painter);
    void drawExtremums(QPainter& painter);
    void drawPoint(QPainter& painter, const DataPoint& point);
    
    // ========================================================================
    // Методы навигации
    // ========================================================================
    
    void updateTransform();
    void ensurePointVisible(int index);
    QPoint indexToScreen(int index) const;
    int screenToIndex(const QPoint& screenPos) const;
    QRect getPointRect(int index) const;
    
    // ========================================================================
    // Методы анализа
    // ========================================================================
    
    void calculateStatistics();
    void detectPeaks();
    void detectValleys();
    QString formatValue(double value) const;
    QString formatIndex(int index) const;
    
    // ========================================================================
    // Переменные
    // ========================================================================
    
    // Данные
    QVector<double> data;
    QVector<DataPoint> points;
    int pointCount;
    
    // Режимы
    PlotMode plotMode;
    bool showPoints;
    bool showExtremums;
    bool showGrid;
    bool showXAxis;
    bool showYAxis;
    bool showLegend;
    
    // Стили
    QColor lineColor;
    int lineWidth;
    QColor pointColor;
    int pointSize;
    QColor gridColor;
    QColor backgroundColor;
    QColor selectionColor;
    
    // Масштабирование
    double zoomLevel;
    double minZoom;
    double maxZoom;
    QPointF panOffset;
    QTransform transform;
    
    // Выделение
    PlotSelection selection;
    bool isSelecting;
    QPoint selectionStart;
    bool selectionEnabled;
    
    // Наведение
    int hoverIndex;
    bool isHovering;
    
    // Пики и впадины
    QVector<Extremum> peaks;
    QVector<Extremum> valleys;
    double peakThreshold;
    double valleyThreshold;
    
    // Оси
    QString title;
    QString xAxisLabel;
    QString yAxisLabel;
    bool autoScale;
    double yMin;
    double yMax;
    
    // Статистика
    double minValue;
    double maxValue;
    double meanValue;
    double sumValue;
    double stdDev;
    
    // Интерактивность
    bool interactive;
    bool editable;
    bool isPanning;
    QPoint lastPanPos;
    
    // Контекстное меню
    QMenu* contextMenu;
    QToolBar* toolbar;
    
    // Действия
    QAction* copyAction;
    QAction* pasteAction;
    QAction* selectAllAction;
    QAction* saveImageAction;
    QAction* saveCSVAction;
    QAction* loadCSVAction;
    QAction* saveJSONAction;
    QAction* loadJSONAction;
    QAction* printAction;
    QAction* copyImageAction;
    QAction* zoomInAction;
    QAction* zoomOutAction;
    QAction* resetZoomAction;
    QAction* fitToWidgetAction;
    QAction* lineModeAction;
    QAction* scatterModeAction;
    QAction* splineModeAction;
    QAction* areaModeAction;
    QAction* barModeAction;
    QAction* stepModeAction;
    QAction* showPointsAction;
    QAction* showGridAction;
    QAction* showExtremumsAction;
    QAction* findPeaksAction;
    QAction* findValleysAction;
    QAction* clearExtremumsAction;
    
    // Палитры
    QMenu* colorMenu;
    QActionGroup* colorGroup;
};

} // namespace proxima

#endif // CENTAURI_VECTORPLOT_H