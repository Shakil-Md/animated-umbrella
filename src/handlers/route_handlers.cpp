#include "route_handlers.h"
#include "../webserver/html_components.h"
#include "../utils/display_utils.h"
#include "../utils/time_utils.h"
#include "../utils/sd_utils.h"
#include "../utils/security_utils.h"
#include "../components/fingerprint.h"
#include "../components/network.h"
#include <vector>

// External variables
extern bool fingerprintReady;  // Declare as external to access from main project file

void handleRoot() {
  // Reset RGB LED
  rgbLED.setPixelColor(0, rgbLED.Color(0, 0, 0));
  rgbLED.show();

  // Reset fingerprint sensor
  while (finger.getImage() != FINGERPRINT_NOFINGER) {
    delay(100);  // Wait for the finger to be removed
  }
  delay(1000);  // Allow the sensor to reset

  // Reset continuous scanning flag
  isBlinking = false;

  // Check peripheral statuses
  bool wifiConnected = (WiFi.status() == WL_CONNECTED);
  String wifiStatus = wifiConnected ? "Connected to " + WiFi.SSID() : "Disconnected";

  // Check fingerprint sensor
  bool fingerprintOk = finger.verifyPassword();
  String fingerprintStatus = fingerprintOk ? "Ready for scanning" : "Not responding";

  // Properly check SD card status
  bool sdCardOk = checkSDCardStatus();
  String sdCardStatus = sdCardOk ? "SD Card operational" : "SD Card error or not detected";

  // If SD card is working and we haven't loaded data yet, load it
  if (sdCardOk && namid == 0) {
    loadStudentData();
  }

  // Check Firebase connection
  bool firebaseOk = Firebase.ready();
  String firebaseStatus = firebaseOk ? "Connected and synced" : "Connection error";

  // Get today's attendance count - only if SD card is working
  int presentCount = 0;
  int totalStudents = namid;
  String todayFile = getAttendanceFilePath(getCurrentDate());

  if (sdCardOk && SD.exists(todayFile)) {
    File file = SD.open(todayFile, FILE_READ);
    if (file) {
      while (file.available()) {
        String line = file.readStringUntil('\n');
        if (line.startsWith("<tr>")) {
          presentCount++;
        }
      }
      file.close();
    }
  }

  // Calculate attendance percentage
  float attendancePercentage = totalStudents > 0 ? (float)presentCount / totalStudents * 100 : 0;

  String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Smart Attendance System</title>
        <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
        <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" rel="stylesheet">
        )rawliteral" + getGlassmorphismStyles() + R"rawliteral(
        <style>
            .stats-icon {
                width: 45px;
                height: 45px;
                border-radius: 15px;
                display: flex;
                align-items: center;
                justify-content: center;
                font-size: 1.4rem;
                color: white;
                margin-bottom: 1rem;
                background: rgba(255, 255, 255, 0.1);
                backdrop-filter: blur(4px);
                border: 1px solid rgba(255, 255, 255, 0.1);
                box-shadow: 0 4px 12px rgba(31, 38, 135, 0.1);
            }
            
            .stats-info h3 {
                font-size: 2rem;
                font-weight: 700;
                margin-bottom: 0.5rem;
                color: white;
            }
            
            .stats-info p {
                color: rgba(255, 255, 255, 0.7);
                margin: 0;
            }
            
            .action-icon {
                width: 45px;
                height: 45px;
                border-radius: 12px;
                display: flex;
                align-items: center;
                justify-content: center;
                font-size: 1.4rem;
                color: white;
                margin-right: 1rem;
                background: rgba(255, 255, 255, 0.1);
                backdrop-filter: blur(4px);
                border: 1px solid rgba(255, 255, 255, 0.1);
                box-shadow: 0 4px 12px rgba(31, 38, 135, 0.1);
                transition: all 0.3s ease;
            }
            
            .action-button {
                background: rgba(255, 255, 255, 0.1);
                backdrop-filter: blur(12px);
                -webkit-backdrop-filter: blur(12px);
                border: 1px solid rgba(255, 255, 255, 0.1);
                border-radius: 16px;
                padding: 1.2rem;
                width: 100%;
                text-align: left;
                transition: all 0.3s ease;
                margin-bottom: 1rem;
                display: flex;
                align-items: center;
                text-decoration: none;
                color: white;
                overflow: hidden;
                position: relative;
            }
            
            .action-button::after {
                content: '';
                position: absolute;
                top: 0;
                left: 0;
                width: 100%;
                height: 100%;
                background: linear-gradient(120deg, rgba(255,255,255,0) 0%, rgba(255,255,255,0.1) 50%, rgba(255,255,255,0) 100%);
                transform: translateX(-100%);
                transition: all 0.6s ease;
            }
            
            .action-button:hover {
                transform: translateY(-3px);
                box-shadow: 0 10px 25px rgba(0, 0, 0, 0.1);
                border: 1px solid rgba(255, 255, 255, 0.2);
            }
            
            .action-button:hover::after {
                transform: translateX(100%);
            }
            
            .action-info h4 {
                color: white;
                font-weight: 600;
                margin: 0;
            }
            
            .action-info p {
                color: rgba(255, 255, 255, 0.7);
                font-size: 0.9rem;
                margin: 0;
            }
            
            .status-item {
                display: flex;
                align-items: center;
                margin-bottom: 1rem;
                padding: 1rem;
                background: rgba(255, 255, 255, 0.1);
                backdrop-filter: blur(12px);
                -webkit-backdrop-filter: blur(12px);
                border-radius: 16px;
                border: 1px solid rgba(255, 255, 255, 0.1);
                transition: all 0.3s ease;
                overflow: hidden;
                position: relative;
            }
            
            .status-item::after {
                content: '';
                position: absolute;
                top: 0;
                left: 0;
                width: 100%;
                height: 100%;
                background: linear-gradient(120deg, rgba(255,255,255,0) 0%, rgba(255,255,255,0.05) 50%, rgba(255,255,255,0) 100%);
                transform: translateX(-100%);
                transition: all 0.6s ease;
            }
            
            .status-item:hover {
                transform: translateX(5px);
                background: rgba(255, 255, 255, 0.15);
                border: 1px solid rgba(255, 255, 255, 0.2);
            }
            
            .status-item:hover::after {
                transform: translateX(100%);
            }
            
            .status-icon {
                width: 45px;
                height: 45px;
                border-radius: 50%;
                display: flex;
                align-items: center;
                justify-content: center;
                font-size: 1.3rem;
                color: white;
                margin-right: 1rem;
                box-shadow: 0 0 15px rgba(31, 38, 135, 0.2);
            }
            
            .status-info h4 {
                color: white;
                font-weight: 600;
                margin: 0;
                font-size: 1rem;
            }
            
            .status-info p {
                color: rgba(255, 255, 255, 0.7);
                margin: 0;
                font-size: 0.9rem;
            }
            
            .header {
                text-align: center;
                margin-bottom: 2rem;
                color: white;
            }
            
            .header h1 {
                font-weight: 700;
                font-size: 2.5rem;
                margin-bottom: 0.5rem;
            }
            
            .header p {
                color: rgba(255, 255, 255, 0.8);
                font-size: 1.1rem;
            }
            
            .progress {
                height: 8px;
                border-radius: 4px;
                margin-top: 1rem;
                background: rgba(255, 255, 255, 0.1);
                overflow: hidden;
            }
            
            .progress-bar {
                background: linear-gradient(to right, rgba(46, 204, 113, 0.7), rgba(39, 174, 96, 0.8));
                box-shadow: 0 0 10px rgba(46, 204, 113, 0.5);
            }
            
            .glass-card h3 {
                color: white;
                font-weight: 600;
                margin-bottom: 1.5rem;
            }
            
            @media (max-width: 768px) {
                .glass-card {
                    padding: 15px;
                }
                
                .action-button, .status-item {
                    padding: 0.8rem;
                }
                
                .stats-icon, .action-icon, .status-icon {
                    width: 35px;
                    height: 35px;
                    font-size: 1rem;
                }
            }
        </style>
    </head>
    <body>
  )rawliteral" + getNavbarHtml() + R"rawliteral(
        <div class="container">
            <div class="header">
                <p>Efficient and Secure Biometric Attendance Management</p>
            </div>
            
            <div class="row">
                <!-- Today's Stats -->
                <div class="col-6 col-md-3 mb-2 mb-md-4">
                    <div class="glass-card">
                        <div class="d-flex align-items-center justify-content-center">
                          <div class="stats-icon" style="background: rgb(46, 204, 112);">
                            <i class="fas fa-user-check"></i>
                        </div>
                            <h3 class="mb-0" style="font-size: 1.5rem; font-weight: 700;">)rawliteral" + String(presentCount) + R"rawliteral(</h3>
                        </div>
                        <p class="text-center mt-2" style="font-size: 0.9rem; color: rgba(255, 255, 255, 0.8);">Present Today</p>
                    </div>
                </div>
                
                <div class="col-6 col-md-3 mb-2 mb-md-4">
                    <div class="glass-card">
                        <div class="d-flex align-items-center justify-content-center">
                            <div class="stats-icon" style="background: rgba(241, 196, 15, 0.99);">
                            <i class="fas fa-user-times"></i>
                        </div>
                            <h3 class="mb-0" style="font-size: 1.5rem; font-weight: 700;">)rawliteral" + String(totalStudents - presentCount) + R"rawliteral(</h3>
                        </div>
                        <p class="text-center mt-2" style="font-size: 0.9rem; color: rgba(255, 255, 255, 0.8);">Absent Today</p>
                    </div>
                </div>
                
                <div class="col-6 col-md-3 mb-2 mb-md-4">
                    <div class="glass-card">
                        <div class="d-flex align-items-center justify-content-center">
                            <div class="stats-icon" style="background: rgb(52, 152, 219);">
                            <i class="fas fa-users"></i>
                        </div>
                            <h3 class="mb-0" style="font-size: 1.5rem; font-weight: 700;">)rawliteral" + String(totalStudents) + R"rawliteral(</h3>
                        </div>
                        <p class="text-center mt-2" style="font-size: 0.9rem; color: rgba(255, 255, 255, 0.8);">Total Students</p>
                    </div>
                </div>
                
                <div class="col-6 col-md-3 mb-2 mb-md-4">
                    <div class="glass-card">
                        <div class="d-flex align-items-center justify-content-center">
                            <div class="stats-icon" style="background: rgb(231, 77, 60);">
                            <i class="fas fa-chart-pie"></i>
                        </div>
                            <h3 class="mb-0" style="font-size: 1.5rem; font-weight: 700;">)rawliteral" + String(int(attendancePercentage)) + R"rawliteral(%</h3>
                        </div>
                        <p class="text-center mt-2" style="font-size: 0.9rem; color: rgba(255, 255, 255, 0.8);">Attendance Rate</p>
                        <div class="progress mt-2">
                            <div class="progress-bar" role="progressbar" style="width: )rawliteral" + String(int(attendancePercentage)) + R"rawliteral(%" 
                                 aria-valuenow=")rawliteral" + String(int(attendancePercentage)) + R"rawliteral(" aria-valuemin="0" aria-valuemax="100"></div>
                        </div>
                    </div>
                </div>
            </div>
            
            <div class="row">
                <div class="col-md-8">
                    <div class="glass-card">
                        <h3 class="mb-4">Quick Actions</h3>
                        <div class="row">
                            <div class="col-6 col-md-6 mb-2 mb-md-3">
                                <a href="/scan" class="action-button">
                                    <div class="action-icon" style="background: rgb(231, 77, 60);">
                                        <i class="fas fa-fingerprint"></i>
                                    </div>
                                    <div class="action-info">
                                        <h5>Take Attendance</h5>
                                        <p>Start fingerprint scanning</p>
                                    </div>
                                </a>
                            </div>
                            <div class="col-6 col-md-6 mb-2 mb-md-3">
                                <a href="/addnew" class="action-button">
                                    <div class="action-icon" style="background: rgb(46, 204, 112);">
                                        <i class="fas fa-user-plus"></i>
                                    </div>
                                    <div class="action-info">
                                        <h5>Add New Student</h5>
                                        <p>Register fingerprint</p>
                                    </div>
                                </a>
                            </div>
                            <div class="col-6 col-md-6 mb-2 mb-md-3">
                                <a href="/names" class="action-button">
                                    <div class="action-icon" style="background: rgb(52, 152, 219);">
                                        <i class="fas fa-users"></i>
                                    </div>
                                    <div class="action-info">
                                        <h5>Manage Students</h5>
                                        <p>View records</p>
                                    </div>
                                </a>
                            </div>
                            <div class="col-6 col-md-6 mb-2 mb-md-3">
                                <a href="/a2z" class="action-button">
                                    <div class="action-icon" style="background: rgb(241, 196, 15);">
                                        <i class="fas fa-calendar-alt"></i>
                                    </div>
                                    <div class="action-info">
                                        <h5>View Attendance</h5>
                                        <p>Check records</p>
                                    </div>
                                </a>
                            </div>
                        </div>
                    </div>
                </div>
                
                <div class="col-md-4">
                    <div class="glass-card">
                        <h3 class="mb-3 mb-md-4">System Status</h3>
                        
                        <div class="status-item">
                            <div class="status-icon" style="background: )rawliteral" + (wifiConnected ? "rgb(46, 204, 112)" : "rgb(231, 77, 60)") + R"rawliteral(;">
                                <i class="fas fa-wifi"></i>
                            </div>
                            <div class="status-info">
                                <h4>WiFi</h4>
                                <p>)rawliteral" + wifiStatus + R"rawliteral(</p>
                            </div>
                        </div>
                        
                        <div class="status-item">
                            <div class="status-icon" style="background: )rawliteral" + (fingerprintOk ? "rgb(46, 204, 112)" : "rgb(231, 77, 60)") + R"rawliteral(;">
                                <i class="fas fa-fingerprint"></i>
                            </div>
                            <div class="status-info">
                                <h4>Fingerprint</h4>
                                <p>)rawliteral" + fingerprintStatus + R"rawliteral(</p>
                            </div>
                        </div>
                        
                        <div class="status-item">
                            <div class="status-icon" style="background: )rawliteral" + (sdCardOk ? "rgb(46, 204, 112)" : "rgb(231, 77, 60)") + R"rawliteral(;">
                                <i class="fas fa-sd-card"></i>
                            </div>
                            <div class="status-info">
                                <h4>SD Card</h4>
                                <p>)rawliteral" + sdCardStatus + R"rawliteral(</p>
                            </div>
                        </div>
                        
                        <div class="status-item">
                            <div class="status-icon" style="background: )rawliteral" + (firebaseOk ? "rgb(46, 204, 112)" : "rgb(231, 77, 60)") + R"rawliteral(;">
                                <i class="fas fa-database"></i>
                            </div>
                            <div class="status-info">
                                <h4>Firebase</h4>
                                <p>)rawliteral" + firebaseStatus + R"rawliteral(</p>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
        
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleShowname() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Student Records</title>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
      <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css">
      )rawliteral" + getGlassmorphismStyles() + R"rawliteral(
      <style>
        .back-icon {
          position: fixed;
          top: 20px;
          left: 20px;
          font-size: 24px;
          color: white;
          background: rgba(255, 255, 255, 0.15);
          backdrop-filter: blur(10px);
          width: 40px;
          height: 40px;
          border-radius: 50%;
          display: flex;
          align-items: center;
          justify-content: center;
          text-decoration: none;
          box-shadow: 0 8px 32px rgba(31, 38, 135, 0.1);
          transition: all 0.3s ease;
          z-index: 1000;
          border: 1px solid rgba(255, 255, 255, 0.18);
        }
        .back-icon:hover {
          transform: scale(1.1);
          background: rgba(255, 255, 255, 0.25);
          box-shadow: 0 8px 32px rgba(31, 38, 135, 0.2);
          text-decoration: none;
        }

        /* Styles for the student records table on /names page */
        .table.glass-table {
            background: rgba(255, 255, 255, 0.9) !important; /* High opacity white background */
            backdrop-filter: blur(10px);
            -webkit-backdrop-filter: blur(10px);
            border-radius: 12px; /* Slightly more rounded */
            overflow: hidden;
            box-shadow: 0 6px 20px rgba(0, 0, 0, 0.1); /* Refined shadow */
            color: #212529 !important; /* Bootstrap's default dark text color */
            margin-bottom: 1.5rem; /* More spacing */
            width: 100%;
        }

        .table.glass-table thead th {
            background: rgba(52, 58, 64, 0.9) !important; /* Bootstrap darkish header, slightly transparent */
            color: white !important;
            font-weight: 600;
            padding: 1rem 1.25rem;
            border: none;
            text-align: left; /* Align header text */
        }

        .table.glass-table tbody td {
            padding: 0.9rem 1.25rem;
            vertical-align: middle;
            border-bottom: 1px solid rgba(0, 0, 0, 0.08);
            color: #212529 !important; /* Dark text for body cells */
        }
        
        .table.glass-table tbody tr:last-child td {
            border-bottom: none;
        }

        .table.glass-table tbody tr:hover {
            background-color: rgba(0, 0, 0, 0.03) !important; /* Very subtle hover */
        }

        .table.glass-table .btn-danger {
            background-color: #dc3545 !important;
            border-color: #dc3545 !important;
            color: white !important;
            padding: 0.375rem 0.75rem; /* Standard Bootstrap button padding */
        }
        .table.glass-table .btn-danger i {
            color: white !important;
        }
        
        .checkbox-cell {
            text-align: center;
            vertical-align: middle;
        }
        
        #recordsTable th:first-child, #recordsTable td:first-child { /* Checkbox column */
             width: 50px; /* Fixed width for checkbox column */
        }
        #recordsTable th:last-child, #recordsTable td:last-child { /* Action column */
            width: 100px; /* Fixed width for action column */
            text-align: center;
        }
      </style>
    </head>
    <body>
  )rawliteral" + getNavbarHtml() + R"rawliteral(
      <div class="container mt-5">
        <div class="glass-card shadow-sm p-4">
          <h2 class="text-center mb-4">Student Records</h2>
          <div class="mb-3 d-flex gap-2 flex-wrap">
            <button class="btn btn-glass btn-glass-danger" onclick="deleteAllStudents()">
              <i class="fas fa-users-slash me-2"></i>Delete All Student Records
            </button>
            <button class="btn btn-glass btn-glass-danger" onclick="deleteSelected()" id="deleteSelectedBtn" disabled>
              <i class="fas fa-trash-alt me-2"></i>Delete Selected
            </button>
          </div>
          <div class="mb-3">
            <input type="text" id="searchInput" class="form-control" placeholder="Search by name or roll number..." onkeyup="searchTable()">
          </div>
        <div class="table-responsive">
            <table class="table glass-table" id="recordsTable">
              <thead>
                    <tr>
                  <th style="width:40px; text-align:center;">
                    <input type="checkbox" id="selectAll" onclick="toggleSelectAll()">
                  </th>
                  <th>ID</th>
                  <th>Roll Number</th>
                  <th>Name</th>
                  <th>Action</th>
                    </tr>
                </thead>
                <tbody>
  )rawliteral";

  // Read and display names from SD card
  File file = SD.open("/name.txt", FILE_READ);
  if (file) {
    while (file.available()) {
      String line = file.readStringUntil('\n');
      if (line.length() > 0) {
        int firstSpace = line.indexOf(' ');
        int secondSpace = line.indexOf(' ', firstSpace + 1);
        if (firstSpace != -1 && secondSpace != -1) {
          String id = line.substring(0, firstSpace);
          String roll = line.substring(firstSpace + 1, secondSpace);
          String name = line.substring(secondSpace + 1);
          html += "<tr><td class='checkbox-cell'><input type='checkbox' class='student-checkbox' value='" + id + "' onchange='updateDeleteButton()'></td><td>" + id + "</td><td>" + roll + "</td><td>" + name + "</td><td><button class='btn btn-danger' onclick='deleteRecord(" + id + ")'><i class='fas fa-trash-alt'></i></button></td></tr>";
        }
      }
    }
    file.close();
  }

  html += R"rawliteral(
                </tbody>
            </table>
        </div>
          <div id="status" class="status"></div>
    </div>
      </div>

      <script>
        const searchTable = () => {
          const input = document.getElementById('searchInput');
          const filter = input.value.toLowerCase();
          const table = document.getElementById('recordsTable');
          const tr = table.getElementsByTagName('tr');

          for (let i = 1; i < tr.length; i++) {
            const td = tr[i].getElementsByTagName('td');
            let found = false;
            for (let j = 1; j < td.length - 1; j++) {
              const cell = td[j];
              if (cell) {
                const text = cell.textContent || cell.innerText;
                if (text.toLowerCase().indexOf(filter) > -1) {
                  found = true;
                  break;
                }
              }
            }
            tr[i].style.display = found ? '' : 'none';
          }
        };

        const toggleSelectAll = () => {
          const selectAll = document.getElementById('selectAll');
          const checkboxes = document.getElementsByClassName('student-checkbox');
          for (let checkbox of checkboxes) {
            checkbox.checked = selectAll.checked;
          }
          updateDeleteButton();
        };

        const updateDeleteButton = () => {
          const checkboxes = document.getElementsByClassName('student-checkbox');
          const deleteSelectedBtn = document.getElementById('deleteSelectedBtn');
          let anyChecked = false;
          for (let checkbox of checkboxes) {
            if (checkbox.checked) {
              anyChecked = true;
              break;
            }
          }
          deleteSelectedBtn.disabled = !anyChecked;
        };

        const deleteSelected = () => {
          const checkboxes = document.getElementsByClassName('student-checkbox');
          const selectedIds = [];
          for (let checkbox of checkboxes) {
            if (checkbox.checked) {
              selectedIds.push(checkbox.value);
            }
          }

          if (selectedIds.length > 0 && confirm('Are you sure you want to delete the selected records?')) {
            const statusDiv = document.getElementById('status');
            statusDiv.style.display = 'block';
            statusDiv.className = 'status warning';
            statusDiv.innerHTML = '<i class="fas fa-spinner fa-spin me-2"></i>Deleting selected records...';
            
            // Delete each selected record
            let completed = 0;
            let failed = 0;
            
            selectedIds.forEach(id => {
              fetch('/erase', {
                method: 'POST',
                headers: {
                  'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: 'id=' + id
              })
              .then(response => response.text())
              .then(data => {
                completed++;
                if (completed + failed === selectedIds.length) {
                  if (failed === 0) {
                    statusDiv.className = 'status success';
                    statusDiv.innerHTML = '<i class="fas fa-check-circle me-2"></i>All selected records deleted successfully';
                    setTimeout(() => {
                      window.location.reload();
                    }, 1000);
                  } else {
                    statusDiv.className = 'status error';
                    statusDiv.innerHTML = '<i class="fas fa-exclamation-circle me-2"></i>Some records failed to delete';
                  }
                }
              })
              .catch(error => {
                failed++;
                if (completed + failed === selectedIds.length) {
                  statusDiv.className = 'status error';
                  statusDiv.innerHTML = '<i class="fas fa-exclamation-circle me-2"></i>Error deleting records';
                }
              });
            });
          }
        };

        const deleteRecord = (id) => {
          if (confirm('Are you sure you want to delete this record?')) {
            const statusDiv = document.getElementById('status');
            statusDiv.style.display = 'block';
            statusDiv.className = 'status warning';
            statusDiv.innerHTML = '<i class="fas fa-spinner fa-spin me-2"></i>Deleting record...';
            
            fetch('/erase', {
              method: 'POST',
              headers: {
                'Content-Type': 'application/x-www-form-urlencoded',
              },
              body: 'id=' + id
            })
            .then(response => response.text())
            .then(data => {
              if (data.includes('Success')) {
                statusDiv.className = 'status success';
                statusDiv.innerHTML = '<i class="fas fa-check-circle me-2"></i>Record deleted successfully';
                setTimeout(() => {
                  window.location.reload();
                }, 1000);
              } else {
                statusDiv.className = 'status error';
                statusDiv.innerHTML = '<i class="fas fa-exclamation-circle me-2"></i>' + data;
              }
            })
            .catch(error => {
              statusDiv.className = 'status error';
              statusDiv.innerHTML = '<i class="fas fa-exclamation-circle me-2"></i>Error: ' + error;
            });
          }
        };

        const deleteAllStudents = () => {
          if (confirm('Are you sure you want to delete all student records? This will also delete all associated fingerprints.')) {
            const statusDiv = document.getElementById('status');
            statusDiv.style.display = 'block';
            statusDiv.className = 'status warning';
            statusDiv.innerHTML = '<i class="fas fa-spinner fa-spin me-2"></i>Deleting all student records...';
            
            fetch('/deleteAllStudents', {
              method: 'POST',
              headers: {
                'Content-Type': 'application/json'
              }
            })
            .then(response => response.text())
            .then(data => {
              statusDiv.className = 'status success';
              statusDiv.innerHTML = '<i class="fas fa-check-circle me-2"></i>' + data;
              setTimeout(() => {
                window.location.reload();
              }, 1000);
            })
            .catch(error => {
              statusDiv.className = 'status error';
              statusDiv.innerHTML = '<i class="fas fa-exclamation-circle me-2"></i>Error: ' + error;
            });
          }
        };
      </script>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleDltname() {
  if (server.method() == HTTP_POST) {
    String id = server.arg("id");
    if (id.length() > 0) {
      int index = id.toInt();

      // 1. Delete from fingerprint sensor database
      if (finger.deleteModel(index) == FINGERPRINT_OK) {
        Serial.println("Fingerprint deleted from sensor database");
      } else {
        Serial.println("Failed to delete fingerprint from sensor database");
      }

      // 2. Delete from SD card
      File file = SD.open("/name.txt", FILE_READ);
      if (file) {
        String content = "";
        while (file.available()) {
          String line = file.readStringUntil('\n');
          if (line.length() > 0) {
            int firstSpace = line.indexOf(' ');
            if (firstSpace != -1) {
              String lineId = line.substring(0, firstSpace);
              if (lineId.toInt() != index) {
                content += line + "\n";
              }
            }
          }
        }
        file.close();

        // Write back the filtered content
        file = SD.open("/name.txt", FILE_WRITE);
        if (file) {
          file.print(content);
          file.close();
          Serial.println("Name deleted from SD card");
        }
      }

      // 3. Delete from Firebase
      String path = "/students/" + id;
      if (Firebase.deleteNode(firebaseData, path.c_str())) {
        Serial.println("Data deleted from Firebase");
      } else {
        Serial.println("Failed to delete from Firebase");
        Serial.println("Error: " + firebaseData.errorReason());
      }

      // Update local array
      for (int i = 0; i < namid; i++) {
        if (name[i][1].toInt() == index) {
          // Found the record to delete, now shift remaining records
          for (int j = i; j < namid - 1; j++) {
            name[j][0] = name[j + 1][0];
            name[j][1] = name[j + 1][1];
            name[j][2] = name[j + 1][2];
          }
          namid--;
          break;
        }
      }

      server.send(200, "text/plain", "Success");
    } else {
      server.send(400, "text/plain", "Error: No ID provided");
    }
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

void handleDeleteAllStudents() {
  if (server.method() == HTTP_POST) {
    bool success = true;
    String errorMessage = "";

    // 1. Clear the fingerprint sensor's internal database
    if (finger.emptyDatabase() != FINGERPRINT_OK) {
      success = false;
      errorMessage += "Failed to clear fingerprint database. ";
    } else {
      Serial.println("Fingerprint database cleared successfully");
    }

    // 2. Delete the name file from the SD card
    if (SD.exists("/name.txt")) {
      if (!SD.remove("/name.txt")) {
        success = false;
        errorMessage += "Failed to delete name.txt from SD card. ";
      } else {
        Serial.println("name.txt deleted from SD card");
      }
    }

    // 3. Clear the name array
    for (int i = 0; i < 128; i++) {
      name[i][0] = "";
      name[i][1] = "";
      name[i][2] = "";
    }
    namid = 0;
    addid = 1;

    // 4. Delete all student records from Firebase
    if (firebaseConfig.host != "" && firebaseConfig.signer.tokens.legacy_token != "") {
      String studentsPath = "/students";
      if (Firebase.deleteNode(firebaseData, studentsPath.c_str())) {
        Serial.println("Student records deleted from Firebase");
      } else {
        success = false;
        errorMessage += "Failed to delete student records from Firebase. ";
        errorMessage += firebaseData.errorReason() + ". ";
      }
    }

    if (success) {
      server.send(200, "text/plain", "All student records cleared successfully from both local storage and Firebase.");
    } else {
      server.send(500, "text/plain", "Error: " + errorMessage);
    }
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

void handleAddnew() {
  if (server.method() == HTTP_POST) {
    String studentName = server.arg("name");
    String roll = server.arg("roll");
    String fingerprintStatus = server.arg("fingerprintStatus");

    if (studentName.length() > 0 && roll.length() > 0 && fingerprintStatus == "scanned") {
      // Save to SD card in format "ID RollNumber Name"
      File file = SD.open("/name.txt", FILE_APPEND);
      if (file) {
        String line = String(addid) + " " + roll + " " + studentName;
        file.println(line);
        file.close();

        // Update local name array
        name[namid][0] = studentName;
        name[namid][1] = String(addid);
        name[namid][2] = roll;
        namid++;
        addid++;

        // Upload to Firebase
        String path = "/students/" + String(addid - 1);
        FirebaseJson json;
        json.set("rollNumber", roll);
        json.set("name", studentName);

        if (Firebase.setJSON(firebaseData, path.c_str(), json)) {
          Serial.println("Uploaded to Firebase - ID: " + String(addid - 1) + ", Roll: " + roll + ", Name: " + studentName);
        } else {
          Serial.println("Failed to upload to Firebase - ID: " + String(addid - 1));
          Serial.println("Error: " + firebaseData.errorReason());
        }
      }

      // Show thank you page
      String html = R"rawliteral(
    <!DOCTYPE html>
        <html>
<head>
          <title>Thank You</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
          <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
          <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css">
          )rawliteral" + getGlassmorphismStyles() + R"rawliteral(
          <style>
            .success-icon {
              color: rgba(46, 204, 113, 0.9);
              font-size: 4rem;
              margin-bottom: 20px;
            }
            .btn-home {
              background: rgba(46, 204, 113, 0.4);
              backdrop-filter: blur(12px);
              -webkit-backdrop-filter: blur(12px);
              border: 1px solid rgba(255, 255, 255, 0.18);
              color: white;
              padding: 12px 30px;
              border-radius: 25px;
              font-weight: 600;
              transition: all 0.3s ease;
              margin-top: 20px;
            }
            .btn-home:hover {
              transform: translateY(-2px);
              box-shadow: 0 8px 32px rgba(31, 38, 135, 0.2);
              background: rgba(46, 204, 113, 0.5);
            }
          </style>
        </head>
        <body>
          )rawliteral" + getNavbarHtml() + R"rawliteral(
          <div class="glass-card">
            <i class="fas fa-check-circle success-icon"></i>
            <h2 class="mb-4">Success!</h2>
            <p class="mb-4">The fingerprint and student details have been successfully saved.</p>
            <a href="/" class="btn btn-home">
              <i class="fas fa-home me-2"></i>Return to Home
            </a>
          </div>
        </body>
        </html>
      )rawliteral";
      server.send(200, "text/html", html);
      return;
    }
  }

  // Show the form if not POST or if validation failed
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Add New Fingerprint</title>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
      <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css">
      )rawliteral" + getGlassmorphismStyles() + R"rawliteral(
      <style>
        body {
          /* Modern deep blue/purple gradient background */
          background: linear-gradient(135deg, #23243a 0%, #3a3d5c 100%);
          min-height: 100vh;
          padding: 0;
          font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
          position: relative;
        }
        /* SVG pattern overlay for subtle depth */
        body::before {
          content: '';
          position: fixed;
          top: 0; left: 0; right: 0; bottom: 0;
          z-index: 0;
          pointer-events: none;
          opacity: 0.18;
          background: url('data:image/svg+xml;utf8,<svg width="100" height="100" viewBox="0 0 100 100" fill="none" xmlns="http://www.w3.org/2000/svg"><circle cx="50" cy="50" r="48" stroke="%23677b9c" stroke-width="2" fill="none"/><circle cx="50" cy="50" r="30" stroke="%23677b9c" stroke-width="1" fill="none"/><circle cx="50" cy="50" r="10" stroke="%23677b9c" stroke-width="1" fill="none"/></svg>');
          background-size: 200px 200px;
          background-repeat: repeat;
        }
        .card, .back-icon, .container {
          position: relative;
          z-index: 1;
        }
        .form-control {
          border-radius: 10px;
          padding: 12px;
          border: 1px solid rgba(0, 0, 0, 0.1);
          background: rgba(255, 255, 255, 0.8);
        }
        .form-control:focus {
          box-shadow: 0 0 0 0.2rem rgba(76, 175, 80, 0.25);
          border-color: #4CAF50;
        }
        .btn-scan {
          background: linear-gradient(135deg, #4CAF50 0%, #45a049 100%);
          border: none;
          color: white;
          padding: 12px 30px;
          border-radius: 25px;
          font-weight: 600;
          transition: all 0.3s ease;
        }
        .btn-scan:hover {
          transform: translateY(-2px);
          box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
        }
        .btn-save {
          background: linear-gradient(135deg, #2196F3 0%, #1976D2 100%);
          border: none;
          color: white;
          padding: 12px 30px;
          border-radius: 25px;
          font-weight: 600;
          transition: all 0.3s ease;
        }
        .btn-save:hover {
          transform: translateY(-2px);
          box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
        }
        .btn-save:disabled {
          background: #cccccc;
          cursor: not-allowed;
        }
        .status {
          margin-top: 20px;
          padding: 15px;
          border-radius: 10px;
          display: none;
        }
        .status.success {
          background-color: #d4edda;
          color: #155724;
          border: 1px solid #c3e6cb;
        }
        .status.error {
          background-color: #f8d7da;
          color: #721c24;
          border: 1px solid #f5c6cb;
        }
        .status.warning {
          background-color: #fff3cd;
          color: #856404;
          border: 1px solid #ffeeba;
        }
        .back-icon {
          position: fixed;
          top: 20px;
          left: 20px;
          font-size: 24px;
          color: #333;
          background: rgba(255, 255, 255, 0.9);
          width: 40px;
          height: 40px;
          border-radius: 50%;
          display: flex;
          align-items: center;
          justify-content: center;
          text-decoration: none;
          box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
          transition: all 0.3s ease;
          z-index: 1000;
        }
        .back-icon:hover {
          transform: scale(1.1);
          color: #000;
          text-decoration: none;
        }
      </style>
    </head>
    <body>
      )rawliteral" + getNavbarHtml() + R"rawliteral(
      <div class="container mt-4">
        <div class="glass-card shadow-sm p-4">
          <h2 class="text-center mb-4">Add New Fingerprint</h2>
          <form id="fingerprintForm" action="/addnew" method="POST">
            <div class="mb-3">
              <label for="roll" class="form-label">Roll Number</label>
              <input type="text" class="form-control" id="roll" name="roll" required>
            </div>
            <div class="mb-3">
              <label for="name" class="form-label">Name</label>
              <input type="text" class="form-control" id="name" name="name" required>
            </div>
            <input type="hidden" id="fingerprintStatus" name="fingerprintStatus" value="not_scanned">
            <div class="d-grid gap-2">
              <button type="button" class="btn btn-primary" onclick="scanFingerprint()">
                <i class="fas fa-fingerprint me-2"></i>Scan Fingerprint
              </button>
              <button type="submit" class="btn btn-success" id="saveButton" disabled>
                <i class="fas fa-save me-2"></i>Save
              </button>
            </div>
            <div id="status" class="alert mt-3 d-none"></div>
          </form>
        </div>
      </div>
      
      <script>
        function scanFingerprint() {
          const statusDiv = document.getElementById('status');
          statusDiv.classList.remove('d-none');
          statusDiv.classList.add('alert-info');
          statusDiv.textContent = 'Scanning fingerprint... Please place your finger on the sensor.';
          
          fetch('/scanFingerprint', { method: 'POST' })
            .then(response => response.text())
            .then(data => {
              if (data.includes('success')) {
                statusDiv.classList.remove('alert-info');
                statusDiv.classList.add('alert-success');
                statusDiv.textContent = 'Fingerprint scanned successfully! You can now save the record.';
                document.getElementById('fingerprintStatus').value = 'scanned';
                document.getElementById('saveButton').disabled = false;
              } else {
                statusDiv.classList.remove('alert-info');
                statusDiv.classList.add('alert-danger');
                statusDiv.textContent = data;
              }
            })
            .catch(error => {
              statusDiv.classList.remove('alert-info');
              statusDiv.classList.add('alert-danger');
              statusDiv.textContent = 'Error: ' + error;
            });
        }
      </script>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void handleFormSubmit() {
  if (server.hasArg("name") && server.hasArg("rollNumber")) {
    String userName = server.arg("name");
    String rollNumber = server.arg("rollNumber");
    Serial.println("Received Name: " + userName);
    Serial.println("Received Roll Number: " + rollNumber);

    tft.fillScreen(TFT_WHITE);                // Clear the screen
    tft.fillRect(0, 15, 128, 30, TFT_WHITE);  // Clear area below time and WiFi icon
    tft.setCursor(2, 20);                     // Start below the time/WiFi area
    tft.setTextColor(TFT_BLACK);              // Set text color to black
    tft.setTextSize(1);                       // Set text size
    tft.println("Name and Roll Number received!");
    tft.println("Name: " + userName);
    tft.println("Roll Number: " + rollNumber);

    // Save the name, roll number, and ID to the SD card
    File file = SD.open("/name.txt", FILE_APPEND);
    if (file) {
      file.print(String(addid) + " ");  // Use the current `addid` as the ID
      file.print(rollNumber + " ");     // Save the roll number
      file.println(userName);           // Save the name
      file.close();

      // Upload to Firebase immediately
      if (Firebase.ready()) {
        String path = "/students/" + String(addid);
        FirebaseJson json;
        json.set("rollNumber", rollNumber);
        json.set("name", userName);
        if (Firebase.setJSON(firebaseData, path.c_str(), json)) {
          Serial.println("Uploaded to Firebase - ID: " + String(addid) + ", Roll: " + rollNumber + ", Name: " + userName);
        } else {
          Serial.println("Failed to upload to Firebase - ID: " + String(addid));
          Serial.println("Error: " + firebaseData.errorReason());
        }
      }
    } else {
      Serial.println("Failed to save name and roll number to SD card.");
      tft.println("Failed to save name and roll number.");
      server.send(500, "text/plain", "Failed to save name and roll number.");
      return;
    }

    // Update the `name` array
    name[namid][0] = userName;
    name[namid][1] = String(addid);
    name[namid][2] = rollNumber;  // Add roll number to the array
    namid++;
    addid++;  // Increment the ID for the next fingerprint

    // Send a "Thank You" page
    String html = R"rawliteral(
      <!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Thank You</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" rel="stylesheet">
    )rawliteral" + getGlassmorphismStyles() + R"rawliteral(
</head>
<body>
    )rawliteral" + getNavbarHtml() + R"rawliteral(
    <div class="container bg-white p-5 rounded shadow mt-5">
        <h1 class="h4 text-center text-success mb-4">Thank You!</h1>
        <p class="text-center">Your details have been successfully added to the system.</p>
        <p class="text-center"><strong>Name:</strong> )rawliteral"
                  + userName + R"rawliteral(</p>
        <p class="text-center"><strong>Roll Number:</strong> )rawliteral"
                  + rollNumber + R"rawliteral(</p>
        <p class="text-center"><strong>ID:</strong> )rawliteral"
                  + String(addid - 1) + R"rawliteral(</p>
        <div class="text-center mt-4">
            <a href="/addnew" class="btn btn-primary">Add Another Fingerprint</a>
            <a href="/" class="btn btn-secondary">Go Back to Home</a>
        </div>
    </div>
</body>
</html>
    )rawliteral";

    server.send(200, "text/html", html);
  } else {
    server.send(400, "text/plain", "Error: Name or Roll Number not provided");
    tft.println("Error: Name or Roll Number not provided.");
  }
}

void handleScanFingerprint() {
  if (server.method() == HTTP_POST) {
    // Use the scanFingerprint implementation from the components directory
    scanFingerprint();
    // The response will be sent by the scanFingerprint function
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

// Attendance management
void handleAllfile() {
  // For compatibility, now just call the new a2z() function
  a2z();
}

void handleDeleteAll() {
  tft.fillScreen(TFT_WHITE);                // Clear the screen
  tft.fillRect(0, 15, 128, 30, TFT_WHITE);  // Clear area below time and WiFi icon
  tft.setCursor(2, 20);                     // Start below the time/WiFi area
  tft.setTextColor(TFT_BLACK);              // Set text color to black
  tft.setTextSize(1);                       // Set text size
  tft.println("Deleting all records...");

  // Delete the name file from the SD card
  if (SD.exists("/name.txt")) {
    SD.remove("/name.txt");
  }

  // Clear the name array
  for (int i = 0; i < 128; i++) {
    name[i][0] = "";
    name[i][1] = "";
    name[i][2] = "";
  }
  namid = 0;
  addid = 1;

  // Clear the fingerprint sensor's internal database
  if (finger.emptyDatabase() == FINGERPRINT_OK) {
    Serial.println("Fingerprint database cleared successfully.");
    tft.println("Fingerprint database cleared successfully.");
  } else {
    Serial.println("Failed to clear fingerprint database.");
    tft.println("Failed to clear fingerprint database.");
  }

  tft.println("All records deleted.");
  server.send(200, "text/plain", "All records and fingerprint database cleared successfully.");
}

void handleDeleteSelectedDates() {
  if (server.hasArg("plain")) {
    String jsonStr = server.arg("plain");
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, jsonStr);

    if (error) {
      server.send(400, "text/plain", "Invalid JSON format");
      return;
    }

    JsonArray dates = doc["dates"];
    int successCount = 0;
    int failCount = 0;

    for (JsonVariant date : dates) {
      String dateStr = date.as<String>();
      String fileName = getAttendanceFilePath(dateStr);
      
      if (SD.exists(fileName)) {
        if (SD.remove(fileName)) {
          successCount++;
          
          // Also delete from Firebase if configured
          if (firebaseConfig.host != "" && firebaseConfig.signer.tokens.legacy_token != "") {
            String month = dateStr.substring(3, 5);  // Extract month (MM) from DD-MM-YYYY
            String path = "/attendance/" + month + "/" + dateStr;
            Firebase.deleteNode(firebaseData, path.c_str());
          }
        } else {
          failCount++;
        }
      } else {
        // File doesn't exist, consider it a successful deletion
        successCount++;
      }
    }

    String response = "Successfully deleted " + String(successCount) + " date(s)";
    if (failCount > 0) {
      response += ", failed to delete " + String(failCount) + " date(s)";
    }
    server.send(200, "text/plain", response);
  } else {
    server.send(400, "text/plain", "No dates provided");
  }
}

void handleDeleteAllAttendance() {
  // Implement a function to delete all attendance records
  bool success = true;
  String errorMessage = "";

  // Iterate through the Attendance directory and delete all files
  File root = SD.open("/Attendance");
  if (root) {
    File monthDir = root.openNextFile();
    while (monthDir) {
      if (monthDir.isDirectory()) {
        String monthPath = "/Attendance/" + String(monthDir.name());
        File dateFile = SD.open(monthPath);
        if (dateFile) {
          File file = dateFile.openNextFile();
          while (file) {
            String fileName = file.name();
            if (fileName.endsWith(".txt")) {
              String filePath = monthPath + "/" + fileName;
              if (!SD.remove(filePath)) {
                success = false;
                errorMessage += "Failed to delete " + filePath + ". ";
              }
            }
            file = dateFile.openNextFile();
          }
          dateFile.close();
        }
      }
      monthDir = root.openNextFile();
    }
    root.close();
  } else {
    success = false;
    errorMessage = "Failed to open Attendance directory.";
  }

  // Delete from Firebase if credentials are set
  if (firebaseConfig.host != "" && firebaseConfig.signer.tokens.legacy_token != "") {
    String path = "/attendance";
    if (!Firebase.deleteNode(firebaseData, path.c_str())) {
      success = false;
      errorMessage += "Failed to delete attendance records from Firebase. " + firebaseData.errorReason();
    }
  }

  if (success) {
    server.send(200, "text/plain", "All attendance records deleted successfully.");
  } else {
    server.send(500, "text/plain", "Error: " + errorMessage);
  }
}

void handleGetAttendanceCount() {
  if (server.hasArg("date")) {
    String date = server.arg("date");
    String filePath = getAttendanceFilePath(date);

    int presentCount = 0;
    int totalStudents = namid;  // Total number of students is the size of the `name` array

    if (SD.exists(filePath)) {
      File file = SD.open(filePath, FILE_READ);
      if (file) {
        while (file.available()) {
          String line = file.readStringUntil('\n');
          line.trim();
          if (line.startsWith("<tr>") && line.endsWith("</tr>")) {
            // Only count valid attendance records
            int idStart = line.indexOf("<td>", line.indexOf("<td>", line.indexOf("<td>") + 4) + 4) + 4;
            int idEnd = line.indexOf("</td>", idStart);
            if (idStart > 0 && idEnd > 0) {
              String id = line.substring(idStart, idEnd);
              if (id.toInt() > 0) {  // Only count if fingerprint ID is valid
                presentCount++;
              }
            }
          }
        }
        file.close();
      }
    }

    String jsonResponse = "{\"present\":" + String(presentCount) + ",\"total\":" + String(totalStudents) + "}";
    server.send(200, "application/json", jsonResponse);
  } else {
    server.send(400, "application/json", "{\"error\":\"No date provided\"}");
  }
}

void handleGetAttendanceData() {
  if (server.hasArg("date")) {
    String date = server.arg("date");
    String filePath = getAttendanceFilePath(date);
    
    if (SD.exists(filePath)) {
      String html = "";
      html += "<table class='table table-bordered table-striped'>";
      html += "<thead class='thead-dark'>";
      html += "<tr><th>Roll Number</th><th>Name</th><th>Fingerprint ID</th><th>In Time</th><th>Out Time</th></tr>";
      html += "</thead><tbody>";
      
      File file = SD.open(filePath, FILE_READ);
      if (file) {
        while (file.available()) {
          String line = file.readStringUntil('\n');
          line.trim();
          if (line.length() > 0) {
            int nameStart = line.indexOf("<td>", line.indexOf("<td>") + 4) + 4;
            int nameEnd = line.indexOf("</td>", nameStart);
            if (nameStart > 0 && nameEnd > 0) {
              String name = line.substring(nameStart, nameEnd);
              if (name.endsWith("[U]")) {
                String cleanName = name.substring(0, name.length() - 3);
                line = line.substring(0, nameStart) + cleanName + line.substring(nameEnd);
              }
            }
            html += line;
          }
        }
        file.close();
      } else {
        html += "<tr><td colspan='5' class='text-center'>Failed to open attendance file.</td></tr>";
      }
      
      html += "</tbody></table>";
      
      if (html.indexOf("<tr class=") == -1 && html.indexOf("<tr>") == -1) {
        // No records found
        html = "<div class='alert alert-info text-center p-4 rounded shadow-sm'><i class='fas fa-info-circle me-2'></i>No attendance records found for this date.</div>";
      }
      
      server.send(200, "text/html", html);
    } else {
      server.send(404, "text/html", "<div class='alert alert-warning text-center p-4 rounded shadow-sm'><i class='fas fa-exclamation-triangle me-2'></i>No attendance file found for the selected date.</div>");
    }
  } else {
    server.send(400, "text/html", "<div class='alert alert-danger text-center p-4 rounded shadow-sm'><i class='fas fa-times-circle me-2'></i>No date parameter provided.</div>");
  }
}

// Settings management
void handleSettings() {
  // Read current WiFi credentials
  String currentSSID = "";
  String currentPassword = "";
  readWiFiCredentials(currentSSID, currentPassword);

  // Read current Firebase credentials
  String currentFirebaseHost = "";
  String currentFirebaseAuth = "";
  File firebaseFile = SD.open("/firebase.txt", FILE_READ);
  if (firebaseFile) {
    while (firebaseFile.available()) {
      String line = firebaseFile.readStringUntil('\n');
      line.trim();
      if (line.startsWith("HOST=")) {
        currentFirebaseHost = line.substring(5);
      } else if (line.startsWith("AUTH=")) {
        currentFirebaseAuth = line.substring(5);
      }
    }
    firebaseFile.close();
  }

  // Read current Telegram credentials
  String currentBotToken = "";
  String currentChatId = "";
  File telegramFile = SD.open("/telegram.txt", FILE_READ);
  if (telegramFile) {
    while (telegramFile.available()) {
      String line = telegramFile.readStringUntil('\n');
      line.trim();
      if (line.startsWith("BOT_TOKEN=")) {
        currentBotToken = line.substring(10);
      } else if (line.startsWith("CHAT_ID=")) {
        currentChatId = line.substring(8);
      }
    }
    telegramFile.close();
  }

  // Read current admin credentials (username/password)
  String currentAdminUser = DEFAULT_USERNAME;
  String currentAdminPass = DEFAULT_PASSWORD;
  File adminFile = SD.open("/admin.txt", FILE_READ);
  if (adminFile) {
    while (adminFile.available()) {
      String line = adminFile.readStringUntil('\n');
      line.trim();
      if (line.startsWith("USER=")) {
        currentAdminUser = line.substring(5);
      } else if (line.startsWith("PASS=")) {
        currentAdminPass = line.substring(5);
      }
    }
    adminFile.close();
  }

  // Generate CSRF token
  String csrf = generateCSRFToken();
  String csrfInput = "<input type='hidden' name='csrf_token' value='" + csrf + "'>";

  String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Settings</title>
        <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css" rel="stylesheet">
        <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.4/css/all.min.css" rel="stylesheet">
        )rawliteral" + getGlassmorphismStyles() + R"rawliteral(
        <style>
            .settings-section {
                margin-bottom: 30px;
                padding-bottom: 20px;
                border-bottom: 1px solid rgba(255, 255, 255, 0.1);
            }
            .settings-title {
                font-weight: 600;
                margin-bottom: 10px;
                color: white;
            }
            .settings-description {
                color: rgba(255, 255, 255, 0.7);
                margin-bottom: 15px;
                font-size: 0.9rem;
            }
            .reinit-options {
                display: none;
                margin-top: 15px;
            }
            .reinit-options.show {
                display: block;
            }
            .status-text {
                margin-top: 20px;
                padding: 10px;
                border-radius: 5px;
                display: none;
                font-weight: 500;
            }
            .status-text.show {
                display: block;
            }
            .status-text.success {
                background-color: rgba(46, 204, 113, 0.2);
                color: #2ecc71;
            }
            .status-text.error {
                background-color: rgba(231, 76, 60, 0.2);
                color: #e74c3c;
            }
        </style>
    </head>
    <body>
    )rawliteral" + getNavbarHtml() + R"rawliteral(
      <div class="container mt-4">
        <div class="glass-card">
            <div class="glass-card-header">
                <i class="fas fa-cog"></i> System Settings
            </div>
            <div class="card-body">
                <div class="settings-section">
                    <h5 class="settings-title">System Reinitialization</h5>
                    <p class="settings-description">Reinitialize system components</p>
                    <button onclick="toggleReinitOptions()" class="btn btn-glass btn-glass-warning">
                        <i class="fas fa-tools"></i> Show Reinitialization Options
                    </button>
                    <div id="reinitOptions" class="reinit-options">
                        <div class="mb-3">
                            <button onclick="reinitializeDisplay()" class="btn btn-glass btn-glass-secondary">
                                <i class="fas fa-sync"></i> Reinitialize Display
                            </button>
                        </div>
                        <div class="mb-3">
                            <button onclick="reinitializeSD()" class="btn btn-glass btn-glass-secondary">
                                <i class="fas fa-sync"></i> Reinitialize SD Card
                            </button>
                        </div>
                        <div class="mb-3">
                            <button onclick="reinitializeFingerprint()" class="btn btn-glass btn-glass-secondary">
                                <i class="fas fa-sync"></i> Reinitialize Fingerprint Sensor
                            </button>
                        </div>
                    </div>
                </div>

                <div class="settings-section">
                    <h5 class="settings-title">Change Admin Credentials</h5>
                    <p class="settings-description">Change the username and password for the admin login.</p>
                    <form id="adminForm" onsubmit="updateAdmin(event)">)rawliteral" + csrfInput + R"rawliteral(
                        <div class="mb-3">
                            <label for="adminUser" class="form-label">Username</label>
                            <input type="text" class="form-control" id="adminUser" name="adminUser" value=")rawliteral" + currentAdminUser + R"rawliteral(" required>
                        </div>
                        <div class="mb-3">
                            <label for="adminPass" class="form-label">Password</label>
                            <input type="password" class="form-control" id="adminPass" name="adminPass" value=")rawliteral" + currentAdminPass + R"rawliteral(" required>
                        </div>
                        <button type="submit" class="btn btn-glass btn-glass-primary">
                            <i class="fas fa-save"></i> Update Admin
                        </button>
                    </form>
                </div>

                <div class="settings-section">
                    <h5 class="settings-title">WiFi Settings</h5>
                    <p class="settings-description">Update WiFi credentials</p>
                    <form id="wifiForm" onsubmit="updateWiFi(event)">)rawliteral" + csrfInput + R"rawliteral(
                        <div class="mb-3">
                            <label for="ssid" class="form-label">WiFi SSID</label>
                            <input type="text" class="form-control" id="ssid" name="ssid" value=")rawliteral" + currentSSID + R"rawliteral(" required>
                        </div>
                        <div class="mb-3">
                            <label for="password" class="form-label">WiFi Password</label>
                            <input type="password" class="form-control" id="password" name="password" value=")rawliteral" + currentPassword + R"rawliteral(" required>
                        </div>
                        <button type="submit" class="btn btn-glass btn-glass-primary">
                            <i class="fas fa-save"></i> Update WiFi
                        </button>
                    </form>
                </div>

                <!-- Firebase Settings Form -->
                <div class="settings-section">
                    <h5 class="settings-title">Firebase Settings</h5>
                    <p class="settings-description">Update Firebase credentials</p>
                    <form id="firebaseForm" onsubmit="updateFirebase(event)">)rawliteral" + csrfInput + R"rawliteral(
                        <div class="mb-3">
                            <label for="firebaseHost" class="form-label">Firebase Host URL</label>
                            <input type="text" class="form-control" id="firebaseHost" name="firebaseHost" value=")rawliteral" + currentFirebaseHost + R"rawliteral(" required>
                        </div>
                        <div class="mb-3">
                            <label for="firebaseAuth" class="form-label">Firebase Auth Token</label>
                            <input type="text" class="form-control" id="firebaseAuth" name="firebaseAuth" value=")rawliteral" + currentFirebaseAuth + R"rawliteral(" required>
                        </div>
                        <button type="submit" class="btn btn-glass btn-glass-primary">
                            <i class="fas fa-save"></i> Update Firebase
                        </button>
                    </form>
                </div>

                <!-- Telegram Settings Form -->
                <div class="settings-section">
                    <h5 class="settings-title">Telegram Settings</h5>
                    <p class="settings-description">Update Telegram bot credentials</p>
                    <form id="telegramForm" onsubmit="updateTelegram(event)">)rawliteral" + csrfInput + R"rawliteral(
                        <div class="mb-3">
                            <label for="botToken" class="form-label">Bot Token</label>
                            <input type="text" class="form-control" id="botToken" name="botToken" value=")rawliteral" + currentBotToken + R"rawliteral(" required>
                        </div>
                        <div class="mb-3">
                            <label for="chatId" class="form-label">Chat ID</label>
                            <input type="text" class="form-control" id="chatId" name="chatId" value=")rawliteral" + currentChatId + R"rawliteral(" required>
                        </div>
                        <button type="submit" class="btn btn-glass btn-glass-primary">
                            <i class="fas fa-save"></i> Update Telegram
                        </button>
                    </form>
                </div>

                <!-- Data Management Section -->
                <div class="settings-section">
                    <h5 class="settings-title">Data Management</h5>
                    <p class="settings-description">Manage attendance records and student data</p>
                    <button onclick="syncData()" class="btn btn-glass btn-glass-success">
                        <i class="fas fa-sync"></i> Sync Names & Attendance
                    </button>
                    <button onclick="deleteAllAttendance()" class="btn btn-glass btn-glass-danger">
                        <i class="fas fa-trash"></i> Delete All Attendance
                    </button>
                    <button onclick="deleteAllStudents()" class="btn btn-glass btn-glass-danger">
                        <i class="fas fa-trash"></i> Delete All Students
                    </button>
                </div>

                <p id="status" class="status-text"></p>
            </div>
        </div>
      </div>

      <script>
          // JavaScript functions for the settings page
          function toggleReinitOptions() {
              const options = document.getElementById('reinitOptions');
              options.classList.toggle('show');
              const button = event.target.closest('button');
              if (options.classList.contains('show')) {
                  button.innerHTML = '<i class="fas fa-tools"></i> Hide Reinitialization Options';
              } else {
                  button.innerHTML = '<i class="fas fa-tools"></i> Show Reinitialization Options';
              }
          }

          function showStatus(message, isError = false) {
              const statusDiv = document.getElementById('status');
              statusDiv.textContent = message;
              statusDiv.className = 'status-text show ' + (isError ? 'error' : 'success');
              setTimeout(() => {
                  statusDiv.className = 'status-text';
              }, 5000); // Hide after 5 seconds
          }

          function updateAdmin(event) {
              event.preventDefault();
              const formData = new FormData(event.target);
              showStatus('Updating admin credentials...', false);
              fetch('/updateAdmin', {
                  method: 'POST',
                  body: formData
              })
              .then(response => response.text())
              .then(data => {
                  showStatus(data, data.includes('Error'));
                  if (data.includes('successfully')) {
                      setTimeout(() => {
                          window.location.reload();
                      }, 2000);
                  }
              })
              .catch(error => {
                  showStatus('Error: ' + error, true);
              });
          }

          function reinitializeDisplay() {
              if (confirm('Are you sure you want to reinitialize the display?')) {
                  showStatus('Reinitializing display...', false);
                  fetch('/reinitializeDisplay', { method: 'POST' })
                      .then(response => response.text())
                      .then(data => {
                          showStatus(data, data.includes('Error'));
                      })
                      .catch(error => {
                          showStatus('Error: ' + error, true);
                      });
              }
          }

          function reinitializeSD() {
              if (confirm('Are you sure you want to reinitialize the SD card?')) {
                  showStatus('Reinitializing SD card...', false);
                  fetch('/reinitializeSD', { method: 'POST' })
                      .then(response => response.text())
                      .then(data => {
                          showStatus(data, data.includes('Error'));
                      })
                      .catch(error => {
                          showStatus('Error: ' + error, true);
                      });
              }
          }

          function reinitializeFingerprint() {
              if (confirm('Are you sure you want to reinitialize the fingerprint sensor?')) {
                  showStatus('Reinitializing fingerprint sensor...', false);
                  fetch('/reinitializeFingerprint', { method: 'POST' })
                      .then(response => response.text())
                      .then(data => {
                          showStatus(data, data.includes('Error'));
                      })
                      .catch(error => {
                          showStatus('Error: ' + error, true);
                      });
              }
          }

          function updateWiFi(event) {
              event.preventDefault();
              const formData = new FormData(event.target);
              showStatus('Updating WiFi credentials...', false);
              
              fetch('/updateWiFi', {
                  method: 'POST',
                  body: formData
              })
              .then(response => response.text())
              .then(data => {
                  showStatus(data, data.includes('Error'));
                  if (data.includes('successfully')) {
                      setTimeout(() => {
                          window.location.reload();
                      }, 2000);
                  }
              })
              .catch(error => {
                  showStatus('Error: ' + error, true);
              });
          }

          function updateFirebase(event) {
              event.preventDefault();
              const formData = new FormData(event.target);
              showStatus('Updating Firebase credentials...', false);
              
              fetch('/updateFirebase', {
                  method: 'POST',
                  body: formData
              })
              .then(response => response.text())
              .then(data => {
                  showStatus(data, data.includes('Error'));
              })
              .catch(error => {
                  showStatus('Error: ' + error, true);
              });
          }

          function updateTelegram(event) {
              event.preventDefault();
              const formData = new FormData(event.target);
              showStatus('Updating Telegram settings...', false);
              
              fetch('/updateTelegram', {
                  method: 'POST',
                  body: formData
              })
              .then(response => response.text())
              .then(data => {
                  showStatus(data, data.includes('Error'));
              })
              .catch(error => {
                  showStatus('Error: ' + error, true);
              });
          }

          function syncData() {
              showStatus('Syncing names and attendance...', false);
              fetch('/syncData', { method: 'POST' })
                  .then(response => response.text())
                  .then(data => {
                      showStatus(data, data.includes('Error'));
                  })
                  .catch(error => {
                      showStatus('Error: ' + error, true);
                  });
          }

          function deleteAllAttendance() {
              if (confirm('Are you sure you want to delete all attendance records? This action cannot be undone.')) {
                  showStatus('Deleting all attendance records...', false);
                  fetch('/deleteAllAttendance', { method: 'POST' })
                      .then(response => response.text())
                      .then(data => {
                          showStatus(data, data.includes('Error'));
                      })
                      .catch(error => {
                          showStatus('Error: ' + error, true);
                      });
              }
          }

          function deleteAllStudents() {
              if (confirm('Are you sure you want to delete all student records? This action cannot be undone.')) {
                  showStatus('Deleting all student records...', false);
                  fetch('/deleteAllStudents', { method: 'POST' })
                      .then(response => response.text())
                      .then(data => {
                          showStatus(data, data.includes('Error'));
                      })
                      .catch(error => {
                          showStatus('Error: ' + error, true);
                      });
              }
          }
      </script>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleUpdateFirebase() {
  if (!verifyCSRFToken()) {
    server.send(403, "text/plain", "Invalid CSRF token");
    return;
  }
  if (server.hasArg("firebaseHost") && server.hasArg("firebaseAuth")) {
    String newHost = server.arg("firebaseHost");
    String newAuth = server.arg("firebaseAuth");

    // Save the new Firebase credentials to the SD card
    File file = SD.open("/firebase.txt", FILE_WRITE);
    if (file) {
      file.println("HOST=" + newHost);
      file.println("AUTH=" + newAuth);
      file.close();

      // Update the in-memory credentials
      firebaseConfig.host = newHost.c_str();
      firebaseConfig.signer.tokens.legacy_token = newAuth.c_str();

      // Reinitialize Firebase
      Firebase.begin(&firebaseConfig, &firebaseAuth);
      Firebase.reconnectWiFi(true);

      server.send(200, "text/plain", "Firebase settings updated successfully. System will restart to apply changes.");
      delay(1000);
      ESP.restart();
    } else {
      server.send(500, "text/plain", "Failed to update Firebase settings.");
    }
  } else {
    server.send(400, "text/plain", "Missing required parameters.");
  }
}

void handleUpdateWiFi() {
  if (!verifyCSRFToken()) {
    server.send(403, "text/plain", "Invalid CSRF token");
    return;
  }
  if (!server.hasArg("ssid") || !server.hasArg("password")) {
    server.send(400, "text/plain", "Missing SSID or password");
    return;
  }

  String newSSID = server.arg("ssid");
  String newPassword = server.arg("password");

  // Save new credentials to SD card
  File wifiFile = SD.open("/wifi.txt", FILE_WRITE);
  if (wifiFile) {
    wifiFile.println("SSID=" + newSSID);
    wifiFile.println("PASSWORD=" + newPassword);
    wifiFile.close();
    server.send(200, "text/plain", "WiFi credentials updated successfully. System will restart to apply changes.");
    delay(1000);
    ESP.restart();
  } else {
    server.send(500, "text/plain", "Failed to save WiFi credentials");
  }
}

void handleUpdateTelegram() {
  if (!verifyCSRFToken()) {
    server.send(403, "text/plain", "Invalid CSRF token");
    return;
  }
  if (!server.hasArg("botToken") || !server.hasArg("chatId")) {
    server.send(400, "text/plain", "Missing bot token or chat ID");
    return;
  }

  String newBotToken = server.arg("botToken");
  String newChatId = server.arg("chatId");

  File telegramFile = SD.open("/telegram.txt", FILE_WRITE);
  if (telegramFile) {
    telegramFile.println("BOT_TOKEN=" + newBotToken);
    telegramFile.println("CHAT_ID=" + newChatId);
    telegramFile.close();

    telegramBotToken = newBotToken;
    telegramChatId = newChatId;

    server.send(200, "text/plain", "Telegram settings updated successfully");
  } else {
    server.send(500, "text/plain", "Failed to save Telegram settings");
  }
}

void handleUpdateAdmin() {
  if (!verifyCSRFToken()) {
    server.send(403, "text/plain", "Invalid CSRF token");
    return;
  }
  if (!server.hasArg("adminUser") || !server.hasArg("adminPass")) {
    server.send(400, "text/plain", "Missing username or password");
    return;
  }
  String newUser = server.arg("adminUser");
  String newPass = server.arg("adminPass");

  // Save to SD card
  File adminFile = SD.open("/admin.txt", FILE_WRITE);
  if (adminFile) {
    adminFile.println("USER=" + newUser);
    adminFile.println("PASS=" + newPass);
    adminFile.close();
    server.send(200, "text/plain", "Admin credentials updated successfully");
  } else {
    server.send(500, "text/plain", "Failed to save admin credentials");
  }
}

void handleStartContinuousScanning() {
  if (server.method() == HTTP_POST) {
    // Add debugging information
    Serial.println("handleStartContinuousScanning called - activating fingerprint scanning");
    
    // Check if fingerprint sensor is ready, try to reinitialize if not
    if (!fingerprintReady) {
      Serial.println("Fingerprint sensor not ready, attempting to reinitialize...");
      
      // Try to reinitialize the sensor
      finger.begin(57600);
      if (setupFingerprint()) {
        fingerprintReady = true;
        Serial.println("Fingerprint sensor successfully reinitialized");
      } else {
        Serial.println("Failed to reinitialize fingerprint sensor");
        server.send(500, "text/plain", "Fingerprint sensor not ready. Please check connections.");
        return;
      }
    }
    
    isBlinking = true;
    setRGBColor(0, 0, 55);  // Set RGB LED to blue when starting scanning
    
    // Log the current scanning status
    Serial.println("isBlinking set to: " + String(isBlinking ? "true" : "false"));
    Serial.println("fingerprintReady status: " + String(fingerprintReady ? "true" : "false"));
    
    server.send(200, "text/plain", "Continuous scanning started");
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

void handleStopContinuousScanning() {
  if (server.method() == HTTP_POST) {
    isBlinking = false;
    setRGBColor(0, 0, 0);  // Turn off LED when scanning is stopped
    server.send(200, "text/plain", "Continuous scanning stopped");
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

void handleReinitializeDisplay() {
  tft.init(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(2, 2);
  tft.println("Display reinitialized");
  server.send(200, "text/plain", "Display reinitialized successfully. Screen has been reset.");
}

void handleReinitializeSD() {
  if (sdCardInitialized) {
    SD.end();
    delay(100);
  }

  sdCardInitialized = false;

  if (!setsd()) {
    server.send(500, "text/plain", "Error: SD card not found or initialization failed. Please check if the SD card is properly inserted.");
    return;
  }

  // Try to verify card is working by reading a file
  File testFile = SD.open("/name.txt", FILE_READ);
  if (!testFile) {
    server.send(500, "text/plain", "Error: SD card initialized but cannot read files. Please check card format and contents.");
    return;
  }
  testFile.close();

  // Reload student data
  loadStudentData();

  server.send(200, "text/plain", "SD card reinitialized successfully. Found " + String(namid) + " student records.");
}

void handleSyncData() {
  if (!checkAuth()) {
    server.send(401, "text/plain", "Unauthorized");
    return;
  }
  uploadNamesToFirebase();
  syncAttendanceWithFirebase();
  server.send(200, "text/plain", "Names and attendance synced successfully.");
}

void handleReinitializeFingerprint() {
  finger.begin(57600);
  if (!setupFingerprint()) {
    server.send(500, "text/plain", "Error: Fingerprint sensor not responding. Please check the connection.");
    return;
  }
  finger.getTemplateCount();
  server.send(200, "text/plain", "Fingerprint sensor reinitialized successfully. Sensor is ready for scanning.");
}

void handleScanningPage() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Today's Attendance Records</title>
        <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
        <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" rel="stylesheet">
        )rawliteral" + getGlassmorphismStyles() + R"rawliteral(
        <style>
            .container {
                max-width: 1200px;
                margin: 0 auto;
                padding: 20px;
            }

            .card {
                background: rgba(255, 255, 255, 0.9);
                border-radius: 15px;
                box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
                backdrop-filter: blur(10px);
                border: 1px solid rgba(255, 255, 255, 0.2);
                padding: 30px;
                margin-bottom: 20px;
            }

            .table {
                background: rgba(255, 255, 255, 0.9);
                backdrop-filter: blur(12px);
                -webkit-backdrop-filter: blur(12px);
                border-radius: 10px;
                overflow: hidden;
                box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
                color: #333; /* Add explicit text color */
            }

            .table thead th {
                background: rgba(44, 62, 80, 0.9);
                color: white;
                font-weight: 600;
                padding: 15px;
                border: none;
            }

            .table tbody td {
                padding: 12px 15px;
                vertical-align: middle;
                border-bottom: 1px solid rgba(0, 0, 0, 0.1);
                color: #333; /* Add explicit text color */
            }

            .table tbody tr:hover {
                background-color: rgba(0, 0, 0, 0.05);
            }

            .btn-custom {
                padding: 12px 25px;
                border-radius: 25px;
                font-weight: 600;
                transition: all 0.3s ease;
                margin: 0 5px;
                display: inline-flex;
                align-items: center;
                justify-content: center;
                gap: 8px;
            }

            .btn-custom:hover {
                transform: translateY(-2px);
                box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
            }

            .btn-custom i {
                font-size: 1.1em;
            }

            .status-text {
                margin-top: 20px;
                padding: 15px;
                border-radius: 10px;
                background: #f8f9fa;
                text-align: center;
                font-weight: 500;
            }

            .action-buttons {
                display: flex;
                justify-content: center;
                gap: 15px;
                margin-top: 30px;
                flex-wrap: wrap;
            }

            .in-time {
                color: #28a745;
                font-weight: 500;
            }

            .out-time {
                color: #dc3545;
                font-weight: 500;
            }

            @media (max-width: 768px) {
                .container {
                    padding: 10px;
                }
                
                .card {
                    padding: 20px;
                }

                .btn-custom {
                    width: 100%;
                    margin: 5px 0;
                }

                .action-buttons {
                    flex-direction: column;
                }
            }
        </style>
    </head>
    <body>
        )rawliteral" + getNavbarHtml() + R"rawliteral(
        <div class="container">
            <div class="glass-card">
                <h2 class="text-center mb-4">Today's Attendance Records</h2>
                <div class="table-responsive">
                    <table class="table">
                        <thead>
                            <tr>
                                <th>Roll Number</th>
                                <th>Name</th>
                                <th>Fingerprint ID</th>
                                <th>In Time</th>
                                <th>Out Time</th>
                            </tr>
                        </thead>
                        <tbody id="attendanceData">
    )rawliteral";

  // Read the current day's attendance records
  String currentDate = getCurrentDate();
  String filePath = getAttendanceFilePath(currentDate);
  
  if (SD.exists(filePath)) {
    File file = SD.open(filePath, FILE_READ);
    if (file) {
      while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.startsWith("<tr>") && line.endsWith("</tr>")) {
          html += line;
        }
      }
      file.close();
    } else {
      html += "<tr><td colspan='5' class='text-center'>No records found for today.</td></tr>";
    }
  } else {
    html += "<tr><td colspan='5' class='text-center'>No records found for today.</td></tr>";
  }

  html += R"rawliteral(
                        </tbody>
                    </table>
                </div>
                <div class="action-buttons">
                    <button onclick="startContinuousScanning()" class="btn btn-glass btn-glass-warning">
                        <i class="fas fa-fingerprint"></i>
                        Start Scanning
                    </button>
                    <button onclick="stopContinuousScanning()" class="btn btn-glass btn-glass-danger">
                        <i class="fas fa-stop"></i>
                        Stop Scanning
                    </button>
                    <button onclick="refreshPage()" class="btn btn-glass btn-glass-primary">
                        <i class="fas fa-sync"></i>
                        Refresh
                    </button>
                </div>
                <p id="status" class="status-text">Ready to scan</p>
            </div>
        </div>

        <script>
            function startContinuousScanning() {
                console.log('Starting fingerprint scanning...');
                document.getElementById('status').innerText = "Starting scanning...";
                document.getElementById('status').style.color = "#ffc107";  // Warning color
                
                fetch('/startContinuousScanning', { method: 'POST' })
                    .then(response => {
                        console.log('Scan request response status:', response.status);
                        return response.text();
                    })
                    .then(data => {
                        console.log('Scan request response:', data);
                        document.getElementById('status').innerText = "Scanning in progress...";
                        document.getElementById('status').style.color = "#28a745";  // Success color
                    })
                    .catch(error => {
                        console.error('Error:', error);
                        document.getElementById('status').innerText = "Error: " + error.message;
                        document.getElementById('status').style.color = "#dc3545";  // Danger color
                        alert('An error occurred while starting continuous scanning.');
                    });
            }

            function stopContinuousScanning() {
                console.log('Stopping fingerprint scanning...');
                document.getElementById('status').innerText = "Stopping scanning...";
                document.getElementById('status').style.color = "#ffc107";  // Warning color
                
                fetch('/stopContinuousScanning', { method: 'POST' })
                    .then(response => {
                        console.log('Stop scan request response status:', response.status);
                        return response.text();
                    })
                    .then(data => {
                        console.log('Stop scan request response:', data);
                        document.getElementById('status').innerText = "Scanning stopped.";
                        document.getElementById('status').style.color = "#dc3545";  // Danger color
                    })
                    .catch(error => {
                        console.error('Error:', error);
                        document.getElementById('status').innerText = "Error: " + error.message;
                        document.getElementById('status').style.color = "#dc3545";  // Danger color
                        alert('An error occurred while stopping continuous scanning.');
                    });
            }

            function refreshPage() {
                console.log('Refreshing page...');
                location.reload();
            }
            
            // Auto-refresh the data every 10 seconds
            setInterval(() => {
                fetch('/getAttendanceData?date=' + new Date().toISOString().split('T')[0])
                    .then(response => response.text())
                    .then(html => {
                        const parser = new DOMParser();
                        const doc = parser.parseFromString(html, 'text/html');
                        const rows = doc.querySelectorAll('tr');
                        if (rows.length > 0) {
                            document.getElementById('attendanceData').innerHTML = html;
                        }
                    })
                    .catch(error => console.error('Error refreshing data:', error));
            }, 10000);
        </script>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void a2z() {
  String html = R"rawliteral(
    <!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Attendance Records</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" rel="stylesheet">
    )rawliteral" + getGlassmorphismStyles() + R"rawliteral(
    <style>
    :root {
        --primary-color: #2c3e50;
        --secondary-color: #3498db;
        --accent-color: #e74c3c;
        --light-color: #ecf0f1;
        --dark-color: #2d5884;
    }

    body {
        background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
        min-height: 100vh;
        font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        margin: 0;
        display: flex;
        align-items: center;
        justify-content: center;
        width: 100vw;
        box-sizing: border-box;
        /* Remove any padding-top here as it's handled in navbar.html */
    }

    .container {
        max-width: 1200px;
        min-width: 0;
        width: 100%;
        padding: 1.5rem;
        margin: 0 auto;
        position: relative;
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
    }

    .glass-card,
    .card-body,
    .calendar,
    .calendar-header,
    .selection-controls {
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        text-align: center
    }

    .calendar-header {
        width: 100%;
    }

    .nav-buttons {
        display: flex;
        gap: 10px;
        justify-content: center;
        align-items: center;
        margin-bottom: 10px;
    }

    .selection-controls {
        flex-direction: row;
        gap: 10px;
        justify-content: center;
        align-items: center;
        margin-top: 10px;
    }

    #currentMonthYear {
        width: 100%;
        text-align: center;
        margin: 10px 0;
    }

    .calendar-grid {
        justify-items: center;
        align-items: center;
        width: 100%;
    }

    .card {
        border: none;
        border-radius: 15px;
        box-shadow: 0 10px 20px rgba(0, 0, 0, 0.1);
        transition: transform 0.3s ease;
        background: rgba(255, 255, 255, 0.856);
        backdrop-filter: blur(10px);
    }

    .card:hover {
        transform: translateY(-5px);
    }

    .card-header {
        background: var(--primary-color);
        color: white;
        border-radius: 15px 15px 0 0 !important;
        padding: 1.5rem;
    }

    .card-body {
        padding: 2rem;
    }

    .calendar {
        width: 100%;
        max-width: 1000px;
        min-width: fit-content;
        margin: 0 auto;
        margin-bottom: 20px;
        padding: 0;
        box-sizing: border-box;
        position: relative;
    }

    .calendar-header {
        display: flex;
        justify-content: space-between;
        align-items: center;
        margin-bottom: 10px;
        position: relative;
        padding: 0;
        flex-wrap: wrap;
        gap: 10px;
    }

    .calendar-header h5 {
        margin: 0;
        flex: 1;
        text-align: center;
        min-width: 100px;
        font-size: 1.2em;
    }

    .calendar-header button {
        white-space: nowrap;
        padding: 8px 12px;
        font-size: 1em;
        position: static;
    }

    .selection-controls {
        position: static;
        display: flex;
        gap: 10px;
        margin-left: auto;
    }

    .calendar-grid {
        display: grid;
        grid-template-columns: repeat(7, 1fr);
        gap: 8px;
        padding: 0;
    }

    .calendar-day {
        padding: 12px;
        min-width: 70px;
        min-height: 50px;
        text-align: center;
        background-color: #f8f9fa;
        border-radius: 5px;
        font-size: 1.1em;
        position: relative;
        display: flex;
        flex-direction: column;
        justify-content: center;
        align-items: center;
    }

    .calendar-day.header {
        font-weight: bold;
        background: rgba(44, 62, 80, 0.5);
        color: white;
        padding: 8px;
        min-height: 40px;
        font-size: 1.1em;
    }

    .calendar-day.has-data {
        background: rgba(46, 204, 113, 0.4);
        color: white;
        cursor: pointer;
        border: 1px solid rgba(46, 204, 113, 0.5);
    }

    .calendar-day.has-data:hover {
        background: rgba(46, 204, 113, 0.5);
        transform: translateY(-2px);
        box-shadow: 0 8px 32px rgba(31, 38, 135, 0.2);
    }

    .calendar-day.empty {
        background: rgba(255, 255, 255, 0.05);
        color: rgba(255, 255, 255, 0.5);
    }

    .calendar-day.future-date {
        background: rgba(255, 255, 255, 0.05);
        color: rgba(255, 255, 255, 0.4);
        cursor: not-allowed;
    }

    .calendar-day.future-date:hover {
        background: rgba(255, 255, 255, 0.05);
    }

    .attendance-count {
        font-size: 0.9em;
        margin-top: 4px;
        opacity: 0.8;
    }

    .table-responsive {
        margin-top: 20px;
        border-radius: 10px;
        overflow: hidden;
        box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
    }

    .table {
        margin-bottom: 0;
    }

    .table thead th {
        background-color: var(--primary-color);
        color: white;
        border: none;
        padding: 15px;
    }

    .table tbody td {
        padding: 12px 15px;
        vertical-align: middle;
    }

    .table tbody tr:nth-child(even) {
        background-color: rgba(0, 0, 0, 0.02);
    }

    .table tbody tr:hover {
        background-color: rgba(0, 0, 0, 0.04);
    }

    .btn-custom {
        padding: 10px 20px;
        border-radius: 8px;
        font-weight: 600;
        transition: all 0.3s ease;
        display: inline-flex;
        align-items: center;
        gap: 8px;
    }

    .btn-custom:hover {
        transform: translateY(-2px);
        box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
    }

    .btn-custom i {
        font-size: 1.1em;
    }

    .status {
        padding: 10px;
        border-radius: 5px;
        margin-top: 10px;
        display: none;
    }

    .status.success {
        background-color: #d4edda;
        color: #155724;
        border: 1px solid #c3e6cb;
    }

    .status.error {
        background-color: #f8d7da;
        color: #721c24;
        border: 1px solid #f5c6cb;
    }

    .status.warning {
        background-color: #fff3cd;
        color: #856404;
        border: 1px solid #ffeeba;
    }

    .back-icon {
        position: fixed;
        top: 20px;
        left: 20px;
        background: var(--primary-color);
        color: white;
        width: 50px;
        height: 50px;
        border-radius: 50%;
        display: flex;
        align-items: center;
        justify-content: center;
        cursor: pointer;
        transition: all 0.3s ease;
        box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
        z-index: 1000;
        text-decoration: none;
    }

    .back-icon:hover {
        background: var(--secondary-color);
        transform: scale(1.1);
    }

    @media (max-width: 768px) {
        @media (max-width: 768px) {
            .glass-card{
            width: 90% !important;

            }

            .glass-card,
            .card-body,
            .calendar,
            .calendar-header,
            .selection-controls {
                align-items: center !important;
                justify-content: center !important;
                text-align: center !important;
            }

            .calendar-header {
                flex-direction: column;
            }

            .selection-controls {
                flex-direction: row;
                margin-top: 10px;
            }
        }

        .calendar {
            width: 100%;
            margin: 0 8px;
            /* Add left and right margin for mobile */
            max-width: none;
            min-width: 320px;
            /* Optional: minimum width for the calendar */
            box-sizing: border-box;
        }

        .calendar-grid {
            width: 90%;
            min-width: 0;
            box-sizing: border-box;
        }

        .container {
            padding: 1rem;
        }

        .calendar-header {
            flex-direction: column;
            margin-left: 5%;

        }

        .calendar-header h5 {
            order: -1;
            margin-bottom: 10px;
        }

        .selection-controls {
            margin-left: 0;
            justify-content: center;
        }

        .calendar-day.header {
            max-width: 30px;
            min-height: 35px !important;
            /* Reduced from 40px */
            padding: 6px !important;
        }

        .calendar-day {
            padding: 5px;
            font-size: 0.9em;
            min-width: 40px;
            /* Ensures all date cells have a minimum width */
            box-sizing: border-box;
            min-height: 60px !important;
            /* Reduced from 70px */
            padding: 8px !important;
            /* Reduced from 12px */
            font-size: 0.9em !important;
        }

        .attendance-count {
            font-size: 0.8em !important;
            margin-top: 2px !important;
        }

        .table thead th {
            padding: 10px;
        }

        .table tbody td {
            padding: 8px 10px;
        }
    }

    @media (max-width: 480px) {
        .calendar-day {
            min-height: 50px !important;
            /* Even smaller for very small screens */
            padding: 6px !important;
            font-size: 0.85em !important;
        }

        .calendar-day.header {
            min-height: 30px !important;
            padding: 4px !important;
        }

        .attendance-count {
            font-size: 0.75em !important;
            margin-top: 1px !important;
        }
    }

    .attendance-table {
        position: fixed;
        top: 50%;
        left: 50%;
        transform: translate(-50%, -50%);
        width: 90%;
        max-width: 1000px;
        max-height: 80vh;
        background: rgba(255, 255, 255, 0.15);
        backdrop-filter: blur(12px);
        -webkit-backdrop-filter: blur(12px);
        border-radius: 15px;
        border: 1px solid rgba(255, 255, 255, 0.18);
        box-shadow: 0 8px 32px rgba(31, 38, 135, 0.1);
        z-index: 1000;
        display: none;
        overflow: hidden;
        color: white;
    }

    .table-container {
        height: 100%;
        display: flex;
        flex-direction: column;
    }

    .table-header {
        display: flex;
        justify-content: space-between;
        align-items: center;
        padding: 15px 20px;
        background: rgba(44, 62, 80, 0.5);
        color: white;
    }

    .table-title {
        margin: 0;
        font-size: 1.2em;
        font-weight: 600;
    }

    .close-table {
        background: rgba(255, 255, 255, 0.1);
        border: 1px solid rgba(255, 255, 255, 0.18);
        color: white;
        width: 30px;
        height: 30px;
        border-radius: 50%;
        display: flex;
        align-items: center;
        justify-content: center;
        cursor: pointer;
        transition: all 0.3s ease;
    }

    .close-table:hover {
        background: rgba(255, 255, 255, 0.2);
        transform: scale(1.1);
    }

    .overlay {
        position: fixed;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        background: rgba(0, 0, 0, 0.5);
        backdrop-filter: blur(5px);
        z-index: 999;
        display: none;
    }

    @media (min-width: 992px) {
        .calendar-grid {
            gap: 6px;
        }

        .attendance-count {
            font-size: 0.9em;
            margin-top: 4px;
        }
    }

    .month-picker {
        cursor: pointer;
        padding: 8px 16px;
        border-radius: 8px;
        transition: all 0.3s ease;
    }

    .month-picker:hover {
        background: rgba(255, 255, 255, 0.1);
    }

    .month-picker-menu {
        display: none;
        position: absolute;
        top: 100%;
        left: 50%;
        transform: translateX(-50%);
        background: rgba(255, 255, 255, 0.95);
        backdrop-filter: blur(10px);
        border-radius: 12px;
        padding: 16px;
        box-shadow: 0 8px 32px rgba(31, 38, 135, 0.1);
        z-index: 1000;
        width: 300px;
        margin-top: 8px;
    }

    .month-picker-header {
        display: flex;
        justify-content: space-between;
        align-items: center;
        margin-bottom: 16px;
        padding: 8px;
        border-bottom: 1px solid rgba(0, 0, 0, 0.1);
    }

    .month-picker-header span {
        font-size: 1.2em;
        font-weight: bold;
        min-width: 80px;
        text-align: center;
    }

    .month-picker-header button {
        padding: 4px 12px;
        border: none;
        background: rgba(52, 152, 219, 0.1);
        border-radius: 4px;
        cursor: pointer;
        transition: all 0.2s ease;
    }

    .month-picker-header button:hover {
        background: rgba(52, 152, 219, 0.2);
    }

    .month-grid {
        display: grid;
        grid-template-columns: repeat(3, 1fr);
        gap: 8px;
    }

    .month-item {
        padding: 8px;
        text-align: center;
        cursor: pointer;
        border-radius: 6px;
        transition: all 0.2s ease;
    }

    .month-item:hover {
        background: rgba(52, 152, 219, 0.1);
    }

    .month-item.active {
        background: rgba(52, 152, 219, 0.2);
        font-weight: bold;
    }
    </style>
</head>
<body>
  )rawliteral" + getNavbarHtml() + R"rawliteral(
   <div class="container">
        <div class="glass-card">
            <div class="glass-card-header">
                <h2 class="text-center mb-0">Attendance Records</h2>
            </div>
            <div class="card-body">
                <div class="calendar">
                    <div class="calendar-header">
                        <div class="nav-buttons">
                            <button class="btn btn-glass btn-glass-primary" onclick="previousMonth()">
                                <i class="fas fa-chevron-left"></i> Previous
                            </button>
                            <button class="btn btn-glass btn-glass-primary" onclick="nextMonth()">
                                Next <i class="fas fa-chevron-right"></i>
                            </button>
                        </div>
                        <h5 id="currentMonthYear" class="m-0 month-picker" onclick="toggleMonthPicker()">Month Year</h5>
                        <div id="monthPickerMenu" class="month-picker-menu">
                            <div class="month-picker-header">
                                <button onclick="changeYear(-1)" class="btn btn-sm btn-glass"><i
                                        class="fas fa-chevron-left"></i></button>
                                <span id="yearDisplay"></span>
                                <button onclick="changeYear(1)" class="btn btn-sm btn-glass"><i
                                        class="fas fa-chevron-right"></i></button>
                            </div>
                            <div class="month-grid">
                            </div>
                        </div>
                        <div class="selection-controls">
                            <button id="selectButton" class="btn btn-glass btn-glass-warning"
                                onclick="toggleSelectionMode()">
                                <i class="fas fa-check-square"></i> Select
                            </button>
                            <button id="deleteSelectedButton" class="btn btn-glass btn-glass-danger"
                                onclick="deleteSelectedDates()" style="display: none;">
                                <i class="fas fa-trash"></i> Delete Selected
                            </button>
                            <button class="btn btn-glass btn-glass-danger" onclick="deleteAllAttendance()">
                                <i class="fas fa-trash-alt"></i> Delete All
                            </button>
                        </div>
                    </div>
                    <div id="calendarGrid" class="calendar-grid">
                    </div>
                </div>

                <div id="status" class="status"></div>
            </div>
        </div>
    </div>

    <div class="overlay" id="overlay"></div>

    <script>
      let activeTable = null;
      let currentMonth = new Date().getMonth();
      let currentYear = new Date().getFullYear();
      let attendanceDates = [
)rawliteral";

  // Collect attendance date strings in the JavaScript array
  File root = SD.open("/Attendance");
  if (root) {
    File monthDir = root.openNextFile();
    while (monthDir) {
      if (monthDir.isDirectory()) {
        String monthPath = "/Attendance/" + String(monthDir.name());
        File dateDir = SD.open(monthPath);
        if (dateDir) {
          File file = dateDir.openNextFile();
          while (file) {
            String fileName = file.name();
            if (fileName.endsWith(".txt") && fileName.length() == 14) {
              String date = fileName.substring(0, fileName.length() - 4);
              html += "        \"" + date + "\",\n";
            }
            file = dateDir.openNextFile();
          }
          dateDir.close();
        }
      }
      monthDir = root.openNextFile();
    }
    root.close();
  }

  html += R"rawliteral(
      ];
    let selectedDates = new Set();
    let isSelectionMode = false;

    function toggleSelectionMode() {
        isSelectionMode = !isSelectionMode;
        const selectButton = document.getElementById('selectButton');
        const deleteSelectedButton = document.getElementById('deleteSelectedButton');

        if (isSelectionMode) {
            selectButton.innerHTML = '<i class="fas fa-times"></i> Cancel';
            selectButton.classList.remove('btn-warning');
            selectButton.classList.add('btn-secondary');
            deleteSelectedButton.style.display = 'inline-flex';
        } else {
            selectButton.innerHTML = '<i class="fas fa-check-square"></i> Select';
            selectButton.classList.remove('btn-secondary');
            selectButton.classList.add('btn-warning');
            deleteSelectedButton.style.display = 'none';
            selectedDates.clear();
            updateCalendarDisplay();
        }
    }

    function toggleDateSelection(date) {
        if (!isSelectionMode) return;

        if (selectedDates.has(date)) {
            selectedDates.delete(date);
        } else {
            selectedDates.add(date);
        }
        updateCalendarDisplay();
    }

    function updateCalendarDisplay() {
        const days = document.querySelectorAll('.calendar-day');
        days.forEach(day => {
            if (day.classList.contains('has-data')) {
                const date = day.getAttribute('data-date');
                if (selectedDates.has(date)) {
                    day.style.backgroundColor = '#dc3545';
                } else {
                    day.style.backgroundColor = '#28a745';
                }
            }
        });
    }

    function deleteSelectedDates() {
        if (selectedDates.size === 0) {
            alert('Please select at least one date to delete.');
            return;
        }

        if (confirm(`Are you sure you want to delete ${selectedDates.size} selected attendance records?`)) {
            const statusDiv = document.getElementById('status');
            statusDiv.style.display = 'block';
            statusDiv.className = 'status warning';
            statusDiv.innerHTML = '<i class="fas fa-spinner fa-spin me-2"></i>Deleting selected records...';

            const dates = Array.from(selectedDates);
            fetch('/deleteSelectedDates', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({
                        dates: dates
                    })
                })
                .then(response => response.text())
                .then(data => {
                    statusDiv.className = 'status success';
                    statusDiv.innerHTML = '<i class="fas fa-check-circle me-2"></i>' + data;
                    setTimeout(() => {
                        location.reload();
                    }, 2000);
                })
                .catch(error => {
                    statusDiv.className = 'status error';
                    statusDiv.innerHTML = '<i class="fas fa-exclamation-circle me-2"></i>Error: ' + error;
                });
        }
    }

    function deleteAllAttendance() {
        if (confirm('Are you sure you want to delete all attendance records? This action cannot be undone.')) {
            const statusDiv = document.getElementById('status');
            statusDiv.style.display = 'block';
            statusDiv.className = 'status warning';
            statusDiv.innerHTML = '<i class="fas fa-spinner fa-spin me-2"></i>Deleting all attendance records...';

            fetch('/deleteAllAttendance', {
                    method: 'POST'
                })
                .then(response => response.text())
                .then(data => {
                    statusDiv.className = 'status success';
                    statusDiv.innerHTML = '<i class="fas fa-check-circle me-2"></i>' + data;
                    setTimeout(() => {
                        location.reload();
                    }, 2000);
                })
                .catch(error => {
                    statusDiv.className = 'status error';
                    statusDiv.innerHTML = '<i class="fas fa-exclamation-circle me-2"></i>Error: ' + error;
                });
        }
    }

    function renderCalendar() {
        const calendarGrid = document.getElementById('calendarGrid');
        const currentMonthYear = document.getElementById('currentMonthYear');

        // Clear the calendar
        calendarGrid.innerHTML = '';

        // Set the month and year
        const monthNames = ['January', 'February', 'March', 'April', 'May', 'June',
            'July', 'August', 'September', 'October', 'November', 'December'
        ];
        currentMonthYear.textContent = `${monthNames[currentMonth]} ${currentYear}`;

        // Add day headers
        const days = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
        days.forEach(day => {
            const dayHeader = document.createElement('div');
            dayHeader.className = 'calendar-day header';
            dayHeader.textContent = day;
            calendarGrid.appendChild(dayHeader);
        });

        // Get the first day of the month
        const firstDay = new Date(currentYear, currentMonth, 1).getDay();

        // Get the number of days in the month
        const daysInMonth = new Date(currentYear, currentMonth + 1, 0).getDate();

        // Add empty cells for days before the first day of the month
        for (let i = 0; i < firstDay; i++) {
            const emptyDay = document.createElement('div');
            emptyDay.className = 'calendar-day empty';
            calendarGrid.appendChild(emptyDay);
        }

        // Add days of the month
        const today = new Date();
        for (let day = 1; day <= daysInMonth; day++) {
            const dayCell = document.createElement('div');
            dayCell.className = 'calendar-day';
            dayCell.textContent = day;

            // Format the date as DD-MM-YYYY
            const date =
                `${day.toString().padStart(2, '0')}-${(currentMonth + 1).toString().padStart(2, '0')}-${currentYear}`;

            // Check if this date has attendance data
            if (attendanceDates.includes(date)) {
                dayCell.classList.add('has-data');
                dayCell.setAttribute('data-date', date);
                dayCell.onclick = () => {
                    if (isSelectionMode) {
                        toggleDateSelection(date);
                    } else {
                        toggleTable(date);
                    }
                };

                // Fetch attendance count
                fetch('/getAttendanceCount?date=' + date)
                    .then(response => response.json())
                    .then(data => {
                        const presentCount = data.present || 0;
                        const totalStudents = data.total || 0;
                        const countElement = document.createElement('div');
                        countElement.className = 'attendance-count';
                        countElement.textContent = presentCount + '/' + totalStudents;
                        dayCell.appendChild(countElement);
                    })
                    .catch(error => {
                        console.error('Error fetching attendance count:', error);
                    });
            }

            // Check if this is a future date
            const currentDate = new Date(currentYear, currentMonth, day);
            if (currentDate > today) {
                dayCell.classList.add('future-date');
            }

            calendarGrid.appendChild(dayCell);
        }
    }

    function previousMonth() {
        currentMonth--;
        if (currentMonth < 0) {
            currentMonth = 11;
            currentYear--;
        }
        renderCalendar();
    }

    function nextMonth() {
        currentMonth++;
        if (currentMonth > 11) {
            currentMonth = 0;
            currentYear++;
        }
        renderCalendar();
    }

    function toggleTable(date) {
        // First, fetch the attendance data for this date
        fetch('/getAttendanceData?date=' + date)
            .then(response => response.text())
            .then(html => {
                // Create or update the attendance table
                let tableElement = document.getElementById('table-' + date);
                if (!tableElement) {
                    tableElement = document.createElement('div');
                    tableElement.id = 'table-' + date;
                    tableElement.className = 'attendance-table';
                    document.body.appendChild(tableElement);
                }

                tableElement.innerHTML = html;

                // Show the overlay and table
                document.getElementById('overlay').style.display = 'block';
                tableElement.style.display = 'block';

                // Set as active table
                activeTable = tableElement;
            })
            .catch(error => {
                console.error('Error fetching attendance data:', error);
                alert('Error fetching attendance data. Please try again.');
            });
    }

    function closeTable(date) {
        const table = document.getElementById('table-' + date);
        if (table) {
            table.style.display = 'none';
        }
        document.getElementById('overlay').style.display = 'none';
        activeTable = null;
    }

    // Close table when clicking outside
    document.getElementById('overlay').addEventListener('click', function() {
        if (activeTable) {
            activeTable.style.display = 'none';
            this.style.display = 'none';
            activeTable = null;
        }
    });

    // Add these functions to your existing script
    function toggleMonthPicker() {
        const menu = document.getElementById('monthPickerMenu');
        if (menu.style.display === 'none' || !menu.style.display) {
            showMonthPicker();
        } else {
            hideMonthPicker();
        }
    }

    function showMonthPicker() {
        const menu = document.getElementById('monthPickerMenu');
        const yearDisplay = document.getElementById('yearDisplay');
        const monthGrid = menu.querySelector('.month-grid');

        // Clear existing months
        monthGrid.innerHTML = '';

        // Set current year
        yearDisplay.textContent = currentYear;

        // Add months
        const monthNames = ['January', 'February', 'March', 'April', 'May', 'June',
            'July', 'August', 'September', 'October', 'November', 'December'
        ];

        monthNames.forEach((month, index) => {
            const monthDiv = document.createElement('div');
            monthDiv.className = 'month-item';
            if (index === currentMonth) monthDiv.classList.add('active');
            monthDiv.textContent = month;
            monthDiv.onclick = () => {
                currentMonth = index;
                currentYear = parseInt(yearDisplay.textContent);
                renderCalendar();
                hideMonthPicker();
            };
            monthGrid.appendChild(monthDiv);
        });

        menu.style.display = 'block';

        // Close when clicking outside
        document.addEventListener('click', closeMonthPickerOutside);
    }

    function hideMonthPicker() {
        const menu = document.getElementById('monthPickerMenu');
        menu.style.display = 'none';
        document.removeEventListener('click', closeMonthPickerOutside);
    }

    function closeMonthPickerOutside(event) {
        const menu = document.getElementById('monthPickerMenu');
        const monthPicker = document.getElementById('currentMonthYear');

        if (!menu.contains(event.target) && !monthPicker.contains(event.target)) {
            hideMonthPicker();
        }
    }

    function changeYear(delta) {
        const yearDisplay = document.getElementById('yearDisplay');
        const newYear = parseInt(yearDisplay.textContent) + delta;
        yearDisplay.textContent = newYear;
        currentYear = newYear;
        renderMonthGrid(); // Re-render months for the new year
    }

    // Add this new function to render month grid
    function renderMonthGrid() {
        const monthGrid = document.querySelector('.month-grid');
        const monthNames = ['January', 'February', 'March', 'April', 'May', 'June',
            'July', 'August', 'September', 'October', 'November', 'December'
        ];

        monthGrid.innerHTML = '';

        monthNames.forEach((month, index) => {
            const monthDiv = document.createElement('div');
            monthDiv.className = 'month-item';
            if (index === currentMonth && parseInt(document.getElementById('yearDisplay').textContent) ===
                currentYear) {
                monthDiv.classList.add('active');
            }
            monthDiv.textContent = month;
            monthDiv.onclick = () => {
                currentMonth = index;
                currentYear = parseInt(document.getElementById('yearDisplay').textContent);
                renderCalendar();
                hideMonthPicker();
            };
            monthGrid.appendChild(monthDiv);
        });
    }

    // Update the showMonthPicker function
    function showMonthPicker() {
        const menu = document.getElementById('monthPickerMenu');
        const yearDisplay = document.getElementById('yearDisplay');

        // Set current year
        yearDisplay.textContent = currentYear;

        // Render month grid
        renderMonthGrid();

        menu.style.display = 'block';

        // Close when clicking outside
        document.addEventListener('click', closeMonthPickerOutside);
    }

    // Initialize the calendar when the page loads
    window.onload = function() {
        renderCalendar();
        updateYearDisplay();
    };
    </script>
</body>

</html>
  )rawliteral";

  server.send(200, "text/html", html);
}

