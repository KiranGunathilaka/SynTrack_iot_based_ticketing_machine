#ifndef CONFIG_H
#define CONFIG_H

// Device Identification
#define DEVICE_ID "bus10001"

// Serial Port Configurations
#define GPS_SERIAL_NUM 2    // UART2
#define PRINTER_SERIAL_NUM 1   // UART1
#define DEBUG_BAUD_RATE 115200
#define GPS_BAUD_RATE 9600
#define GPRS_BAUD_RATE 38400

// GPS Pin Configuration
#define GPS_RX_PIN 16  // Using numeric value instead of GPIO_NUM_16
#define GPS_TX_PIN 17  // Using numeric value instead of GPIO_NUM_17

// GPRS Pin Configuration
#define PRINTER_RX_PIN 5  // Using numeric value instead of GPIO_NUM_5
#define PRINTER_TX_PIN 4  // Using numeric value instead of GPIO_NUM_4

// GPRS Network Configuration
#define GPRS_APN "internet"
#define DEFAULT_SMS_NUMBER "+94712345678"

// TCP Server Configuration
#define TCP_SERVER "your-server.com"
#define TCP_PORT 8080

// Timing Configuration
#define TCP_UPDATE_INTERVAL 60000    // 1 minute in milliseconds
#define GPS_UPDATE_INTERVAL 1000     // 1 second in milliseconds

// Buffer Sizes
#define TCP_MESSAGE_BUFFER_SIZE 256  // Use the larger buffer size

// Screen settings
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// WiFi settings
#define WIFI_SSID "Chiki Chiki Bamba"
#define WIFI_PASSWORD "DiscreteFourierTransform"

// Firebase settings
#define FIREBASE_API_KEY "AIzaSyAq2WTJzbLY3nO_cjqc54rK3BNsouW8Ypk"
#define FIREBASE_DATABASE_URL "https://syntrack-52802-default-rtdb.firebaseio.com"

const unsigned long FIREBASE_INTERVAL = 10000; // milliseconds

const int BUZZER_PIN = 2;

#endif // CONFIG_H

