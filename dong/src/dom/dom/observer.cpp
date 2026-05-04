#include "observer.hpp"
#include "dom_node.hpp"
#include <algorithm>
#include <chrono>

namespace dong::dom {

namespace {
// 时间原点（模块加载时初始化，用于计算相对时间戳）
static auto g_time_origin = std::chrono::steady_clock::now();

// 获取当前时间戳（毫秒），类似于 performance.now()
static double getPerformanceNow() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(now - g_time_origin).count();
}
} // anonymous namespace

// MutationObserver implementation
MutationObserver::MutationObserver(Callback callback) 
    : callback_(std::move(callback)) {
    ObserverRegistry::instance().registerMutationObserver(this);
}

MutationObserver::~MutationObserver() {
    disconnect();
    ObserverRegistry::instance().unregisterMutationObserver(this);
}

void MutationObserver::observe(DOMNodePtr target, const MutationObserverInit& options) {
    if (!target) return;
    
    // Check if already observing this target
    for (auto& obs : observations_) {
        if (obs.first.lock() == target) {
            obs.second = options;
            return;
        }
    }
    
    observations_.emplace_back(target, options);
    connected_ = true;
}

void MutationObserver::disconnect() {
    observations_.clear();
    pendingRecords_.clear();
    connected_ = false;
}

std::vector<MutationRecord> MutationObserver::takeRecords() {
    std::vector<MutationRecord> records = std::move(pendingRecords_);
    pendingRecords_.clear();
    return records;
}

void MutationObserver::notifyMutation(const MutationRecord& record) {
    if (!connected_) return;
    
    for (const auto& obs : observations_) {
        auto target = obs.first.lock();
        if (!target) continue;
        
        const auto& options = obs.second;
        
        // Check if this mutation matches the observation
        bool matches = false;
        
        if (record.type == MutationRecord::Type::CHILD_LIST && options.childList) {
            if (record.target == target || (options.subtree && target->contains(record.target))) {
                matches = true;
            }
        } else if (record.type == MutationRecord::Type::ATTRIBUTES && options.attributes) {
            if (record.target == target || (options.subtree && target->contains(record.target))) {
                if (options.attributeFilter.empty()) {
                    matches = true;
                } else {
                    for (const auto& attr : options.attributeFilter) {
                        if (attr == record.attributeName) {
                            matches = true;
                            break;
                        }
                    }
                }
            }
        } else if (record.type == MutationRecord::Type::CHARACTER_DATA && options.characterData) {
            if (record.target == target || (options.subtree && target->contains(record.target))) {
                matches = true;
            }
        }
        
        if (matches) {
            pendingRecords_.push_back(record);
            break;
        }
    }
    
    // Invoke callback if there are pending records
    if (!pendingRecords_.empty() && callback_) {
        auto records = takeRecords();
        callback_(records, this);
    }
}

// ResizeObserver implementation
ResizeObserver::ResizeObserver(Callback callback)
    : callback_(std::move(callback)) {
    ObserverRegistry::instance().registerResizeObserver(this);
}

ResizeObserver::~ResizeObserver() {
    disconnect();
    ObserverRegistry::instance().unregisterResizeObserver(this);
}

void ResizeObserver::observe(DOMNodePtr target) {
    if (!target) return;
    
    // Check if already observing
    for (const auto& obs : observations_) {
        if (obs.target.lock() == target) {
            return;
        }
    }
    
    ObservationData data;
    data.target = target;
    data.lastWidth = target->getOffsetWidth();
    data.lastHeight = target->getOffsetHeight();
    observations_.push_back(data);
    connected_ = true;
}

void ResizeObserver::unobserve(DOMNodePtr target) {
    observations_.erase(
        std::remove_if(observations_.begin(), observations_.end(),
            [&target](const ObservationData& obs) {
                return obs.target.lock() == target;
            }),
        observations_.end());
    
    if (observations_.empty()) {
        connected_ = false;
    }
}

