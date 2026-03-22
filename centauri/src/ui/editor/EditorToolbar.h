#ifndef CENTAURI_EDITORTOOLBAR_H
#define CENTAURI_EDITORTOOLBAR_H

#include <QToolBar>
#include <QComboBox>
#include <QAction>
#include <QActionGroup>
#include "editor/CodeEditor.h"

namespace proxima {

class EditorToolbar : public QToolBar {
    Q_OBJECT

public:
    explicit EditorToolbar(QWidget *parent = nullptr);

    void setEditor(CodeEditor* editor);
    CodeEditor* getEditor() const { return editor; }

signals:
    void displayModeChanged(DisplayMode mode);

private slots:
    void onDisplayModeChanged(int index);
    void onAuthorModeSelected();
    void onAgeModeSelected();

private:
    void setupDisplayModeSelector();
    void setupAuthorTools();
    void setupAgeTools();

    CodeEditor* editor;
    QComboBox* displayModeCombo;
    QActionGroup* displayModeGroup;

    // Инструменты для режима авторов
    QComboBox* authorFilterCombo;
    QAction* showAllAuthorsAction;

    // Инструменты для режима возраста
    QComboBox* ageRangeCombo;
    QAction* refreshAgeAction;
};

} // namespace proxima

#endif // CENTAURI_EDITORTOOLBAR_H
