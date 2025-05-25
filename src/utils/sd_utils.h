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

// CSV utility functions
String escapeCSV(String input);
String unescapeCSV(String input);
bool writeCSVLine(File &file, String id, String roll, String name);
bool readCSVLine(File &file, String &id, String &roll, String &name);

#endif // SD_UTILS_H 