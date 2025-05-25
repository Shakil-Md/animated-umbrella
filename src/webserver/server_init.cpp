#include "server_init.h"
#include "../handlers/route_handlers.h"
#include "../utils/security_utils.h"
#include "../components/fingerprint.h"

void serverInit() {
  // Unprotected routes
  server.on("/login", handleLogin);
  server.on("/logout", handleLogout);

  // Protected routes
  server.on("/", []() {
    if (!checkAuth()) {
      server.sendHeader("Location", "/login");
      server.send(302, "text/plain", "");
      return;
    }
    handleRoot();
  });

  // Add authentication check to all other routes
  server.on("/addnew", []() {
    if (!checkAuth()) {
      server.sendHeader("Location", "/login");
      server.send(302, "text/plain", "");
      return;
    }
    handleAddnew();
  });

  server.on("/submit", []() {
    if (!checkAuth()) {
      server.sendHeader("Location", "/login");
      server.send(302, "text/plain", "");
      return;
    }
    handleFormSubmit();
  });

  server.on("/scanFingerprint", HTTP_POST, []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleScanFingerprint();
  });

  server.on("/names", []() {
    if (!checkAuth()) {
      server.sendHeader("Location", "/login");
      server.send(302, "text/plain", "");
      return;
    }
    handleShowname();
  });

  server.on("/a2z", []() {
    if (!checkAuth()) {
      server.sendHeader("Location", "/login");
      server.send(302, "text/plain", "");
      return;
    }
    a2z();
  });

  server.on("/scan", []() {
    if (!checkAuth()) {
      server.sendHeader("Location", "/login");
      server.send(302, "text/plain", "");
      return;
    }
    handleScanningPage();
  });

  server.on("/erase", []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleDltname();
  });

  server.on("/deleteall", []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleDeleteAll();
  });

  server.on("/deleteAllStudents", HTTP_POST, []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleDeleteAllStudents();
  });

  server.on("/deleteSelectedDates", HTTP_POST, []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleDeleteSelectedDates();
  });

  server.on("/getAttendanceCount", []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleGetAttendanceCount();
  });

  server.on("/getAttendanceData", []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleGetAttendanceData();
  });

  server.on("/deleteAllAttendance", []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleDeleteAllAttendance();
  });

  server.on("/settings", []() {
    if (!checkAuth()) {
      server.sendHeader("Location", "/login");
      server.send(302, "text/plain", "");
      return;
    }
    handleSettings();
  });

  server.on("/updateFirebase", HTTP_POST, []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleUpdateFirebase();
  });

  server.on("/updateWiFi", HTTP_POST, []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleUpdateWiFi();
  });

  server.on("/updateTelegram", HTTP_POST, []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleUpdateTelegram();
  });

  server.on("/startContinuousScanning", HTTP_POST, []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleStartContinuousScanning();
  });

  server.on("/stopContinuousScanning", HTTP_POST, []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleStopContinuousScanning();
  });

  server.on("/reinitializeDisplay", HTTP_POST, []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleReinitializeDisplay();
  });

  server.on("/reinitializeSD", HTTP_POST, []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleReinitializeSD();
  });

  server.on("/reinitializeFingerprint", HTTP_POST, []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleReinitializeFingerprint();
  });

  server.on("/exportAttendance", HTTP_GET, []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleExportAttendance();
  });

  server.on("/updateAdmin", HTTP_POST, []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleUpdateAdmin();
  });

  server.on("/syncData", HTTP_POST, []() {
    if (!checkAuth()) {
      server.send(401, "text/plain", "Unauthorized");
      return;
    }
    handleSyncData();
  });

  // Handle not found (404)
  server.onNotFound([]() {
    if (!checkAuth()) {
      server.sendHeader("Location", "/login");
      server.send(302, "text/plain", "");
      return;
    }
    server.send(404, "text/plain", "Not found");
  });

  // Start server
  server.begin();
  Serial.println("HTTP server started");
  tft.fillScreen(TFT_WHITE);
  tft.fillRect(0, 15, 128, 30, TFT_WHITE);
  tft.setCursor(2, 20);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.println("HTTP server started");
} 