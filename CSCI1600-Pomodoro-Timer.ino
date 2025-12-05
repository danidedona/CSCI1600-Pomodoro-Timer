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
// TODO: fix TFT calibration
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
uint16_t pastelPink = tft.color565(255, 182, 193);

// === FSM States ===
#define HOME_SCREEN 0
#define FOCUS_ACTIVE 1
#define FOCUS_PAUSED 2
#define SHORT_BREAK_ACTIVE 3
#define SHORT_BREAK_PAUSED 4
#define LONG_BREAK_ACTIVE 5
#define LONG_BREAK_PAUSED 6
#define TRANSITION 7

// === Buzzer Pins ===
#define BUZZER_PIN 4

// === Global Variables ===
int currentState = HOME_SCREEN;
int prevState = HOME_SCREEN;
int completedSessions = 0;
bool running = false;
bool isPaused = false;
bool justResumed = false;

int minutes = 25;
int seconds = 0;

unsigned long lastUpdate = 0;
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200;

// Wi-fi
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS; 
WiFiClient client;
unsigned long lastWifiSend = 0;

// Time scaling (0.01 = fast demo)
float timeScale = 0.01;

// Simple short tunes
// TODO: make these actually sound good
const String SONG_FOCUS_DONE  = "FocusDone:d=4,o=6,b=180:8g,8b,8d7,4g7";       
const String SONG_BREAK_DONE  = "BreakDone:d=4,o=6,b=160:8c7,8a,8f,8d";          
const String SONG_CYCLE_DONE  = "CycleDone:d=4,o=6,b=200:8c,8e,8g,8c7,8p,8g,8e"; 

int noteFrequencies[100];
int noteDurations[100];
int songLen;

// === Functions ===
void drawHomeScreen();
void drawUI();
void drawButtons();
void updateTimerDisplay();
void updateDisplayText(const char* text);
void setLEDColor(int r, int g, int b);
void changeState(int newState);
void handleButtons();
void handleTouch();
void timerTick();

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
  
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  drawHomeScreen();
}

void loop() {
  handleButtons();
  handleTouch();

  if (running && millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    timerTick();
  }
}

// === HOME SCREEN ===
void drawHomeScreen() {
  tft.fillScreen(pastelPink);
  setLEDColor(255, 255, 255);
  running = false;
  isPaused = false;
  completedSessions = 0;

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(40, 60);
  //tft.setFont(& FreeMonoBold9pt7b);
  drawCenteredText(tft, "Pomodoro Timer", 60);
  //tft.print("Pomodoro Timer");

  // Big green Start button
  tft.fillRect(60, 210, 120, 60, ILI9341_GREEN);

  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);
  tft.setCursor(75, 230);
  tft.print("START");
}

// === STATE HANDLER ===
void changeState(int newState) {
  prevState = currentState;
  currentState = newState;

  switch (newState) {
    case HOME_SCREEN:
      drawHomeScreen();
      return;

    case FOCUS_ACTIVE:
      drawUI();
      updateDisplayText("Focus Session");
      setLEDColor(255, 0, 0);
      running = true;
      isPaused = false;
      if (!justResumed) {
        int scaledFocus = int(25 * 60 * timeScale);
        minutes = scaledFocus / 60;
        seconds = scaledFocus % 60;
      }
      justResumed = false;
      lastUpdate = millis();
      break;

    case FOCUS_PAUSED:
      updateDisplayText("Paused - Focus");
      setLEDColor(255, 255, 255);
      running = false;
      isPaused = true;
      break;

    case SHORT_BREAK_ACTIVE:
      drawUI();
      updateDisplayText("Short Break");
      setLEDColor(0, 255, 0);
      running = true;
      isPaused = false;
      if (!justResumed) {
        int scaledShort = int(5 * 60 * timeScale);
        minutes = scaledShort / 60;
        seconds = scaledShort % 60;
      }
      justResumed = false;
      lastUpdate = millis();
      break;

    case SHORT_BREAK_PAUSED:
      updateDisplayText("Paused - Break");
      setLEDColor(255, 255, 255);
      running = false;
      isPaused = true;
      break;

    case LONG_BREAK_ACTIVE:
      drawUI();
      updateDisplayText("Long Break");
      setLEDColor(0, 0, 255);
      running = true;
      isPaused = false;
      if (!justResumed) {
        int scaledLong = int(15 * 60 * timeScale);
        minutes = scaledLong / 60;
        seconds = scaledLong % 60;
      }
      justResumed = false;
      lastUpdate = millis();
      break;

    case LONG_BREAK_PAUSED:
      updateDisplayText("Paused - Long Break");
      setLEDColor(255, 255, 255);
      running = false;
      isPaused = true;
      break;
  }

  updateTimerDisplay();
  drawButtons();
}

// === TIMER LOGIC ===
void timerTick() {
  if (!running) return;

  if (--seconds < 0) {
    seconds = 59;
    if (--minutes < 0) {
      minutes = 0;
      seconds = 0;
      running = false;

      Serial.print("Session done: ");
      Serial.println(currentState);

      // Buzzer feedback 
      if (currentState == FOCUS_ACTIVE) {
        completedSessions++;
        playRTTTL(SONG_FOCUS_DONE);
        // At end of focus:
        if (completedSessions < 4)
          changeState(SHORT_BREAK_ACTIVE);
        else {
          changeState(LONG_BREAK_ACTIVE);
          completedSessions = 0;
        }
      }
      else if (currentState == SHORT_BREAK_ACTIVE) {
        playRTTTL(SONG_BREAK_DONE); 
        changeState(FOCUS_ACTIVE);
      }
      else if (currentState == LONG_BREAK_ACTIVE) {
        playRTTTL(SONG_CYCLE_DONE); 
        changeState(FOCUS_ACTIVE);
      }
      return;
    }
  }
  updateTimerDisplay();

  // send pomodoro status ever 5 seconds
  if (millis() - lastWifiSend > 5000) {
    sendPomodoroStatus();
    lastWifiSend = millis();
  }
}

