#ifndef MEMORY_UTILS_H
#define MEMORY_UTILS_H

#include "../config/config.h"
#include <SPIFFS.h>

// Function declarations for memory management
bool initSPIFFS();
void storeToSPIFFS(const String& key, const String& value);
String loadFromSPIFFS(const String& key);
void clearSPIFFSCache();
int getFreeMemory();
void monitorMemory();
bool isMemoryLow();

// HTML content management
void cacheHTMLContent(const String& pageName, const String& content);
String getHTMLContent(const String& pageName);
void serveHTMLFromCache(const String& pageName);
void streamHTMLResponse(const String& content);

// Large page handling
class HTMLBuilder {
public:
    HTMLBuilder(const String& pageName);
    void addChunk(const String& chunk);
    void streamAndClear();
    void finalize();
private:
    String pageName;
    String currentBuffer;
    const size_t BUFFER_THRESHOLD = 8192; // 8KB buffer threshold
};

#endif // MEMORY_UTILS_H 