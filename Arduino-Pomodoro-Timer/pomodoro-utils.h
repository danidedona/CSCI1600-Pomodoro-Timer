// this has all the variables and functions needed for the test file
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
const String SONG_FOCUS_DONE  = "FocusDone:d=4,o=6,b=425:8b,8p,8b,8p,8b,8p,8b,2p,4p,8b,8p,8b,8p,8b,8p,8b,8p";       
const String SONG_BREAK_DONE  = "BreakDone:d=4,o=6,b=425:8b,8p,8b,8p,8b,8p,8b,2p,4p,8b,8p,8b,8p,8b,8p,8b,8p";          
const String SONG_CYCLE_DONE  = "CycleDone:d=4,o=6,b=425:8b,8p,8b,8p,8b,8p,8b,2p,4p,8b,8p,8b,8p,8b,8p,8b,8p"; 
int noteFrequencies[100];
int noteDurations[100];
int songLen;

// === Interrupt Variables ===
volatile bool startButtonPressed = false;
volatile bool resetButtonPressed = false;
bool shouldFetchConfig = false;

// === Misc. Variables ===
bool fastTesting = false;
bool running = false;
bool isPaused = false;
int minutes = 25;
int seconds = 0;
unsigned long lastUpdate = 0;
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200;
unsigned long remainingAtPause = 0;