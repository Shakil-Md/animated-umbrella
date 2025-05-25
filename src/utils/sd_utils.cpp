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

  // Check if students.csv exists, if not create it with header
  if (!SD.exists("/students.csv")) {
    Serial.println("Creating new students.csv file with header");
    File file = SD.open("/students.csv", FILE_WRITE);
    if (file) {
      file.println("ID,Roll Number,Name");
      file.close();
      Serial.println("Created students.csv with header");
    } else {
      Serial.println("Failed to create students.csv");
      return false;
    }
  }

  File file = SD.open("/students.csv", FILE_READ);
  if (file) {
    Serial.println("Opened /students.csv successfully.");
    int maxID = 0;
    
    // Skip header line if it exists
    if (file.available()) {
      String header = file.readStringUntil('\n');
      if (!header.startsWith("ID,Roll Number,Name")) {
        // If first line is not a header, rewind file
        file.seek(0);
      }
    }
    
    while (file.available()) {
      String id, roll, nameStr;
      if (readCSVLine(file, id, roll, nameStr) && namid < 128) {
        name[namid][1] = id;         // ID
        name[namid][2] = roll;       // Roll Number
        name[namid][0] = nameStr;    // Name

        Serial.println("Stored - ID: " + name[namid][1] + ", Roll: " + name[namid][2] + ", Name: " + name[namid][0]);

        // Update maxID
        int currentID = name[namid][1].toInt();
        if (currentID > maxID) {
          maxID = currentID;
        }

        namid++;
      }
    }
    addid = maxID + 1;
    file.close();
  } else {
    Serial.println("Failed to open /students.csv for reading.");
    addid = 1;
  }

  Serial.println("Finished reading /students.csv. Total names: " + String(namid));
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
  File file = SD.open("/students.csv", FILE_READ);
  if (file) {
    Serial.println("Loading student data from SD card...");
    int maxID = 0;

    // Skip header line if it exists
    if (file.available()) {
      String header = file.readStringUntil('\n');
      if (!header.startsWith("ID,Roll Number,Name")) {
        // If first line is not a header, rewind file
        file.seek(0);
      }
    }

    while (file.available()) {
      String id, roll, nameStr;
      if (readCSVLine(file, id, roll, nameStr) && namid < 128) {
        name[namid][1] = id;         // ID
        name[namid][2] = roll;       // Roll Number
        name[namid][0] = nameStr;    // Name

        // Update maxID
        int currentID = name[namid][1].toInt();
        if (currentID > maxID) {
          maxID = currentID;
        }

        namid++;
        Serial.println("Loaded student - ID: " + name[namid - 1][1] + ", Roll: " + name[namid - 1][2] + ", Name: " + name[namid - 1][0]);
      }
    }

    addid = maxID + 1;
    file.close();
    Serial.println("Finished loading student data. Total students: " + String(namid));
    Serial.println("Next available ID: " + String(addid));
  } else {
    Serial.println("Failed to open students.csv for reading");
    addid = 1;
  }
}

String getAttendanceFilePath(String dateStr) {
  // Extract month from date (format: DD-MM-YYYY)
  String month = dateStr.substring(3, 5);
  return "/Attendance/" + month + "/" + dateStr + ".csv";
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
    Serial.println("SD card not initialized");
    return false;
  }

  // Try to create a test file to verify write access
  File testFile = SD.open("/test.txt", FILE_WRITE);
  if (!testFile) {
    Serial.println("Failed to create test file - SD card may be read-only or full");
    return false;
  }
  testFile.println("test");
  testFile.close();

  // Try to read the test file
  testFile = SD.open("/test.txt", FILE_READ);
  if (!testFile) {
    Serial.println("Failed to read test file - SD card may be corrupted");
    return false;
  }
  testFile.close();

  // Clean up test file
  SD.remove("/test.txt");

  // Check if students.csv exists
  if (!SD.exists("/students.csv")) {
    Serial.println("students.csv not found - creating new file");
    File file = SD.open("/students.csv", FILE_WRITE);
    if (!file) {
      Serial.println("Failed to create students.csv");
      return false;
    }
    file.println("ID,Roll Number,Name");
    file.close();
  }

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

