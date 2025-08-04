#ifndef BUS_PREFERENCES_H
#define BUS_PREFERENCES_H

#include <Arduino.h>
#include <Preferences.h>
#include "Bus_Data.h"

class BusPreferences {
public:
    BusPreferences() : isInitialized(false) {}
    
    bool begin() {
        if (prefs.begin("busSettings", false)) {
            isInitialized = true;
            return true;
        }
        return false;
    }
    
    void end() {
        if (isInitialized) {
            prefs.end();
            isInitialized = false;
        }
    }
    
    // Save bus data to preferences
    bool saveBusData(const BusData& busData) {
        if (!isInitialized && !begin()) {
            return false;
        }
        
        // Save identification data
        prefs.putString("regNumber", busData.registrationNumber);
        prefs.putString("companyName", busData.companyName);
        prefs.putString("routeNumber", busData.routeNumber);
        prefs.putString("refNumber", busData.referenceNumber);
        
        // Save route info
        prefs.putString("routeName", busData.routeName);
        
        // Save pricing info
        prefs.putFloat("unitPrice", busData.unitPrice);
        
        // Save contact info
        prefs.putString("driverPhone", busData.driverPhone);
        prefs.putString("hotline", busData.hotline);
        
        return true;
    }
    
    // Load bus data from preferences
    bool loadBusData(BusData& busData) {
        if (!isInitialized && !begin()) {
            return false;
        }
        
        // Load identification data
        busData.registrationNumber = prefs.getString("regNumber", "");
        busData.companyName = prefs.getString("companyName", "");
        busData.routeNumber = prefs.getString("routeNumber", "");
        busData.referenceNumber = prefs.getString("refNumber", "");
        
        // Load route info
        busData.routeName = prefs.getString("routeName", "");
        
        // Load pricing info
        busData.unitPrice = prefs.getFloat("unitPrice", 0.0f);
        
        // Load contact info
        busData.driverPhone = prefs.getString("driverPhone", "");
        busData.hotline = prefs.getString("hotline", "");
        
        return true;
    }
    
    // Save reference number separately (commonly updated)
    bool saveReferenceNumber(const String& refNumber) {
        if (!isInitialized && !begin()) {
            return false;
        }
        
        prefs.putString("refNumber", refNumber);
        return true;
    }
    
    // Load reference number separately
    String loadReferenceNumber() {
        if (!isInitialized && !begin()) {
            return "";
        }
        
        return prefs.getString("refNumber", "00000");
    }
    
    // Increment and save reference number
    String incrementReferenceNumber() {
        String currentRef = loadReferenceNumber();
        int refNum = currentRef.toInt();
        refNum++;
        
        // Format with leading zeros (5 digits)
        char newRef[6];
        sprintf(newRef, "%05d", refNum);
        String newRefStr = String(newRef);
        
        saveReferenceNumber(newRefStr);
        return newRefStr;
    }
    
    // Generic save methods for different data types
    bool saveString(const char* key, const String& value) {
        if (!isInitialized && !begin()) {
            return false;
        }
        
        prefs.putString(key, value);
        return true;
    }
    
    String loadString(const char* key, const String& defaultValue = "") {
        if (!isInitialized && !begin()) {
            return defaultValue;
        }
        
        return prefs.getString(key, defaultValue);
    }
    
    bool saveInt(const char* key, int value) {
        if (!isInitialized && !begin()) {
            return false;
        }
        
        prefs.putInt(key, value);
        return true;
    }
    
    int loadInt(const char* key, int defaultValue = 0) {
        if (!isInitialized && !begin()) {
            return defaultValue;
        }
        
        return prefs.getInt(key, defaultValue);
    }
    
    bool saveFloat(const char* key, float value) {
        if (!isInitialized && !begin()) {
            return false;
        }
        
        prefs.putFloat(key, value);
        return true;
    }
    
    float loadFloat(const char* key, float defaultValue = 0.0f) {
        if (!isInitialized && !begin()) {
            return defaultValue;
        }
        
        return prefs.getFloat(key, defaultValue);
    }
    
    bool saveBool(const char* key, bool value) {
        if (!isInitialized && !begin()) {
            return false;
        }
        
        prefs.putBool(key, value);
        return true;
    }
    
    bool loadBool(const char* key, bool defaultValue = false) {
        if (!isInitialized && !begin()) {
            return defaultValue;
        }
        
        return prefs.getBool(key, defaultValue);
    }
    
    // Clear all saved preferences
    bool clearAll() {
        if (!isInitialized && !begin()) {
            return false;
        }
        
        return prefs.clear();
    }
    
    // Check if a key exists
    bool hasKey(const char* key) {
        if (!isInitialized && !begin()) {
            return false;
        }
        
        return prefs.isKey(key);
    }
    
    // Remove a specific key
    bool removeKey(const char* key) {
        if (!isInitialized && !begin()) {
            return false;
        }
        
        return prefs.remove(key);
    }

private:
    Preferences prefs;
    bool isInitialized;
};

#endif // BUS_PREFERENCES_H