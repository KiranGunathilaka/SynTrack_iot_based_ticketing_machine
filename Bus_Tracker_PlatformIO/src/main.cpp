#include <Arduino.h>
#include "Config.h"
#include "GPS.h"
#include "Key_Receiver.h"
#include "Bus_Ticket_Printer.h"
#include "Bus_Data.h"
#include "Bus_Display.h"
#include "Firebase_Connector.h"
#include "TimeManager.h"
#include "BusPreferences.h"
#include <Ticker.h>

// Define hardware serial ports
HardwareSerial gpsSerial(GPS_SERIAL_NUM);

// Create instances
GPS gps(&gpsSerial, GPS_RX_PIN, GPS_TX_PIN);
KeyReceiver keyReceiver;
BusTicketPrinter ticketPrinter;
BusDisplay display;
BusData bus;
FirebaseConnector firebase;
TimeManager timeManager;
BusPreferences preferences;


// Ticker objects for timers
Ticker gpsTicker;
Ticker displayTicker;
Ticker timeManagerTicker;

// Firebase timing using millis() instead of ticker
unsigned long previousFirebaseMillis = 0;

// Forward declarations
void updateGPS();
void updateTime();
void updateDisplay();
void sendToFirebase();

void setup() {
    // Initialize debug serial first at high baud rate
    Serial.begin(115200);
    Serial.println("\n\nESP32 Bus System Starting");

    pinMode(BUZZER_PIN, OUTPUT);
  
    tone(BUZZER_PIN, 2000);
    delay(1000);
    noTone(BUZZER_PIN);
  
    
    // Initialize bus preferences first to load saved values
    Serial.println("Initializing preferences...");
    if (preferences.begin()) {
        Serial.println("Preferences initialized successfully");
        // Load saved bus data
        if (preferences.loadBusData(bus)) {
            Serial.println("Loaded saved bus data from preferences");
        } else {
            Serial.println("No saved data found, using defaults");
            
            // Set default bus data if no saved data is found
            bus.registrationNumber = "ND-2314";
            bus.companyName = "Syntech Transit (Pvt) Ltd";
            bus.routeNumber = "0012";
            
            // Get a new reference number or start with default
            bus.referenceNumber = preferences.loadReferenceNumber();
            if (bus.referenceNumber.isEmpty()) {
                bus.referenceNumber = "10000000001";
                preferences.saveReferenceNumber(bus.referenceNumber);
            }
          
            bus.routeName = "Moratuwa - Nittambuwa";
            bus.fromHalt = "Katubedda";
            bus.toHalt = "Kadawatha";
            bus.isFullTicket = true;
            bus.ticketCount = 2;
            bus.unitPrice = 130.00;
          
            bus.driverPhone = "0703482664";
            bus.hotline = "0332297800";
            
            // Save these default values
            preferences.saveBusData(bus);
        }
    } else {
        Serial.println("Preferences initialization failed, using default values");
        
        // Default bus data when preferences fails
        bus.registrationNumber = "ND-2314";
        bus.companyName = "Syntech Transit (Pvt) Ltd";
        bus.routeNumber = "0012";
        bus.referenceNumber = "10000000001";
        
        bus.routeName = "Moratuwa - Nittambuwa";
        bus.fromHalt = "Katubedda";
        bus.toHalt = "Kadawatha";
        bus.isFullTicket = true;
        
        bus.ticketCount = 2;
        bus.unitPrice = 130.00;
        
        bus.driverPhone = "0703482664";
        bus.hotline = "0332297800";
    }
    
    // Set default date and time values that will be updated by TimeManager
    bus.date = "2025/03/21";
    bus.time = "00:00:00";
    
    Serial.println("Initializing display...");
    // Initialize components - start with display
    display.begin();
    delay(100); // Give the display time to initialize
    
    // Initialize time manager
    Serial.println("Initializing TimeManager...");
    if (timeManager.begin()) {
        Serial.println("TimeManager initialized successfully");
        // Update time immediately after successful initialization
        updateTime();
    } else {
        Serial.println("TimeManager initialization failed, using default time");
    }
    
    // Now initialize other components
    keyReceiver.begin();
    ticketPrinter.begin();
    
    // Initialize GPS serial
    Serial.println("Initializing GPS...");
    gpsSerial.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    
    // Initialize Firebase
    Serial.println("Initializing Firebase...");
    if (firebase.begin()) {
        Serial.println("Firebase connection successful");
    } else {
        Serial.println("Firebase connection failed, will retry later");
    }
    
    // Set up display data
    display.setLocation(bus.fromHalt.c_str());
    
    // Set up tickers
    Serial.println("Setting up tickers...");
    displayTicker.attach_ms(20, updateDisplay);  // 200 Hz refresh rate
    gpsTicker.attach(1.0, updateGPS);           // Update GPS every second
    timeManagerTicker.attach(1.0, updateTime);  // Update time every second
    
    // Initialize the previous millis for Firebase timing
    previousFirebaseMillis = millis();

    // Set ticket printer data using the method with BusData and TimeManager
    ticketPrinter.setTicketDataFromBus(bus, timeManager);

    Serial.println("Bus Ticketing System Ready");
}

void updateGPS() {
    gps.update();  // Parse new GPS data
    bus.latitude = gps.getLatitude();
    bus.longitude = gps.getLongitude();
    bus.speed = gps.getSpeedKmh();
}

void updateTime() {
    // Update time manager (checks if NTP sync is needed)
    timeManager.update();
    
    // Get time from TimeManager
    bus.time = timeManager.getTimeString();
    bus.date = timeManager.getDateString();
    
    // Update the time on the display
    display.setTime(
        timeManager.getHour(),
        timeManager.getMinute(),
        timeManager.getSecond()
    );
    
    // Update date on display only if it has changed
    static int lastDay = -1;
    int currentDay = timeManager.getDay();
    
    if (currentDay != lastDay) {
        display.setDate(
            timeManager.getYear(),
            timeManager.getMonth() - 1, // Display expects 0-based month
            timeManager.getDay(),
            timeManager.getDayOfWeek()
        );
        lastDay = currentDay;
    }
}

void updateDisplay() {
    display.update();
    display.setLocation(bus.getCurrentHaltName());
}

// Function to send data to Firebase
void sendToFirebase() {
    // Only send data if Firebase is initialized correctly
    if (firebase.isConnected()) {
        firebase.sendBusData(
            DEVICE_ID,
            bus.latitude,
            bus.longitude,
            bus.speed,
            bus.time,
            bus.getCurrentHaltName(),
            bus.passengerCount,
            bus.getDestinationHaltName()
        );
    } else {
        // Try to reconnect if not connected
        firebase.begin();
    }
}

void loop() {
    // Handle key inputs with the state machine
    keyReceiver.check_Key_and_Execute();
    
    // Check if it's time to send data to Firebase
    unsigned long currentMillis = millis();
    if (currentMillis - previousFirebaseMillis >= FIREBASE_INTERVAL) {
        previousFirebaseMillis = currentMillis;
        sendToFirebase();
    }
}