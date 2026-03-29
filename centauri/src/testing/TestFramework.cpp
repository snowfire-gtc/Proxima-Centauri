#include "TestFramework.h"
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <cmath>
#include "utils/Logger.h"

namespace proxima {

TestFramework& TestFramework::getInstance() {
    static TestFramework instance;
    return instance;
}

TestFramework::TestFramework()
    : running(false)
    , isVerbose(false)
    , stopOnFailure(false)
    , repeatCount(1)
    , currentTestCase(nullptr)
    , shouldSkip(false) {
    
    report.projectName = "Proxima Tests";
    auto now = std::time(nullptr);
    report.timestamp = std::chrono::system_clock::from_time_t(now);
    report.totalTests = 0;
    report.totalPassed = 0;
    report.totalFailed = 0;
    report.totalSkipped = 0;
    report.totalErrors = 0;
    report.totalDuration = 0;
}

TestFramework::~TestFramework() {}

void TestFramework::registerSuite(const std::string& name) {
    if (suites.find(name) == suites.end()) {
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

void TestFramework::registerTest(const std::string& suite, const std::string& name,
                                 std::function<void()> testFunc) {
    std::string key = suite + "::" + name;
    testFunctions[key] = testFunc;
    
    auto it = suites.find(suite);
    if (it != suites.end()) {
        TestCase testCase;
        testCase.name = name;
        testCase.suite = suite;
        testCase.result = TestResult::Skipped;
        it->second.testCases.push_back(testCase);
    }
}

void TestFramework::runAllTests() {
    running = true;
    report = TestReport();
    auto now = std::time(nullptr);
    report.timestamp = std::chrono::system_clock::from_time_t(now);
    
    int totalTests = 0;
    for (const auto& suite : suites) {
        totalTests += suite.testCases.size();
    }
    report.totalTests = totalTests * repeatCount;
    
    int currentTest = 0;
    
    for (auto& suitePair : suites) {
        TestSuite& suite = suitePair.second;
        
        for (int r = 0; r < repeatCount; r++) {
            for (size_t i = 0; i < suite.testCases.size(); i++) {
                TestCase& testCase = suite.testCases[i];
                
                // Apply filter
                if (!testFilter.empty() && 
                    testCase.name.find(testFilter) == std::string::npos &&
                    suite.name.find(testFilter) == std::string::npos) {
                    continue;
                }
                
                currentTest++;
                
                executeTest(testCase);
                
                if (stopOnFailure && testCase.result == TestResult::Failed) {
                    running = false;
                    return;
                }
            }
        }
    }
    
    calculateStatistics();
    running = false;
    
    LOG_INFO("Tests completed: " + std::to_string(report.totalPassed) + "/" +
            std::to_string(report.totalTests) + " passed");
}

void TestFramework::runSuite(const std::string& suiteName) {
    auto it = suites.find(suiteName);
    if (it == suites.end()) {
        LOG_ERROR("Test suite not found: " + suiteName);
        return;
    }
    
    running = true;
    TestSuite& suite = it->second;
    
    for (TestCase& testCase : suite.testCases) {
        executeTest(testCase);
        
        if (stopOnFailure && testCase.result == TestResult::Failed) {
            break;
        }
    }
    
    calculateStatistics();
    running = false;
}

void TestFramework::runTest(const std::string& suiteName, const std::string& testName) {
    auto it = suites.find(suiteName);
    if (it == suites.end()) {
        LOG_ERROR("Test suite not found: " + suiteName);
        return;
    }
    
    running = true;
    TestSuite& suite = it->second;
    
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
    auto now = std::time(nullptr);
    testCase.timestamp = std::chrono::system_clock::from_time_t(now);
    shouldSkip = false;
    skipMessage.clear();
    
    auto start = std::chrono::steady_clock::now();
    
    std::string key = testCase.suite + "::" + testCase.name;
    auto it = testFunctions.find(key);
    if (it != testFunctions.end()) {
        try {
            it->second();
        } catch (const std::exception& e) {
            recordResult(testCase, TestResult::Error, std::string(e.what()));
        } catch (...) {
            recordResult(testCase, TestResult::Error, "Unknown exception");
        }
    }
    
    auto end = std::chrono::steady_clock::now();
    testCase.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    if (shouldSkip) {
        testCase.result = TestResult::Skipped;
        testCase.message = skipMessage;
    }
    
    auto suiteIt = suites.find(testCase.suite);
    if (suiteIt != suites.end()) {
        if (testCase.result == TestResult::Passed) {
            suiteIt->second.passed++;
        } else if (testCase.result == TestResult::Failed) {
            suiteIt->second.failed++;
        } else if (testCase.result == TestResult::Skipped) {
            suiteIt->second.skipped++;
        } else {
            suiteIt->second.errors++;
        }
        
        suiteIt->second.totalDuration += testCase.duration;
    }
    
    if (isVerbose) {
        std::string status;
        switch (testCase.result) {
            case TestResult::Passed: status = "PASS"; break;
            case TestResult::Failed: status = "FAIL"; break;
            case TestResult::Skipped: status = "SKIP"; break;
            case TestResult::Error: status = "ERROR"; break;
        }
        LOG_INFO("[" + status + "] " + testCase.suite + "::" + testCase.name + 
                " (" + std::to_string(testCase.duration) + "ms)");
    }
    
    currentTestCase = nullptr;
}

void TestFramework::recordAssertion(const TestAssertion& assertion) {
    if (currentTestCase) {
        currentTestCase->assertions.push_back(assertion);
    }
}

void TestFramework::recordResult(TestCase& testCase, TestResult result, const std::string& message) {
    testCase.result = result;
    testCase.message = message;
}

void TestFramework::assertTrue(bool condition, const std::string& message,
                               const std::string& file, int line) {
    TestAssertion assertion;
    assertion.passed = condition;
    assertion.message = message;
    assertion.file = file;
    assertion.line = line;
    assertion.expression = "assertTrue";
    
    if (!condition) {
        assertion.message = message.empty() ? "Assertion failed" : message;
        recordResult(*currentTestCase, TestResult::Failed, assertion.message);
    }
    
    recordAssertion(assertion);
}

void TestFramework::assertFalse(bool condition, const std::string& message,
                                const std::string& file, int line) {
    assertTrue(!condition, message, file, line);
}

void TestFramework::assertEquals(const std::string& expected, const std::string& actual,
                                 const std::string& message, const std::string& file, int line) {
    TestAssertion assertion;
    assertion.passed = (expected == actual);
    assertion.file = file;
    assertion.line = line;
    assertion.expression = "assertEquals";
    
    if (!assertion.passed) {
        assertion.message = message.empty() ?
            "Expected '" + expected + "', got '" + actual + "'" : message;
        recordResult(*currentTestCase, TestResult::Failed, assertion.message);
    }
    
    recordAssertion(assertion);
}

void TestFramework::assertEquals(int expected, int actual,
                                 const std::string& message, const std::string& file, int line) {
    TestAssertion assertion;
    assertion.passed = (expected == actual);
    assertion.file = file;
    assertion.line = line;
    assertion.expression = "assertEquals";
    
    if (!assertion.passed) {
        assertion.message = message.empty() ?
            "Expected " + std::to_string(expected) + ", got " + std::to_string(actual) : message;
        recordResult(*currentTestCase, TestResult::Failed, assertion.message);
    }
    
    recordAssertion(assertion);
}

void TestFramework::assertEquals(double expected, double actual, double tolerance,
                                 const std::string& message, const std::string& file, int line) {
    TestAssertion assertion;
    assertion.passed = (std::abs(expected - actual) <= tolerance);
    assertion.file = file;
    assertion.line = line;
    assertion.expression = "assertEquals";
    
    if (!assertion.passed) {
        assertion.message = message.empty() ?
            "Expected " + std::to_string(expected) + " ± " + std::to_string(tolerance) + 
            ", got " + std::to_string(actual) : message;
        recordResult(*currentTestCase, TestResult::Failed, assertion.message);
    }
    
    recordAssertion(assertion);
}

void TestFramework::assertNull(void* pointer, const std::string& message,
                               const std::string& file, int line) {
    TestAssertion assertion;
    assertion.passed = (pointer == nullptr);
    assertion.file = file;
    assertion.line = line;
    assertion.expression = "assertNull";
    
    if (!assertion.passed) {
        assertion.message = message.empty() ? "Expected null pointer" : message;
        recordResult(*currentTestCase, TestResult::Failed, assertion.message);
    }
    
    recordAssertion(assertion);
}

void TestFramework::assertNotNull(void* pointer, const std::string& message,
                                  const std::string& file, int line) {
    TestAssertion assertion;
    assertion.passed = (pointer != nullptr);
    assertion.file = file;
    assertion.line = line;
    assertion.expression = "assertNotNull";
    
    if (!assertion.passed) {
        assertion.message = message.empty() ? "Expected non-null pointer" : message;
        recordResult(*currentTestCase, TestResult::Failed, assertion.message);
    }
    
    recordAssertion(assertion);
}

void TestFramework::fail(const std::string& message, const std::string& file, int line) {
    TestAssertion assertion;
    assertion.passed = false;
    assertion.message = message.empty() ? "Test failed" : message;
    assertion.file = file;
    assertion.line = line;
    assertion.expression = "fail";
    
    recordResult(*currentTestCase, TestResult::Failed, assertion.message);
    recordAssertion(assertion);
}

void TestFramework::skip(const std::string& message) {
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
        report.suites.push_back(suite);
        report.totalPassed += suite.passed;
        report.totalFailed += suite.failed;
        report.totalSkipped += suite.skipped;
        report.totalErrors += suite.errors;
        report.totalDuration += suite.totalDuration;
    }
    
    report.totalTests = report.totalPassed + report.totalFailed + 
                       report.totalSkipped + report.totalErrors;
}

std::string TestFramework::getReportHTML() const {
    std::ostringstream html;
    html << "<!DOCTYPE html><html><head><title>Test Report</title>";
    html << "<style>body{font-family:Arial;margin:20px;} ";
    html << ".pass{color:green;} .fail{color:red;} .skip{color:orange;} ";
    html << "table{border-collapse:collapse;width:100%;} ";
    html << "th,td{border:1px solid #ddd;padding:8px;text-align:left;} ";
    html << "th{background-color:#4CAF50;color:white;}</style></head><body>";
    
    html << "<h1>Test Report</h1>";
    html << "<p>Generated: " << std::ctime(&report.timestamp) << "</p>";
    html << "<p>Total: " << report.totalTests << " | ";
    html << "<span class='pass'>Passed: " << report.totalPassed << "</span> | ";
    html << "<span class='fail'>Failed: " << report.totalFailed << "</span> | ";
    html << "<span class='skip'>Skipped: " << report.totalSkipped << "</span></p>";
    
    for (const TestSuite& suite : report.suites) {
        html << "<h2>" << suite.name << "</h2>";
        html << "<table><tr><th>Test</th><th>Result</th><th>Duration</th><th>Message</th></tr>";
        
        for (const TestCase& test : suite.testCases) {
            std::string cssClass;
            switch (test.result) {
                case TestResult::Passed: cssClass = "pass"; break;
                case TestResult::Failed: cssClass = "fail"; break;
                case TestResult::Skipped: cssClass = "skip"; break;
                default: cssClass = "";
            }
            
            html << "<tr class='" << cssClass << "'>";
            html << "<td>" << test.name << "</td>";
            html << "<td>" << static_cast<int>(test.result) << "</td>";
            html << "<td>" << test.duration << "ms</td>";
            html << "<td>" << escapeHTML(test.message) << "</td>";
            html << "</tr>";
        }
        
        html << "</table>";
    }
    
    html << "</body></html>";
    return html.str();
}

std::string TestFramework::getReportText() const {
    std::ostringstream text;
    text << "Test Report\n";
    text << "============\n\n";
    text << "Generated: " << std::ctime(&report.timestamp) << "\n\n";
    text << "Summary:\n";
    text << "  Total:   " << report.totalTests << "\n";
    text << "  Passed:  " << report.totalPassed << "\n";
    text << "  Failed:  " << report.totalFailed << "\n";
    text << "  Skipped: " << report.totalSkipped << "\n";
    text << "  Errors:  " << report.totalErrors << "\n";
    text << "  Duration:" << report.totalDuration << "ms\n\n";
    
    for (const TestSuite& suite : report.suites) {
        text << "Suite: " << suite.name << "\n";
        text << "----------------------------------------\n";
        
        for (const TestCase& test : suite.testCases) {
            std::string status;
            switch (test.result) {
                case TestResult::Passed: status = "PASS"; break;
                case TestResult::Failed: status = "FAIL"; break;
                case TestResult::Skipped: status = "SKIP"; break;
                case TestResult::Error: status = "ERROR"; break;
            }
            
            text << "  [" << status << "] " << test.name << " (" << 
                   test.duration << "ms)\n";
            
            if (!test.message.empty()) {
                text << "    " << test.message << "\n";
            }
        }
        text << "\n";
    }
    
    return text.str();
}

void TestFramework::saveReport(const std::string& path, const std::string& format) {
    std::ofstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("Failed to save report: " + path);
        return;
    }
    
    if (format == "html") {
        file << getReportHTML();
    } else if (format == "xml") {
        file << getReportXML();
    } else {
        file << getReportText();
    }
    
    file.close();
    LOG_INFO("Test report saved: " + path);
}

std::string TestFramework::escapeHTML(const std::string& text) const {
    std::string escaped = text;
    size_t pos = 0;
    while ((pos = escaped.find("&", pos)) != std::string::npos) {
        escaped.replace(pos, 1, "&amp;");
        pos += 5;
    }
    pos = 0;
    while ((pos = escaped.find("<", pos)) != std::string::npos) {
        escaped.replace(pos, 1, "&lt;");
        pos += 4;
    }
    pos = 0;
    while ((pos = escaped.find(">", pos)) != std::string::npos) {
        escaped.replace(pos, 1, "&gt;");
        pos += 4;
    }
    pos = 0;
    while ((pos = escaped.find("\"", pos)) != std::string::npos) {
        escaped.replace(pos, 1, "&quot;");
        pos += 6;
    }
    return escaped;
}

std::string TestFramework::escapeXML(const std::string& text) const {
    return escapeHTML(text);
}

} // namespace proxima