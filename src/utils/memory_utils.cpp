#include "memory_utils.h"

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
    }
    
    if (isMemoryLow()) {
      Serial.println("WARNING: Low memory condition detected!");
    }
    
    lastMemCheck = millis();
  }
}

bool isMemoryLow() {
  const int LOW_MEMORY_THRESHOLD = 10000; // 10KB
  const int CRITICAL_MEMORY_THRESHOLD = 5000; // 5KB
  
  int freeHeap = ESP.getFreeHeap();
  if (freeHeap < CRITICAL_MEMORY_THRESHOLD) {
    return true;
  }
  
  // Also check largest free block
  int maxBlock = ESP.getMaxAllocHeap();
  if (maxBlock < CRITICAL_MEMORY_THRESHOLD) {
    return true;
  }
  
  return freeHeap < LOW_MEMORY_THRESHOLD;
} 