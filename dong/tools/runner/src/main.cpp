#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace fs = std::filesystem;

struct Case {
    const char* name;
    bool needs_data_dir;
    bool set_layer_cache_env;
    int timeout_seconds; // 0 = use default
};

static const Case kCases[] = {
    {"3d_cube", false, false, 5},
    {"3d_screen", false, false, 5},
    {"3d_screen_html", true, false, 10},
    {"3d_screen_script", true, false, 10},

    {"gpu_screenshot_demo", false, false, 10},
    {"gpu_screenshot_demo_fontonly", false, false, 10},
    {"gpu_screenshot_demo_basic_layout", false, false, 10},
    {"gpu_screenshot_demo_glyph_stress", false, false, 15},
    {"gpu_screenshot_analysis", false, false, 10},

    {"interactive_demo", false, true, 5},

    {"integration_demo", false, false, 5},
    {"sdl_gpu_demo", false, false, 5},

    // New plugin-based app
    {"dong_app", false, false, 5},
};

static constexpr int kDefaultTimeoutSeconds = 10;

struct TestResult {
    std::string name;
    int exit_code;
    double duration_ms;
    bool timed_out;
};

static void usage() {
    std::cout
        << "dong_runner - Test runner for Dong demos\n\n"
        << "Usage:\n"
        << "  dong_runner --list\n"
        << "  dong_runner --run-all --bin-dir <dir> [--timeout <seconds>]\n"
        << "  dong_runner --filter <substr> --bin-dir <dir> [--timeout <seconds>]\n"
        << "\nOptions:\n"
        << "  --list          List all available test cases\n"
        << "  --run-all       Run all test cases\n"
        << "  --filter <str>  Run only cases matching <str>\n"
        << "  --bin-dir <dir> Directory containing test executables\n"
        << "  --timeout <sec> Default timeout per test (default: 10)\n"
        << "  --json          Output results in JSON format\n";
}

static void ensure_data_dir(const fs::path& bin_dir, const fs::path& data_src_dir) {
    const fs::path dst = bin_dir / "data";
    if (fs::exists(dst)) {
        return;
    }
    if (!fs::exists(data_src_dir)) {
        std::cerr << "[dong_runner] data source not found: " << data_src_dir << "\n";
        return;
    }
    std::error_code ec;
    fs::create_directories(dst, ec);
    fs::copy(data_src_dir, dst, fs::copy_options::recursive, ec);
    if (ec) {
        std::cerr << "[dong_runner] failed to copy data dir: " << ec.message() << "\n";
    }
}

static TestResult run_one(const Case& c, const fs::path& bin_dir, const fs::path& data_src_dir, int default_timeout) {
    TestResult result;
    result.name = c.name;
    result.exit_code = -1;
    result.duration_ms = 0;
    result.timed_out = false;

    int timeout = c.timeout_seconds > 0 ? c.timeout_seconds : default_timeout;

#if defined(_WIN32)
    if (c.set_layer_cache_env) {
        SetEnvironmentVariableA("DONG_LAYER_CACHE", "1");
    }
#endif

    if (c.needs_data_dir) {
        ensure_data_dir(bin_dir, data_src_dir);
    }

#if defined(_WIN32)
    fs::path exe = bin_dir / (std::string(c.name) + ".exe");
#else
    fs::path exe = bin_dir / c.name;
#endif

    if (!fs::exists(exe)) {
        std::cerr << "[dong_runner] missing exe: " << exe << "\n";
        result.exit_code = 2;
        return result;
    }

    std::cout << "[dong_runner] RUN " << c.name << " (timeout=" << timeout << "s)\n";

    auto start = std::chrono::steady_clock::now();

#if defined(_WIN32)
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};

    std::string cmd = '"' + exe.string() + '"';
    BOOL ok = CreateProcessA(
        nullptr,
        cmd.data(),
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        bin_dir.string().c_str(),
        &si,
        &pi
    );
    if (!ok) {
        std::cerr << "[dong_runner] CreateProcess failed: " << GetLastError() << "\n";
        result.exit_code = 2;
        return result;
    }

    DWORD wait_result = WaitForSingleObject(pi.hProcess, timeout * 1000);
    
    auto end = std::chrono::steady_clock::now();
    result.duration_ms = std::chrono::duration<double, std::milli>(end - start).count();

    if (wait_result == WAIT_TIMEOUT) {
        std::cerr << "[dong_runner] TIMEOUT " << c.name << " after " << timeout << "s\n";
        TerminateProcess(pi.hProcess, 1);
        result.timed_out = true;
        result.exit_code = -1;
    } else {
        DWORD exit_code = 0;
        GetExitCodeProcess(pi.hProcess, &exit_code);
        result.exit_code = (int)exit_code;
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    const char* status = result.timed_out ? "TIMEOUT" : (result.exit_code == 0 ? "PASS" : "FAIL");
    std::cout << "[dong_runner] " << status << " " << c.name 
              << " exit=" << result.exit_code 
              << " time=" << std::fixed << std::setprecision(1) << result.duration_ms << "ms\n";
#else
    // Portable fallback (no timeout support)
    int rc = std::system(exe.string().c_str());
    auto end = std::chrono::steady_clock::now();
    result.duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
    result.exit_code = rc;
    std::cout << "[dong_runner] DONE " << c.name << " rc=" << rc << "\n";
#endif

    return result;
}

