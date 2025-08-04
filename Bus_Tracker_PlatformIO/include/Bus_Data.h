#ifndef BUS_DATA_H
#define BUS_DATA_H

#include <Arduino.h>

class BusData;

extern BusData bus;

class BusData {
public:
    // Identification
    String registrationNumber;
    String companyName;
    String routeNumber;
    String referenceNumber;

    // Travel Info
    String routeName = "255";
    String fromHalt = "PILIYANDALA";
    String toHalt = "GALKISSA";
    bool isFullTicket = true;

    // Date & Time
    String date;
    String time;

    // Ticket Info
    uint8_t ticketCount = 1;
    float unitPrice = 100.0f;

    // Contact Info
    String driverPhone;
    String hotline;

    // GPS Location
    float latitude = 0.0f;
    float longitude = 0.0f;
    float speed = 0.0f;

    
    // Bus halts along Galkissa – Piliyandala route
    enum Halt {
        GALKISSA,
        RATMALANA,
        ANGULANA,
        MOUNT_LAVINIA,
        DEHIWALA,
        KOHUWALA,
        NUGEGODA,
        WIJERAMA,
        NAVINNA,
        MAHARAGAMA,
        BORALASGAMUWA,
        POLHENA,
        KAHATHUDUWA,
        PANDURA,
        PILIYANDALA,
        NUM_HALTS
    };

    // Passenger tracking data
    struct HaltStats {
        uint16_t ticketsIssued = 0;
        uint16_t expectedPassengers = 0;
        uint16_t alightingPassengers = 0;
    };

    HaltStats haltStats[NUM_HALTS];
    uint8_t destinationHaltIndex = 0;
    uint8_t passengerCount = 0;


    static const char* haltNames[NUM_HALTS];

    // Track current halt index
    uint8_t currentHaltIndex = 0;

    // Functions to access and manipulate current halt
    const char* getCurrentHaltName() const {
        return haltNames[currentHaltIndex];
    }

    void goToNextHalt() {
        if (currentHaltIndex < NUM_HALTS - 1) {
            currentHaltIndex++;
            //Serial.println(getCurrentHaltName());
        }
    }

    void goToPreviousHalt() {
        if (currentHaltIndex > 0) {
            currentHaltIndex--;
            //Serial.println(getCurrentHaltName());
        }
    }

    void updateHaltStats() {
        // Reset current halt stats
        haltStats[currentHaltIndex].ticketsIssued = 0;
        haltStats[currentHaltIndex].expectedPassengers = 0;
        haltStats[currentHaltIndex].alightingPassengers = 0;

        // Update expected passengers for upcoming halts
        for (uint8_t i = currentHaltIndex + 1; i < NUM_HALTS; i++) {
            haltStats[i].expectedPassengers += passengerCount;
        }

        // Update alighting passengers for destination
        if (destinationHaltIndex > currentHaltIndex) {
            haltStats[destinationHaltIndex].alightingPassengers += passengerCount;
        }
    }

    void issueTicket(uint8_t count) {
        passengerCount = count;
        haltStats[currentHaltIndex].ticketsIssued += count;
        updateHaltStats();
    }

    void setDestinationHalt(uint8_t haltIndex) {
        if (haltIndex > currentHaltIndex && haltIndex < NUM_HALTS) {
            destinationHaltIndex = haltIndex;
            updateHaltStats();
        }
    }

    const char* getDestinationHaltName() const {
        return haltNames[destinationHaltIndex];
    }

    void printHaltStats() {
        Serial.println("\nCurrent Halt Statistics:");
        Serial.printf("Current Halt: %s\n", getCurrentHaltName());
        Serial.printf("Destination: %s\n", getDestinationHaltName());
        Serial.printf("Tickets Issued: %d\n", haltStats[currentHaltIndex].ticketsIssued);
        Serial.printf("Expected Passengers: %d\n", haltStats[currentHaltIndex].expectedPassengers);
        Serial.printf("Alighting Passengers: %d\n", haltStats[currentHaltIndex].alightingPassengers);
    }

    BusData() = default;
};

// Define halt names
const char* BusData::haltNames[BusData::NUM_HALTS] = {
    "Galkissa",
    "Ratmalana",
    "Angulana",
    "Mount Lavinia",
    "Dehiwala",
    "Kohuwala",
    "Nugegoda",
    "Wijerama",
    "Navinna",
    "Maharagama",
    "Boralesgamuwa",
    "Polhena",
    "Kahathuduwa",
    "Pandura",
    "Piliyandala"
};

#endif // BUS_DATA_H
