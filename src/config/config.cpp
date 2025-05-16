#include "config.h"

// Initialize global objects
TFT_eSPI tft = TFT_eSPI();
Adafruit_NeoPixel rgbLED(NUM_LEDS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
HardwareSerial fingerSerial(2);
Adafruit_Fingerprint finger(&fingerSerial);
WebServer server(80);

// Firebase configuration
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;
FirebaseData firebaseData;

// Timing Constants
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000;  // 1 second

// Network Configuration
const char *ntpServer = "216.239.35.0";  // IP address for time.google.com
const long gmtOffset_sec = 19800;        // Offset for IST (GMT+5:30)
const int daylightOffset_sec = 0;

// IP Configuration (initialized in setupWiFi())
IPAddress local_IP;
IPAddress gateway;
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 31, 1);
IPAddress secondaryDNS(8, 8, 8, 8);

// Authentication
const char *DEFAULT_USERNAME = "admin";
const char *DEFAULT_PASSWORD = "admin123";
bool isAuthenticated = false;
String sessionToken = "";
unsigned long sessionExpiry = 0;
const unsigned long SESSION_TIMEOUT = 3600000;  // 1 hour in milliseconds

// Web Security
String csrfToken = "";
const size_t MAX_POST_SIZE = 1024;
const size_t MAX_HEADER_SIZE = 512;

// Global State Variables
String attendanceData = "";
String day, month, year;
struct tm timeinfo;
int TDAttendance[50], Atindex = 0;
int lastSecond = -1;
int lastMinute = -1;
int lastHour = -1;
unsigned long lastDisplayUpdate = 0;
bool isBlinking = false;
bool sdCardInitialized = false;
String telegramBotToken = "";
String telegramChatId = "";

// Data Arrays
String data[128], nc[128];
int did = 0, ncid = 0, ncheck = 0;
int addid = 0, namid = 0, dayid = 1;
String name[128][9]; 