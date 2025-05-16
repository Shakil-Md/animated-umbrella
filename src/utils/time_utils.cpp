#include "time_utils.h"

void timeInit() {
  // Configure time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  while (!getLocalTime(&timeinfo)) {
    tft.fillScreen(TFT_WHITE);                // Clear the screen
    tft.fillRect(0, 15, 128, 30, TFT_WHITE);  // Clear area below time and WiFi icon
    tft.setCursor(2, 20);                     // Start below the time/WiFi area
    tft.setTextColor(TFT_BLACK);              // Set text color to black
    tft.setTextSize(1);                       // Set text size
    tft.println("Waiting for time sync...");
    delay(1000);
  }

  // Update date variables with leading zeros for single-digit values
  day = (timeinfo.tm_mday < 10) ? "0" + String(timeinfo.tm_mday) : String(timeinfo.tm_mday);
  month = (timeinfo.tm_mon + 1 < 10) ? "0" + String(timeinfo.tm_mon + 1) : String(timeinfo.tm_mon + 1);
  year = String(timeinfo.tm_year + 1900);
}

String getCurrentDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "00-00-0000";
  }
  char dateStr[11];
  strftime(dateStr, sizeof(dateStr), "%d-%m-%Y", &timeinfo);
  return String(dateStr);
}

String getCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "00:00:00";
  }
  char timeStr[9];
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  return String(timeStr);
}

String getCurrentTime12() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "00:00:00 AM";
  }
  char timeStr[12];
  strftime(timeStr, sizeof(timeStr), "%I:%M:%S %p", &timeinfo);
  return String(timeStr);
} 