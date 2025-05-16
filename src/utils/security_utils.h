#ifndef SECURITY_UTILS_H
#define SECURITY_UTILS_H

#include "../config/config.h"

// Function declarations for security utilities
bool checkAuth();
void handleLogin();
void handleLogout();
void showLoginPage(bool showError);
String generateCSRFToken();
bool verifyCSRFToken();
int getFreeMemory();

#endif // SECURITY_UTILS_H 
 