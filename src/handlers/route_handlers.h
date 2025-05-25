#ifndef ROUTE_HANDLERS_H
#define ROUTE_HANDLERS_H

#include "../config/config.h"

// Home page
void handleRoot();

// Fingerprint management
void handleAddnew();
void handleFormSubmit();
void handleScanFingerprint();

// Student management
void handleShowname();
void handleDltname();
void handleDeleteAllStudents();

// Attendance management
void handleAllfile();
void a2z(); // Calendar view with improved rendering
void handleScanningPage();
void handleDeleteAll();
void handleDeleteSelectedDates();
void handleDeleteAllAttendance();
void handleGetAttendanceCount();
void handleGetAttendanceData();

// Settings
void handleSettings();
void handleUpdateFirebase();
void handleUpdateWiFi();
void handleUpdateTelegram();
void handleUpdateAdmin();
void handleStartContinuousScanning();
void handleStopContinuousScanning();
void handleReinitializeDisplay();
void handleReinitializeSD();
void handleReinitializeFingerprint();
void handleSyncData();

// Export functions
void handleExportAttendance();
void exportAllAttendanceRecords();
void exportMonthAttendanceRecords(String month, String year);
void exportSelectedDatesAttendanceRecords(const std::vector<String>& dates);

#endif // ROUTE_HANDLERS_H 