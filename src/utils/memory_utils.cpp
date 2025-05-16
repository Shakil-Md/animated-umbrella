#include "memory_utils.h"

bool initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
    return false;
  }
  
  // Create HTML directory if it doesn't exist
  if (!SPIFFS.exists("/html")) {
    File dir = SPIFFS.open("/html", FILE_WRITE);
    if (dir) {
      dir.close();
    } else {
      Serial.println("Failed to create HTML directory");
      return false;
    }
  }
  
  // Check available space
  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes = SPIFFS.usedBytes();
  Serial.println("SPIFFS Total: " + String(totalBytes) + " bytes");
  Serial.println("SPIFFS Used: " + String(usedBytes) + " bytes");
  
  if (totalBytes < 512000) { // 512KB minimum
    Serial.println("WARNING: SPIFFS partition may be too small");
  }
  
  Serial.println("SPIFFS initialized successfully");
  return true;
}

void storeToSPIFFS(const String& key, const String& value) {
  if (isMemoryLow()) {
    clearSPIFFSCache();
  }
  
  File file = SPIFFS.open("/" + key + ".txt", FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  
  // Write in chunks to avoid memory issues
  const size_t chunkSize = 512;
  for (size_t i = 0; i < value.length(); i += chunkSize) {
    size_t end = min(i + chunkSize, value.length());
    if (!file.print(value.substring(i, end))) {
      Serial.println("Write failed");
      file.close();
      return;
    }
    yield(); // Allow other processes to run
  }
  
  file.close();
}

String loadFromSPIFFS(const String& key) {
  File file = SPIFFS.open("/" + key + ".txt", FILE_READ);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return "";
  }
  
  String content;
  content.reserve(file.size()); // Pre-allocate memory
  
  // Read in chunks
  const size_t chunkSize = 512;
  uint8_t buf[chunkSize];
  while (file.available()) {
    size_t bytesRead = file.read(buf, chunkSize);
    if (bytesRead > 0) {
      content += String((char*)buf, bytesRead);
    }
    yield();
  }
  
  file.close();
  return content;
}

void clearSPIFFSCache() {
  Dir dir = SPIFFS.openDir("/html");
  int filesDeleted = 0;
  
  while (dir.next()) {
    if (dir.fileName().endsWith(".html")) {
      if (SPIFFS.remove(dir.fileName())) {
        filesDeleted++;
      }
    }
    yield();
  }
  
  Serial.println("Cleared " + String(filesDeleted) + " cached files");
}

int getFreeMemory() {
  return ESP.getFreeHeap();
}

void monitorMemory() {
  static unsigned long lastMemCheck = 0;
  const unsigned long MEM_CHECK_INTERVAL = 60000; // Check every minute
  
  if (millis() - lastMemCheck >= MEM_CHECK_INTERVAL) {
    int freeHeap = ESP.getFreeHeap();
    int totalHeap = ESP.getHeapSize();
    int usedHeap = totalHeap - freeHeap;
    float heapPercentage = ((float)usedHeap / totalHeap) * 100;
    
    // Check heap fragmentation
    int maxBlock = ESP.getMaxAllocHeap();
    float fragmentation = 100 - ((float)maxBlock / freeHeap * 100);
    
    Serial.println("Memory Status:");
    Serial.println("Free Heap: " + String(freeHeap) + " bytes");
    Serial.println("Total Heap: " + String(totalHeap) + " bytes");
    Serial.println("Used Heap: " + String(usedHeap) + " bytes");
    Serial.println("Heap Usage: " + String(heapPercentage) + "%");
    Serial.println("Largest Free Block: " + String(maxBlock) + " bytes");
    Serial.println("Fragmentation: " + String(fragmentation) + "%");
    
    if (fragmentation > 70) {
      Serial.println("WARNING: High memory fragmentation!");
      clearSPIFFSCache(); // Try to free up some memory
    }
    
    if (isMemoryLow()) {
      Serial.println("WARNING: Low memory condition detected!");
      clearSPIFFSCache();
    }
    
    lastMemCheck = millis();
  }
}

