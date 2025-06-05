#include "fingerprint.h"
#include "../utils/display_utils.h"
#include "../utils/time_utils.h"
#include "../utils/sd_utils.h"

bool setupFingerprint() {
  fingerSerial.begin(57600, SERIAL_8N1, RX_PIN, TX_PIN);
  finger.begin(57600);
  if (finger.verifyPassword()) {
    tft.fillScreen(TFT_WHITE);                      // Clear the screen
    tft.fillRect(0, 15, 128, 30, TFT_WHITE);        // Clear area below time and WiFi icon
    tft.setCursor(2, 20);                           // Start below the time/WiFi area
    tft.setTextColor(TFT_BLACK);                    // Set text color to black
    tft.setTextSize(1);                             // Set text size
    tft.println("Fingerprint sensor initialized");  // Display message on TFT
    return true;
  } else {
    digitalWrite(25, 1);
    Serial.println("Failed to initialize fingerprint sensor. Check connections.");
    return false;
  }

  finger.getTemplateCount();
  Serial.println("Templates: " + String(finger.templateCount));
  return true;
}

// Function to save fingerprint template to SD card
bool saveTemplateToSD(uint16_t id, const uint8_t *templateData, uint16_t templateSize) {
  // Create fingerprint templates directory if it doesn't exist
  if (!SD.exists("/fingerprints")) {
    if (!SD.mkdir("/fingerprints")) {
      Serial.println("Failed to create fingerprints directory");
      return false;
    }
  }

  // Create file path for the template
  String filePath = "/fingerprints/" + String(id) + ".dat";
  
  // Open file for writing
  File file = SD.open(filePath, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to create template file");
    return false;
  }

  // Write template data
  if (file.write(templateData, templateSize) != templateSize) {
    Serial.println("Failed to write template data");
    file.close();
    return false;
  }

  file.close();
  return true;
}

