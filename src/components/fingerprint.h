#ifndef FINGERPRINT_H
#define FINGERPRINT_H

#include "../config/config.h"

// External declarations
extern Adafruit_Fingerprint finger;

// Function declarations for fingerprint module
bool setupFingerprint();
void scanFingerprint();
void continuousFingerprintScan();

#endif // FINGERPRINT_H 