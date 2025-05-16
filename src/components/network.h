#ifndef NETWORK_H
#define NETWORK_H

#include "../config/config.h"

// Function declarations for network module
void setupWiFi();
bool connectWifi(String ssid, String password);
bool SetDB();
void sendTelegramMessage(const char *message);
void updateWiFiCredentials();
void uploadNamesToFirebase();
void syncAttendanceWithFirebase();

#endif // NETWORK_H 