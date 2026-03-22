#include "EditorToolbar.h"
#include <QLabel>
#include <QToolTip>

namespace proxima {

EditorToolbar::EditorToolbar(QWidget *parent)
    : QToolBar(parent)
    , editor(nullptr) {

    setupDisplayModeSelector();
    setupAuthorTools();
    setupAgeTools();
}

void EditorToolbar::setEditor(CodeEditor* editor) {
    this->editor = editor;

    if (editor) {
        connect(this, &EditorToolbar::displayModeChanged,
                editor, &CodeEditor::setDisplayMode);
    }
}

void EditorToolbar::setupDisplayModeSelector() {
    addSeparator();

    // Выпадающий список режимов отображения
    displayModeCombo = new QComboBox(this);
    displayModeCombo->setToolTip("Режим отображения редактора");
    displayModeCombo->addItem("Стандартный", static_cast<int>(DisplayMode::Standard));
    displayModeCombo->addItem("Типы переменных", static_cast<int>(DisplayMode::TypeHighlight));
    displayModeCombo->addItem("Размерность", static_cast<int>(DisplayMode::DimensionHighlight));
    displayModeCombo->addItem("Время выполнения", static_cast<int>(DisplayMode::TimingHighlight));
    displayModeCombo->addItem("Частота выполнения", static_cast<int>(DisplayMode::FrequencyHighlight));
    displayModeCombo->addItem("Git Diff", static_cast<int>(DisplayMode::GitDiff));
    displayModeCombo->addItem("Авторы", static_cast<int>(DisplayMode::AuthorHighlight));
    displayModeCombo->addItem("Возраст правок", static_cast<int>(DisplayMode::AgeHighlight));

    connect(displayModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EditorToolbar::onDisplayModeChanged);

    addWidget(displayModeCombo);
    addSeparator();
}

void EditorToolbar::setupAuthorTools() {
    // Фильтр по авторам (активен только в режиме авторов)
    authorFilterCombo = new QComboBox(this);
    authorFilterCombo->setToolTip("Фильтр по авторам");
    authorFilterCombo->addItem("Все авторы");
    authorFilterCombo->setEnabled(false);

    connect(authorFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (editor && displayModeCombo->currentData().toInt() == static_cast<int>(DisplayMode::AuthorHighlight)) {
            // В полной реализации - фильтрация по автору
        }
    });

    addWidget(authorFilterCombo);

    showAllAuthorsAction = new QAction("Показать всех", this);
    showAllAuthorsAction->setToolTip("Показать всех авторов");
    connect(showAllAuthorsAction, &QAction::triggered, this, [this]() {
        authorFilterCombo->setCurrentIndex(0);
    });
    addAction(showAllAuthorsAction);

    addSeparator();
}

void EditorToolbar::setupAgeTools() {
    // Фильтр по возрасту (активен только в режиме возраста)
    ageRangeCombo = new QComboBox(this);
    ageRangeCombo->setToolTip("Фильтр по возрасту правок");
    ageRangeCombo->addItem("Все правки");
    ageRangeCombo->addItem("< 1 часа");
    ageRangeCombo->addItem("< 1 дня");
    ageRangeCombo->addItem("< 1 недели");
    ageRangeCombo->addItem("< 1 месяца");
    ageRangeCombo->addItem("> 1 месяца");
    ageRangeCombo->setEnabled(false);

    connect(ageRangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (editor && displayModeCombo->currentData().toInt() == static_cast<int>(DisplayMode::AgeHighlight)) {
            // В полной реализации - фильтрация по возрасту
        }
    });

    addWidget(ageRangeCombo);

    refreshAgeAction = new QAction("Обновить", this);
    refreshAgeAction->setToolTip("Обновить возраст правок");
    refreshAgeAction->setIcon(QIcon(":/icons/refresh.svg"));
    connect(refreshAgeAction, &QAction::triggered, this, [this]() {
        if (editor) {
            // В полной реализации - обновление информации о возрасте из Git
            editor->setDisplayMode(DisplayMode::AgeHighlight);
        }
    });
    addAction(refreshAgeAction);

    addSeparator();
}

void EditorToolbar::onDisplayModeChanged(int index) {
    DisplayMode mode = static_cast<DisplayMode>(
        displayModeCombo->itemData(index).toInt());

    emit displayModeChanged(mode);

    // Обновление доступности инструментов
    bool authorMode = (mode == DisplayMode::AuthorHighlight);
    bool ageMode = (mode == DisplayMode::AgeHighlight);

    authorFilterCombo->setEnabled(authorMode);
    ageRangeCombo->setEnabled(ageMode);

    if (editor) {
        editor->setDisplayMode(mode);
    }
}

void EditorToolbar::onAuthorModeSelected() {
    displayModeCombo->setCurrentIndex(6);  // Индекс режима авторов
}

void EditorToolbar::onAgeModeSelected() {
    displayModeCombo->setCurrentIndex(7);  // Индекс режима возраста
}

} // namespace proxima
