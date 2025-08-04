#ifndef GPRS_H
#define GPRS_H

#include <Arduino.h>
#include <HardwareSerial.h>

class GPRS {
private:
    HardwareSerial* gprsSerial;
    bool isConnected;
    
    bool waitForResponse(uint32_t timeout = 1000) {
        uint32_t start = millis();
        while (millis() - start < timeout) {
            if (gprsSerial->available()) {
                String response = gprsSerial->readStringUntil('\n');
                Serial.println(response);
                if (response.indexOf("OK") >= 0) return true;
                if (response.indexOf("ERROR") >= 0) return false;
            }
            delay(10);
        }
        return false;
    }

public:
    GPRS(HardwareSerial* serial, int rxPin, int txPin, int baud = 38400) {
        gprsSerial = serial;
        gprsSerial->begin(baud, SERIAL_8N1, rxPin, txPin);
        isConnected = false;
        delay(1000); // Give module time to initialize
    }

    bool begin() {
        gprsSerial->println("AT");
        if (!waitForResponse()) return false;
        
        gprsSerial->println("AT+CMGF=1"); // Set SMS to text mode
        return waitForResponse();
    }

    bool initGPRS(const char* apn) {
        // Disable echo
        gprsSerial->println("ATE0");
        waitForResponse();

        // Check if module is registered to network
        gprsSerial->println("AT+CREG?");
        waitForResponse();

        // Set APN
        gprsSerial->print("AT+CGDCONT=1,\"IP\",\"");
        gprsSerial->print(apn);
        gprsSerial->println("\"");
        if (!waitForResponse()) return false;

        // Attach to GPRS service
        gprsSerial->println("AT+CGATT=1");
        return waitForResponse(10000);
    }

    bool connectTCP(const char* host, int port) {
        // Close any existing connection
        gprsSerial->println("AT+CIPSHUT");
        waitForResponse(5000);

        // Start task and set APN
        gprsSerial->println("AT+CSTT=\"internet\"");
        if (!waitForResponse()) return false;

        // Bring up wireless connection
        gprsSerial->println("AT+CIICR");
        if (!waitForResponse(10000)) return false;

        // Get local IP address
        gprsSerial->println("AT+CIFSR");
        waitForResponse(5000);

        // Start TCP connection
        gprsSerial->print("AT+CIPSTART=\"TCP\",\"");
        gprsSerial->print(host);
        gprsSerial->print("\",\"");
        gprsSerial->print(port);
        gprsSerial->println("\"");
        
        if (waitForResponse(10000)) {
            isConnected = true;
            return true;
        }
        return false;
    }

    bool sendTCPData(const char* data) {
        if (!isConnected) return false;

        // Send data length command
        gprsSerial->print("AT+CIPSEND=");
        gprsSerial->println(strlen(data));
        delay(500);

        // Send the actual data
        gprsSerial->println(data);
        return waitForResponse(10000);
    }

    bool disconnectTCP() {
        gprsSerial->println("AT+CIPCLOSE");
        if (!waitForResponse(5000)) return false;

        gprsSerial->println("AT+CIPSHUT");
        if (waitForResponse(5000)) {
            isConnected = false;
            return true;
        }
        return false;
    }

    bool sendSMS(const char* phoneNumber, const char* message) {
        // Set SMS mode
        gprsSerial->println("AT+CMGF=1");
        if (!waitForResponse()) return false;

        // Set recipient number
        gprsSerial->print("AT+CMGS=\"");
        gprsSerial->print(phoneNumber);
        gprsSerial->println("\"");
        delay(500);

        // Send message content
        gprsSerial->print(message);
        gprsSerial->write(26); // CTRL+Z to send
        
        return waitForResponse(10000); // Longer timeout for SMS sending
    }

    String readResponse() {
        if (gprsSerial->available()) {
            return gprsSerial->readStringUntil('\n');
        }
        return "";
    }

    void update() {
        while (gprsSerial->available()) {
            String response = gprsSerial->readStringUntil('\n');
            if (response.length() > 0) {
                Serial.println(response); // Forward to debug serial
            }
        }
    }
};

#endif // GPRS_H
