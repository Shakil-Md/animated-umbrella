#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include "../config/config.h"

// Function declarations for display utilities
void drawWiFiIcon(bool isConnected);
void updateTimeDisplay();
void displayStatusMessage(const char *message, uint16_t color);
void displayAttendanceRecord(int id, String roll, String name, bool isInTime);
String getFormattedTime();
void setRGBColor(uint8_t red, uint8_t green, uint8_t blue);

#endif // DISPLAY_UTILS_H 