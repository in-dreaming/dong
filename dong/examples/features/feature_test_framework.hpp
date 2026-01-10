#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace dong::test {

// Feature test result
struct FeatureTestResult {
    std::string name;
    std::string category;
    bool passed = false;
    std::string description;
    std::string screenshot_path;
    std::string error_message;
};

// Feature test case
struct FeatureTestCase {
    std::string name;
    std::string category;
    std::string html;
    std::string description;
    int width = 800;
    int height = 600;
    
    // Optional validation function (can be used for pixel analysis)
    std::function<bool(const uint8_t* pixels, int width, int height)> validator;
};

// Feature test runner
class FeatureTestRunner {
public:
    FeatureTestRunner();
    ~FeatureTestRunner();
    
    // Add test cases
    void addTest(const FeatureTestCase& test);
    void addTests(const std::vector<FeatureTestCase>& tests);
    
    // Run all tests
    std::vector<FeatureTestResult> runAll(const std::string& output_dir = "zig-out/tmp/features");
    
    // Run single test
    FeatureTestResult runTest(const FeatureTestCase& test, const std::string& output_dir);
    
    // Generate HTML report
    void generateReport(const std::vector<FeatureTestResult>& results, const std::string& output_path);

private:
    std::vector<FeatureTestCase> tests_;
    
    bool initializeRenderer(int width, int height);
    void cleanupRenderer();
    bool renderToPixels(const std::string& html, int width, int height, std::vector<uint8_t>& pixels);
    bool saveBMP(const std::string& path, int width, int height, const uint8_t* pixels);
};

// Pre-defined feature test cases
namespace tests {

// CSS Box Model tests
std::vector<FeatureTestCase> getBoxModelTests();

// CSS Flexbox tests
std::vector<FeatureTestCase> getFlexboxTests();

// CSS Transform tests
std::vector<FeatureTestCase> getTransformTests();

// CSS Transition/Animation tests
std::vector<FeatureTestCase> getAnimationTests();

// CSS Visual Effects tests (shadows, gradients, filters)
std::vector<FeatureTestCase> getVisualEffectsTests();

// CSS Text tests
std::vector<FeatureTestCase> getTextTests();

// DOM API tests
std::vector<FeatureTestCase> getDOMAPITests();

// Get all tests
std::vector<FeatureTestCase> getAllTests();

} // namespace tests

} // namespace dong::test
