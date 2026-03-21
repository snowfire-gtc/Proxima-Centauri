#ifndef CENTAURI_COMPATIBILITYREPORTDIALOG_H
#define CENTAURI_COMPATIBILITYREPORTDIALOG_H

#include <QDialog>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include "core/Project.h"

namespace proxima {

enum class CompatibilityStatus {
    Compatible,
    Warning,
    Error,
    Unknown
};

struct CompatibilityItem {
    QString feature;
    CompatibilityStatus status;
    QString message;
    QString requiredVersion;
    QString currentVersion;
};

class CompatibilityReportDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit CompatibilityReportDialog(Project* project, QWidget *parent = nullptr);
    ~CompatibilityReportDialog();
    
    void checkCompatibility();
    
private slots:
    void onExport();
    void onClose();
    void onRefresh();
    
private:
    void setupUI();
    void populateTree();
    void checkCompilerCompatibility();
    void checkIDECompatibility();
    void checkCapabilityCompatibility();
    void checkDependencyCompatibility();
    QIcon getStatusIcon(CompatibilityStatus status) const;
    QColor getStatusColor(CompatibilityStatus status) const;
    
    Project* project;
    QTreeWidget* tree;
    
    QVector<CompatibilityItem> items;
    
    QPushButton* exportButton;
    QPushButton* refreshButton;
    QPushButton* closeButton;
    
    int compatibleCount;
    int warningCount;
    int errorCount;
};

} // namespace proxima

#endif // CENTAURI_COMPATIBILITYREPORTDIALOG_H