#include "TestFramework.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QFileInfo>
#include <cmath>
#include "utils/Logger.h"

namespace proxima {

TestFramework& TestFramework::getInstance() {
    static TestFramework instance;
    return instance;
}

TestFramework::TestFramework(QObject *parent)
    : QObject(parent)
    , running(false)
    , isVerbose(false)
    , stopOnFailure(false)
    , repeatCount(1)
    , currentTestCase(nullptr)
    , shouldSkip(false) {
    
    report.projectName = "Proxima Tests";
    report.timestamp = QDateTime::currentDateTime();
    report.totalTests = 0;
    report.totalPassed = 0;
    report.totalFailed = 0;
    report.totalSkipped = 0;
    report.totalErrors = 0;
    report.totalDuration = 0;
}

TestFramework::~TestFramework() {}

void TestFramework::registerSuite(const QString& name) {
    if (!suites.contains(name)) {
        TestSuite suite;
        suite.name = name;
        suite.passed = 0;
        suite.failed = 0;
        suite.skipped = 0;
        suite.errors = 0;
        suite.totalDuration = 0;
        suites[name] = suite;
    }
}

void TestFramework::registerTest(const QString& suite, const QString& name,
                                 std::function<void()> testFunc) {
    QString key = suite + "::" + name;
    testFunctions[key] = testFunc;
    
    if (suites.contains(suite)) {
        TestCase testCase;
        testCase.name = name;
        testCase.suite = suite;
        testCase.result = TestResult::Skipped;
        suites[suite].testCases.append(testCase);
    }
}

void TestFramework::runAllTests() {
    running = true;
    report = TestReport();
    report.timestamp = QDateTime::currentDateTime();
    
    int totalTests = 0;
    for (const auto& suite : suites) {
        totalTests += suite.testCases.size();
    }
    report.totalTests = totalTests * repeatCount;
    
    int currentTest = 0;
    
    for (auto& suitePair : suites) {
        TestSuite& suite = suitePair.second;
        
        emit suiteStarted(suite.name);
        
        for (int r = 0; r < repeatCount; r++) {
            for (int i = 0; i < suite.testCases.size(); i++) {
                TestCase& testCase = suite.testCases[i];
                
                // Apply filter
                if (!testFilter.isEmpty() && 
                    !testCase.name.contains(testFilter) &&
                    !suite.name.contains(testFilter)) {
                    continue;
                }
                
                currentTest++;
                emit progressChanged(currentTest, report.totalTests);
                
                executeTest(testCase);
                
                if (stopOnFailure && testCase.result == TestResult::Failed) {
                    running = false;
                    return;
                }
            }
        }
        
        emit suiteFinished(suite.name);
    }
    
    calculateStatistics();
    running = false;
    
    emit allTestsFinished();
    
    LOG_INFO("Tests completed: " + QString::number(report.totalPassed) + "/" +
            QString::number(report.totalTests) + " passed");
}

void TestFramework::runSuite(const QString& suiteName) {
    if (!suites.contains(suiteName)) {
        LOG_ERROR("Test suite not found: " + suiteName);
        return;
    }
    
    running = true;
    TestSuite& suite = suites[suiteName];
    
    emit suiteStarted(suiteName);
    
    for (TestCase& testCase : suite.testCases) {
        executeTest(testCase);
        
        if (stopOnFailure && testCase.result == TestResult::Failed) {
            break;
        }
    }
    
    emit suiteFinished(suiteName);
    
    calculateStatistics();
    running = false;
}

void TestFramework::runTest(const QString& suiteName, const QString& testName) {
    if (!suites.contains(suiteName)) {
        LOG_ERROR("Test suite not found: " + suiteName);
        return;
    }
    
    running = true;
    TestSuite& suite = suites[suiteName];
    
    for (TestCase& testCase : suite.testCases) {
        if (testCase.name == testName) {
            executeTest(testCase);
            break;
        }
    }
    
    calculateStatistics();
    running = false;
}

void TestFramework::stop() {
    running = false;
}

void TestFramework::executeTest(TestCase& testCase) {
    currentSuite = testCase.suite;
    currentTest = testCase.name;
    currentTestCase = &testCase;
    
    testCase.assertions.clear();
    testCase.result = TestResult::Passed;
    testCase.timestamp = QDateTime::currentDateTime();
    shouldSkip = false;
    skipMessage.clear();
    
    emit testStarted(testCase.suite, testCase.name);
    
    auto start = QDateTime::currentMSecsSinceEpoch();
    
    QString key = testCase.suite + "::" + testCase.name;
    if (testFunctions.contains(key)) {
        try {
            testFunctions[key]();
        } catch (const std::exception& e) {
            recordResult(testCase, TestResult::Error, QString::fromUtf8(e.what()));
        } catch (...) {
            recordResult(testCase, TestResult::Error, "Unknown exception");
        }
    }
    
    auto end = QDateTime::currentMSecsSinceEpoch();
    testCase.duration = end - start;
    
    if (shouldSkip) {
        testCase.result = TestResult::Skipped;
        testCase.message = skipMessage;
    }
    
    if (testCase.result == TestResult::Passed) {
        suites[testCase.suite].passed++;
    } else if (testCase.result == TestResult::Failed) {
        suites[testCase.suite].failed++;
    } else if (testCase.result == TestResult::Skipped) {
        suites[testCase.suite].skipped++;
    } else {
        suites[testCase.suite].errors++;
    }
    
    suites[testCase.suite].totalDuration += testCase.duration;
    
    emit testFinished(testCase.suite, testCase.name, testCase.result);
    
    if (isVerbose) {
        QString status;
        switch (testCase.result) {
            case TestResult::Passed: status = "PASS"; break;
            case TestResult::Failed: status = "FAIL"; break;
            case TestResult::Skipped: status = "SKIP"; break;
            case TestResult::Error: status = "ERROR"; break;
        }
        LOG_INFO(QString("[%1] %2::%3 (%4ms)")
            .arg(status, testCase.suite, testCase.name)
            .arg(testCase.duration));
    }
    
    currentTestCase = nullptr;
}

void TestFramework::recordAssertion(const TestAssertion& assertion) {
    if (currentTestCase) {
        currentTestCase->assertions.append(assertion);
        
        if (!assertion.passed) {
            emit assertionFailed(assertion);
        }
    }
}

void TestFramework::recordResult(TestCase& testCase, TestResult result, const QString& message) {
    testCase.result = result;
    testCase.message = message;
}

void TestFramework::assertTrue(bool condition, const QString& message,
                               const QString& file, int line) {
    TestAssertion assertion;
    assertion.passed = condition;
    assertion.message = message;
    assertion.file = file;
    assertion.line = line;
    assertion.expression = "assertTrue";
    
    if (!condition) {
        assertion.message = message.isEmpty() ? "Assertion failed" : message;
        recordResult(*currentTestCase, TestResult::Failed, assertion.message);
    }
    
    recordAssertion(assertion);
}

void TestFramework::assertFalse(bool condition, const QString& message,
                                const QString& file, int line) {
    assertTrue(!condition, message, file, line);
}

void TestFramework::assertEquals(const QString& expected, const QString& actual,
                                 const QString& message, const QString& file, int line) {
    TestAssertion assertion;
    assertion.passed = (expected == actual);
    assertion.file = file;
    assertion.line = line;
    assertion.expression = "assertEquals";
    
    if (!assertion.passed) {
        assertion.message = message.isEmpty() ?
            QString("Expected '%1', got '%2'").arg(expected, actual) : message;
        recordResult(*currentTestCase, TestResult::Failed, assertion.message);
    }
    
    recordAssertion(assertion);
}

void TestFramework::assertEquals(int expected, int actual,
                                 const QString& message, const QString& file, int line) {
    TestAssertion assertion;
    assertion.passed = (expected == actual);
    assertion.file = file;
    assertion.line = line;
    assertion.expression = "assertEquals";
    
    if (!assertion.passed) {
        assertion.message = message.isEmpty() ?
            QString("Expected %1, got %2").arg(expected).arg(actual) : message;
        recordResult(*currentTestCase, TestResult::Failed, assertion.message);
    }
    
    recordAssertion(assertion);
}

void TestFramework::assertEquals(double expected, double actual, double tolerance,
                                 const QString& message, const QString& file, int line) {
    TestAssertion assertion;
    assertion.passed = (std::abs(expected - actual) <= tolerance);
    assertion.file = file;
    assertion.line = line;
    assertion.expression = "assertEquals";
    
    if (!assertion.passed) {
        assertion.message = message.isEmpty() ?
            QString("Expected %1 ± %2, got %3")
                .arg(expected).arg(tolerance).arg(actual) : message;
        recordResult(*currentTestCase, TestResult::Failed, assertion.message);
    }
    
    recordAssertion(assertion);
}

void TestFramework::assertNull(void* pointer, const QString& message,
                               const QString& file, int line) {
    TestAssertion assertion;
    assertion.passed = (pointer == nullptr);
    assertion.file = file;
    assertion.line = line;
    assertion.expression = "assertNull";
    
    if (!assertion.passed) {
        assertion.message = message.isEmpty() ? "Expected null pointer" : message;
        recordResult(*currentTestCase, TestResult::Failed, assertion.message);
    }
    
    recordAssertion(assertion);
}

void TestFramework::assertNotNull(void* pointer, const QString& message,
                                  const QString& file, int line) {
    TestAssertion assertion;
    assertion.passed = (pointer != nullptr);
    assertion.file = file;
    assertion.line = line;
    assertion.expression = "assertNotNull";
    
    if (!assertion.passed) {
        assertion.message = message.isEmpty() ? "Expected non-null pointer" : message;
        recordResult(*currentTestCase, TestResult::Failed, assertion.message);
    }
    
    recordAssertion(assertion);
}

void TestFramework::fail(const QString& message, const QString& file, int line) {
    TestAssertion assertion;
    assertion.passed = false;
    assertion.message = message.isEmpty() ? "Test failed" : message;
    assertion.file = file;
    assertion.line = line;
    assertion.expression = "fail";
    
    recordResult(*currentTestCase, TestResult::Failed, assertion.message);
    recordAssertion(assertion);
}

void TestFramework::skip(const QString& message) {
    shouldSkip = true;
    skipMessage = message;
}

void TestFramework::calculateStatistics() {
    report.totalPassed = 0;
    report.totalFailed = 0;
    report.totalSkipped = 0;
    report.totalErrors = 0;
    report.totalDuration = 0;
    report.suites.clear();
    
    for (const auto& suitePair : suites) {
        const TestSuite& suite = suitePair.second;
        report.suites.append(suite);
        report.totalPassed += suite.passed;
        report.totalFailed += suite.failed;
        report.totalSkipped += suite.skipped;
        report.totalErrors += suite.errors;
        report.totalDuration += suite.totalDuration;
    }
    
    report.totalTests = report.totalPassed + report.totalFailed + 
                       report.totalSkipped + report.totalErrors;
}

QString TestFramework::getReportHTML() const {
    QString html = "<!DOCTYPE html><html><head><title>Test Report</title>";
    html += "<style>body{font-family:Arial;margin:20px;} ";
    html += ".pass{color:green;} .fail{color:red;} .skip{color:orange;} ";
    html += "table{border-collapse:collapse;width:100%;} ";
    html += "th,td{border:1px solid #ddd;padding:8px;text-align:left;} ";
    html += "th{background-color:#4CAF50;color:white;}</style></head><body>";
    
    html += "<h1>Test Report</h1>";
    html += "<p>Generated: " + report.timestamp.toString() + "</p>";
    html += "<p>Total: " + QString::number(report.totalTests) + " | ";
    html += "<span class='pass'>Passed: " + QString::number(report.totalPassed) + "</span> | ";
    html += "<span class='fail'>Failed: " + QString::number(report.totalFailed) + "</span> | ";
    html += "<span class='skip'>Skipped: " + QString::number(report.totalSkipped) + "</span></p>";
    
    for (const TestSuite& suite : report.suites) {
        html += "<h2>" + suite.name + "</h2>";
        html += "<table><tr><th>Test</th><th>Result</th><th>Duration</th><th>Message</th></tr>";
        
        for (const TestCase& test : suite.testCases) {
            QString cssClass;
            switch (test.result) {
                case TestResult::Passed: cssClass = "pass"; break;
                case TestResult::Failed: cssClass = "fail"; break;
                case TestResult::Skipped: cssClass = "skip"; break;
                default: cssClass = "";
            }
            
            html += "<tr class='" + cssClass + "'>";
            html += "<td>" + test.name + "</td>";
            html += "<td>" + QString::number(static_cast<int>(test.result)) + "</td>";
            html += "<td>" + QString::number(test.duration) + "ms</td>";
            html += "<td>" + escapeHTML(test.message) + "</td>";
            html += "</tr>";
        }
        
        html += "</table>";
    }
    
    html += "</body></html>";
    return html;
}

QString TestFramework::getReportText() const {
    QString text = "Test Report\n";
    text += "============\n\n";
    text += "Generated: " + report.timestamp.toString() + "\n\n";
    text += "Summary:\n";
    text += "  Total:   " + QString::number(report.totalTests) + "\n";
    text += "  Passed:  " + QString::number(report.totalPassed) + "\n";
    text += "  Failed:  " + QString::number(report.totalFailed) + "\n";
    text += "  Skipped: " + QString::number(report.totalSkipped) + "\n";
    text += "  Errors:  " + QString::number(report.totalErrors) + "\n";
    text += "  Duration:" + QString::number(report.totalDuration) + "ms\n\n";
    
    for (const TestSuite& suite : report.suites) {
        text += "Suite: " + suite.name + "\n";
        text += QString('-').repeated(40) + "\n";
        
        for (const TestCase& test : suite.testCases) {
            QString status;
            switch (test.result) {
                case TestResult::Passed: status = "PASS"; break;
                case TestResult::Failed: status = "FAIL"; break;
                case TestResult::Skipped: status = "SKIP"; break;
                case TestResult::Error: status = "ERROR"; break;
            }
            
            text += "  [" + status + "] " + test.name + " (" + 
                   QString::number(test.duration) + "ms)\n";
            
            if (!test.message.isEmpty()) {
                text += "    " + test.message + "\n";
            }
        }
        text += "\n";
    }
    
    return text;
}

void TestFramework::saveReport(const QString& path, const QString& format) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR("Failed to save report: " + path);
        return;
    }
    
    QTextStream out(&file);
    
    if (format == "html") {
        out << getReportHTML();
    } else if (format == "xml") {
        out << getReportXML();
    } else {
        out << getReportText();
    }
    
    file.close();
    LOG_INFO("Test report saved: " + path);
}

QString TestFramework::escapeHTML(const QString& text) const {
    QString escaped = text;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    escaped.replace("\"", "&quot;");
    return escaped;
}

QString TestFramework::escapeXML(const QString& text) const {
    return escapeHTML(text);
}

} // namespace proxima