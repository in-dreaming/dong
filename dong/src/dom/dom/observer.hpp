#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_set>

namespace dong::dom {

class DOMNode;
using DOMNodePtr = std::shared_ptr<DOMNode>;
using DOMNodeWeakPtr = std::weak_ptr<DOMNode>;

// MutationRecord - represents a single DOM mutation
struct MutationRecord {
    enum class Type {
        ATTRIBUTES,
        CHARACTER_DATA,
        CHILD_LIST
    };

    Type type;
    DOMNodePtr target;
    std::vector<DOMNodePtr> addedNodes;
    std::vector<DOMNodePtr> removedNodes;
    DOMNodePtr previousSibling;
    DOMNodePtr nextSibling;
    std::string attributeName;
    std::string attributeNamespace;
    std::string oldValue;
};

// MutationObserverInit - options for MutationObserver
struct MutationObserverInit {
    bool childList = false;
    bool attributes = false;
    bool characterData = false;
    bool subtree = false;
    bool attributeOldValue = false;
    bool characterDataOldValue = false;
    std::vector<std::string> attributeFilter;
};

// MutationObserver - observes DOM mutations
class MutationObserver {
public:
    using Callback = std::function<void(const std::vector<MutationRecord>&, MutationObserver*)>;

    explicit MutationObserver(Callback callback);
    ~MutationObserver();

    void observe(DOMNodePtr target, const MutationObserverInit& options);
    void disconnect();
    std::vector<MutationRecord> takeRecords();

    // Internal: called by DOM to notify mutations
    void notifyMutation(const MutationRecord& record);

private:
    Callback callback_;
    std::vector<std::pair<DOMNodeWeakPtr, MutationObserverInit>> observations_;
    std::vector<MutationRecord> pendingRecords_;
    bool connected_ = false;
};

// ResizeObserverEntry - represents a resize observation
struct ResizeObserverEntry {
    DOMNodePtr target;

    struct DOMRectReadOnly {
        float x = 0, y = 0, width = 0, height = 0;
        float top = 0, right = 0, bottom = 0, left = 0;
    };

    DOMRectReadOnly contentRect;

    struct ResizeObserverSize {
        float inlineSize = 0;
        float blockSize = 0;
    };

    std::vector<ResizeObserverSize> contentBoxSize;
    std::vector<ResizeObserverSize> borderBoxSize;
    std::vector<ResizeObserverSize> devicePixelContentBoxSize;
};

// ResizeObserver - observes element size changes
class ResizeObserver {
public:
    using Callback = std::function<void(const std::vector<ResizeObserverEntry>&, ResizeObserver*)>;

    explicit ResizeObserver(Callback callback);
    ~ResizeObserver();

    void observe(DOMNodePtr target);
    void unobserve(DOMNodePtr target);
    void disconnect();

    // Internal: called by layout engine to notify size changes
    void notifyResize(DOMNodePtr target, float width, float height);

private:
    struct ObservationData {
        DOMNodeWeakPtr target;
        float lastWidth = 0;
        float lastHeight = 0;
    };

    Callback callback_;
    std::vector<ObservationData> observations_;
    bool connected_ = false;
};

// IntersectionObserverEntry - represents an intersection observation
struct IntersectionObserverEntry {
    DOMNodePtr target;

    struct DOMRectReadOnly {
        float x = 0, y = 0, width = 0, height = 0;
        float top = 0, right = 0, bottom = 0, left = 0;
    };

    DOMRectReadOnly boundingClientRect;
    DOMRectReadOnly intersectionRect;
    DOMRectReadOnly rootBounds;
    float intersectionRatio = 0;
    bool isIntersecting = false;
    double time = 0;
};

// IntersectionObserverInit - options for IntersectionObserver
struct IntersectionObserverInit {
    DOMNodePtr root;  // null means viewport
    std::string rootMargin = "0px";
    std::vector<float> threshold = {0.0f};
};

// IntersectionObserver - observes element visibility
class IntersectionObserver {
public:
    using Callback = std::function<void(const std::vector<IntersectionObserverEntry>&, IntersectionObserver*)>;

    explicit IntersectionObserver(Callback callback, const IntersectionObserverInit& options = {});
    ~IntersectionObserver();

    void observe(DOMNodePtr target);
    void unobserve(DOMNodePtr target);
    void disconnect();
    std::vector<IntersectionObserverEntry> takeRecords();

    // Getters
    DOMNodePtr root() const { return options_.root; }
    std::string rootMargin() const { return options_.rootMargin; }
    const std::vector<float>& thresholds() const { return options_.threshold; }

    // Internal: called by render/scroll to check intersections
    void checkIntersections(float viewportX, float viewportY, float viewportWidth, float viewportHeight);

private:
    struct ObservationData {
        DOMNodeWeakPtr target;
        bool wasIntersecting = false;
        float lastRatio = 0;
    };

    Callback callback_;
    IntersectionObserverInit options_;
    std::vector<ObservationData> observations_;
    std::vector<IntersectionObserverEntry> pendingEntries_;
    bool connected_ = false;
};

// Global observer registry for managing all observers
class ObserverRegistry {
public:
    static ObserverRegistry& instance();

    void registerMutationObserver(MutationObserver* observer);
    void unregisterMutationObserver(MutationObserver* observer);

    void registerResizeObserver(ResizeObserver* observer);
    void unregisterResizeObserver(ResizeObserver* observer);

    void registerIntersectionObserver(IntersectionObserver* observer);
    void unregisterIntersectionObserver(IntersectionObserver* observer);

    // Notify all observers
    void notifyMutation(const MutationRecord& record);
    void notifyResize(DOMNodePtr target, float width, float height);
    void checkIntersections(float viewportX, float viewportY, float viewportWidth, float viewportHeight);

    // Process pending callbacks (call from main loop)
    void processPendingCallbacks();

private:
    ObserverRegistry() = default;

    std::unordered_set<MutationObserver*> mutationObservers_;
    std::unordered_set<ResizeObserver*> resizeObservers_;
    std::unordered_set<IntersectionObserver*> intersectionObservers_;
};

} // namespace dong::dom