void ResizeObserver::disconnect() {
    observations_.clear();
    connected_ = false;
}

void ResizeObserver::notifyResize(DOMNodePtr target, float width, float height) {
    if (!connected_ || !callback_) return;
    
    std::vector<ResizeObserverEntry> entries;
    
    for (auto& obs : observations_) {
        auto observedTarget = obs.target.lock();
        if (!observedTarget || observedTarget != target) continue;
        
        // Check if size actually changed
        if (obs.lastWidth == width && obs.lastHeight == height) {
            continue;
        }
        
        obs.lastWidth = width;
        obs.lastHeight = height;
        
        ResizeObserverEntry entry;
        entry.target = target;
        
        auto rect = target->getBoundingClientRect();
        entry.contentRect.x = rect.x;
        entry.contentRect.y = rect.y;
        entry.contentRect.width = width;
        entry.contentRect.height = height;
        entry.contentRect.top = rect.top;
        entry.contentRect.left = rect.left;
        entry.contentRect.right = rect.right;
        entry.contentRect.bottom = rect.bottom;
        
        ResizeObserverEntry::ResizeObserverSize contentSize;
        contentSize.inlineSize = width;
        contentSize.blockSize = height;
        entry.contentBoxSize.push_back(contentSize);
        entry.borderBoxSize.push_back(contentSize);
        entry.devicePixelContentBoxSize.push_back(contentSize);
        
        entries.push_back(entry);
    }
    
    if (!entries.empty()) {
        callback_(entries, this);
    }
}

// IntersectionObserver implementation
IntersectionObserver::IntersectionObserver(Callback callback, const IntersectionObserverInit& options)
    : callback_(std::move(callback)), options_(options) {
    if (options_.threshold.empty()) {
        options_.threshold.push_back(0.0f);
    }
    ObserverRegistry::instance().registerIntersectionObserver(this);
}

IntersectionObserver::~IntersectionObserver() {
    disconnect();
    ObserverRegistry::instance().unregisterIntersectionObserver(this);
}

void IntersectionObserver::observe(DOMNodePtr target) {
    if (!target) return;
    
    // Check if already observing
    for (const auto& obs : observations_) {
        if (obs.target.lock() == target) {
            return;
        }
    }
    
    ObservationData data;
    data.target = target;
    observations_.push_back(data);
    connected_ = true;
}

void IntersectionObserver::unobserve(DOMNodePtr target) {
    observations_.erase(
        std::remove_if(observations_.begin(), observations_.end(),
            [&target](const ObservationData& obs) {
                return obs.target.lock() == target;
            }),
        observations_.end());
    
    if (observations_.empty()) {
        connected_ = false;
    }
}

void IntersectionObserver::disconnect() {
    observations_.clear();
    pendingEntries_.clear();
    connected_ = false;
}

std::vector<IntersectionObserverEntry> IntersectionObserver::takeRecords() {
    std::vector<IntersectionObserverEntry> entries = std::move(pendingEntries_);
    pendingEntries_.clear();
    return entries;
}

