#ifndef CENTAURI_MODULE_H
#define CENTAURI_MODULE_H

#include <QObject>
#include <QString>
#include <QDateTime>

namespace proxima {

struct ModuleInfo {
    QString name;
    QString path;
    QString namespace_;
    int lineCount;
    qint64 size;
    QDateTime lastModified;
    QDateTime lastSaved;
    bool isModified;
    bool isReadOnly;
};

class Module : public QObject {
    Q_OBJECT
    
public:
    explicit Module(QObject *parent = nullptr);
    ~Module();
    
    // File operations
    bool load(const QString& path);
    bool save();
    bool saveAs(const QString& path);
    
    // Accessors
    QString getName() const { return info.name; }
    QString getPath() const { return info.path; }
    QString getNamespace() const { return info.namespace_; }
    QString getContent() const { return content; }
    int getLineCount() const { return info.lineCount; }
    qint64 getSize() const { return info.size; }
    bool isModified() const { return info.isModified; }
    bool isReadOnly() const { return info.isReadOnly; }
    QDateTime getLastModified() const { return info.lastModified; }
    QDateTime getLastSaved() const { return info.lastSaved; }
    ModuleInfo getInfo() const { return info; }
    
    // Content manipulation
    void setContent(const QString& content);
    void insertText(int position, const QString& text);
    void removeText(int start, int end);
    QString getLine(int line) const;
    QString getLines(int start, int end) const;
    
    // Analysis
    void analyze();
    QStringList getDependencies() const { return dependencies; }
    QStringList getFunctions() const { return functions; }
    QStringList getClasses() const { return classes; }
    
    // Versioning
    void markSaved();
    void markModified();
    
signals:
    void loaded(const QString& path);
    void saved(const QString& path);
    void modified();
    void contentChanged();
    void dependenciesChanged(const QStringList& deps);
    
private:
    void parseContent();
    void extractDependencies();
    void extractFunctions();
    void extractClasses();
    void updateInfo();
    int countLines(const QString& text) const;
    
    ModuleInfo info;
    QString content;
    QStringList dependencies;
    QStringList functions;
    QStringList classes;
};

} // namespace proxima

#endif // CENTAURI_MODULE_H
