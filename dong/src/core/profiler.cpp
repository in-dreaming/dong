#include "profiler.h"

#include <atomic>
#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>
#include <unordered_map>

#if DONG_PROFILER_ENABLED

// ============================================================================
// 内部数据结构
// ============================================================================

namespace {

// Chrome Trace 事件类型
enum class EventType : uint8_t {
    Begin = 'B',      // Duration event begin
    End = 'E',        // Duration event end
    Instant = 'i',    // Instant event
    FrameBegin = 's', // 内部用：帧开始
    FrameEnd = 'f',   // 内部用：帧结束
};

// 单个事件记录
struct ProfileEvent {
    const char* name;        // 事件名（必须是静态字符串）
    const char* category;    // 分类
    int64_t timestamp_us;    // 微秒时间戳（相对于 profiler 启动）
    uint32_t thread_id;      // 线程 ID
    EventType type;          // 事件类型
    uint8_t padding[3];
};

// 每线程缓冲区
struct ThreadBuffer {
    std::vector<ProfileEvent> events;
    uint32_t thread_id = 0;
    
    ThreadBuffer() {
        events.reserve(16384);  // 预分配避免频繁扩容
    }
};

// 全局状态
struct ProfilerState {
    std::atomic<bool> initialized{false};
    std::atomic<bool> enabled{true};
    std::atomic<uint64_t> frame_count{0};
    
    // 起始时间点
    std::chrono::high_resolution_clock::time_point start_time;
    
    // 线程缓冲区管理
    std::mutex buffers_mutex;
    std::vector<ThreadBuffer*> all_buffers;
    
    // 线程局部缓冲区
    static thread_local ThreadBuffer* tls_buffer;
    
    ProfilerState() = default;
    
    ~ProfilerState() {
        // 清理所有缓冲区
        std::lock_guard<std::mutex> lock(buffers_mutex);
        for (auto* buf : all_buffers) {
            delete buf;
        }
        all_buffers.clear();
    }
};

thread_local ThreadBuffer* ProfilerState::tls_buffer = nullptr;

// 全局单例
ProfilerState& getState() {
    static ProfilerState state;
    return state;
}

// 获取当前线程 ID（简化版）
uint32_t getCurrentThreadId() {
    static std::atomic<uint32_t> next_id{1};
    static thread_local uint32_t tls_id = 0;
    if (tls_id == 0) {
        tls_id = next_id.fetch_add(1, std::memory_order_relaxed);
    }
    return tls_id;
}

// 获取当前时间戳（微秒）
int64_t getTimestampUs() {
    auto& state = getState();
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(
        now - state.start_time
    ).count();
}

// 获取或创建当前线程的缓冲区
ThreadBuffer* getThreadBuffer() {
    auto& state = getState();
    
    if (!ProfilerState::tls_buffer) {
        auto* buf = new ThreadBuffer();
        buf->thread_id = getCurrentThreadId();
        
        std::lock_guard<std::mutex> lock(state.buffers_mutex);
        state.all_buffers.push_back(buf);
        ProfilerState::tls_buffer = buf;
    }
    
    return ProfilerState::tls_buffer;
}

// 记录事件
void recordEvent(const char* name, const char* category, EventType type) {
    auto& state = getState();
    
    if (!state.enabled.load(std::memory_order_relaxed)) {
        return;
    }
    
    // 确保已初始化
    if (!state.initialized.load(std::memory_order_acquire)) {
        dong_profiler_init();
    }
    
    ThreadBuffer* buf = getThreadBuffer();
    
    ProfileEvent event;
    event.name = name;
    event.category = category;
    event.timestamp_us = getTimestampUs();
    event.thread_id = buf->thread_id;
    event.type = type;
    
    buf->events.push_back(event);
}

// 转义 JSON 字符串
void writeJsonString(FILE* fp, const char* str) {
    fputc('"', fp);
    if (str) {
        for (const char* p = str; *p; ++p) {
            switch (*p) {
                case '"':  fputs("\\\"", fp); break;
                case '\\': fputs("\\\\", fp); break;
                case '\n': fputs("\\n", fp); break;
                case '\r': fputs("\\r", fp); break;
                case '\t': fputs("\\t", fp); break;
                default:   fputc(*p, fp); break;
            }
        }
    }
    fputc('"', fp);
}

} // anonymous namespace

// ============================================================================
// C API 实现
// ============================================================================

