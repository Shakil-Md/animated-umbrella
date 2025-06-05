# ESP32 Smart Attendance System - User Manual

## Table of Contents
1. [System Overview](#system-overview)
2. [Getting Started](#getting-started)
3. [Web Interface](#web-interface)
4. [Features](#features)
5. [Maintenance](#maintenance)
6. [Troubleshooting](#troubleshooting)
7. [Technical Specifications](#technical-specifications)

## System Overview

The ESP32 Smart Attendance System is a sophisticated biometric attendance management solution that combines fingerprint recognition with cloud synchronization capabilities. The system features:

- Biometric attendance tracking
- Real-time cloud synchronization
- Web-based dashboard
- Mobile notifications via Telegram
- Temperature monitoring
- Battery status monitoring
- Local SD card storage
- Automatic WiFi reconnection

## Getting Started

### First-Time Setup

1. **Power On**: Connect the power supply to the ESP32 board
2. **Initial Boot**: The system will:
   - Initialize all components
   - Connect to WiFi
   - Sync with Firebase
   - Setup the fingerprint sensor
   - Display the IP address on the TFT screen

3. **Accessing the Dashboard**:
   - Note the IP address shown on the TFT display
   - Open a web browser
   - Enter the IP address in the browser's address bar
   - Default login credentials:
     - Username: admin
     - Password: admin123

### Display Interface

The TFT display shows:
- System status
- WiFi connection status
- Current operation mode
- Battery level
- Time and date
- Error messages (if any)

## Web Interface

### Navigation Menu
- **Home**: Dashboard with system overview
- **Attendance**: View and manage attendance records
- **Students**: Manage student database
- **Add New**: Register new students
- **Settings**: System configuration

### Key Features

1. **Student Management**:
   - Add new students
   - Register fingerprints
   - View student list
   - Delete student records

2. **Attendance Management**:
   - View attendance records
   - Export attendance data
   - Filter by date range
   - Sort by student name/ID

3. **System Settings**:
   - WiFi configuration
   - Firebase settings
   - Telegram notifications
   - Display settings
   - Time synchronization

## Features

### Fingerprint Registration
1. Click "Add New" in the web interface
2. Enter student details
3. Click "Register Fingerprint"
4. Follow the on-screen instructions:
   - Place finger on sensor when LED blinks
   - Repeat 2-3 times for better accuracy

### Taking Attendance
1. The system continuously monitors for fingerprints when in scanning mode
2. Students place their finger on the sensor
3. The system will:
   - Verify the fingerprint
   - Record the attendance
   - Display confirmation on screen
   - Sync with Firebase (if connected)
   - Send notification (if configured)

### Data Export
1. Go to the Attendance page
2. Select date range
3. Click "Export"
4. Choose format (CSV/PDF)
5. Download the file

## Maintenance

### Regular Maintenance Tasks

1. **Memory Management**:
   - System automatically monitors memory usage
   - Warning appears if memory is low (< 10KB)
   - Regularly check system status in Settings

2. **WiFi Connectivity**:
   - Automatic reconnection if connection is lost
   - Status displayed on screen
   - Check Settings page for connection strength

3. **SD Card**:
   - Regularly backup attendance data
   - Check available storage in Settings
   - Format card if errors persist

4. **Fingerprint Sensor**:
   - Clean sensor surface regularly
   - Check recognition quality in Settings
   - Recalibrate if needed

## Troubleshooting

### Common Issues and Solutions

1. **WiFi Connection Issues**:
   - Check WiFi credentials
   - Verify router is within range
   - System will attempt automatic reconnection
   - Reset WiFi settings if problems persist

2. **Fingerprint Recognition Problems**:
   - Clean the sensor surface
   - Ensure proper finger placement
   - Re-register fingerprint if necessary
   - Check sensor connections if error persists

3. **Display Issues**:
   - Check physical connections
   - Verify power supply
   - Use Settings to reinitialize display

4. **Firebase Sync Issues**:
   - Verify internet connection
   - Check Firebase credentials
   - System will continue offline if needed
   - Data will sync when connection restored

5. **System Not Responding**:
   - Check power supply
   - Press reset button
   - Check system memory usage
   - Verify all components are connected

### Error Messages

- **"WiFi Error!"**: Connection to WiFi failed
- **"Firebase Error!"**: Unable to connect to Firebase
- **"SD Card Error!"**: SD card not detected or failed
- **"Fingerprint Error!"**: Sensor not responding
- **"Low memory!"**: System memory below 10KB

### LED Indicators

- **Solid Green**: System ready
- **Blinking Blue**: Scanning mode active
- **Red**: Error condition
- **Off**: System initializing or power issue

## Technical Specifications

- **Power Supply**: 5V DC
- **WiFi**: 2.4GHz, supports 802.11 b/g/n
- **Storage**: SD card (FAT32)
- **Display**: 128x160 TFT
- **Fingerprint Capacity**: Up to 128 templates
- **Temperature Range**: -55°C to +125°C
- **Memory**: Automatic management with warning at 10KB
- **Backup**: Automatic to SD card and Firebase

For technical support or questions, please refer to the project documentation or contact system administrator.
