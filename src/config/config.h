#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <TFT_eSPI.h>
#include <Adafruit_NeoPixel.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <SD.h>
#include <time.h>

// Pin Definitions
#define RGB_LED_PIN 16   // The data pin for the built-in WS2812 LED
#define NUM_LEDS 1       // Usually 1 built-in LED
#define ONE_WIRE_BUS 32  // DS18B20 data pin connected to GPIO
#define RX_PIN 17        // Serial2 RX
#define TX_PIN 3         // Serial2 TX
#define SD_CS 5          // Define your SD card CS pin

// Global Objects
extern TFT_eSPI tft;                    // TFT display
extern Adafruit_NeoPixel rgbLED;        // RGB LED
extern OneWire oneWire;                 // OneWire instance
extern DallasTemperature sensors;       // Temperature sensor
extern HardwareSerial fingerSerial;     // Serial for fingerprint sensor
extern Adafruit_Fingerprint finger;     // Fingerprint sensor
extern WebServer server;                // Web server
extern FirebaseConfig firebaseConfig;   // Firebase configuration
extern FirebaseAuth firebaseAuth;       // Firebase authentication
extern FirebaseData firebaseData;       // Firebase data

// Timing Constants
extern const unsigned long DISPLAY_UPDATE_INTERVAL;

// Network Configuration
extern const char *ntpServer;
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;
extern IPAddress local_IP;
extern IPAddress gateway;
extern IPAddress subnet;
extern IPAddress primaryDNS;
extern IPAddress secondaryDNS;

// Authentication
extern const char *DEFAULT_USERNAME;
extern const char *DEFAULT_PASSWORD;
extern bool isAuthenticated;
extern String sessionToken;
extern unsigned long sessionExpiry;
extern const unsigned long SESSION_TIMEOUT;

// Global State Variables
extern String attendanceData;
extern String day, month, year;
extern struct tm timeinfo;
extern int TDAttendance[];
extern int Atindex;
extern int lastSecond;
extern int lastMinute;
extern int lastHour;
extern unsigned long lastDisplayUpdate;
extern bool isBlinking;
extern bool sdCardInitialized;
extern String telegramBotToken;
extern String telegramChatId;
extern String csrfToken;
extern const size_t MAX_POST_SIZE;
extern const size_t MAX_HEADER_SIZE;

// Data Arrays
extern String data[], nc[];
extern int did, ncid, ncheck;
extern int addid, namid, dayid;
extern String name[][9];

#endif // CONFIG_H 