extern "C" {

void dong_profiler_init(void) {
    auto& state = getState();

    // 无论是否已初始化，都重置时间戳并清空已有事件，避免“自动初始化”把早期噪声写进 trace
    state.start_time = std::chrono::high_resolution_clock::now();
    state.frame_count.store(0, std::memory_order_relaxed);

    {
        std::lock_guard<std::mutex> lock(state.buffers_mutex);
        for (auto* buf : state.all_buffers) {
            buf->events.clear();
        }
    }

    state.initialized.store(true, std::memory_order_release);
}

void dong_profiler_shutdown(void) {
    auto& state = getState();
    
    if (state.initialized.exchange(false, std::memory_order_acq_rel)) {
        std::lock_guard<std::mutex> lock(state.buffers_mutex);
        for (auto* buf : state.all_buffers) {
            delete buf;
        }
        state.all_buffers.clear();
        ProfilerState::tls_buffer = nullptr;
    }
}

void dong_profiler_begin(const char* name, const char* category) {
    recordEvent(name, category, EventType::Begin);
}

void dong_profiler_end(void) {
    recordEvent(nullptr, nullptr, EventType::End);
}

void dong_profiler_instant(const char* name, const char* category) {
    recordEvent(name, category, EventType::Instant);
}

void dong_profiler_frame_begin(void) {
    auto& state = getState();
    state.frame_count.fetch_add(1, std::memory_order_relaxed);
    recordEvent("Frame", "frame", EventType::Begin);
}

void dong_profiler_frame_end(void) {
    recordEvent("Frame", "frame", EventType::End);
}

int dong_profiler_dump(const char* filepath) {
    auto& state = getState();
    
    if (!state.initialized.load(std::memory_order_acquire)) {
        return -1;
    }
    
    FILE* fp = fopen(filepath, "w");
    if (!fp) {
        return -1;
    }
    
    // Chrome Trace JSON 格式
    fputs("{\"traceEvents\":[\n", fp);
    
    bool first = true;
    
    // 收集所有线程的事件
    std::lock_guard<std::mutex> lock(state.buffers_mutex);
    
    for (const auto* buf : state.all_buffers) {
        struct StackEntry {
            const char* name;
            const char* category;
        };

        // 用于匹配 begin/end 的栈（同时恢复 name + category，保证 Chrome Trace 能配对统计）
        std::vector<StackEntry> stack;
        
        for (const auto& event : buf->events) {
            if (!first) {
                fputs(",\n", fp);
            }
            first = false;
            
            const char* name = event.name;
            const char* category = event.category;
            char ph = static_cast<char>(event.type);
            
            if (event.type == EventType::Begin) {
                stack.push_back({event.name, event.category ? event.category : "default"});
                ph = 'B';
                category = event.category ? event.category : "default";
            } else if (event.type == EventType::End) {
                if (!stack.empty()) {
                    const auto top = stack.back();
                    stack.pop_back();
                    name = top.name ? top.name : "";
                    category = top.category ? top.category : "default";
                } else {
                    name = "unknown";
                    category = "default";
                }
                ph = 'E';
            } else if (event.type == EventType::Instant) {
                ph = 'i';
                category = event.category ? event.category : "default";
            }
            
            // 写入事件
            fputs("{", fp);
            fputs("\"name\":", fp);
            writeJsonString(fp, name ? name : "");
            fputs(",\"cat\":", fp);
            writeJsonString(fp, category ? category : "default");
            fprintf(fp, ",\"ph\":\"%c\"", ph);
            fprintf(fp, ",\"ts\":%" PRId64, event.timestamp_us);
            fprintf(fp, ",\"pid\":1");
            fprintf(fp, ",\"tid\":%u", event.thread_id);
            
            // Instant 事件需要 scope
            if (event.type == EventType::Instant) {
                fputs(",\"s\":\"g\"", fp);  // global scope
            }
            
            fputs("}", fp);
        }
    }
    
    // 添加元数据事件
    if (!first) {
        fputs(",\n", fp);
    }
    
    // 进程名
    fputs("{\"name\":\"process_name\",\"ph\":\"M\",\"pid\":1,\"args\":{\"name\":\"Dong Engine\"}}", fp);
    
    // 线程名
    for (const auto* buf : state.all_buffers) {
        fprintf(fp, ",\n{\"name\":\"thread_name\",\"ph\":\"M\",\"pid\":1,\"tid\":%u,\"args\":{\"name\":\"Thread %u\"}}",
                buf->thread_id, buf->thread_id);
    }
    
    fputs("\n]}\n", fp);
    fclose(fp);
    
    return 0;
}

void dong_profiler_clear(void) {
    auto& state = getState();
    
    std::lock_guard<std::mutex> lock(state.buffers_mutex);
    for (auto* buf : state.all_buffers) {
        buf->events.clear();
    }
    state.frame_count.store(0, std::memory_order_relaxed);
}

uint64_t dong_profiler_get_frame(void) {
    return getState().frame_count.load(std::memory_order_relaxed);
}

void dong_profiler_set_enabled(int enabled) {
    getState().enabled.store(enabled != 0, std::memory_order_relaxed);
}

int dong_profiler_is_enabled(void) {
    return getState().enabled.load(std::memory_order_relaxed) ? 1 : 0;
}

} // extern "C"

#else // !DONG_PROFILER_ENABLED

// Stub implementations when profiler is disabled
extern "C" {
void dong_profiler_init(void) {}
void dong_profiler_shutdown(void) {}
void dong_profiler_begin(const char*, const char*) {}
void dong_profiler_end(void) {}
void dong_profiler_instant(const char*, const char*) {}
void dong_profiler_frame_begin(void) {}
void dong_profiler_frame_end(void) {}
int dong_profiler_dump(const char*) { return -1; }
void dong_profiler_clear(void) {}
uint64_t dong_profiler_get_frame(void) { return 0; }
void dong_profiler_set_enabled(int) {}
int dong_profiler_is_enabled(void) { return 0; }
}

#endif // DONG_PROFILER_ENABLED
