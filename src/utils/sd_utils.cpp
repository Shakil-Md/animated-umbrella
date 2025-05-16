#include "sd_utils.h"
#include "display_utils.h"

bool setsd() {
  SPI.begin();
  sdCardInitialized = false;

  int n = 0;
  while (!SD.begin(SD_CS)) {
    if (n == 5)
      break;
    n++;
  }

  if (!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    tft.init(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(TFT_WHITE);
    tft.fillRect(0, 15, 128, 30, TFT_WHITE);  // Clear area below time and WiFi icon
    tft.setTextColor(TFT_RED);
    tft.setCursor(2, 20);
    tft.println("SD Card Mount Failed");
    tft.setTextSize(1);
    rgbLED.setPixelColor(0, rgbLED.Color(55, 0, 0));  // Set RGB LED to red (failure)
    rgbLED.show();
    return false;
  }

  sdCardInitialized = true;
  Serial.println("SD Card initialized.");
  delay(500);
  tft.init(INITR_BLACKTAB);
  tft.fillScreen(TFT_WHITE);
  tft.fillRect(0, 15, 128, 30, TFT_WHITE);  // Clear area below time and WiFi icon
  tft.setRotation(1);
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(1);
  tft.setCursor(2, 20);
  tft.println("SD Card initialized.");
  addid = 1;
  namid = 0;
  for (int i = 0; i < 128; i++) {
    name[i][0] = "";
    name[i][1] = "";
    name[i][2] = "";
  }
  String line = "";

  File file = SD.open("/name.txt", FILE_READ);
  if (file) {
    Serial.println("Opened /name.txt successfully.");
    int maxID = 0;
    while (file.available()) {
      line = file.readStringUntil('\n');
      Serial.println("Read line: " + line);

      // Parse ID, roll number, and name from the line
      // Format: "ID RollNumber Name"
      int firstSpace = line.indexOf(' ');
      if (firstSpace > 0 && namid < 128) {
        // Get remaining string after ID
        String remaining = line.substring(firstSpace + 1);
        // Find space between roll number and name
        int secondSpace = remaining.indexOf(' ');

        if (secondSpace > 0) {
          name[namid][1] = line.substring(0, firstSpace);         // ID
          name[namid][2] = remaining.substring(0, secondSpace);   // Roll Number
          name[namid][0] = remaining.substring(secondSpace + 1);  // Name

          Serial.println("Stored - ID: " + name[namid][1] + ", Roll: " + name[namid][2] + ", Name: " + name[namid][0]);

          // Update maxID
          int currentID = name[namid][1].toInt();
          if (currentID > maxID) {
            maxID = currentID;
          }

          namid++;
        } else {
          Serial.println("Error: Invalid line format (missing second space)");
        }
      } else {
        Serial.println("Error: Invalid line format or name array out of bounds");
      }
    }
    addid = maxID + 1;
    file.close();
  } else {
    Serial.println("Failed to open /name.txt for reading.");
    addid = 1;
  }

  Serial.println("Finished reading /name.txt. Total names: " + String(namid));
  Serial.println("Next available ID: " + String(addid));
  rgbLED.setPixelColor(0, rgbLED.Color(0, 55, 0));  // Set RGB LED to green (success)
  rgbLED.show();
  delay(500);
  rgbLED.setPixelColor(0, rgbLED.Color(0, 0, 0));  // Set RGB LED to off
  rgbLED.show();
  return true;
}

void loadStudentData() {
  // Reset student data
  addid = 1;
  namid = 0;
  for (int i = 0; i < 128; i++) {
    name[i][0] = "";
    name[i][1] = "";
    name[i][2] = "";
  }

  // Read student data from SD card
  File file = SD.open("/name.txt", FILE_READ);
  if (file) {
    Serial.println("Loading student data from SD card...");
    int maxID = 0;

    while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim();

      // Parse ID, roll number, and name from the line
      int firstSpace = line.indexOf(' ');
      if (firstSpace > 0 && namid < 128) {
        String remaining = line.substring(firstSpace + 1);
        int secondSpace = remaining.indexOf(' ');

        if (secondSpace > 0) {
          name[namid][1] = line.substring(0, firstSpace);         // ID
          name[namid][2] = remaining.substring(0, secondSpace);   // Roll Number
          name[namid][0] = remaining.substring(secondSpace + 1);  // Name

          // Update maxID
          int currentID = name[namid][1].toInt();
          if (currentID > maxID) {
            maxID = currentID;
          }

          namid++;
          Serial.println("Loaded student - ID: " + name[namid - 1][1] + ", Roll: " + name[namid - 1][2] + ", Name: " + name[namid - 1][0]);
        }
      }
    }

    addid = maxID + 1;
    file.close();
    Serial.println("Finished loading student data. Total students: " + String(namid));
    Serial.println("Next available ID: " + String(addid));
  } else {
    Serial.println("Failed to open name.txt for reading");
    addid = 1;
  }
}

