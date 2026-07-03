#include "pomodoro.h"
#include "settings.h"

Pomodoro g_pomodoro;

uint32_t Pomodoro::phaseDurationMs(PomoPhase p) const {
  switch (p) {
    case PomoPhase::FOCUS:       return (uint32_t)g_settings.workMin * 60000UL;
    case PomoPhase::SHORT_BREAK: return (uint32_t)g_settings.shortBreakMin * 60000UL;
    case PomoPhase::LONG_BREAK:  return (uint32_t)g_settings.longBreakMin * 60000UL;
  }
  return 25 * 60000UL;
}

void Pomodoro::begin() {
  enterPhase(PomoPhase::FOCUS);
}

void Pomodoro::enterPhase(PomoPhase p) {
  _phase = p;
  _totalMs = phaseDurationMs(p);
  _remainingMs = _totalMs;
  _state = PomoState::IDLE;
}

void Pomodoro::reload() {
  if (_state == PomoState::IDLE) {
    _totalMs = phaseDurationMs(_phase);
    _remainingMs = _totalMs;
  }
}

void Pomodoro::startPause() {
  switch (_state) {
    case PomoState::IDLE:
    case PomoState::PAUSED:
      _state = PomoState::RUNNING;
      _lastTick = millis();
      break;
    case PomoState::RUNNING:
      _state = PomoState::PAUSED;
      break;
  }
}

void Pomodoro::reset() {
  enterPhase(_phase);
}

void Pomodoro::skip() {
  if (_phase == PomoPhase::FOCUS) {
    _completedInCycle++;
    if (_completedInCycle >= g_settings.sessionsUntilLong) {
      _completedInCycle = 0;
      enterPhase(PomoPhase::LONG_BREAK);
    } else {
      enterPhase(PomoPhase::SHORT_BREAK);
    }
  } else {
    enterPhase(PomoPhase::FOCUS);
  }
}

bool Pomodoro::justFinished() {
  if (_finishedFlag) {
    _finishedFlag = false;
    return true;
  }
  return false;
}

void Pomodoro::tick() {
  if (_state != PomoState::RUNNING) return;

  uint32_t now = millis();
  uint32_t elapsed = now - _lastTick;
  _lastTick = now;

  if (elapsed >= _remainingMs) {
    // Fin de phase
    _remainingMs = 0;
    _finishedFlag = true;
    if (_phase == PomoPhase::FOCUS) _completedTotal++;
    skip();
    if (g_settings.autoStart) {
      _state = PomoState::RUNNING;
      _lastTick = millis();
    }
  } else {
    _remainingMs -= elapsed;
  }
}
