#include "helpers/base_clock.hpp"

#include "helpers/str_label.hpp"

#include <QElapsedTimer>
#include <QTime>
#include <QTimer>

#include <utility>

struct BaseClock::base_clock_state {
    QTimer timer;
    QElapsedTimer elapsed_timer;
    qint64 elapsed_ms = 0;
    qint64 last_tick_ms = 0;
    bool running = false;
    bool elapsed_active = false;
    bool single_shot = false;
};

BaseClock::BaseClock(QObject* parent)
    : QObject(parent)
    , state(std::make_unique<base_clock_state>()) {
    state->timer.setInterval(100);
    QObject::connect(
        &state->timer, &QTimer::timeout, this, &BaseClock::on_timeout
    );
}

BaseClock::~BaseClock() = default;

void BaseClock::set_interval(int interval_ms) {
    state->timer.setInterval(interval_ms);
}

void BaseClock::set_single_shot(bool single_shot) {
    state->single_shot = single_shot;
    state->timer.setSingleShot(single_shot);
}

void BaseClock::start(bool emit_immediately) {
    if (state->running) {
        return;
    }
    state->running = true;
    state->elapsed_active = true;
    state->elapsed_timer.start();
    state->timer.start();
    if (emit_immediately) {
        on_timeout();
    }
}

void BaseClock::pause() {
    if (!state->running) {
        return;
    }
    state->elapsed_ms += state->elapsed_timer.elapsed();
    state->running = false;
    state->elapsed_active = false;
    state->timer.stop();
    on_timeout();
}

void BaseClock::stop() {
    state->timer.stop();
    state->running = false;
    state->elapsed_active = false;
}

void BaseClock::reset() {
    state->elapsed_ms = 0;
    state->last_tick_ms = 0;
    state->running = false;
    state->elapsed_active = false;
    state->timer.stop();
    on_timeout();
}

bool BaseClock::is_active() const { return state->timer.isActive(); }

void BaseClock::restart_elapsed() {
    state->elapsed_ms = 0;
    state->last_tick_ms = 0;
    state->running = false;
    state->elapsed_active = true;
    state->elapsed_timer.start();
}

qint64 BaseClock::elapsed_time_ms() const {
    qint64 total_ms = state->elapsed_ms;
    if (state->running || state->elapsed_active) {
        total_ms += state->elapsed_timer.elapsed();
    }
    return total_ms;
}

double BaseClock::elapsed_time_sec() const {
    return static_cast<double>(elapsed_time_ms()) / 1000.0;
}

QString BaseClock::time_string_mm_ss() const {
    const qint64 total_ms = elapsed_time_ms();
    auto display_time = QTime(0, 0).addMSecs(static_cast<int>(total_ms));
    return display_time.toString(str_label("mm:ss"));
}

QString BaseClock::time_string_hh_mm_ss() const {
    const qint64 total_ms = elapsed_time_ms();
    auto display_time = QTime(0, 0).addMSecs(static_cast<int>(total_ms));
    return display_time.toString(str_label("hh:mm:ss"));
}

void BaseClock::single_shot(
    const int interval_ms, QObject* context, std::function<void()> handler
) {
    QTimer::singleShot(interval_ms, context, std::move(handler));
}

void BaseClock::on_timeout() {
    emit timeout();

    const qint64 total_ms = elapsed_time_ms();
    const qint64 delta_ms = total_ms - state->last_tick_ms;
    state->last_tick_ms = total_ms;
    emit ticked(total_ms, delta_ms);
    emit ticked_seconds(
        static_cast<double>(total_ms) / 1000.0,
        static_cast<double>(delta_ms) / 1000.0
    );
}
