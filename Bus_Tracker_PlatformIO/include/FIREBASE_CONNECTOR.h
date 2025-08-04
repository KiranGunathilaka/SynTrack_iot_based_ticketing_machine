#ifndef FIREBASE_CONNECTOR_H
#define FIREBASE_CONNECTOR_H

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "Config.h"
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

class FirebaseConnector
{
public:
    FirebaseConnector()
        : _isConnected(false), _lastAttempt(0), signupOK(false) {}

    // Initialize Wi-Fi and Firebase
    bool begin()
    {
        Serial.println("Connecting to WiFi...");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
            if (millis() - startTime > 20000)
            {
                Serial.println("\nWiFi connection failed!");
                return false;
            }
        }
        Serial.println("\nConnected to WiFi");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());

        // Configure Firebase credentials
        config.api_key = FIREBASE_API_KEY;
        config.database_url = FIREBASE_DATABASE_URL;

        // Sign in with Anonymous user
        if (Firebase.signUp(&config, &auth, "", ""))
        {
            Serial.println("Anonymous auth successful");
            signupOK = true;
        }
        else
        {
            Serial.printf("Firebase auth failed: %s\n", config.signer.signupError.message.c_str());
        }

        config.token_status_callback = tokenStatusCallback;
        // Initialize Firebase with provided credentials
        Firebase.begin(&config, &auth);

        // Enable auto reconnect to WiFi
        Firebase.reconnectWiFi(true);

        // Set database read timeout
        config.timeout.socketConnection = 10 * 1000; // 10 seconds
        config.timeout.serverResponse = 60 * 1000;   // 60 seconds

        _isConnected = true;
        Serial.println("Firebase initialized");
        return true;
    }

    // Push bus-location JSON to /buses/{deviceId}/location
    bool sendBusData(
        const String &deviceId,
        float latitude,
        float longitude,
        float speed,
        const String &time,
        const String &currentHalt,
        int passengerCount,
        const String &destinationHalt)
    {
        if (!_isConnected || WiFi.status() != WL_CONNECTED)
        {
            if (!reconnect())
                return false;
        }

        if (!Firebase.ready())
        {
            Serial.println("Firebase not ready");
            return false;
        }

        // Format latitude and longitude with 6 decimal places
        char latStr[15], lonStr[15], spd[10];
        snprintf(latStr, sizeof(latStr), "%.6f", latitude);
        snprintf(lonStr, sizeof(lonStr), "%.6f", longitude);
        snprintf(spd, sizeof(spd), "%.2f", speed);

        FirebaseJson json;
        json.set("id", deviceId);
        json.set("lat", latStr);
        json.set("lon", lonStr);
        json.set("spd", speed);
        json.set("time", time);
        json.set("halt", currentHalt);
        json.set("passenCount", passengerCount);
        json.set("dest", destinationHalt);

        String path = "buses/" + deviceId + "/location";

        if (Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json))
        {
            Serial.println("Firebase data sent successfully");
            String tsPath = "buses/" + deviceId + "/lastUpdate";
            if (Firebase.RTDB.setString(&fbdo, tsPath.c_str(), time.c_str()))
            {
                Serial.println("Timestamp updated");
            }
            else
            {
                Serial.println("Failed to update timestamp: " + String(fbdo.errorReason().c_str()));
            }
            return true;
        }
        else
        {
            Serial.println("Failed to send data to Firebase: " + String(fbdo.errorReason().c_str()));
            return false;
        }
    }

    // Check Wi-Fi + Firebase status
    bool isConnected() {
        return _isConnected && (WiFi.status() == WL_CONNECTED) && Firebase.ready() && signupOK;
    }

    // Attempt to reconnect (Wi-Fi then Firebase)
    bool reconnect()
    {
        if (millis() - _lastAttempt < 10000)
            return false;
        _lastAttempt = millis();
        Serial.println("Attempting to reconnect...");

        if (WiFi.status() != WL_CONNECTED)
        {
            WiFi.disconnect();
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            unsigned long startTime = millis();
            while (WiFi.status() != WL_CONNECTED)
            {
                delay(500);
                Serial.print(".");
                if (millis() - startTime > 10000)
                {
                    Serial.println("\nWiFi reconnection failed!");
                    return false;
                }
            }
        }

        _isConnected = Firebase.ready();
        Serial.println(_isConnected ? "Reconnected successfully"
                                    : "Firebase reconnection failed");
        return _isConnected;
    }

private:
    FirebaseData fbdo;
    FirebaseAuth auth;
    FirebaseConfig config;
    bool _isConnected;
    unsigned long _lastAttempt;
    bool signupOK;
};

#endif // FIREBASE_CONNECTOR_H