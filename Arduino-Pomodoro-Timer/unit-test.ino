// comment and uncomment all when switching between unit and integration tests after these two lines
// unit tests


#include <Arduino.h>
// #define TEST_MODE    

#include "pomodoro-utils.h"

#ifdef TEST_MODE


// Bring in globals & functions from main sketch
extern bool running;
extern bool isPaused;
extern bool fastTesting;
extern Phase currentPhase;
extern uint8_t completedPomodoroSessions;
extern uint32_t phaseDuration;
extern uint32_t phaseStartTime;
extern unsigned long lastButtonPress;
extern const unsigned long debounceDelay;

extern void resetStateForTest();
extern void simulateStartButtonPress();
extern void simulateResetButtonPress();
extern void handleButtons();
extern void timerLogic();
extern void changePhase();

// ============ assertion helpers ============

void assertEqualInt(const char* name, int expected, int actual) {
  if (expected == actual) {
    Serial.print("[PASS] ");
  } else {
    Serial.print("[FAIL] ");
  }
  Serial.print(name);
  Serial.print(" expected=");
  Serial.print(expected);
  Serial.print(" actual=");
  Serial.println(actual);
}

void assertEqualUL(const char* name, unsigned long expected, unsigned long actual) {
  if (expected == actual) {
    Serial.print("[PASS] ");
  } else {
    Serial.print("[FAIL] ");
  }
  Serial.print(name);
  Serial.print(" expected=");
  Serial.print(expected);
  Serial.print(" actual=");
  Serial.println(actual);
}

void assertTrue(const char* name, bool cond) {
  if (cond) {
    Serial.print("[PASS] ");
  } else {
    Serial.print("[FAIL] ");
  }
  Serial.println(name);
}

// ============ Tests ============

// testing that everything resent
void test_ResetState() {
  resetStateForTest();

  assertTrue("currentPhase == IDLE", currentPhase == IDLE);
  assertTrue("running == false", !running);
  assertTrue("isPaused == false", !isPaused);
  assertEqualInt("completedPomodoroSessions == 0", 0, completedPomodoroSessions);
  assertEqualUL("phaseDuration == 0", 0UL, phaseDuration);
}

// testing start button
void test_IdleToFocus() {
  resetStateForTest();
  Serial.println("---- Test start button ----");
  fastTesting = true;

  currentPhase = IDLE;

  simulateStartButtonPress();
  handleButtons();

  assertTrue("running after start", running);
  assertTrue("phase is FOCUS", currentPhase == FOCUS);
}

// testing pause and resume while in focus
void test_FocusPauseResume() {
  resetStateForTest();
  Serial.println("---- Test pause and resume while in focus ----");
  fastTesting = true;

  // Start from IDLE -> FOCUS
  currentPhase = IDLE;
  simulateStartButtonPress();
  handleButtons();
  assertTrue("phase is FOCUS after start", currentPhase == FOCUS);

  // Wait past debounce
  delay(debounceDelay + 10);

  // Pause
  simulateStartButtonPress();
  handleButtons();
  assertTrue("isPaused after pause", isPaused);
  assertTrue("running false while paused", !running);

  // Wait past debounce
  delay(debounceDelay + 10);

  // Resume
  simulateStartButtonPress();
  handleButtons();
  assertTrue("isPaused after resume", !isPaused);
  assertTrue("running true after resume", running);
}

// testing that it will change to short break once time runs out
void test_FocusToShortBreak() {
  resetStateForTest();
  Serial.println("---- test change focus to short break ----");
  fastTesting = true;

  // Start a FOCUS session
  currentPhase = IDLE;
  simulateStartButtonPress();
  handleButtons();
  assertTrue("phase is FOCUS after start", currentPhase == FOCUS);

  // Simulate that the focus timer has expired
  running = true;
  isPaused = false;
  phaseStartTime = millis() - phaseDuration;  // pretend time is up

  timerLogic();  // should finish FOCUS and transition to SHORT_BREAK

  assertTrue("phase is SHORT_BREAK after FOCUS done", currentPhase == SHORT_BREAK);
  assertEqualInt("completedPomodoroSessions == 1", 1, completedPomodoroSessions);
}

// tests that after 4 sessions it turns into a long break
void test_FourFocusThenLongBreak() {
  resetStateForTest();
  Serial.println("---- test focus (4 sessions) to long break ----");
  fastTesting = true;

  // Manually simulate 4 FOCUS completions with changePhase
  currentPhase = IDLE;

  for (int i = 0; i < 4; i++) {
    changePhase(); // IDLE->FOCUS or BREAK->FOCUS/SHORT/LONG
    // After entering FOCUS, simulate completion
    currentPhase = FOCUS;
    changePhase(); // FOCUS -> SHORT or LONG
  }

  assertTrue("currentPhase == LONG_BREAK after 4th focus", currentPhase == LONG_BREAK);
}

// test reset button in focus
void test_ResetFromFocus() {
  resetStateForTest();
  Serial.println("---- test reseting from focus ----");
  fastTesting = true;

  // Start into FOCUS
  currentPhase = IDLE;
  simulateStartButtonPress();
  handleButtons();
  assertTrue("in FOCUS before reset", currentPhase == FOCUS);

  // Wait past debounce and trigger reset
  delay(debounceDelay + 10);
  simulateResetButtonPress();
  handleButtons();

  assertTrue("currentPhase == IDLE after reset", currentPhase == IDLE);
  assertTrue("running == false after reset", !running);
  assertTrue("isPaused == false after reset", !isPaused);
}

// ============ Test runner ============


void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("=== Unit Tests ===");

  fastTesting = true;
  resetStateForTest();

  test_ResetState();
  test_IdleToFocus();
  test_FocusPauseResume();
  test_FocusToShortBreak();
  test_FourFocusThenLongBreak();
  test_ResetFromFocus();

  Serial.println("=== Tests complete ===");
}

void loop() {
  // Nothing
}

#endif