String escapeCSV(String input) {
  // If the string contains commas, quotes, or newlines, wrap it in quotes
  if (input.indexOf(',') != -1 || input.indexOf('"') != -1 || input.indexOf('\n') != -1) {
    // Double up any quotes
    input.replace("\"", "\"\"");
    // Wrap in quotes
    input = "\"" + input + "\"";
  }
  return input;
}

String unescapeCSV(String input) {
  // Remove surrounding quotes if present
  if (input.startsWith("\"") && input.endsWith("\"")) {
    input = input.substring(1, input.length() - 1);
    // Un-double any quotes
    input.replace("\"\"", "\"");
  }
  return input;
}

bool writeCSVLine(File &file, String id, String roll, String name) {
  String line = escapeCSV(id) + "," + escapeCSV(roll) + "," + escapeCSV(name) + "\n";
  return file.print(line);
}

bool readCSVLine(File &file, String &id, String &roll, String &name) {
  if (!file.available()) return false;
  
  String line = file.readStringUntil('\n');
  line.trim();
  
  // Parse CSV line
  int pos1 = 0;
  int pos2 = 0;
  
  // Find first comma
  pos1 = line.indexOf(',');
  if (pos1 == -1) return false;
  
  // Find second comma
  pos2 = line.indexOf(',', pos1 + 1);
  if (pos2 == -1) return false;
  
  // Extract fields
  id = unescapeCSV(line.substring(0, pos1));
  roll = unescapeCSV(line.substring(pos1 + 1, pos2));
  name = unescapeCSV(line.substring(pos2 + 1));
  
  return true;
}

// Add new functions for CSV attendance handling
bool writeAttendanceCSVLine(File &file, String roll, String name, String id, String inTime, String outTime) {
  if (!file) return false;
  
  // Escape fields that might contain commas
  roll = escapeCSV(roll);
  name = escapeCSV(name);
  id = escapeCSV(id);
  inTime = escapeCSV(inTime);
  outTime = escapeCSV(outTime);
  
  // Write the CSV line
  file.print(roll);
  file.print(",");
  file.print(name);
  file.print(",");
  file.print(id);
  file.print(",");
  file.print(inTime);
  file.print(",");
  file.println(outTime);
  
  return true;
}

bool readAttendanceCSVLine(File &file, String &roll, String &name, String &id, String &inTime, String &outTime) {
  if (!file.available()) {
    return false;
  }

  String line = file.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) {
    return false;
  }

  // Split the line into fields
  int fieldCount = 0;
  int startPos = 0;
  int endPos = 0;
  bool inQuotes = false;
  String fields[5];  // Roll, Name, ID, In Time, Out Time

  for (int i = 0; i < line.length(); i++) {
    if (line[i] == '"') {
      inQuotes = !inQuotes;
    } else if (line[i] == ',' && !inQuotes) {
      if (fieldCount < 5) {
        fields[fieldCount] = line.substring(startPos, i);
        fieldCount++;
        startPos = i + 1;
      }
    }
  }
  // Get the last field
  if (fieldCount < 5) {
    fields[fieldCount] = line.substring(startPos);
    fieldCount++;
  }

  // Check if we have all required fields
  if (fieldCount != 5) {
    return false;
  }

  // Unescape and assign fields
  roll = unescapeCSV(fields[0]);
  name = unescapeCSV(fields[1]);
  id = unescapeCSV(fields[2]);
  inTime = unescapeCSV(fields[3]);
  outTime = unescapeCSV(fields[4]);

  return true;
}

bool createAttendanceCSVFile(String filePath) {
  File file = SD.open(filePath, FILE_WRITE);
  if (!file) return false;
  
  // Write CSV header
  file.println("Roll Number,Name,Fingerprint ID,In Time,Out Time");
  file.close();
  return true;
} 