// Original scanFingerprint function with modifications
void scanFingerprint() {
  rgbLED.setPixelColor(0, rgbLED.Color(0, 0, 0));
  rgbLED.show();  // Ensure LED is off initially
  Serial.println("Starting fingerprint enrollment...");
  tft.fillScreen(TFT_WHITE);                // Clear the screen
  tft.fillRect(0, 15, 128, 30, TFT_WHITE);  // Clear area below time and WiFi icon
  tft.setCursor(2, 20);                     // Start below the time/WiFi area
  tft.setTextColor(TFT_BLACK);              // Set text color to black
  tft.setTextSize(1);                       // Set text size
  tft.println("Place your finger on the scanner...");

  // Set RGB LED to blue (waiting for scan)
  setRGBColor(0, 0, 55);

  // Wait for the finger to be placed on the scanner
  while (finger.getImage() != FINGERPRINT_OK) {
    delay(500);
  }

  // Convert the first image to a template
  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("Failed to convert first fingerprint image to template.");
    tft.println("Failed to process fingerprint.");
    rgbLED.setPixelColor(0, rgbLED.Color(55, 0, 0));
    rgbLED.show();  // Set RGB LED to red (failure)
    server.send(200, "text/plain", "Failed to process fingerprint");
    return;
  }
  rgbLED.setPixelColor(0, rgbLED.Color(55, 35, 0));
  rgbLED.show();  // Set RGB LED to orange (scanning)
  Serial.println("Keep your finger on the scanner...");
  tft.println("Keep your finger on the scanner...");

  // Wait for the second scan
  delay(2000);  // Add a delay to ensure the sensor is ready for the second scan

  // Convert the second image to a template
  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    Serial.println("Failed to convert second fingerprint image to template.");
    tft.println("Failed to process fingerprint.");
    rgbLED.setPixelColor(0, rgbLED.Color(55, 0, 0));
    rgbLED.show();  // Set RGB LED to red (failure)
    server.send(200, "text/plain", "Failed to process fingerprint");
    return;
  }

  // Check if the fingerprint already exists in the sensor database
  if (finger.fingerFastSearch() == FINGERPRINT_OK) {
    Serial.println("Duplicate fingerprint detected!");
    tft.println("Duplicate fingerprint detected!");
    rgbLED.setPixelColor(0, rgbLED.Color(55, 0, 0));  // Changed from yellow to red for better visibility
    rgbLED.show();
    server.send(200, "text/plain", "Duplicate fingerprint detected");
    return;
  }

  // Create a model from the two templates
  if (finger.createModel() != FINGERPRINT_OK) {
    Serial.println("Failed to create fingerprint model. Ensure the same finger is used.");
    tft.println("Failed to create fingerprint model.");
    rgbLED.setPixelColor(0, rgbLED.Color(55, 0, 0));
    rgbLED.show();  // Set RGB LED to red (failure)
    server.send(200, "text/plain", "Failed to create fingerprint model");
    return;
  }

  // Store the model in the fingerprint sensor
  if (finger.storeModel(addid) == FINGERPRINT_OK) {
    Serial.println("Fingerprint enrolled successfully!");
    tft.println("Fingerprint enrolled successfully!");
    
    // Get the template data from the sensor
    uint8_t templateBuffer[512];  // Buffer to store template data
    uint16_t templateSize = 0;
    
    if (finger.getTemplate(addid, templateBuffer, &templateSize) == FINGERPRINT_OK) {
      // Save template to SD card
      if (saveTemplateToSD(addid, templateBuffer, templateSize)) {
        Serial.println("Template backup saved to SD card");
        tft.println("Template backup saved");
        setRGBColor(0, 255, 0);  // Set RGB LED to green (complete success)
      } else {
        Serial.println("Failed to save template backup");
        tft.println("Warning: Backup failed");
        setRGBColor(0, 255, 55);  // Set RGB LED to blue-green (partial success)
      }
    } else {
      Serial.println("Failed to read template from sensor");
      tft.println("Warning: Backup failed");
      setRGBColor(0, 255, 55);  // Set RGB LED to blue-green (partial success)
    }
    
    server.send(200, "text/plain", "Fingerprint enrolled successfully");
  } else {
    Serial.println("Failed to store fingerprint model.");
    tft.println("Failed to store fingerprint model.");
    setRGBColor(255, 0, 0);  // Set RGB LED to red (failure)
    server.send(200, "text/plain", "Failed to store fingerprint model");
  }

  // Reset the fingerprint sensor
  Serial.println("Resetting fingerprint sensor...");
  tft.println("Resetting fingerprint sensor...");
  while (finger.getImage() != FINGERPRINT_NOFINGER) {
    delay(100);  // Wait for the finger to be removed
  }
  delay(1000);  // Allow the sensor to reset
  Serial.println("Fingerprint sensor reset complete.");
  tft.println("Fingerprint sensor reset complete.");
  setRGBColor(0, 0, 0);  // Turn off the RGB LED
}

