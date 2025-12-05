#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <TouchScreen.h>
#include <SPI.h>
#include "rtttl_parser.h"
#include <WiFiS3.h>
#include "arduino_secrets.h"
#include <Fonts/FreeMonoBold9pt7b.h>

// === TFT Display Pins ===
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// === Touchscreen Pins ===
#define YP A2
#define XM A3
#define YM 8
#define XP 9
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// === Touch Calibration ===
#define MINPRESSURE 10
#define MAXPRESSURE 1000
#define TS_MINX 920
#define TS_MAXX 200
#define TS_MINY 130
#define TS_MAXY 1038

// === Buttons ===
#define START_BUTTON_PIN 2
#define RESET_BUTTON_PIN 3

// === RGB LED Pins ===
#define LED_RED   5
#define LED_GREEN 6
#define LED_BLUE  7

// === COLORS ===
uint16_t bgColor      = tft.color565(125, 75, 85);    // warm wood-pink
uint16_t boxOutline   = tft.color565(245, 235, 220);  // warm cream
uint16_t btnStart     = tft.color565(94, 138, 96);    // forest sage green
uint16_t btnPause     = tft.color565(204, 153, 80);   // amber gold
uint16_t btnReset     = tft.color565(192, 94, 94);    // burnt coral

// === Buzzer Pins ===
#define BUZZER_PIN 4

// === Pomorodo Time Configurations ===
uint32_t FOCUS_DURATION_MS = 25UL * 60UL * 1000UL; 
uint32_t SHORT_BREAK_MS = 5UL  * 60UL * 1000UL;  
uint32_t LONG_BREAK_MS = 15UL * 60UL * 1000UL;

enum Phase {
    IDLE,
    FOCUS,
    SHORT_BREAK,
    LONG_BREAK
};
uint8_t completedPomodoroSessions = 0; // counts completed FOCUS phases (0–3 then long break)
Phase currentPhase = IDLE;
uint32_t phaseStartTime = 0; // Timestamp when this phase started
uint32_t phaseDuration = 0; // How long this phase should last

// === Watchdog Configurations ===
uint32_t WATCHDOG_PATIENCE_MS = 0; // allowed timing drift +/-5ms
uint32_t expectedEndTime = 0;
bool watchdogArmed = false;


// === WIFI Variables ===
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS; 
WiFiClient client;
unsigned long lastWifiSend = 0;

// === Buzzer Songs ===
const String SONG_FOCUS_DONE  = "FocusDone:d=4,o=6,b=180:8g,8b,8d7,4g7";       
const String SONG_BREAK_DONE  = "BreakDone:d=4,o=6,b=160:8c7,8a,8f,8d";          
const String SONG_CYCLE_DONE  = "CycleDone:d=4,o=6,b=200:8c,8e,8g,8c7,8p,8g,8e"; 
int noteFrequencies[100];
int noteDurations[100];
int songLen;

// === Interrupt Variables ===
volatile bool startButtonPressed = false;
volatile bool resetButtonPressed = false;

// === Misc. Variables ===
bool fastTesting = true;
bool running = false;
bool isPaused = false;
int minutes = 25;
int seconds = 0;
unsigned long lastUpdate = 0;
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200;
unsigned long remainingAtPause = 0;


// === Function Definitions ===

//==============================================================================
// SETUP AND LOOP
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  tft.begin();
  tft.setRotation(0);

  // wi-fi setup taken from the ConnectWithWPA wifis3 examples ketch
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();
  // start up server connection
  ensureConnected();

  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(START_BUTTON_PIN), startButtonISR, FALLING);

  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RESET_BUTTON_PIN), resetButtonISR, FALLING);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  drawHomeScreen();
}

void loop() {
  if (startButtonPressed) {
        Serial.println("ISR: Start button interrupt fired!");
    }
    if (resetButtonPressed) {
        Serial.println("ISR: Reset button interrupt fired!");
    }

  handleButtons();
  handleTouch();

  checkWatchdog();    

  if (running && millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    timerLogic();
  }
}
//==============================================================================


