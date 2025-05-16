#include "network.h"
#include "../utils/sd_utils.h"
#include "../utils/display_utils.h"

bool connectWifi(String ssid, String password) {
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting to WiFi...");
  tft.fillScreen(TFT_WHITE);                // Clear the screen
  tft.fillRect(0, 15, 128, 30, TFT_WHITE);  // Clear area below time and WiFi icon
  tft.setCursor(2, 20);                     // Start below the time/WiFi area
  tft.setTextColor(TFT_BLACK);              // Set text color to black
  tft.setTextSize(1);                       // Set text size
  tft.println("Connecting to WiFi...");
  drawWiFiIcon(false);  // Show disconnected WiFi icon

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {  // Retry for 10 seconds
    Serial.print(".");
    tft.print(".");  // Show progress on TFT
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected using  card credentials.");
    Serial.println("IP Address: " + WiFi.localIP().toString());
    return true;
  }
  return false;
}

void setupWiFi() {
  String ssid, password;
  // Read WiFi credentials from the SD card
  if (!readWiFiCredentials(ssid, password)) {
    Serial.println("Failed to read WiFi credentials from SD card.");
    ssid = "Realme 8";      // Fallback SSID
    password = "88888888";  // Fallback password
  }
  Serial.print("Connecting to WiFi using SD card credentials...");
  bool wifistatus = connectWifi(ssid, password);
  if (wifistatus) {
    Serial.println("\nWiFi connected using SD credentials.");
    return;
  } else {
    if (connectWifi("Realme 8", "88888888")) {
      Serial.println("\nWiFi connected using fallback credentials.");
      return;
    }
  }
  // If both SD card and fallback credentials fail, prompt to update WiFi credentials
  Serial.println("\nFailed to connect to WiFi. Do you want to update WiFi credentials? (y/n)");
  tft.println("\nFailed to connect to WiFi.");
  tft.println("Update WiFi credentials? (y/n)");

  while (!Serial.available()) {
    delay(100);
  }

  char response = Serial.read();
  if (response == 'y' || response == 'Y') {
    updateWiFiCredentials();
    setupWiFi();  // Retry with updated credentials
  } else {
    Serial.println("WiFi connection failed. Proceeding without WiFi.");
    tft.println("WiFi connection failed.");
  }
}

bool SetDB() {
  // Attempt to read Firebase credentials from the SD card
  if (!readFirebaseCredentials()) {
    Serial.println("Failed to read Firebase credentials from SD card.");
    return false;
  }

  // Initialize Firebase
  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);

  if (Firebase.ready()) {
    Serial.println("Firebase initialized successfully.");
    return true;
  } else {
    Serial.println("Failed to initialize Firebase.");
    return false;
  }
}

void updateWiFiCredentials() {
  Serial.println("Enter new WiFi SSID:");
  String newSSID = "";
  while (newSSID.length() == 0) {  // Wait until a valid input is received
    if (Serial.available()) {
      newSSID = Serial.readStringUntil('\n');
      newSSID.trim();  // Remove any leading/trailing whitespace
    }
  }

  Serial.println("Enter new WiFi Password:");
  String newPassword = "";
  while (newPassword.length() == 0) {  // Wait until a valid input is received
    if (Serial.available()) {
      newPassword = Serial.readStringUntil('\n');
      newPassword.trim();  // Remove any leading/trailing whitespace
    }
  }

  // Save the new credentials to the SD card
  File file = SD.open("/wifi.txt", FILE_WRITE);
  if (file) {
    file.println("SSID=" + newSSID);
    file.println("PASSWORD=" + newPassword);
    file.close();
    Serial.println("WiFi credentials updated successfully.");
  } else {
    Serial.println("Failed to update WiFi credentials.");
  }
}

