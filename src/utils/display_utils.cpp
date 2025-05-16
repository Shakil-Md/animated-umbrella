#include "display_utils.h"

void drawWiFiIcon(bool isConnected) {
  // Draw WiFi icon in top-right corner
  int iconX = tft.width() - 30;  // 30 pixels from right edge for better spacing
  int iconY = 2;                 // 2 pixels from top for better alignment

  if (isConnected) {
    // Draw connected WiFi icon (4 arcs) with better spacing and smoother appearance
    // Parameters: x, y, outer radius, inner radius, start angle, end angle, foreground color, background color, smooth arc
    tft.drawArc(iconX + 12, iconY + 12, 10, 6, 135, 225, TFT_BLUE, TFT_WHITE, true);  // Outer arc
    tft.drawArc(iconX + 12, iconY + 12, 7, 4, 135, 225, TFT_BLUE, TFT_WHITE, true);   // Second arc
    tft.drawArc(iconX + 12, iconY + 12, 4, 3, 135, 225, TFT_BLUE, TFT_WHITE, true);   // Third arc
    tft.drawArc(iconX + 12, iconY + 12, 2, 1, 135, 225, TFT_BLUE, TFT_WHITE, true);   // Inner arc
    // Draw dot in center
    tft.fillCircle(iconX + 12, iconY + 12, 1, TFT_BLUE);
  } else {
    // Draw disconnected WiFi icon (X mark)
    tft.drawLine(iconX + 5, iconY + 5, iconX + 20, iconY + 20, TFT_RED);
    tft.drawLine(iconX + 20, iconY + 5, iconX + 5, iconY + 20, TFT_RED);
  }
}

void updateTimeDisplay() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }

  // Only update if seconds have changed
  if (timeinfo.tm_sec != lastSecond) {
    // Clear the time area
    tft.fillRect(0, 0, 128, 15, TFT_WHITE);

    // Format time in 12-hour format
    char timeStr[12];
    strftime(timeStr, sizeof(timeStr), "%I:%M:%S %p", &timeinfo);

    // Display time
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(2, 2);
    tft.println(timeStr);

    // Update WiFi icon
    drawWiFiIcon(WiFi.status() == WL_CONNECTED);

    lastSecond = timeinfo.tm_sec;
  }
}

void displayStatusMessage(const char *message, uint16_t color) {
  // Clear the status area
  tft.fillRect(0, 15, 128, 30, TFT_WHITE);

  // Display the message
  tft.setTextColor(color);
  tft.setTextSize(1);
  tft.setCursor(2, 20);
  tft.println(message);
}

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Failed to get time";
  }

  char timeString[12];
  strftime(timeString, sizeof(timeString), "%I:%M:%S %p", &timeinfo);
  return String(timeString);
}

void displayAttendanceRecord(int id, String roll, String name, bool isInTime) {
  // Clear the content area
  tft.fillRect(0, 45, 128, 83, TFT_WHITE);

  // Display the record
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);

  // Display ID
  tft.setCursor(2, 50);
  tft.print("ID: ");
  tft.println(id);

  // Display Roll Number
  tft.setCursor(2, 60);
  tft.print("Roll: ");
  tft.println(roll);

  // Display Name
  tft.setCursor(2, 70);
  tft.print("Name: ");
  tft.println(name);

  // Display Status
  tft.setCursor(2, 80);
  tft.print("Status: ");
  tft.println(isInTime ? "IN" : "OUT");

  // Display Time
  tft.setCursor(2, 90);
  tft.print("Time: ");
  tft.println(getFormattedTime());
}

void setRGBColor(uint8_t red, uint8_t green, uint8_t blue) {
  rgbLED.setPixelColor(0, rgbLED.Color(red, green, blue));
  rgbLED.show();
} 