String getAttendanceFilePath(String dateStr) {
  // Extract month from date (format: DD-MM-YYYY)
  String month = dateStr.substring(3, 5);
  return "/Attendance/" + month + "/" + dateStr + ".txt";
}

bool ensureAttendanceDirectory(String dateStr) {
  String month = dateStr.substring(3, 5);
  String attendanceDir = "/Attendance";
  String monthDir = attendanceDir + "/" + month;

  // Create Attendance directory if it doesn't exist
  if (!SD.exists(attendanceDir)) {
    if (!SD.mkdir(attendanceDir)) {
      Serial.println("Failed to create Attendance directory");
      return false;
    }
  }

  // Create month directory if it doesn't exist
  if (!SD.exists(monthDir)) {
    if (!SD.mkdir(monthDir)) {
      Serial.println("Failed to create month directory");
      return false;
    }
  }

  return true;
}

bool checkSDCardStatus() {
  if (!sdCardInitialized) {
    return false;
  }

  File testFile = SD.open("/name.txt", FILE_READ);
  if (!testFile) {
    return false;
  }
  testFile.close();
  return true;
}

bool readFirebaseCredentials() {
  File file = SD.open("/firebase.txt", FILE_READ);
  if (!file) {
    Serial.println("Failed to open /firebase.txt");
    return false;
  }
  Serial.println("Reading Firebase credentials from /firebase.txt");
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();  // Remove any leading/trailing whitespace

    if (line.startsWith("HOST=")) {
      firebaseConfig.host = line.substring(5).c_str();  // Extract Firebase Host URL after "HOST="
    } else if (line.startsWith("AUTH=")) {
      firebaseConfig.signer.tokens.legacy_token = line.substring(5).c_str();  // Extract Firebase Auth after "AUTH="
    }
  }

  file.close();
  return true;
}

bool readWiFiCredentials(String &ssid, String &password) {
  File file = SD.open("/wifi.txt", FILE_READ);
  if (!file) {
    Serial.println("Failed to open /wifi.txt");
    return false;
  }
  Serial.println("Reading WiFi credentials from /wifi.txt");
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();  // Remove any leading/trailing whitespace

    if (line.startsWith("SSID=")) {
      ssid = line.substring(5);  // Extract SSID after "SSID="
    } else if (line.startsWith("PASSWORD=")) {
      password = line.substring(9);  // Extract password after "PASSWORD="
    }
  }

  file.close();
  return true;
}

void readTelegramCredentials() {
  File telegramFile = SD.open("/telegram.txt", FILE_READ);
  if (telegramFile) {
    while (telegramFile.available()) {
      String line = telegramFile.readStringUntil('\n');
      line.trim();
      if (line.startsWith("BOT_TOKEN=")) {
        telegramBotToken = line.substring(10);
      } else if (line.startsWith("CHAT_ID=")) {
        telegramChatId = line.substring(8);
      }
    }
    telegramFile.close();
  }
} 