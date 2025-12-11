#pragma once
#include <Arduino.h>

enum Phase {
  IDLE,
  FOCUS,
  SHORT_BREAK,
  LONG_BREAK
};

extern Phase currentPhase;
extern bool running;
extern bool isPaused;
extern uint8_t completedPomodoroSessions;

void initPomodoroCore(bool fastTestMode);
void timerLogic();
void handleButtons();
void changePhase();

// helpers for tests
void resetStateForTest();
void simulateStartButtonPress();
void simulateResetButtonPress();