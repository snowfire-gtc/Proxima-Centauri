// Добавить в private секцию класса DocGenerator:

private:
    // ... существующие методы ...
    
    // Извлечение документации
    void extractDocFromComments(DeclarationNodePtr decl, DocItem& item);
    QString getSourceCodeForFile(const QString& filename) const;
    QString extractBlockComment(const QStringList& lines, int endLine);
    void parseDocComment(const QString& comment, DocItem& item);
    void extractDocFromBody(DeclarationNodePtr decl, DocItem& item);
    
    // Обработка statement'ов
    void processStatement(StatementNodePtr stmt, const QString& filename);
    void extractDocFromCondition(ExpressionNodePtr condition, const QString& filename, int line);
    void extractDocFromLoop(StatementNodePtr loopStmt, const QString& filename);
    void extractDocFromExpression(ExpressionNodePtr expr, const QString& filename, int line);
    
    // Обработка специальных конструкций
    void extractRegionDocumentation(const QString& content, DocModule& module, const QString& filename);
    void extractNamedBlockComments(const QString& content, DocModule& module, const QString& filename);
    void extractGEMInterfaceDocumentation(DocItem& item);
    void extractCodeExamples(const QString& comment, DocItem& item);
    void extractCrossReferences(DocItem& item);
    void extractLatexFormulas(DocItem& item);
    
    // Валидация и утилиты
    bool validateDocItem(DocItem& item);
    void updateStatistics();
    void setTypeChecker(TypeChecker* checker);
    void extractTypeInfo(DocItem& item);
    QString typeCategoryToString(TypeCategory category) const;
    
    TypeChecker* typeChecker;