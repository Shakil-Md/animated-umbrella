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
    setRGBColor(0, 255, 0);  // Set RGB LED to green (success)
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
        String existingRow = "";
        String foundRoll = "";
        String foundName = "";

        // First, get the name and roll number
        File nameFile = SD.open("/name.txt", FILE_READ);
        if (nameFile) {
          while (nameFile.available()) {
            String line = nameFile.readStringUntil('\n');
            int firstSpace = line.indexOf(' ');
            if (firstSpace > 0) {
              String idStr = line.substring(0, firstSpace);
              if (idStr.toInt() == fingerId) {
                String remaining = line.substring(firstSpace + 1);
                int secondSpace = remaining.indexOf(' ');
                if (secondSpace > 0) {
                  foundRoll = remaining.substring(0, secondSpace);
                  foundName = remaining.substring(secondSpace + 1);
                }
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
            File file = SD.open(filePath, FILE_READ);
            if (file) {
              while (file.available()) {
                String line = file.readStringUntil('\n');
                line.trim();
                if (line.startsWith("<tr>") && line.endsWith("</tr>")) {
                  // Extract fingerprint ID from the line
                  int idStart = line.indexOf("<td>", line.indexOf("<td>", line.indexOf("<td>") + 4) + 4) + 4;
                  int idEnd = line.indexOf("</td>", idStart);
                  if (idStart > 0 && idEnd > 0) {
                    String lineId = line.substring(idStart, idEnd);
                    if (lineId.toInt() == fingerId) {
                      hasEntry = true;
                      existingRow = line;
                      // Extract in-time and out-time
                      int inTimeStart = line.indexOf("<td>", idEnd) + 4;
                      int inTimeEnd = line.indexOf("</td>", inTimeStart);
                      int outTimeStart = line.indexOf("<td>", inTimeEnd) + 4;
                      int outTimeEnd = line.indexOf("</td>", outTimeStart);
                      if (inTimeStart > 0 && inTimeEnd > 0) {
                        inTime = line.substring(inTimeStart, inTimeEnd);
                        inTime.trim();
                      }
                      if (outTimeStart > 0 && outTimeEnd > 0) {
                        outTime = line.substring(outTimeStart, outTimeEnd);
                        outTime.trim();
                      }
                    }
                  }
                }
              }
              file.close();
            }

            // Write the attendance record to the file
            if (!hasEntry) {
              // First scan - record in-time
              file = SD.open(filePath, FILE_APPEND);
              if (file) {
                file.println("<tr><td>" + foundRoll + "</td><td>" + foundName + "</td><td>" + String(fingerId) + "</td><td>" + currentTime + "</td><td>-</td></tr>");
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
            } else if (inTime != "" && outTime == "-") {
              // Second scan - update with out-time
              Serial.println("Processing out-time update...");
              Serial.println("Current time: " + currentTime);
              Serial.println("Existing row: " + existingRow);

              // First, read all lines
              File file = SD.open(filePath, FILE_READ);
              if (file) {
                Serial.println("Successfully opened file for reading");
                String trimmedExistingRow = existingRow;
                trimmedExistingRow.trim();
                
                // Create a new temp file to avoid memory issues with large strings
                String tempFilePath = filePath + ".tmp";
                File tempFile = SD.open(tempFilePath, FILE_WRITE);
                
                if (tempFile) {
                  // Process line by line to avoid memory issues
                  while (file.available()) {
                    String line = file.readStringUntil('\n');
                    line.trim();
                    if (line != trimmedExistingRow) {
                      tempFile.println(line);
                    } else {
                      // Replace the existing row with the updated one
                      String newRow = "<tr><td>" + foundRoll + "</td><td>" + foundName + "</td><td>" + String(fingerId) + "</td><td>" + inTime + "</td><td>" + currentTime + "</td></tr>";
                      Serial.println("New row to be written: " + newRow);
                      tempFile.println(newRow);
                    }
                  }
                  file.close();
                  tempFile.close();
                  
                  // Delete original file and rename temp file
                  SD.remove(filePath);
                  SD.rename(tempFilePath, filePath);
                  
                  Serial.println("Successfully wrote updated data to file");

                  // Upload updated out-time to Firebase
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
                  Serial.println("Failed to create temp file");
                  displayStatusMessage("Failed to update record", TFT_RED);
                }
              } else {
                Serial.println("Failed to open file for reading");
                displayStatusMessage("Failed to read record", TFT_RED);
              }
            } else if (inTime != "" && outTime != "-") {
              // Already has both in-time and out-time - duplicate entry
              Serial.println("Duplicate entry - ID: " + String(fingerId) + ", Roll: " + foundRoll + ", Name: " + foundName);

              // Update display
              tft.fillRect(0, 45, 128, 83, TFT_WHITE);
              tft.setTextColor(TFT_BLACK);
              tft.setTextSize(1);
              tft.setCursor(2, 50);
              tft.println("Duplicate Entry!");
              tft.setCursor(2, 60);
              tft.println("ID: " + String(fingerId));
              tft.setCursor(2, 70);
              tft.println("Roll: " + foundRoll);
              tft.setCursor(2, 80);
              tft.println("Name: " + foundName);
              tft.setCursor(2, 90);
              tft.println("Already recorded");

              setRGBColor(55, 55, 0);  // Set RGB LED to yellow for duplicate
              delay(1000);             // Keep yellow for 1 second
              setRGBColor(0, 0, 55);   // Return to blue
            }
          }
        }
      }
    }
  }
} 