//==============================================================================
// TIMER
//------------------------------------------------------------------------------
void timerLogic() {
    if (!running || isPaused) return;

    unsigned long rawElapsed = millis() - phaseStartTime;

    // Remaining ms
    long remaining = (long)phaseDuration - (long)rawElapsed;

    if (remaining <= 0) {
        minutes = 0;
        seconds = 0;
        updateTimerDisplay();

        running = false;

        Serial.print("Session done: ");
        Serial.println(currentPhase);

        // Play buzzer tones
        if (currentPhase == FOCUS) playRTTTL(SONG_FOCUS_DONE);
        else if (currentPhase == SHORT_BREAK) playRTTTL(SONG_BREAK_DONE);
        else if (currentPhase == LONG_BREAK) playRTTTL(SONG_CYCLE_DONE);

        petWatchdog(); // mark phase as successfully completed

        changePhase(); // start next session
        return;
    }

    // Convert remaining ms → mm:ss
    minutes = remaining / 60000;
    seconds = (remaining % 60000) / 1000;

    updateTimerDisplay();

    // Wifi update every 5 seconds
    if (millis() - lastWifiSend > 5000) {
        sendPomodoroStatus();
        lastWifiSend = millis();
    }
}
//==============================================================================


//==============================================================================
// BUTTONS
//------------------------------------------------------------------------------
void handleButtons() {

    // Copy & clear ISR flags atomically
    bool startEvent = false;
    bool resetEvent = false;

    noInterrupts();
    if (startButtonPressed) {
        startEvent = true;
        startButtonPressed = false;
    }
    if (resetButtonPressed) {
        resetEvent = true;
        resetButtonPressed = false;
    }
    interrupts();

    // Debounce
    if (millis() - lastButtonPress < debounceDelay) return;

    // START / PAUSE BUTTON
    if (startEvent) {

        // -------- Start from Home Screen --------
        if (currentPhase == IDLE) {
            completedPomodoroSessions = 0;
            isPaused = false;
            running = true;
            changePhase();  // enters FOCUS phase and updates UI
            lastButtonPress = millis();
            return;
        }

        // -------- Pause / Resume (any phase) --------
        if (!isPaused) {
            isPaused = true;
            running = false;

            // Save remaining time at pause moment
            unsigned long rawElapsed = millis() - phaseStartTime;
            remainingAtPause = phaseDuration - rawElapsed;

            changeVisual(currentPhase, true);
        }
        else {
            // --- RESUME PRESSED ---
            isPaused = false;
            running = true;

            // Reset the phaseStartTime so countdown restarts from remainingAtPause
            phaseDuration = remainingAtPause;
            phaseStartTime = millis();

            expectedEndTime = phaseStartTime + phaseDuration;
            watchdogArmed = true;

            changeVisual(currentPhase, false);
        }

        lastButtonPress = millis();
    }

    // RESET BUTTON
    if (resetEvent) {

        completedPomodoroSessions = 0;
        currentPhase = IDLE;
        isPaused = false;
        running = false;

        changeVisual(IDLE, false);   // return to home screen UI

        lastButtonPress = millis();
    }
}
//==============================================================================


//==============================================================================
// TOUCHSCREEN
//------------------------------------------------------------------------------
void handleTouch() {

    pinMode(YP, INPUT);
    pinMode(XM, INPUT);
    TSPoint p = ts.getPoint();
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    digitalWrite(XM, LOW);
    digitalWrite(YP, HIGH);

    if (p.z < MINPRESSURE || p.z > MAXPRESSURE) return;

    int px = touchToPixelX(p.x);
    int py = touchToPixelY(p.y);

    // HOME SCREEN → Start button
    if (currentPhase == IDLE) {
        if (px > 55 && px < 185 && py > 205 && py < 275) {
            Serial.println("Touch Start (Home)");
            completedPomodoroSessions = 0;
            isPaused = false;
            running = true;
            changePhase();       // enters FOCUS
            return;
        }
    }

    // UI BUTTON AREA
    if (py > 245 && py < 295) {

        // PAUSE / RESUME by touch
        if (px > 15 && px < 115) {
            noInterrupts();
            startButtonPressed = true;
            interrupts();
        }

        // RESET by touch
       else if (px > 125 && px < 215) {
            noInterrupts();
            resetButtonPressed = true;
            interrupts();
        }
            }

}

// Convert pixel coordinate to raw touchscreen coordinate
int touchToPixelX(int tx) {
  // touch X: 200 → 845  maps to pixel X: 0 → 240
  return map(tx, 200, 845, 0, 240);
}

int touchToPixelY(int ty) {
  // touch Y: 145 → 890  maps to pixel Y: 0 → 320
  return map(ty, 145, 890, 0, 320);
}
//==============================================================================

