#ifndef PROXIMA_TEST_FRAMEWORK_H
#define PROXIMA_TEST_FRAMEWORK_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <memory>

namespace proxima {

enum class TestResult {
    Passed,
    Failed,
    Skipped,
    Error
};

struct TestAssertion {
    bool passed;
    std::string message;
    std::string file;
    int line;
    std::string expression;
};

struct TestCase {
    std::string name;
    std::string suite;
    TestResult result;
    std::string message;
    std::vector<TestAssertion> assertions;
    qint64 duration; // milliseconds
    QDateTime timestamp;
};

struct TestSuite {
    std::string name;
    std::vector<TestCase> testCases;
    int passed;
    int failed;
    int skipped;
    int errors;
    qint64 totalDuration;
};

struct TestReport {
    std::string projectName;
    QDateTime timestamp;
    std::vector<TestSuite> suites;
    int totalTests;
    int totalPassed;
    int totalFailed;
    int totalSkipped;
    int totalErrors;
    qint64 totalDuration;
    std::string compilerVersion;
    std::string ideVersion;
};

class TestFramework {
    
public:
    static TestFramework& getInstance();
    ~TestFramework();
    
    // Test registration
    void registerSuite(const std::string& name);
    void registerTest(const std::string& suite, const std::string& name, 
                     std::function<void()> testFunc);
    
    // Test execution
    void runAllTests();
    void runSuite(const std::string& suiteName);
    void runTest(const std::string& suiteName, const std::string& testName);
    void stop();
    bool isRunning() const { return running; }
    
    // Assertions
    void assertTrue(bool condition, const std::string& message = "",
                   const std::string& file = "", int line = 0);
    void assertFalse(bool condition, const std::string& message = "",
                    const std::string& file = "", int line = 0);
    void assertEquals(const std::string& expected, const std::string& actual,
                     const std::string& message = "", const std::string& file = "", int line = 0);
    void assertEquals(int expected, int actual,
                     const std::string& message = "", const std::string& file = "", int line = 0);
    void assertEquals(double expected, double actual, double tolerance = 1e-10,
                     const std::string& message = "", const std::string& file = "", int line = 0);
    void assertNull(void* pointer, const std::string& message = "",
                   const std::string& file = "", int line = 0);
    void assertNotNull(void* pointer, const std::string& message = "",
                      const std::string& file = "", int line = 0);
    void fail(const std::string& message = "", const std::string& file = "", int line = 0);
    void skip(const std::string& message = "");
    
    // Results
    TestReport getReport() const { return report; }
    std::string getReportHTML() const;
    std::string getReportXML() const;
    std::string getReportText() const;
    void saveReport(const std::string& path, const std::string& format = "html");
    
    // Configuration
    void setVerbose(bool verbose) { isVerbose = verbose; }
    void setStopOnFailure(bool stop) { stopOnFailure = stop; }
    void setRepeat(int count) { repeatCount = count; }
    void setFilter(const std::string& filter) { testFilter = filter; }
    
    // Statistics
    int getTotalTests() const { return report.totalTests; }
    int getPassedTests() const { return report.totalPassed; }
    int getFailedTests() const { return report.totalFailed; }
    
private:
    void executeTest(TestCase& testCase);
    void recordAssertion(const TestAssertion& assertion);
    void recordResult(TestCase& testCase, TestResult result, const std::string& message);
    void calculateStatistics();
    std::string escapeHTML(const std::string& text) const;
    std::string escapeXML(const std::string& text) const;
    
    std::map<std::string, TestSuite> suites;
    std::map<std::string, std::function<void()>> testFunctions;
    
    TestReport report;
    
    std::string currentSuite;
    std::string currentTest;
    TestCase* currentTestCase;
    
    bool running;
    bool isVerbose;
    bool stopOnFailure;
    int repeatCount;
    std::string testFilter;
    
    bool shouldSkip;
    std::string skipMessage;
};

// Macros for test definition
#define TEST_SUITE(name) \
    class name { \
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