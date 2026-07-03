#pragma once
#include <Arduino.h>

enum class PomoPhase : uint8_t { FOCUS, SHORT_BREAK, LONG_BREAK };
enum class PomoState : uint8_t { IDLE, RUNNING, PAUSED };

class Pomodoro {
public:
  void begin();
  void tick();          // À appeler dans loop()
  void startPause();    // Démarre / met en pause / reprend
  void reset();         // Réinitialise la phase courante
  void skip();          // Passe à la phase suivante
  void reload();        // Recharge les durées depuis les réglages (si IDLE)

  PomoPhase phase() const { return _phase; }
  PomoState state() const { return _state; }
  uint32_t remainingMs() const { return _remainingMs; }
  uint32_t totalMs() const { return _totalMs; }
  uint8_t completedInCycle() const { return _completedInCycle; }
  uint32_t completedToday() const { return _completedTotal; }
  bool justFinished();  // true une seule fois après chaque fin de phase

private:
  void enterPhase(PomoPhase p);
  uint32_t phaseDurationMs(PomoPhase p) const;

  PomoPhase _phase = PomoPhase::FOCUS;
  PomoState _state = PomoState::IDLE;
  uint32_t _totalMs = 0;
  uint32_t _remainingMs = 0;
  uint32_t _lastTick = 0;
  uint8_t _completedInCycle = 0;
  uint32_t _completedTotal = 0;
  bool _finishedFlag = false;
};

extern Pomodoro g_pomodoro;
