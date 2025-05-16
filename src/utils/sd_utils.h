#ifndef SD_UTILS_H
#define SD_UTILS_H

#include "../config/config.h"

// Function declarations for SD card utilities
bool setsd();
String getAttendanceFilePath(String dateStr);
bool ensureAttendanceDirectory(String dateStr);
bool checkSDCardStatus();
void loadStudentData();
bool readFirebaseCredentials();
bool readWiFiCredentials(String &ssid, String &password);
void readTelegramCredentials();

#endif // SD_UTILS_H 