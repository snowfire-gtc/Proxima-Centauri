#ifndef PROXIMA_TEST_FRAMEWORK_H
#define PROXIMA_TEST_FRAMEWORK_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QDateTime>
#include <functional>

namespace proxima {

enum class TestResult {
    Passed,
    Failed,
    Skipped,
    Error
};

struct TestAssertion {
    bool passed;
    QString message;
    QString file;
    int line;
    QString expression;
};

struct TestCase {
    QString name;
    QString suite;
    TestResult result;
    QString message;
    QVector<TestAssertion> assertions;
    qint64 duration; // milliseconds
    QDateTime timestamp;
};

struct TestSuite {
    QString name;
    QVector<TestCase> testCases;
    int passed;
    int failed;
    int skipped;
    int errors;
    qint64 totalDuration;
};

struct TestReport {
    QString projectName;
    QDateTime timestamp;
    QVector<TestSuite> suites;
    int totalTests;
    int totalPassed;
    int totalFailed;
    int totalSkipped;
    int totalErrors;
    qint64 totalDuration;
    QString compilerVersion;
    QString ideVersion;
};

class TestFramework : public QObject {
    Q_OBJECT
    
public:
    explicit TestFramework(QObject *parent = nullptr);
    ~TestFramework();
    
    // Test registration
    void registerSuite(const QString& name);
    void registerTest(const QString& suite, const QString& name, 
                     std::function<void()> testFunc);
    
    // Test execution
    void runAllTests();
    void runSuite(const QString& suiteName);
    void runTest(const QString& suiteName, const QString& testName);
    void stop();
    bool isRunning() const { return running; }
    
    // Assertions
    void assertTrue(bool condition, const QString& message = "",
                   const QString& file = "", int line = 0);
    void assertFalse(bool condition, const QString& message = "",
                    const QString& file = "", int line = 0);
    void assertEquals(const QString& expected, const QString& actual,
                     const QString& message = "", const QString& file = "", int line = 0);
    void assertEquals(int expected, int actual,
                     const QString& message = "", const QString& file = "", int line = 0);
    void assertEquals(double expected, double actual, double tolerance = 1e-10,
                     const QString& message = "", const QString& file = "", int line = 0);
    void assertNull(void* pointer, const QString& message = "",
                   const QString& file = "", int line = 0);
    void assertNotNull(void* pointer, const QString& message = "",
                      const QString& file = "", int line = 0);
    void fail(const QString& message = "", const QString& file = "", int line = 0);
    void skip(const QString& message = "");
    
    // Results
    TestReport getReport() const { return report; }
    QString getReportHTML() const;
    QString getReportXML() const;
    QString getReportText() const;
    void saveReport(const QString& path, const QString& format = "html");
    
    // Configuration
    void setVerbose(bool verbose) { isVerbose = verbose; }
    void setStopOnFailure(bool stop) { stopOnFailure = stop; }
    void setRepeat(int count) { repeatCount = count; }
    void setFilter(const QString& filter) { testFilter = filter; }
    
    // Statistics
    int getTotalTests() const { return report.totalTests; }
    int getPassedTests() const { return report.totalPassed; }
    int getFailedTests() const { return report.totalFailed; }
    
signals:
    void testStarted(const QString& suite, const QString& test);
    void testFinished(const QString& suite, const QString& test, TestResult result);
    void suiteStarted(const QString& suite);
    void suiteFinished(const QString& suite);
    void allTestsFinished();
    void assertionFailed(const TestAssertion& assertion);
    void progressChanged(int current, int total);
    
private:
    void executeTest(TestCase& testCase);
    void recordAssertion(const TestAssertion& assertion);
    void recordResult(TestCase& testCase, TestResult result, const QString& message);
    void calculateStatistics();
    QString escapeHTML(const QString& text) const;
    QString escapeXML(const QString& text) const;
    
    QMap<QString, TestSuite> suites;
    QMap<QString, std::function<void()>> testFunctions;
    
    TestReport report;
    
    QString currentSuite;
    QString currentTest;
    TestCase* currentTestCase;
    
    bool running;
    bool isVerbose;
    bool stopOnFailure;
    int repeatCount;
    QString testFilter;
    
    bool shouldSkip;
    QString skipMessage;
};

// Macros for test definition
#define TEST_SUITE(name) \
    class name : public QObject { \
        Q_OBJECT \
    public: \
        name() { TestFramework::getInstance().registerSuite(#name); }
#define TEST(name) \
    public: \
        void name()
#define TEST_END \
    }; \
    static name instance;

#define ASSERT_TRUE(cond) TestFramework::getInstance().assertTrue(cond, #cond, __FILE__, __LINE__)
#define ASSERT_FALSE(cond) TestFramework::getInstance().assertFalse(cond, #cond, __FILE__, __LINE__)
#define ASSERT_EQ(expected, actual) TestFramework::getInstance().assertEquals(expected, actual, #expected " == " #actual, __FILE__, __LINE__)
#define ASSERT_NEAR(expected, actual, tol) TestFramework::getInstance().assertEquals(expected, actual, tol, #expected " ≈ " #actual, __FILE__, __LINE__)
#define ASSERT_NULL(ptr) TestFramework::getInstance().assertNull(ptr, #ptr, __FILE__, __LINE__)
#define ASSERT_NOT_NULL(ptr) TestFramework::getInstance().assertNotNull(ptr, #ptr, __FILE__, __LINE__)
#define FAIL(msg) TestFramework::getInstance().fail(msg, __FILE__, __LINE__)
#define SKIP(msg) TestFramework::getInstance().skip(msg)

} // namespace proxima

#endif // PROXIMA_TEST_FRAMEWORK_H