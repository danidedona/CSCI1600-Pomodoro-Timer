// // uncomment and comment everything in here to switch between tests
// // integration tests

// #include <Arduino.h>
// #define TEST_MODE       // important: must be defined before including pomodoro header

// #include "pomodoro-utils.h"
// #ifdef TEST_MODE

// // Globals from main sketch
// extern bool running;
// extern bool isPaused;
// extern bool fastTesting;
// extern bool watchdogArmed;
// extern Phase currentPhase;
// extern uint8_t completedPomodoroSessions;
// extern uint32_t phaseDuration;
// extern uint32_t phaseStartTime;
// extern uint32_t expectedEndTime;
// extern unsigned long lastButtonPress;
// extern const unsigned long debounceDelay;

// // Helpers from main sketch
// extern void resetStateForTest();
// extern void simulateStartButtonPress();
// extern void simulateResetButtonPress();
// extern void handleButtons();
// extern void timerLogic();

// // --- Assertion helpers ---

// void assertEqualInt(const char* name, int expected, int actual) {
//   if (expected == actual) {
//     Serial.print("[PASS] ");
//   } else {
//     Serial.print("[FAIL] ");
//   }
//   Serial.print(name);
//   Serial.print(" expected=");
//   Serial.print(expected);
//   Serial.print(" actual=");
//   Serial.println(actual);
// }

// void assertTrue(const char* name, bool cond) {
//   if (cond) {
//     Serial.print("[PASS] ");
//   } else {
//     Serial.print("[FAIL] ");
//   }
//   Serial.println(name);
// }

// // --- Time helpers for tests ---

// // Fake that enough time has passed since the last button press so debounce doesn't block us
// void waitOutDebounce() {
//   lastButtonPress = millis() - debounceDelay - 1;
// }

// // Force the current phase to be "finished" so that timerLogic() will advance it
// void forcePhaseToEnd() {
//   phaseStartTime = millis() - phaseDuration - 1000UL; // 1 second extra
// }

// // -------------------------
// // FOCUS -> SHORT_BREAK when timer completes 
// // -------------------------
// void test_FocusToShortBreak() {
//   Serial.println("---- test_FocusToShortBreak ----");

//   resetStateForTest();
//   fastTesting = true;

//   // Start from IDLE and go to FOCUS
//   currentPhase = IDLE;
//   waitOutDebounce();
//   simulateStartButtonPress();
//   handleButtons();

//   assertTrue("phase is FOCUS after start", currentPhase == FOCUS);
//   assertTrue("running is true after start", running);
//   assertTrue("watchdog is armed after start", watchdogArmed);

//   // Make it look like plenty of time has passed so the phase is done
//   forcePhaseToEnd();
//   timerLogic();   // should end FOCUS, advance to SHORT_BREAK

//   assertTrue("phase is SHORT_BREAK after focus finishes", currentPhase == SHORT_BREAK);
//   assertEqualInt("completedPomodoroSessions == 1", 1, completedPomodoroSessions);
// }

// // -------------------------
// // Reset while in FOCUS, it should go back to IDLE
// // -------------------------
// void test_ResetFromFocus() {
//   Serial.println("---- test_ResetFromFocus ----");

//   resetStateForTest();
//   fastTesting = true;

//   // Go to FOCUS
//   currentPhase = IDLE;
//   waitOutDebounce();
//   simulateStartButtonPress();
//   handleButtons();

//   assertTrue("in FOCUS before reset", currentPhase == FOCUS);
//   assertTrue("running true before reset", running);

//   // Wait out debounce so reset is not ignored
//   waitOutDebounce();
//   simulateResetButtonPress();
//   handleButtons();

//   assertTrue("phase is IDLE after reset", currentPhase == IDLE);
//   assertTrue("running false after reset", !running);
//   assertTrue("isPaused false after reset", !isPaused);
//   assertEqualInt("completedPomodoroSessions reset", 0, completedPomodoroSessions);
// }

// // -------------------------
// // Setup / Loop
// // -------------------------
// void setup() {
//   Serial.begin(9600);
//   delay(1000);
//   Serial.println("=== Integration Tests ===");

//   test_FocusToShortBreak_Integration();
//   test_ResetFromFocus_Integration();

//   Serial.println("=== Tests complete ===");
// }

// void loop() {
//   // nothing
// }

// #endif