//==============================================================================
// BUZZER
//------------------------------------------------------------------------------
void playRTTTL(const String &song) {
  songLen = rtttlToBuffers(song, noteFrequencies, noteDurations);
  if (songLen == -1) {
    Serial.println("Error parsing song!");
    return;
  }

  for (int i = 0; i < songLen; i++) {
    int freq = noteFrequencies[i];
    int dur = noteDurations[i];
    if (freq > 0) tone(BUZZER_PIN, freq, dur);
    delay(dur * 1.3); // pause between notes
  }
  noTone(BUZZER_PIN);
}
//==============================================================================

//==============================================================================
// UI / OTHER VISUALS
//------------------------------------------------------------------------------
void drawCenteredText(Adafruit_ILI9341 &tft, const char *text, int y) {
  int16_t x1, y1;
  uint16_t w, h;

  // Compute the bounding box of the text using current text size & font
  tft.getTextBounds(text, 0, y, &x1, &y1, &w, &h);

  // center X = (screen_width - text_width) / 2
  int x = (tft.width() - w) / 2;

  // Draw the centered text
  tft.setCursor(x, y);
  tft.print(text);
}

void drawButtons() {
  tft.fillRect(30, 250, 80, 50, btnPause);
  tft.fillRect(130, 250, 80, 50, btnReset);

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(40, 270); tft.print("PAUSE");
  tft.setCursor(140, 270); tft.print("RESET");
}

void updateTimerDisplay() {
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", minutes, seconds);
  tft.fillRect(31, 121, 178, 48, bgColor);
  tft.setCursor(77, 135);
  tft.setTextColor(boxOutline);
  tft.setTextSize(3);
  tft.print(timeStr);
}

void updateDisplayText(const char* text) {
  tft.fillRect(30, 180, 180, 50, bgColor);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(42, 190);
  drawCenteredText(tft, text, 190);
}

void setLEDColor(int r, int g, int b) {
  analogWrite(LED_RED, r);
  analogWrite(LED_GREEN, g);
  analogWrite(LED_BLUE, b);
}

void drawHomeScreen() {
  tft.fillScreen(bgColor);
  setLEDColor(255, 255, 255);
  running = false;
  isPaused = false;

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(40, 60);
  drawCenteredText(tft, "Pomodoro Timer", 60);

  //Start button
  tft.fillRect(60, 210, 120, 60, btnStart);

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.setCursor(75, 230);
  tft.print("START");
}

void drawActiveScreen() {
  tft.fillScreen(bgColor);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(40, 80);
  tft.print("Pomodoro Timer");
  tft.drawRect(30, 120, 180, 50, ILI9341_WHITE);
  updateTimerDisplay();
}

void changeVisual(Phase phase, bool paused) {

      bool wasConnected = client.connected();
    if (wasConnected) {
        client.stop();     // close TCP so SPI doesn't corrupt WiFi chip
        delay(5);          // brief settle time
    }

    // IDLE (Home Screen)
    if (phase == IDLE) {
        drawHomeScreen();
        setLEDColor(255, 255, 255);
        running = false;
        isPaused = false;


        // restore TCP if it was open before the redraw
        if (wasConnected){
            ensureConnected();
        }
        return;
    }

    // Non-IDLE phases redraw UI frame
    drawActiveScreen();

    // Phase-specific UI
    switch (phase) {

        case FOCUS:
            if (paused) {
                updateDisplayText("Paused - Focus");
                setLEDColor(255, 255, 255);
                running = false;
            } else {
                updateDisplayText("Focus Session");
                setLEDColor(255, 0, 0);
                running = true;
            }
            break;

        case SHORT_BREAK:
            if (paused) {
                updateDisplayText("Paused - Short Break");
                setLEDColor(255, 255, 255);
                running = false;
            } else {
                updateDisplayText("Short Break");
                setLEDColor(0, 255, 0);
                running = true;
            }
            break;

        case LONG_BREAK:
            if (paused) {
                updateDisplayText("Paused - Long Break");
                setLEDColor(255, 255, 255);
                running = false;
            } else {
                updateDisplayText("Long Break");
                setLEDColor(0, 0, 255);
                running = true;
            }
            break;
    }

    // Timer + Buttons (only non-IDLE)
    updateTimerDisplay();
    drawButtons();

    if (wasConnected) {
        ensureConnected(); // safely restore persistent TCP connection
    }
}
//==============================================================================