bool isMemoryLow() {
  const int LOW_MEMORY_THRESHOLD = 10000; // 10KB
  const int CRITICAL_MEMORY_THRESHOLD = 5000; // 5KB
  
  int freeHeap = ESP.getFreeHeap();
  if (freeHeap < CRITICAL_MEMORY_THRESHOLD) {
    // Emergency cleanup
    clearSPIFFSCache();
    return true;
  }
  
  // Also check largest free block
  int maxBlock = ESP.getMaxAllocHeap();
  if (maxBlock < CRITICAL_MEMORY_THRESHOLD) {
    clearSPIFFSCache();
    return true;
  }
  
  return freeHeap < LOW_MEMORY_THRESHOLD;
}

// HTML content management functions
void cacheHTMLContent(const String& pageName, const String& content) {
  if (isMemoryLow()) {
    clearSPIFFSCache(); // Clear space if needed
  }
  
  String fileName = "/html/" + pageName + ".html";
  File file = SPIFFS.open(fileName, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing HTML cache");
    return;
  }
  
  // Write content in chunks to avoid memory issues
  const int chunkSize = 1024;
  int contentLength = content.length();
  for (int i = 0; i < contentLength; i += chunkSize) {
    int endIndex = min(i + chunkSize, contentLength);
    file.print(content.substring(i, endIndex));
    yield(); // Allow system to handle other tasks
  }
  
  file.close();
}

String getHTMLContent(const String& pageName) {
  String fileName = "/html/" + pageName + ".html";
  if (!SPIFFS.exists(fileName)) {
    return ""; // Page not cached
  }
  
  File file = SPIFFS.open(fileName, FILE_READ);
  if (!file) {
    return "";
  }
  
  String content = "";
  while (file.available()) {
    // Read in chunks to avoid memory issues
    content += file.readString();
    yield();
  }
  file.close();
  return content;
}

void serveHTMLFromCache(const String& pageName) {
  String content = getHTMLContent(pageName);
  if (content.length() > 0) {
    streamHTMLResponse(content);
  } else {
    server.send(404, "text/plain", "Page not found in cache");
  }
}

void streamHTMLResponse(const String& content) {
  // Set response headers
  server.setContentLength(content.length());
  server.sendHeader("Content-Type", "text/html");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  
  // Stream content in chunks
  const int chunkSize = 1024;
  int contentLength = content.length();
  
  // Send the first chunk with headers
  server.sendContent("");
  
  for (int i = 0; i < contentLength; i += chunkSize) {
    int endIndex = min(i + chunkSize, contentLength);
    server.sendContent(content.substring(i, endIndex));
    yield(); // Allow system to handle other tasks
  }
}

HTMLBuilder::HTMLBuilder(const String& pageName) : pageName(pageName) {
    if (SPIFFS.exists("/html/" + pageName + ".html")) {
        SPIFFS.remove("/html/" + pageName + ".html");
    }
    currentBuffer.reserve(BUFFER_THRESHOLD);
}

void HTMLBuilder::addChunk(const String& chunk) {
    // Check if adding this chunk would exceed buffer threshold
    if (currentBuffer.length() + chunk.length() >= BUFFER_THRESHOLD) {
        streamAndClear(); // Flush current buffer first
    }
    
    currentBuffer += chunk;
    yield(); // Allow other processes to run
}

void HTMLBuilder::streamAndClear() {
    if (currentBuffer.length() > 0) {
        File file = SPIFFS.open("/html/" + pageName + ".html", FILE_APPEND);
        if (file) {
            size_t written = file.print(currentBuffer);
            file.close();
            
            if (written != currentBuffer.length()) {
                Serial.println("Warning: Not all data written to file");
            }
        } else {
            Serial.println("Failed to open file for writing chunk");
        }
        
        server.sendContent(currentBuffer);
        currentBuffer = "";
        yield();
    }
}

void HTMLBuilder::finalize() {
    streamAndClear();
    server.sendContent(""); // End the chunked response
} 