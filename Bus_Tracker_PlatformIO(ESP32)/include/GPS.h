#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#include <HardwareSerial.h>

class GPS {
private:
    HardwareSerial* gpsSerial;
    static const uint16_t RX_BUFFER_SIZE = 128;
    uint8_t rxBuffer[RX_BUFFER_SIZE];
    uint8_t rxIndex;
    
    float nmeaLong;
    float nmeaLat;
    float utcTime;
    char northsouth;
    char eastwest;
    char posStatus;
    float decimalLong;
    float decimalLat;
    float speed;

    float nmeaToDecimal(float coordinate) {
        int degree = (int)(coordinate/100);
        float minutes = coordinate - degree * 100;
        float decimalDegree = minutes / 60;
        return degree + decimalDegree;
    }

    void gpsParse(char *strParse) {
        if(!strncmp(strParse, "$GPGGA", 6)) {
            sscanf(strParse, "$GPGGA,%f,%f,%c,%f,%c",
                &utcTime, &nmeaLat, &northsouth, &nmeaLong, &eastwest);
            decimalLat = nmeaToDecimal(nmeaLat);
            decimalLong = nmeaToDecimal(nmeaLong);
        }
        else if (!strncmp(strParse, "$GPGLL", 6)) {
            sscanf(strParse, "$GPGLL,%f,%c,%f,%c,%f",
                &nmeaLat, &northsouth, &nmeaLong, &eastwest, &utcTime);
            decimalLat = nmeaToDecimal(nmeaLat);
            decimalLong = nmeaToDecimal(nmeaLong);
        }
        else if (!strncmp(strParse, "$GPRMC", 6)) {
            // Modified to extract speed information
            // GPRMC format: $GPRMC,time,status,lat,N/S,long,E/W,speed,course,date,mag_var,E/W*checksum
            sscanf(strParse, "$GPRMC,%f,%c,%f,%c,%f,%c,%f",
                &utcTime, &posStatus, &nmeaLat, &northsouth, &nmeaLong, &eastwest, &speed);
            decimalLat = nmeaToDecimal(nmeaLat);
            decimalLong = nmeaToDecimal(nmeaLong);
        }
    }

    bool gpsValidate(char *nmea) {
        char check[3];
        char calculatedString[3];
        int index = 0;
        int calculatedCheck = 0;
        
        if(nmea[index] == '$')
            index++;
        else
            return false;
        
        while((nmea[index] != 0) && (nmea[index] != '*') && (index < 75)) {
            calculatedCheck ^= nmea[index];
            index++;
        }
        
        if(index >= 75) {
            return false;
        }
        
        if (nmea[index] == '*') {
            check[0] = nmea[index+1];
            check[1] = nmea[index+2];
            check[2] = 0;
        }
        else
            return false;
        
        sprintf(calculatedString,"%02X",calculatedCheck);
        return((calculatedString[0] == check[0]) && (calculatedString[1] == check[1]));
    }

public:
    GPS(HardwareSerial* serial, int rxPin, int txPin, int baud = 9600) {
        gpsSerial = serial;
        rxIndex = 0;
        speed = 0.0; 
        memset(rxBuffer, 0, RX_BUFFER_SIZE);
        gpsSerial->begin(baud, SERIAL_8N1, rxPin, txPin);
    }

    void update() {
        while (gpsSerial->available()) {
            char c = gpsSerial->read();
            if (c != '\n' && rxIndex < RX_BUFFER_SIZE - 1) {
                rxBuffer[rxIndex++] = c;
            } else {
                rxBuffer[rxIndex] = '\0';  // Null terminate the string
                if (gpsValidate((char*)rxBuffer)) {
                    gpsParse((char*)rxBuffer);
                }
                rxIndex = 0;
                memset(rxBuffer, 0, RX_BUFFER_SIZE);
            }
        }
    }

    float getLatitude() const { return decimalLat; }
    float getLongitude() const { return decimalLong; }
    float getUTCTime() const { return utcTime; }
    char getNorthSouth() const { return northsouth; }
    char getEastWest() const { return eastwest; }
    char getPosStatus() const { return posStatus; }
    float getSpeedKmh() const { return speed; }
};

#endif // GPS_H