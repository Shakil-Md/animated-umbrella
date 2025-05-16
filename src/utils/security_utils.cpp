#include "security_utils.h"
#include "../webserver/html_components.h"

bool checkAuth() {
  if (!isAuthenticated) {
    String cookie = server.header("Cookie");
    if (cookie.indexOf("session=" + sessionToken) == -1) {
      return false;
    }
  }

  // Check session expiration
  if (millis() > sessionExpiry) {
    isAuthenticated = false;
    sessionToken = "";
    return false;
  }

  return true;
}

void handleLogin() {
  if (server.method() == HTTP_POST) {
    if (server.hasArg("username") && server.hasArg("password")) {
      String username = server.arg("username");
      String password = server.arg("password");

      if (username == DEFAULT_USERNAME && password == DEFAULT_PASSWORD) {
        // Generate a more secure session token using timestamp and random number
        sessionToken = String(random(100000, 999999)) + String(millis());
        isAuthenticated = true;
        sessionExpiry = millis() + SESSION_TIMEOUT;

        // Send success response with session token
        server.sendHeader("Location", "/");
        server.sendHeader("Set-Cookie", "session=" + sessionToken + "; Path=/; Max-Age=3600; SameSite=Strict");
        server.send(302, "text/plain", "");
        return;
      }
    }
    // If we get here, login failed
    showLoginPage(true);
  } else {
    showLoginPage(false);
  }
}

void showLoginPage(bool showError) {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Login - Smart Attendance System</title>
        <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
        <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" rel="stylesheet">
        <style>
            body {
                background: linear-gradient(135deg, #23243a 0%, #3a3d5c 100%);
                min-height: 100vh;
                display: flex;
                align-items: center;
                justify-content: center;
                font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                position: relative;
                background-attachment: fixed;
                background-size: cover;
            }
            
            /* SVG pattern overlay for subtle texture */
            body::before {
                content: '';
                position: fixed;
                top: 0; left: 0; right: 0; bottom: 0;
                z-index: 0;
                pointer-events: none;
                opacity: 0.15;
                background: url('data:image/svg+xml;utf8,<svg width="100" height="100" viewBox="0 0 100 100" fill="none" xmlns="http://www.w3.org/2000/svg"><circle cx="50" cy="50" r="48" stroke="%23ffffff" stroke-width="1" fill="none"/><circle cx="50" cy="50" r="30" stroke="%23ffffff" stroke-width="0.8" fill="none"/><circle cx="50" cy="50" r="10" stroke="%23ffffff" stroke-width="0.5" fill="none"/></svg>');
                background-size: 150px 150px;
                background-repeat: repeat;
            }
            
            .login-container {
                background: rgba(255, 255, 255, 0.7);
                backdrop-filter: blur(12px);
                -webkit-backdrop-filter: blur(12px);
                border-radius: 20px;
                border: 1px solid rgba(255, 255, 255, 0.3);
                box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
                padding: 40px;
                width: 90%;
                max-width: 400px;
                position: relative;
                z-index: 1;
                transition: all 0.3s ease;
            }
            
            .login-container:hover {
                transform: translateY(-5px);
                box-shadow: 0 15px 35px rgba(0, 0, 0, 0.2);
            }
            
            .login-header {
                text-align: center;
                margin-bottom: 30px;
            }
            
            .login-header h1 {
                font-size: 28px;
                color: #2c3e50;
                margin-bottom: 10px;
                font-weight: 600;
            }
            
            .login-header p {
                color: #5d6778;
                font-size: 1rem;
            }
            
            .login-header .logo-icon {
                font-size: 48px;
                color: #3498db;
                margin-bottom: 20px;
                display: inline-block;
                background: rgba(52, 152, 219, 0.2);
                width: 80px;
                height: 80px;
                line-height: 80px;
                border-radius: 50%;
                border: 1px solid rgba(52, 152, 219, 0.3);
            }
            
            .form-floating {
                margin-bottom: 20px;
            }
            
            .form-control {
                background: rgba(255, 255, 255, 0.8);
                border: 1px solid rgba(255, 255, 255, 0.6);
                border-radius: 10px;
                padding: 12px 15px;
                transition: all 0.3s ease;
                height: auto;
                font-size: 16px;
            }
            
            .form-control:focus {
                background: rgba(255, 255, 255, 0.9);
                box-shadow: 0 0 0 3px rgba(52, 152, 219, 0.25);
                border-color: #3498db;
            }
            
            .form-floating label {
                padding: 12px 15px;
            }
            
            .btn-login {
                background: rgba(52, 152, 219, 0.9);
                border: none;
                color: white;
                padding: 12px;
                border-radius: 10px;
                font-weight: 600;
                width: 100%;
                margin-top: 20px;
                transition: all 0.3s ease;
                display: flex;
                align-items: center;
                justify-content: center;
                gap: 8px;
            }
            
            .btn-login:hover {
                transform: translateY(-3px);
                box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
                background: rgba(41, 128, 185, 0.95);
            }
            
            .error-message {
                background: rgba(231, 76, 60, 0.2);
                border: 1px solid rgba(231, 76, 60, 0.3);
                color: #c0392b;
                padding: 12px;
                border-radius: 10px;
                margin-bottom: 20px;
                display: flex;
                align-items: center;
                gap: 8px;
                font-size: 14px;
            }
            
            .form-text {
                color: #7f8c8d;
                font-size: 14px;
                text-align: center;
                margin-top: 15px;
            }
        </style>
        )rawliteral" + getGlassmorphismStyles() + R"rawliteral(
    </head>
    <body>
        <div class="login-container">
            <div class="login-header">
                <div class="logo-icon">
                    <i class="fas fa-fingerprint"></i>
                </div>
                <h1>Smart Attendance</h1>
                <p>Please login to continue</p>
            </div>
)rawliteral";

  if (showError) {
    html += R"rawliteral(
            <div class="error-message" id="errorMessage">
                <i class="fas fa-exclamation-circle"></i> Invalid username or password
            </div>
)rawliteral";
  }

  html += R"rawliteral(
            <form action="/login" method="POST">
                <div class="form-floating">
                    <input type="text" class="form-control" id="username" name="username" placeholder="Username" required>
                    <label for="username">Username</label>
                </div>
                <div class="form-floating">
                    <input type="password" class="form-control" id="password" name="password" placeholder="Password" required>
                    <label for="password">Password</label>
                </div>
                <button type="submit" class="btn btn-login">
                    <i class="fas fa-sign-in-alt"></i> Login
                </button>
                
            </form>
        </div>
    </body>
    </html>
)rawliteral";

  server.send(200, "text/html", html);
}

void handleLogout() {
  isAuthenticated = false;
  sessionToken = "";
  server.sendHeader("Location", "/login");
  server.sendHeader("Set-Cookie", "session=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
  server.send(302, "text/plain", "");
}

String generateCSRFToken() {
  String token = String(random(100000, 999999)) + String(millis());
  csrfToken = token;
  return token;
}

bool verifyCSRFToken() {
  if (!server.hasArg("csrf_token")) {
    return false;
  }
  String token = server.arg("csrf_token");
  return token == csrfToken;
} 