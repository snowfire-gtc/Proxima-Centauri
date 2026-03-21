#ifndef CENTAURI_BUILDREPORTDIALOG_H
#define CENTAURI_BUILDREPORTDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "core/Project.h"

namespace proxima {

struct BuildReport {
    QString projectName;
    QString version;
    QString buildHash;
    QString buildTimestamp;
    QString compilerVersion;
    QString ideVersion;
    qint64 buildDuration;
    int moduleCount;
    int totalLines;
    qint64 outputSize;
    QVector<QString> warnings;
    QVector<QString> errors;
    QMap<QString, QString> participants;
    QMap<QString, bool> capabilities;
};

class BuildReportDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit BuildReportDialog(Project* project, QWidget *parent = nullptr);
    ~BuildReport();
    
    void generateReport();
    void exportToMarkdown(const QString& path);
    void exportToPDF(const QString& path);
    void exportToHTML(const QString& path);
    
private slots:
    void onExportMD();
    void onExportPDF();
    void onExportHTML();
    void onCopy();
    void onClose();
    void onRefresh();
    
private:
    void setupUI();
    void populateReport();
    QString formatReport() const;
    QString generateMD5Hash() const;
    QString formatTimestamp(const QDateTime& dt) const;
    
    Project* project;
    BuildReport report;
    
    QTextEdit* reportText;
    QTableWidget* participantsTable;
    QTableWidget* capabilitiesTable;
    
    QPushButton* exportMDButton;
    QPushButton* exportPDFButton;
    QPushButton* exportHTMLButton;
    QPushButton* copyButton;
    QPushButton* refreshButton;
    QPushButton* closeButton;
};

} // namespace proxima

#endif // CENTAURI_BUILDREPORTDIALOG_H