static void print_summary(const std::vector<TestResult>& results, bool json_output) {
    int passed = 0, failed = 0, timed_out = 0;
    double total_time = 0;

    for (const auto& r : results) {
        total_time += r.duration_ms;
        if (r.timed_out) {
            timed_out++;
        } else if (r.exit_code == 0) {
            passed++;
        } else {
            failed++;
        }
    }

    if (json_output) {
        std::cout << "{\n  \"results\": [\n";
        for (size_t i = 0; i < results.size(); ++i) {
            const auto& r = results[i];
            std::cout << "    {\"name\": \"" << r.name << "\", "
                      << "\"exit_code\": " << r.exit_code << ", "
                      << "\"duration_ms\": " << std::fixed << std::setprecision(1) << r.duration_ms << ", "
                      << "\"timed_out\": " << (r.timed_out ? "true" : "false") << "}";
            if (i + 1 < results.size()) std::cout << ",";
            std::cout << "\n";
        }
        std::cout << "  ],\n"
                  << "  \"summary\": {\n"
                  << "    \"total\": " << results.size() << ",\n"
                  << "    \"passed\": " << passed << ",\n"
                  << "    \"failed\": " << failed << ",\n"
                  << "    \"timed_out\": " << timed_out << ",\n"
                  << "    \"total_time_ms\": " << std::fixed << std::setprecision(1) << total_time << "\n"
                  << "  }\n}\n";
    } else {
        std::cout << "\n========================================\n"
                  << "Test Summary\n"
                  << "========================================\n"
                  << "Total:     " << results.size() << "\n"
                  << "Passed:    " << passed << "\n"
                  << "Failed:    " << failed << "\n"
                  << "Timed out: " << timed_out << "\n"
                  << "Time:      " << std::fixed << std::setprecision(1) << total_time << " ms\n"
                  << "========================================\n";

        if (failed > 0 || timed_out > 0) {
            std::cout << "\nFailed tests:\n";
            for (const auto& r : results) {
                if (r.timed_out || r.exit_code != 0) {
                    std::cout << "  - " << r.name;
                    if (r.timed_out) {
                        std::cout << " (TIMEOUT)";
                    } else {
                        std::cout << " (exit=" << r.exit_code << ")";
                    }
                    std::cout << "\n";
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    bool list = false;
    bool run_all = false;
    bool json_output = false;
    std::string filter;
    fs::path bin_dir;
    int default_timeout = kDefaultTimeoutSeconds;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--list") {
            list = true;
        } else if (a == "--run-all") {
            run_all = true;
        } else if (a == "--filter" && i + 1 < argc) {
            filter = argv[++i];
        } else if (a == "--bin-dir" && i + 1 < argc) {
            bin_dir = fs::path(argv[++i]);
        } else if (a == "--timeout" && i + 1 < argc) {
            default_timeout = std::atoi(argv[++i]);
        } else if (a == "--json") {
            json_output = true;
        } else if (a == "--help" || a == "-h") {
            usage();
            return 0;
        } else {
            std::cerr << "Unknown option: " << a << "\n";
            usage();
            return 2;
        }
    }

    if (list) {
        for (const auto& c : kCases) {
            std::cout << c.name << "\n";
        }
        return 0;
    }

    if (bin_dir.empty()) {
        usage();
        return 2;
    }

    // Source data dir (from repo) used to satisfy 3d_screen_html/script.
    const fs::path data_src_dir = fs::path("d:/mix/agents/game/indr/dong/dong/examples/data");

    std::vector<TestResult> results;

    for (const auto& c : kCases) {
        if (!filter.empty()) {
            if (std::string(c.name).find(filter) == std::string::npos) {
                continue;
            }
        } else if (!run_all) {
            usage();
            return 2;
        }

        TestResult r = run_one(c, bin_dir, data_src_dir, default_timeout);
        results.push_back(r);
    }

    print_summary(results, json_output);

    // Return non-zero if any test failed
    for (const auto& r : results) {
        if (r.timed_out || r.exit_code != 0) {
            return 1;
        }
    }
    return 0;
}