void uploadNamesToFirebase() {
  if (firebaseConfig.host == "" || firebaseConfig.signer.tokens.legacy_token == "") {
    Serial.println("Firebase credentials are not set. Cannot upload names to Firebase.");
    return;
  }

  Serial.println("Uploading names and attendance records to Firebase...");

  // Upload names to Firebase
  for (int i = 0; i < namid; i++) {
    if (name[i][0] != "") {  // Ensure the record is not empty
      String rollNumber = name[i][2];
      String studentId = name[i][1];
      String studentName = name[i][0];

      // Check if this roll number already exists in Firebase
      String path = "/students/" + studentId;
      FirebaseJson json;
      json.set("rollNumber", rollNumber);
      json.set("name", studentName);

      if (Firebase.setJSON(firebaseData, path.c_str(), json)) {
        Serial.println("Uploaded to Firebase - ID: " + studentId + ", Roll: " + rollNumber + ", Name: " + studentName);
      } else {
        Serial.println("Failed to upload to Firebase - ID: " + studentId);
        Serial.println("Error: " + firebaseData.errorReason());
      }
    }
  }

  // Upload attendance records
  File root = SD.open("/");
  File file = root.openNextFile();
  while (file) {
    String fileName = file.name();
    if (fileName.endsWith(".txt") && fileName.length() == 14) {  // Check for DD-MM-YYYY.txt format
      String dateStr = fileName.substring(0, fileName.length() - 4);

      // Read and upload attendance records
      String filePath = "/" + fileName;
      File attendanceFile = SD.open(filePath, FILE_READ);
      if (attendanceFile) {
        bool uploadSuccess = true;
        while (attendanceFile.available()) {
          String line = attendanceFile.readStringUntil('\n');
          line.trim();
          if (line.startsWith("<tr>") && line.endsWith("</tr>")) {
            // Extract data from the HTML table row
            int rollStart = line.indexOf("<td>") + 4;
            int rollEnd = line.indexOf("</td>", rollStart);
            int nameStart = line.indexOf("<td>", rollEnd) + 4;
            int nameEnd = line.indexOf("</td>", nameStart);
            int idStart = line.indexOf("<td>", nameEnd) + 4;
            int idEnd = line.indexOf("</td>", idStart);
            int inTimeStart = line.indexOf("<td>", idEnd) + 4;
            int inTimeEnd = line.indexOf("</td>", inTimeStart);
            int outTimeStart = line.indexOf("<td>", inTimeEnd) + 4;
            int outTimeEnd = line.indexOf("</td>", outTimeStart);

            if (rollStart > 0 && rollEnd > 0 && nameStart > 0 && nameEnd > 0 && idStart > 0 && idEnd > 0 && inTimeStart > 0 && inTimeEnd > 0) {

              String rollNumber = line.substring(rollStart, rollEnd);
              String name = line.substring(nameStart, nameEnd);
              String id = line.substring(idStart, idEnd);
              String inTime = line.substring(inTimeStart, inTimeEnd);
              String outTime = outTimeStart > 0 && outTimeEnd > 0 ? line.substring(outTimeStart, outTimeEnd) : "-";

              String path = "/attendance/" + dateStr + "/" + id;
              FirebaseJson json;
              json.set("rollNumber", rollNumber);
              json.set("name", name);
              json.set("inTime", inTime);
              json.set("outTime", outTime);

              if (!Firebase.setJSON(firebaseData, path.c_str(), json)) {
                Serial.println("Failed to upload attendance record - Date: " + dateStr + ", ID: " + id);
                Serial.println("Error: " + firebaseData.errorReason());
                uploadSuccess = false;
                break;
              }
            }
          }
        }
        attendanceFile.close();
      }
    }
    file = root.openNextFile();
  }

  Serial.println("Finished uploading names and attendance records to Firebase.");
}