void continuousFingerprintScan() {
  static bool scanningInProgress = false;
  static unsigned long lastScanTime = 0;
  static unsigned long lastStatusTime = 0;
  static unsigned long lastDebugTime = 0;
  
  // Debug log every 10 seconds to check if the function is being called
  if (millis() - lastDebugTime > 10000) { // Every 10 seconds
    Serial.println("continuousFingerprintScan function called, isBlinking = " + String(isBlinking ? "true" : "false"));
    lastDebugTime = millis();
  }

  if (!isBlinking) {
    if (scanningInProgress) {
      Serial.println("Continuous scanning stopped.");
      displayStatusMessage("Continuous scanning stopped.", TFT_WHITE);
      scanningInProgress = false;
      setRGBColor(0, 0, 0);  // Turn off the RGB LED
    }
    return;
  }

  if (!scanningInProgress) {
    Serial.println("Starting continuous fingerprint scanning...");
    displayStatusMessage("Scanning in progress...", TFT_WHITE);
    scanningInProgress = true;
    setRGBColor(0, 0, 55);  // Set RGB LED to blue when starting
  }

  // Provide periodic status updates to confirm scanning is active
  if (millis() - lastStatusTime > 5000) { // Every 5 seconds
    Serial.println("Continuous scan active - waiting for fingerprint");
    lastStatusTime = millis();
  }

  // Update time display periodically
  if (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    updateTimeDisplay();
    lastDisplayUpdate = millis();
  }

  if (millis() - lastScanTime < 1000) {  // Add a 1-second delay between scans
    return;
  }
  
  // Print status information
  uint8_t fingerStatus = finger.getImage();
  if (fingerStatus != FINGERPRINT_NOFINGER && fingerStatus != FINGERPRINT_OK) {
    Serial.print("Fingerprint sensor status: ");
    switch (fingerStatus) {
      case FINGERPRINT_PACKETRECIEVEERR: Serial.println("Communication error"); break;
      case FINGERPRINT_IMAGEFAIL: Serial.println("Imaging error"); break;
      default: Serial.println("Unknown error: " + String(fingerStatus)); break;
    }
  }
  
  lastScanTime = millis();

  // Perform fingerprint scanning
  if (fingerStatus == FINGERPRINT_OK) {
    Serial.println("Image taken, processing...");
    if (finger.image2Tz() == FINGERPRINT_OK) {
      if (finger.fingerFastSearch() == FINGERPRINT_OK) {
        int fingerId = finger.fingerID;
        String currentDate = getCurrentDate();
        String currentTime = getCurrentTime12(); // Use 12-hour format
        
        // Check if this fingerprint has an entry for today
        bool hasEntry = false;
        String inTime = "";
        String outTime = "";
        String foundRoll = "";
        String foundName = "";

        // First, get the name and roll number from students.csv
        File nameFile = SD.open("/students.csv", FILE_READ);
        if (nameFile) {
          // Skip header line
          if (nameFile.available()) {
            nameFile.readStringUntil('\n');
          }
          
          while (nameFile.available()) {
            String id, roll, nameStr;
            if (readCSVLine(nameFile, id, roll, nameStr)) {
              if (id.toInt() == fingerId) {
                foundRoll = roll;
                foundName = nameStr;
                break;
              }
            }
          }
          nameFile.close();
        }

        if (foundName != "") {
          // Now check for existing entry
          String filePath = getAttendanceFilePath(currentDate);
          if (ensureAttendanceDirectory(currentDate)) {
            // Create file with header if it doesn't exist
            if (!SD.exists(filePath)) {
              if (!createAttendanceCSVFile(filePath)) {
                Serial.println("Failed to create attendance file");
                displayStatusMessage("File creation failed", TFT_RED);
                return;
              }
            }

            File file = SD.open(filePath, FILE_READ);
            if (file) {
              // Skip header line
              if (file.available()) {
                file.readStringUntil('\n');
              }

              while (file.available()) {
                String roll, name, id, in, out;
                if (readAttendanceCSVLine(file, roll, name, id, in, out)) {
                  if (id.toInt() == fingerId) {
                    hasEntry = true;
                    inTime = in;
                    outTime = out;
                    break;
                  }
                }
              }
              file.close();

              // Write the attendance record to the file
              if (!hasEntry) {
                // First scan - record in-time
                file = SD.open(filePath, FILE_APPEND);
                if (file) {
                  writeAttendanceCSVLine(file, foundRoll, foundName, String(fingerId), currentTime, "-");
                  file.close();
                  Serial.println("In-time recorded - ID: " + String(fingerId) + ", Roll: " + foundRoll + ", Name: " + foundName);

                  // Upload to Firebase immediately
                  if (Firebase.ready()) {
                    String month = currentDate.substring(3, 5);
                    String path = "/attendance/" + month + "/" + currentDate + "/" + String(fingerId);
                    FirebaseJson json;
                    json.set("name", foundName);
                    json.set("rollNumber", foundRoll);
                    json.set("inTime", currentTime);
                    json.set("outTime", "-");

                    if (Firebase.setJSON(firebaseData, path.c_str(), json)) {
                      Serial.println("Attendance uploaded to Firebase successfully");
                    } else {
                      Serial.println("Failed to upload attendance to Firebase");
                      Serial.println("Error: " + firebaseData.errorReason());
                    }
                  }

                  // Update display
                  displayAttendanceRecord(fingerId, foundRoll, foundName, true);
                  setRGBColor(0, 55, 0);  // Set RGB LED to green for successful scan
                  delay(1000);            // Keep green for 1 second
                  setRGBColor(0, 0, 55);  // Return to blue
                }
              } else if (outTime == "-") {
                // Second scan - update out-time
                // Create a temporary file
                String tempPath = filePath + ".tmp";
                File tempFile = SD.open(tempPath, FILE_WRITE);
                if (tempFile) {
                  // Write header
                  tempFile.println("Roll Number,Name,Fingerprint ID,In Time,Out Time");

                  // Copy all records, updating the matching one
                  file = SD.open(filePath, FILE_READ);
                  if (file) {
                    // Skip header
                    if (file.available()) {
                      file.readStringUntil('\n');
                    }

                    while (file.available()) {
                      String roll, name, id, in, out;
                      if (readAttendanceCSVLine(file, roll, name, id, in, out)) {
                        if (id.toInt() == fingerId) {
                          // Update the out time for this record
                          writeAttendanceCSVLine(tempFile, roll, name, id, in, currentTime);
                        } else {
                          // Copy the record as is
                          writeAttendanceCSVLine(tempFile, roll, name, id, in, out);
                        }
                      }
                    }
                    file.close();
                  }
                  tempFile.close();

                  // Replace the original file with the temporary file
                  if (SD.remove(filePath) && SD.rename(tempPath, filePath)) {
                    Serial.println("Out-time recorded - ID: " + String(fingerId) + ", Roll: " + foundRoll + ", Name: " + foundName);

                    // Upload to Firebase
                    if (Firebase.ready()) {
                      String month = currentDate.substring(3, 5);
                      String path = "/attendance/" + month + "/" + currentDate + "/" + String(fingerId);
                      FirebaseJson json;
                      json.set("name", foundName);
                      json.set("rollNumber", foundRoll);
                      json.set("inTime", inTime);
                      json.set("outTime", currentTime);

                      if (Firebase.setJSON(firebaseData, path.c_str(), json)) {
                        Serial.println("Out-time uploaded to Firebase successfully");
                      } else {
                        Serial.println("Failed to upload out-time to Firebase");
                        Serial.println("Error: " + firebaseData.errorReason());
                      }
                    }

                    // Update display
                    displayAttendanceRecord(fingerId, foundRoll, foundName, false);
                    setRGBColor(0, 55, 0);  // Set RGB LED to green for successful scan
                    delay(1000);            // Keep green for 1 second
                    setRGBColor(0, 0, 55);  // Return to blue
                  } else {
                    Serial.println("Failed to update attendance file");
                    displayStatusMessage("Failed to update record", TFT_RED);
                  }
                } else {
                  Serial.println("Failed to create temp file");
                  displayStatusMessage("Failed to update record", TFT_RED);
                }
              } else {
                Serial.println("Student already marked present and out");
                displayStatusMessage("Already marked out", TFT_YELLOW);
                setRGBColor(55, 35, 0);  // Set RGB LED to orange
                delay(1000);            // Keep orange for 1 second
                setRGBColor(0, 0, 55);  // Return to blue
              }
            }
          }
        } else {
          Serial.println("Fingerprint ID not found in students database");
          displayStatusMessage("ID not found", TFT_RED);
          setRGBColor(55, 0, 0);  // Set RGB LED to red
          delay(1000);            // Keep red for 1 second
          setRGBColor(0, 0, 55);  // Return to blue
        }
      } else {
        Serial.println("No match found");
        displayStatusMessage("No match found", TFT_RED);
        setRGBColor(55, 0, 0);  // Set RGB LED to red
        delay(1000);            // Keep red for 1 second
        setRGBColor(0, 0, 55);  // Return to blue
      }
    } else {
      Serial.println("Failed to convert image");
      displayStatusMessage("Image error", TFT_RED);
      setRGBColor(55, 0, 0);  // Set RGB LED to red
      delay(1000);            // Keep red for 1 second
      setRGBColor(0, 0, 55);  // Return to blue
    }
  }
}