// === BUTTON HANDLER ===
void handleButtons() {
  if (millis() - lastButtonPress < debounceDelay) return;

  if (digitalRead(START_BUTTON_PIN) == LOW) {
    if (currentState == HOME_SCREEN) {
      changeState(FOCUS_ACTIVE);
    } 
    else if (currentState == FOCUS_ACTIVE) {
      justResumed = false;
      changeState(FOCUS_PAUSED);
    } 
    else if (currentState == FOCUS_PAUSED) {
      justResumed = true;
      changeState(FOCUS_ACTIVE);
    } 
    else if (currentState == SHORT_BREAK_ACTIVE) {
      justResumed = false;
      changeState(SHORT_BREAK_PAUSED);
    } 
    else if (currentState == SHORT_BREAK_PAUSED) {
      justResumed = true;
      changeState(SHORT_BREAK_ACTIVE);
    } 
    else if (currentState == LONG_BREAK_ACTIVE) {
      justResumed = false;
      changeState(LONG_BREAK_PAUSED);
    } 
    else if (currentState == LONG_BREAK_PAUSED) {
      justResumed = true;
      changeState(LONG_BREAK_ACTIVE);
    }

    lastButtonPress = millis();
  }

  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    completedSessions = 0;
    changeState(HOME_SCREEN);
    lastButtonPress = millis();
  }
}

// === TOUCH HANDLER ===
// TODO: just generally fix this
void handleTouch() {
  pinMode(YP, INPUT);
  pinMode(XM, INPUT);
  TSPoint p = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  digitalWrite(XM, LOW);
  digitalWrite(YP, HIGH);

  if (p.z < MINPRESSURE || p.z > MAXPRESSURE) return;

  //int temp = p.x;
  // p.x = map(p.y, TS_MINY, TS_MAXY, 0, tft.width());
  // p.y = map(temp, TS_MINX, TS_MAXX, tft.height(), 0);
  int px = touchToPixelX(p.x);
  int py = touchToPixelY(p.y);

  if (currentState == HOME_SCREEN) {
    if (px > 55 && px < 185 && py > 205 && py < 275) {
      Serial.println("Touch Start (Home)");
      changeState(FOCUS_ACTIVE);
    }
  } 
  else if (py > 245 && py < 295) {
    if (px > 15 && px < 115) { // START/PAUSE

      handleButtons();
    } else if (px > 125 && px < 215) { // RESET
      changeState(HOME_SCREEN);
    }
  }
  Serial.println(p.y);
}

// === UI HELPERS ===
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

void drawUI() {
  tft.fillScreen(pastelPink);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(50, 10);
  tft.print("Pomodoro Timer");
  tft.drawRect(80, 50, 160, 50, ILI9341_WHITE);
  updateTimerDisplay();
}

void drawButtons() {
  //tft.fillRect(30, 250, 40, 30, ILI9341_GREEN); no need for start button
  tft.fillRect(30, 250, 80, 50, ILI9341_YELLOW);
  tft.fillRect(130, 250, 80, 50, ILI9341_RED);

  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(2);
  //tft.setCursor(50, 160); tft.print("START");
  tft.setCursor(40, 270); tft.print("PAUSE");
  tft.setCursor(140, 270); tft.print("RESET");
}

void updateTimerDisplay() {
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", minutes, seconds);
  tft.fillRect(81, 51, 158, 48, ILI9341_BLACK);
  tft.setCursor(120, 65);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.print(timeStr);
}

void updateDisplayText(const char* text) {
  tft.fillRect(40, 110, 240, 20, ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(60, 110);
  tft.print(text);
}

void setLEDColor(int r, int g, int b) {
  analogWrite(LED_RED, r);
  analogWrite(LED_GREEN, g);
  analogWrite(LED_BLUE, b);
}

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

// Convert pixel coordinate to raw touchscreen coordinate
int touchToPixelX(int tx) {
  // touch X: 200 → 845  maps to pixel X: 0 → 240
  return map(tx, 200, 845, 0, 240);
}

int touchToPixelY(int ty) {
  // touch Y: 145 → 890  maps to pixel Y: 0 → 320
  return map(ty, 145, 890, 0, 320);
}

// === SERVER COMMUNICATION ===
void sendPomodoroStatus() {
  const char* serverIP = "192.168.1.236";  
  int serverPort = 3000;

  Serial.println("Connecting to server...");

  if (client.connect(serverIP, serverPort)) {
    Serial.println("Connected to server!");

    // Build JSON manually
    String json = "{";
    json += "\"state\":" + String(currentState) + ",";
    json += "\"minutes\":" + String(minutes) + ",";
    json += "\"seconds\":" + String(seconds) + ",";
    json += "\"completed\":" + String(completedSessions);
    json += "}";

    // Send POST request
    client.println("POST /update HTTP/1.1");
    client.print("Host: ");
    client.println(serverIP);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(json.length());
    client.println();
    client.print(json);

    delay(10);
    client.stop();
  } 
  else {
    Serial.println("Connection to server FAILED");
  }
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
