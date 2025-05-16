# ESP32 Smart Attendance System

A biometric attendance management system built with ESP32, featuring fingerprint recognition, cloud synchronization, and a web-based dashboard.

## Hardware Requirements

- ESP32 development board
- ST7735 TFT display (128x160 pixels)
- Fingerprint sensor (R307 or similar)
- DS18B20 temperature sensor
- WS2812 RGB LED
- SD card module
- Connecting wires

## Software Requirements

- Arduino IDE 1.8.19 or newer
- ESP32 board support package
- Required libraries:
  - TFT_eSPI
  - Adafruit_Fingerprint
  - Adafruit_NeoPixel
  - FirebaseESP32
  - OneWire
  - DallasTemperature
  - ArduinoJson
  - WiFi
  - SD
  - SPI

## Installation

1. Install the Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software)
2. Add ESP32 board support to Arduino IDE:
   - Open Preferences and add this URL to Additional Boards Manager URLs:
     `https://dl.espressif.com/dl/package_esp32_index.json`
   - Go to Tools > Board > Boards Manager and install ESP32 by Espressif Systems
   - Select "ESP32 Dev Module" from Tools > Board

3. Install required libraries via Arduino Library Manager (Tools > Manage Libraries)

4. Configure TFT_eSPI:
   - The User_Setup.h file is included in this project
   - Make sure it's properly placed in the Arduino libraries folder or in your project directory

5. Setup SD Card:
   - Format the SD card as FAT32
   - Create a directory structure with:
     - /Attendance (for storing attendance records)

6. Setup Firebase:
   - Create a Firebase project at [firebase.google.com](https://firebase.google.com/)
   - Create a file named firebase.txt in the root of your SD card with:
     ```
     HOST=your-firebase-project-url
     AUTH=your-firebase-auth-token
     ```

7. WiFi Configuration:
   - Create a file named wifi.txt in the root of your SD card with:
     ```
     SSID=your-wifi-ssid
     PASSWORD=your-wifi-password
     ```

## Compiling and Uploading

1. Open project.ino in Arduino IDE
2. Select the correct board and port from Tools menu
3. Use the custom partition scheme from partitions.csv if needed
4. Click Upload button

## Usage

1. After booting, the system will initialize components and connect to WiFi
2. Access the web interface by entering the IP address shown on the display in a web browser
3. Default login: username: admin, password: admin123
4. From the web interface, you can:
   - Add new students and register fingerprints
   - View attendance records
   - Manage system settings
   - Sync data with Firebase

## Troubleshooting

- If the display shows errors during initialization, check connections and TFT_eSPI configuration
- For SD card issues, ensure it's formatted correctly and the required files exist
- For fingerprint sensor problems, verify connections and power supply

## Project Structure

- `project.ino`: Main Arduino sketch file
- `src/config`: Configuration files
- `src/components`: Hardware component modules
- `src/utils`: Utility functions
- `src/handlers`: Web request handlers
- `src/webserver`: Web server implementation
- `partitions.csv`: ESP32 memory partition configuration

## License

This project is provided as open source under the MIT License. 