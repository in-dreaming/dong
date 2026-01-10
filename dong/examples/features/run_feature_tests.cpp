/**
 * Feature Test Runner
 * 
 * Runs all CSS/DOM feature tests and generates screenshots + HTML report.
 * 
 * Usage:
 *   ./run_feature_tests [output_dir]
 * 
 * Default output: zig-out/tmp/features/
 */

#include "feature_test_framework.hpp"
#include <SDL3/SDL.h>
#include <cstdio>
#include <string>

int main(int argc, char* argv[]) {
    SDL_Log("=== Dong Feature Test Runner ===\n");
    
    std::string output_dir = "zig-out/tmp/features";
    if (argc > 1) {
        output_dir = argv[1];
    }
    
    SDL_Log("Output directory: %s", output_dir.c_str());
    
    // Create test runner
    dong::test::FeatureTestRunner runner;
    
    // Add all tests
    runner.addTests(dong::test::tests::getAllTests());
    
    // Run all tests
    auto results = runner.runAll(output_dir);
    
    // Generate HTML report
    std::string report_path = output_dir + "/report.html";
    runner.generateReport(results, report_path);
    SDL_Log("\nReport generated: %s", report_path.c_str());
    
    // Return exit code based on results
    int failed = 0;
    for (const auto& r : results) {
        if (!r.passed) ++failed;
    }
    
    return failed > 0 ? 1 : 0;
}