void IntersectionObserver::checkIntersections(float viewportX, float viewportY, 
                                               float viewportWidth, float viewportHeight) {
    if (!connected_ || !callback_) return;
    
    std::vector<IntersectionObserverEntry> entries;
    
    for (auto& obs : observations_) {
        auto target = obs.target.lock();
        if (!target) continue;
        
        auto rect = target->getBoundingClientRect();
        
        // Calculate intersection
        float intersectLeft = std::max(rect.left, viewportX);
        float intersectTop = std::max(rect.top, viewportY);
        float intersectRight = std::min(rect.right, viewportX + viewportWidth);
        float intersectBottom = std::min(rect.bottom, viewportY + viewportHeight);
        
        float intersectWidth = std::max(0.0f, intersectRight - intersectLeft);
        float intersectHeight = std::max(0.0f, intersectBottom - intersectTop);
        
        float targetArea = rect.width * rect.height;
        float intersectArea = intersectWidth * intersectHeight;
        float ratio = targetArea > 0 ? intersectArea / targetArea : 0;
        
        bool isIntersecting = intersectArea > 0;
        
        // Check if we crossed a threshold
        bool shouldNotify = false;
        for (float threshold : options_.threshold) {
            bool wasAbove = obs.lastRatio >= threshold;
            bool isAbove = ratio >= threshold;
            if (wasAbove != isAbove) {
                shouldNotify = true;
                break;
            }
        }
        
        if (shouldNotify || obs.wasIntersecting != isIntersecting) {
            obs.wasIntersecting = isIntersecting;
            obs.lastRatio = ratio;
            
            IntersectionObserverEntry entry;
            entry.target = target;
            entry.boundingClientRect.x = rect.x;
            entry.boundingClientRect.y = rect.y;
            entry.boundingClientRect.width = rect.width;
            entry.boundingClientRect.height = rect.height;
            entry.boundingClientRect.top = rect.top;
            entry.boundingClientRect.left = rect.left;
            entry.boundingClientRect.right = rect.right;
            entry.boundingClientRect.bottom = rect.bottom;
            
            entry.intersectionRect.x = intersectLeft;
            entry.intersectionRect.y = intersectTop;
            entry.intersectionRect.width = intersectWidth;
            entry.intersectionRect.height = intersectHeight;
            entry.intersectionRect.top = intersectTop;
            entry.intersectionRect.left = intersectLeft;
            entry.intersectionRect.right = intersectRight;
            entry.intersectionRect.bottom = intersectBottom;
            
            entry.rootBounds.x = viewportX;
            entry.rootBounds.y = viewportY;
            entry.rootBounds.width = viewportWidth;
            entry.rootBounds.height = viewportHeight;
            entry.rootBounds.top = viewportY;
            entry.rootBounds.left = viewportX;
            entry.rootBounds.right = viewportX + viewportWidth;
            entry.rootBounds.bottom = viewportY + viewportHeight;
            
            entry.intersectionRatio = ratio;
            entry.isIntersecting = isIntersecting;
            entry.time = getPerformanceNow();
            
            entries.push_back(entry);
        }
    }
    
    if (!entries.empty()) {
        callback_(entries, this);
    }
}

// ObserverRegistry implementation
ObserverRegistry& ObserverRegistry::instance() {
    static ObserverRegistry registry;
    return registry;
}

void ObserverRegistry::registerMutationObserver(MutationObserver* observer) {
    mutationObservers_.insert(observer);
}

void ObserverRegistry::unregisterMutationObserver(MutationObserver* observer) {
    mutationObservers_.erase(observer);
}

void ObserverRegistry::registerResizeObserver(ResizeObserver* observer) {
    resizeObservers_.insert(observer);
}

void ObserverRegistry::unregisterResizeObserver(ResizeObserver* observer) {
    resizeObservers_.erase(observer);
}

void ObserverRegistry::registerIntersectionObserver(IntersectionObserver* observer) {
    intersectionObservers_.insert(observer);
}

void ObserverRegistry::unregisterIntersectionObserver(IntersectionObserver* observer) {
    intersectionObservers_.erase(observer);
}

void ObserverRegistry::notifyMutation(const MutationRecord& record) {
    for (auto* observer : mutationObservers_) {
        observer->notifyMutation(record);
    }
}

void ObserverRegistry::notifyResize(DOMNodePtr target, float width, float height) {
    for (auto* observer : resizeObservers_) {
        observer->notifyResize(target, width, height);
    }
}

void ObserverRegistry::checkIntersections(float viewportX, float viewportY, 
                                           float viewportWidth, float viewportHeight) {
    for (auto* observer : intersectionObservers_) {
        observer->checkIntersections(viewportX, viewportY, viewportWidth, viewportHeight);
    }
}

void ObserverRegistry::processPendingCallbacks() {
    // Currently callbacks are invoked immediately
    // This method can be used for batching in the future
}

} // namespace dong::dom
