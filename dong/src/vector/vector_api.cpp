#include "../../include/dong_vector.h"
#include "vector_animation.hpp"
#include <memory>

using namespace dong::vector;

struct DongVectorAnimation {
    std::unique_ptr<VectorAnimation> impl;
    DongVectorFormat format;
};

DongVectorAnimation* dong_vector_load(dong_engine_t* engine, DongVectorFormat format,
                                       const void* data, size_t len) {
    (void)engine;
    auto* va = new DongVectorAnimation();
    va->format = format;

    if (format == DONG_VECTOR_LOTTIE) {
        auto lottie = std::make_unique<LottieAnimation>();
        if (!lottie->load(data, len)) { delete va; return nullptr; }
        va->impl = std::move(lottie);
    } else if (format == DONG_VECTOR_RIVE) {
        auto rive = std::make_unique<RiveAnimation>();
        if (!rive->load(data, len)) { delete va; return nullptr; }
        va->impl = std::move(rive);
    } else {
        delete va; return nullptr;
    }

    return va;
}

DongVectorAnimation* dong_vector_load_file(dong_engine_t* engine, DongVectorFormat format,
                                            const char* path) {
    (void)engine;
    if (!path) return nullptr;
    auto* va = new DongVectorAnimation();
    va->format = format;

    if (format == DONG_VECTOR_LOTTIE) {
        auto lottie = std::make_unique<LottieAnimation>();
        if (!lottie->loadFile(path)) { delete va; return nullptr; }
        va->impl = std::move(lottie);
    } else if (format == DONG_VECTOR_RIVE) {
        auto rive = std::make_unique<RiveAnimation>();
        if (!rive->loadFile(path)) { delete va; return nullptr; }
        va->impl = std::move(rive);
    } else {
        delete va; return nullptr;
    }

    return va;
}

void dong_vector_destroy(DongVectorAnimation* va) {
    delete va;
}

double dong_vector_duration(DongVectorAnimation* va) {
    return va && va->impl ? va->impl->duration() : 0.0;
}

DongVectorState dong_vector_state(DongVectorAnimation* va) {
    if (!va || !va->impl) return DONG_VECTOR_STOPPED;
    switch (va->impl->state()) {
        case PlayState::Playing: return DONG_VECTOR_PLAYING;
        case PlayState::Paused: return DONG_VECTOR_PAUSED;
        default: return DONG_VECTOR_STOPPED;
    }
}

void dong_vector_play(DongVectorAnimation* va) { if (va && va->impl) va->impl->play(); }
void dong_vector_pause(DongVectorAnimation* va) { if (va && va->impl) va->impl->pause(); }
void dong_vector_stop(DongVectorAnimation* va) { if (va && va->impl) va->impl->stop(); }
void dong_vector_seek(DongVectorAnimation* va, double t) { if (va && va->impl) va->impl->seek(t); }
void dong_vector_set_loop(DongVectorAnimation* va, int c) { if (va && va->impl) va->impl->setLoop(c); }
void dong_vector_set_speed(DongVectorAnimation* va, float s) { if (va && va->impl) va->impl->setSpeed(s); }

void dong_vector_tick(DongVectorAnimation* va, float dt) {
    if (va && va->impl) va->impl->tick(dt);
}

void dong_vector_draw(DongVectorAnimation* va, float x, float y, float w, float h) {
    if (!va || !va->impl) return;
    // TODO: Get the current view's display list builder and render into it.
    // For now this is a no-op; the HTML element path handles rendering.
    (void)x; (void)y; (void)w; (void)h;
}

// Rive state machine
int dong_vector_rive_play_state_machine(DongVectorAnimation* va, const char* name) {
    if (!va || !va->impl || va->format != DONG_VECTOR_RIVE) return -1;
    auto* rive = dynamic_cast<RiveAnimation*>(va->impl.get());
    return rive ? rive->playStateMachine(name) : -1;
}

int dong_vector_rive_set_input_bool(DongVectorAnimation* va, const char* name, int value) {
    if (!va || !va->impl || va->format != DONG_VECTOR_RIVE) return -1;
    auto* rive = dynamic_cast<RiveAnimation*>(va->impl.get());
    return rive ? rive->setInputBool(name, value != 0) : -1;
}

int dong_vector_rive_set_input_number(DongVectorAnimation* va, const char* name, double value) {
    if (!va || !va->impl || va->format != DONG_VECTOR_RIVE) return -1;
    auto* rive = dynamic_cast<RiveAnimation*>(va->impl.get());
    return rive ? rive->setInputNumber(name, value) : -1;
}

int dong_vector_rive_fire_trigger(DongVectorAnimation* va, const char* name) {
    if (!va || !va->impl || va->format != DONG_VECTOR_RIVE) return -1;
    auto* rive = dynamic_cast<RiveAnimation*>(va->impl.get());
    return rive ? rive->fireTrigger(name) : -1;
}