//==============================================================================
// PHASE LOGIC
//------------------------------------------------------------------------------
void changePhase() {
    // Determine next phase
    Phase next;
    Phase prev = currentPhase;

    if (currentPhase == IDLE) {
        next = FOCUS;
    }
    else if (currentPhase == FOCUS) {
        completedPomodoroSessions++;
        next = (completedPomodoroSessions < 4) ? SHORT_BREAK : LONG_BREAK;
    }
    else if (currentPhase == SHORT_BREAK) {
        next = FOCUS;
    }
    else if (currentPhase == LONG_BREAK) {
        completedPomodoroSessions = 0; // reset cycle
        next = FOCUS;
    }

    if (prev == FOCUS && next != FOCUS) {
        sendPomodoroStatus();
    }

    currentPhase = next;

    // Assign new phase duration
    if (fastTesting) {
        // FAST MODE: super short durations
        switch (next) {
            case FOCUS:       phaseDuration = 10 * 1000; break; // 10 seconds
            case SHORT_BREAK: phaseDuration = 5  * 1000; break; // 5 seconds
            case LONG_BREAK:  phaseDuration = 8  * 1000; break; // 8 seconds
            case IDLE:        phaseDuration = 0;         break;
        }
    } else {
        // NORMAL MODE
        switch (next) {
            case FOCUS:       phaseDuration = FOCUS_DURATION_MS; break;
            case SHORT_BREAK: phaseDuration = SHORT_BREAK_MS;    break;
            case LONG_BREAK:  phaseDuration = LONG_BREAK_MS;     break;
            case IDLE:        phaseDuration = 0;                 break;
        }
    }

    // Set phase timing
    phaseStartTime = millis();

    // ARM watchdog using new duration
    expectedEndTime = phaseStartTime + phaseDuration;
    watchdogArmed = true;

    // Update display timer
    minutes = (phaseDuration / 1000) / 60;
    seconds = (phaseDuration / 1000) % 60;

    // Update UI
    changeVisual(next, false);
}
//==============================================================================

//==============================================================================
// ISR
//------------------------------------------------------------------------------
void startButtonISR() { startButtonPressed = true; }
void resetButtonISR() { resetButtonPressed = true; }
//==============================================================================


//==============================================================================
// WATCHDOG
//------------------------------------------------------------------------------
void petWatchdog() {
    watchdogArmed = false; // disable until next phase
}

void checkWatchdog() {
    if (!watchdogArmed) return;
    if (!running || isPaused) return;

    unsigned long now = millis();

    // If the timer should be finished, check accuracy
    if (now >= expectedEndTime) {

        long drift = (long)now - (long)expectedEndTime;

        if (abs(drift) > WATCHDOG_PATIENCE_MS) {
            Serial.println("WATCHDOG ERROR: Timing drift detected!");
            Serial.print("Drift = "); Serial.println(drift);

            NVIC_SystemReset(); // complete microcontroller restart
        }
        petWatchdog();
    }
}
//==============================================================================


//==============================================================================
// WIFI / SERVER
//------------------------------------------------------------------------------

bool ensureConnected() {
    // If still connected, all good
    if (client.connected()) {
        return true;
    }

    Serial.println("TCP disconnected — attempting reconnect...");

    // Try reconnecting once
    if (client.connect("192.168.1.236", 3000)) {
        Serial.println("TCP reconnected.");
        return true;
    }

    // If reconnect fails → restart entire system
    Serial.println("TCP reconnect FAILED. REBOOTING SYSTEM.");
    NVIC_SystemReset();     // full microcontroller reset
    return false;           // never reached
}

void sendPomodoroStatus() {
    // ensure connection is alive — may restart the system
    if (!ensureConnected()) return;

    // Build JSON
    String json = "{";
    json += "\"state\":" + String(currentPhase) + ",";
    json += "\"minutes\":" + String(minutes) + ",";
    json += "\"seconds\":" + String(seconds) + ",";
    json += "\"completed\":" + String(completedPomodoroSessions);
    json += "}";

    // Send POST without closing connection
    client.println("POST /update HTTP/1.1");
    client.println("Host: 192.168.1.236");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(json.length());
    client.println();
    client.print(json);

    // IMPORTANT: DO NOT call client.stop()
}

// === Wi-Fi Helpers ===
void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 0; i < 6; i++) {
    if (i > 0) {
      Serial.print(":");
    }
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
  }
  Serial.println();
}
//==============================================================================
