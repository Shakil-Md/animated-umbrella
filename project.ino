#include "src/config/config.h"
#include "src/utils/display_utils.h"
#include "src/utils/time_utils.h"
#include "src/utils/sd_utils.h"
#include "src/utils/security_utils.h"
#include "src/utils/memory_utils.h"
#include "src/components/fingerprint.h"
#include "src/components/network.h"
#include "src/webserver/server_init.h"

// Global variable to track system status
bool systemReady = false;
bool firebaseConnected = false;
bool fingerprintReady = false;
bool sdCardReady = false;
bool wifiConnected = false;

void setup() {
  Serial.begin(115200);

  // Check initial memory
  Serial.println("Initial free memory: " + String(getFreeMemory()) + " bytes");

  // Initialize RGB LED
  rgbLED.begin();
  rgbLED.setPixelColor(0, rgbLED.Color(0, 0, 0));
  rgbLED.show();

  // Initialize display
  tft.init(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(2, 20);
  tft.println("System starting...");

  // Initialize SD card with error handling
  int sdRetries = 0;
  while (!setsd() && sdRetries < 3) {
    Serial.println("SD card initialization failed, retrying...");
    delay(1000);
    sdRetries++;
  }

  if (sdRetries >= 3) {
    Serial.println("Failed to initialize SD card after 3 attempts");
    tft.fillScreen(TFT_WHITE);
    tft.fillRect(0, 15, 128, 30, TFT_WHITE);
    tft.setTextColor(TFT_RED);
    tft.setCursor(2, 20);
    tft.println("SD Card Error!");
    tft.setCursor(2, 30);
    tft.println("Check SD card or");
    tft.setCursor(2, 40);
    tft.println("press reset to retry");
    return;
  } else {
    sdCardReady = true;
  }

  // Load Telegram credentials
  readTelegramCredentials();

  // Initialize WiFi with timeout
  tft.fillScreen(TFT_WHITE);
  tft.fillRect(0, 15, 128, 30, TFT_WHITE);
  tft.setCursor(2, 20);
  tft.println("Connecting to WiFi...");

  unsigned long wifiStartTime = millis();
  setupWiFi();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStartTime < 30000) {
    delay(500);
    tft.setCursor(2, 30);
    tft.println("Connecting... " + String((millis() - wifiStartTime) / 1000) + "s");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection failed after 30 seconds");
    tft.fillScreen(TFT_WHITE);
    tft.fillRect(0, 15, 128, 30, TFT_WHITE);
    tft.setTextColor(TFT_RED);
    tft.setCursor(2, 20);
    tft.println("WiFi Error!");
    tft.setCursor(2, 30);
    tft.println("Check credentials or");
    tft.setCursor(2, 40);
    tft.println("network availability");
    return;
  } else {
    wifiConnected = true;
    tft.setCursor(2, 30);
    tft.println("Connected to:");
    tft.setCursor(2, 40);
    tft.println(WiFi.SSID());
  }

  // Initialize time
  timeInit();

  // Initialize Firebase with error handling
  tft.fillScreen(TFT_WHITE);
  tft.fillRect(0, 15, 128, 30, TFT_WHITE);
  tft.setCursor(2, 20);
  tft.println("Connecting to Firebase...");

  if (!SetDB()) {
    Serial.println("Firebase initialization failed!");
    tft.fillScreen(TFT_WHITE);
    tft.fillRect(0, 15, 128, 30, TFT_WHITE);
    tft.setTextColor(TFT_RED);
    tft.setCursor(2, 20);
    tft.println("Firebase Error!");
    tft.setCursor(2, 30);
    tft.println("Check credentials");
    tft.setCursor(2, 40);
    tft.println("Will continue w/o sync");
    delay(2000);
  } else {
    firebaseConnected = true;
  }

  // Initialize fingerprint sensor with retries
  tft.fillScreen(TFT_WHITE);
  tft.fillRect(0, 15, 128, 30, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(2, 20);
  tft.println("Init fingerprint sensor...");

  int fpRetries = 0;
  while (!setupFingerprint() && fpRetries < 3) {
    Serial.println("Fingerprint sensor initialization failed, retrying...");
    tft.setCursor(2, 30);
    tft.println("Retry " + String(fpRetries + 1) + "/3");
    delay(1000);
    fpRetries++;
  }

  if (fpRetries >= 3) {
    Serial.println("Failed to initialize fingerprint sensor after 3 attempts");
    tft.fillScreen(TFT_WHITE);
    tft.fillRect(0, 15, 128, 30, TFT_WHITE);
    tft.setTextColor(TFT_RED);
    tft.setCursor(2, 20);
    tft.println("Fingerprint Error!");
    tft.setCursor(2, 30);
    tft.println("Check sensor or");
    tft.setCursor(2, 40);
    tft.println("press reset to retry");
    return;
  } else {
    fingerprintReady = true;
    systemReady = true;
  }

  // Initialize temperature sensor
  sensors.begin();

  // Initialize server
  serverInit();

  // Check final memory
  Serial.println("Setup complete. Free memory: " + String(getFreeMemory()) + " bytes");

  tft.fillScreen(TFT_WHITE);
  tft.fillRect(0, 15, 128, 30, TFT_WHITE);
  tft.setCursor(2, 20);
  tft.setTextColor(TFT_BLACK);
  tft.println("System ready!");
  tft.println("IP: " + WiFi.localIP().toString());

  // Display system status
  String statusMsg = "";
  if (!sdCardReady) statusMsg += "SD: NOK ";
  if (!fingerprintReady) statusMsg += "FP: NOK ";
  if (!wifiConnected) statusMsg += "WiFi: NOK ";
  if (!firebaseConnected) statusMsg += "FB: NOK";

  if (statusMsg != "") {
    tft.setCursor(2, 50);
    tft.setTextColor(TFT_RED);
    tft.println(statusMsg);
  }

  // After WiFi is connected and IP is obtained
  if (WiFi.status() == WL_CONNECTED) {
    String ipMessage = "System Started!\nIP Address: " + WiFi.localIP().toString();
    sendTelegramMessage(ipMessage.c_str());
  }
}

void loop() {
  // Skip processing if system is not ready
  if (!systemReady) {
    delay(1000);  // Wait before checking again
    return;
  }

  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiConnected) {
      Serial.println("WiFi disconnected, attempting to reconnect...");
      tft.fillScreen(TFT_WHITE);
      tft.fillRect(0, 15, 128, 30, TFT_WHITE);
      tft.setTextColor(TFT_RED);
      tft.setCursor(2, 20);
      tft.println("WiFi disconnected!");
      tft.setCursor(2, 30);
      tft.println("Reconnecting...");
      wifiConnected = false;
    }
    WiFi.reconnect();
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      tft.fillScreen(TFT_WHITE);
      tft.fillRect(0, 15, 128, 30, TFT_WHITE);
      tft.setTextColor(TFT_GREEN);
      tft.setCursor(2, 20);
      tft.println("WiFi reconnected!");
      delay(1000);
    }
  }

  // Check memory periodically
  static unsigned long lastMemCheck = 0;
  if (millis() - lastMemCheck >= 60000) {  // Check every minute
    int freeMemory = getFreeMemory();
    Serial.println("Free memory: " + String(freeMemory) + " bytes");

    // Warning if memory is low
    if (freeMemory < 10000) {
      Serial.println("WARNING: Low memory!");
      tft.setTextColor(TFT_RED);
      tft.setCursor(2, 40);
      tft.println("Low memory: " + String(freeMemory) + " bytes");
    }
    lastMemCheck = millis();
  }

  // Update time display periodically
  if (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    updateTimeDisplay();
    lastDisplayUpdate = millis();
  }

  // Handle server requests
  server.handleClient();

  // Add debug logging for scanning status
  static unsigned long lastScanStatusCheck = 0;
  if (millis() - lastScanStatusCheck >= 5000) {  // Check every 5 seconds
    Serial.println("Scanning conditions: isBlinking=" + String(isBlinking ? "true" : "false") + 
                    ", fingerprintReady=" + String(fingerprintReady ? "true" : "false") + 
                    ", Will scan: " + String((isBlinking && fingerprintReady) ? "YES" : "NO"));
    lastScanStatusCheck = millis();
  }

  // Handle fingerprint scanning with error checking
  if (isBlinking && fingerprintReady) {
    continuousFingerprintScan();
  }

  delay(10);
}