void syncAttendanceWithFirebase() {
  if (firebaseConfig.host == "" || firebaseConfig.signer.tokens.legacy_token == "") {
    Serial.println("Firebase credentials are not set. Cannot sync with Firebase.");
    return;
  }

  Serial.println("Starting full attendance sync with Firebase...");

  // Open the /Attendance directory
  File attendanceRoot = SD.open("/Attendance");
  if (!attendanceRoot) {
    Serial.println("Attendance directory not found.");
    return;
  }

  // Iterate over all month directories
  File monthDir = attendanceRoot.openNextFile();
  while (monthDir) {
    if (monthDir.isDirectory()) {
      String month = String(monthDir.name());
      File dateFile = SD.open("/Attendance/" + month);
      if (dateFile) {
        File file = dateFile.openNextFile();
        while (file) {
          String fileName = String(file.name());
          if (fileName.endsWith(".txt") && fileName.length() == 14) { // DD-MM-YYYY.txt
            String date = fileName.substring(0, fileName.length() - 4);
            Serial.println("Syncing attendance file: " + date);
            File attendanceFile = SD.open("/Attendance/" + month + "/" + fileName, FILE_READ);
            if (attendanceFile) {
              while (attendanceFile.available()) {
                String line = attendanceFile.readStringUntil('\n');
                line.trim();
                if (line.startsWith("<tr>") && line.endsWith("</tr>")) {
                  // Extract data from the HTML table row
                  int rollStart = line.indexOf("<td>") + 4;
                  int rollEnd = line.indexOf("</td>", rollStart);
                  int nameStart = line.indexOf("<td>", rollEnd) + 4;
                  int nameEnd = line.indexOf("</td>", nameStart);
                  int idStart = line.indexOf("<td>", nameEnd) + 4;
                  int idEnd = line.indexOf("</td>", idStart);
                  int inTimeStart = line.indexOf("<td>", idEnd) + 4;
                  int inTimeEnd = line.indexOf("</td>", inTimeStart);
                  int outTimeStart = line.indexOf("<td>", inTimeEnd) + 4;
                  int outTimeEnd = line.indexOf("</td>", outTimeStart);

                  if (rollStart > 0 && rollEnd > 0 && nameStart > 0 && nameEnd > 0 && idStart > 0 && idEnd > 0 && inTimeStart > 0 && inTimeEnd > 0) {
                    String rollNumber = line.substring(rollStart, rollEnd);
                    String name = line.substring(nameStart, nameEnd);
                    String id = line.substring(idStart, idEnd);
                    String inTime = line.substring(inTimeStart, inTimeEnd);
                    String outTime = outTimeStart > 0 && outTimeEnd > 0 ? line.substring(outTimeStart, outTimeEnd) : "-";

                    // Remove [U] marker if present
                    if (name.endsWith("[U]")) {
                      name = name.substring(0, name.length() - 3);
                    }

                    String path = "/attendance/" + month + "/" + date + "/" + id;
                    FirebaseJson json;
                    json.set("name", name);
                    json.set("rollNumber", rollNumber);
                    json.set("inTime", inTime);
                    json.set("outTime", outTime);

                    if (!Firebase.setJSON(firebaseData, path.c_str(), json)) {
                      Serial.println("Failed to upload attendance record - Date: " + date + ", ID: " + id);
                      Serial.println("Error: " + firebaseData.errorReason());
                    } else {
                      Serial.println("Uploaded attendance record - Date: " + date + ", ID: " + id);
                    }
                  }
                }
              }
              attendanceFile.close();
            }
          }
          file = dateFile.openNextFile();
        }
        dateFile.close();
      }
    }
    monthDir = attendanceRoot.openNextFile();
  }
  attendanceRoot.close();
  Serial.println("Finished syncing all attendance records with Firebase.");
}

void sendTelegramMessage(const char *message) {
  if (telegramBotToken == "" || telegramChatId == "") {
    Serial.println("Telegram credentials not set");
    Serial.println("Bot Token: " + telegramBotToken);
    Serial.println("Chat ID: " + telegramChatId);
    return;
  }

  // Trim whitespace from chat ID
  telegramChatId.trim();

  Serial.println("Attempting to send Telegram message...");
  Serial.println("Bot Token: " + telegramBotToken);
  Serial.println("Chat ID: " + telegramChatId);
  Serial.println("Message: " + String(message));

  HTTPClient http;
  String url = "https://api.telegram.org/bot";
  url += telegramBotToken;
  url += "/sendMessage";

  Serial.println("URL: " + url);

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  // Create JSON payload
  String jsonPayload = "{\"chat_id\":\"" + telegramChatId + "\",\"text\":\"" + String(message) + "\"}";
  Serial.println("JSON Payload: " + jsonPayload);

  int httpCode = http.POST(jsonPayload);
  Serial.println("HTTP Code: " + String(httpCode));

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Telegram Response: " + payload);

    if (httpCode != 200) {
      Serial.println("Error sending Telegram message. HTTP Code: " + String(httpCode));
    }
  } else {
    Serial.println("Error sending Telegram message. HTTP Code: " + String(httpCode));
  }

  http.end();
} 