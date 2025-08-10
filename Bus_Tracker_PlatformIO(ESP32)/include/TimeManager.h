// TimeManager.h
#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <time.h>
#include <WiFi.h>

class TimeManager {
private:
    bool ntpSynced;
    unsigned long lastSyncMillis;
    const unsigned long syncInterval = 86400000;  // Sync with NTP once daily (in milliseconds)
    
    // NTP server settings
    const char* ntpServer1 = "pool.ntp.org";
    const char* ntpServer2 = "time.nist.gov";
    const long gmtOffset_sec = 19800;  // GMT+5:30 for Sri Lanka (5.5*3600)
    const int daylightOffset_sec = 0;  // No DST in Sri Lanka
    
public:
    TimeManager();
    bool begin();
    bool syncWithNTP();
    void update();
    
    // Getters for different time formats - added const to make them callable from const objects
    String getTimeString() const;      // Returns HH:MM:SS
    String getDateString() const;      // Returns YYYY/MM/DD
    int getHour() const;
    int getMinute() const;
    int getSecond() const;
    int getYear() const;
    int getMonth() const;
    int getDay() const;
    int getDayOfWeek() const;
    
    // Utility function to get time for other components
    struct tm getTimeInfo() const;
};

// TimeManager.cpp implementation
TimeManager::TimeManager() : ntpSynced(false), lastSyncMillis(0) {
}

bool TimeManager::begin() {
    // Configure NTP time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
    
    // Try to sync with NTP immediately
    return syncWithNTP();
}

bool TimeManager::syncWithNTP() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Cannot sync with NTP, WiFi not connected");
        return false;
    }
    
    Serial.println("Syncing time with NTP...");
    
    // Get time from NTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
    
    // Wait for time to be set
    int retry = 0;
    const int maxRetries = 10;
    struct tm timeInfo;
    while (!getLocalTime(&timeInfo) && retry < maxRetries) {
        Serial.println("Failed to obtain time from NTP, retrying...");
        delay(500);
        retry++;
    }
    
    if (retry >= maxRetries) {
        Serial.println("Failed to obtain time from NTP after multiple attempts");
        return false;
    }
    
    ntpSynced = true;
    lastSyncMillis = millis();
    Serial.println("NTP sync successful");
    return true;
}

void TimeManager::update() {
    // Check if it's time to sync with NTP again
    if (millis() - lastSyncMillis >= syncInterval) {
        syncWithNTP();
    }
}

String TimeManager::getTimeString() const {
    char buffer[9];  // HH:MM:SS\0
    struct tm timeinfo;
    
    if (getLocalTime(&timeinfo)) {
        sprintf(buffer, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        return String(buffer);
    }
    
    return "00:00:00";  // Default if nothing works
}

String TimeManager::getDateString() const {
    char buffer[11];  // YYYY/MM/DD\0
    struct tm timeinfo;
    
    if (getLocalTime(&timeinfo)) {
        sprintf(buffer, "%04d/%02d/%02d", 
                timeinfo.tm_year + 1900, 
                timeinfo.tm_mon + 1, 
                timeinfo.tm_mday);
        return String(buffer);
    }
    
    return "2025/01/01";  // Default if nothing works
}

int TimeManager::getHour() const {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        return timeinfo.tm_hour;
    }
    return 0;
}

int TimeManager::getMinute() const {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        return timeinfo.tm_min;
    }
    return 0;
}

int TimeManager::getSecond() const {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        return timeinfo.tm_sec;
    }
    return 0;
}

int TimeManager::getYear() const {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        return timeinfo.tm_year + 1900;
    }
    return 2025;
}

int TimeManager::getMonth() const {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        return timeinfo.tm_mon + 1;  // tm_mon is 0-based, we return 1-based
    }
    return 1;
}

int TimeManager::getDay() const {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        return timeinfo.tm_mday;
    }
    return 1;
}

int TimeManager::getDayOfWeek() const {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        return timeinfo.tm_wday;  // 0 = Sunday, 6 = Saturday
    }
    return 0;
}

struct tm TimeManager::getTimeInfo() const {
    struct tm timeinfo;
    
    if (!getLocalTime(&timeinfo)) {
        // If cannot get time, set to epoch start as fallback
        memset(&timeinfo, 0, sizeof(struct tm));
    }
    
    return timeinfo;
}
#endif // TIME_MANAGER_H