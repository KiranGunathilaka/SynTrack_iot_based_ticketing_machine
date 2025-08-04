#ifndef KEY_RECEIVER_H
#define KEY_RECEIVER_H

#include <Arduino.h>
#include "Bus_Data.h"
#include "Bus_Ticket_Printer.h"
#include "BUS_DISPLAY.h"
#include "TimeManager.h"

// External objects defined in main.cpp
extern BusData bus;
extern BusTicketPrinter ticketPrinter;
extern TimeManager timeManager;
extern BusDisplay display;

// 5-bit keypad codes
enum Key
{
    NONE,
    KEY_ESC,  // Escape / cancel
    KEY_F1,   // Function 1
    KEY_CHG,  // Change
    KEY_STBY, // Standby
    KEY_ON,   // On
    KEY_MENU, // Go to menu
    KEY_F2,   // Function 2 / show stats
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_PRT, // Print ticket
    KEY_F3,  // Function 3
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_F4, // Function 4
    KEY_UP, // Next halt
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_LEFT,  // Navigate left
    KEY_DOWN,  // Previous halt
    KEY_RIGHT, // Navigate right
    KEY_0,
    KEY_ENTER // Confirm / Enter
};

// Define all possible states for the system
enum class SystemState
{
    IDLE,               // Default state showing route info
    MENU,               // Menu options display
    TICKET_SELECTION,   // Selecting boarding and destination halts
    PASSENGER_COUNT,    // Enter passenger count
    TICKET_PREVIEW,     // Preview the ticket before printing
    PRINT_CONFIRMATION, // Confirm the ticket is printed
    SHOW_STATISTICS     // Display statistics
};

class KeyReceiver
{
public:
    static const int NUM_DATA_PINS = 5;
    const int DATA_PINS[NUM_DATA_PINS] = {21, 34, 35, 33, 32};
    const int INTERRUPT_PIN = 19;

    volatile bool dataReady = false;
    SystemState currentState = SystemState::IDLE;

    // Variables to track menu selection
    int menuSelection = 0;
    int maxMenuItems = 4; // Number of menu items

    // Variables for ticket selection
    uint8_t selectedBoardingHalt = 0;
    uint8_t selectedDestinationHalt = 0;

    // Passenger count
    uint8_t passengerCount = 1; // Default passenger count

    // Input buffer for numeric input
    String numericInput = "";

    // Current selection focus (for multi-option screens)
    enum FocusField
    {
        BOARDING_HALT,
        DESTINATION_HALT,
        PASSENGER_COUNT
    };

    FocusField currentFocus = BOARDING_HALT;

    KeyReceiver()
    {
        for (int i = 0; i < NUM_DATA_PINS; i++)
        {
            pinMode(DATA_PINS[i], INPUT_PULLDOWN);
        }
        pinMode(INTERRUPT_PIN, INPUT_PULLDOWN);
        instance = this;
    }

    void begin()
    {
        // Install ISR service once
        if (gpio_install_isr_service(ESP_INTR_FLAG_LOWMED) != ESP_OK)
        {
            Serial.println("GPIO ISR service already installed or failed.");
        }
        attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN),
                        isrWrapper, RISING);

        // Initialize state
        transitionToState(SystemState::IDLE);
    }

    void check_Key_and_Execute()
    {
        if (!dataReady)
            return;
        dataReady = false;

        tone(BUZZER_PIN, 1000);
        delay(200);
        noTone(BUZZER_PIN);

        int code = 0;
        for (int i = 0; i < NUM_DATA_PINS; i++)
        {
            code |= (digitalRead(DATA_PINS[i]) << i);
        }

        // Print the enum name for debugging
        Serial.print("Received Key: ");
        Serial.println(getKeyName(code));

        // Handle keys based on current state
        switch (currentState)
        {
        case SystemState::IDLE:
            handleIdleState(code);
            break;

        case SystemState::MENU:
            handleMenuState(code);
            break;

        case SystemState::TICKET_SELECTION:
            handleTicketSelectionState(code);
            break;

        case SystemState::PASSENGER_COUNT:
            handlePassengerCountState(code);
            break;

        case SystemState::TICKET_PREVIEW:
            handleTicketPreviewState(code);
            break;

        case SystemState::PRINT_CONFIRMATION:
            handlePrintConfirmationState(code);
            break;

        case SystemState::SHOW_STATISTICS:
            handleStatisticsState(code);
            break;
        }
    }

    const char *getKeyName(int code)
    {
        switch (code)
        {
        case KEY_ESC:
            return "KEY_ESC";
        case KEY_F1:
            return "KEY_F1";
        case KEY_CHG:
            return "KEY_CHG";
        case KEY_STBY:
            return "KEY_STBY";
        case KEY_ON:
            return "KEY_ON";
        case KEY_MENU:
            return "KEY_MENU";
        case KEY_F2:
            return "KEY_F2";
        case KEY_1:
            return "KEY_1";
        case KEY_2:
            return "KEY_2";
        case KEY_3:
            return "KEY_3";
        case KEY_PRT:
            return "KEY_PRT";
        case KEY_F3:
            return "KEY_F3";
        case KEY_4:
            return "KEY_4";
        case KEY_5:
            return "KEY_5";
        case KEY_6:
            return "KEY_6";
        case KEY_F4:
            return "KEY_F4";
        case KEY_UP:
            return "KEY_UP";
        case KEY_7:
            return "KEY_7";
        case KEY_8:
            return "KEY_8";
        case KEY_9:
            return "KEY_9";
        case KEY_LEFT:
            return "KEY_LEFT";
        case KEY_DOWN:
            return "KEY_DOWN";
        case KEY_RIGHT:
            return "KEY_RIGHT";
        case KEY_0:
            return "KEY_0";
        case KEY_ENTER:
            return "KEY_ENTER";
        default:
            return "NONE";
        }
    }

private:
    static KeyReceiver *instance;

    // ISR wrapper now in flash (no IRAM_ATTR) to avoid relocation errors
    static void isrWrapper()
    {
        if (instance)
            instance->onInterrupt();
    }

    void onInterrupt()
    {
        dataReady = true;
    }

    // State transition function
    void transitionToState(SystemState newState)
    {
        // Exit actions for current state
        switch (currentState)
        {
        case SystemState::IDLE:
            break;
        case SystemState::MENU:
            // Reset menu selection when leaving menu
            menuSelection = 0;
            break;
        case SystemState::TICKET_SELECTION:
            break;
        case SystemState::PASSENGER_COUNT:
            // Clear numeric input when leaving
            numericInput = "";
            break;
        case SystemState::TICKET_PREVIEW:
            break;
        case SystemState::PRINT_CONFIRMATION:
            break;
        case SystemState::SHOW_STATISTICS:
            break;
        }

        // Update state
        currentState = newState;

        // Entry actions for new state
        switch (newState)
        {
        case SystemState::IDLE:
            createIdleUI();
            break;
        case SystemState::MENU:
            menuSelection = 0; // Reset selection
            create_Menu_UI();
            break;
        case SystemState::TICKET_SELECTION:
            // Initialize with current halt and next halt
            selectedBoardingHalt = bus.currentHaltIndex;
            selectedDestinationHalt = min(bus.currentHaltIndex + 1, BusData::NUM_HALTS - 1);
            currentFocus = BOARDING_HALT;
            createTicketSelectionUI();
            break;
        case SystemState::PASSENGER_COUNT:
            numericInput = "1"; // Default to 1 passenger
            passengerCount = 1;
            createPassengerCountUI();
            break;
        case SystemState::TICKET_PREVIEW:
            createTicketPreviewUI();
            break;
        case SystemState::PRINT_CONFIRMATION:
            createPrintConfirmationUI();
            break;
        case SystemState::SHOW_STATISTICS:
            createStatisticsUI();
            break;
        }

        Serial.print("Transitioned to state: ");
        Serial.println(getStateName(newState));
    }

    // Handler for IDLE state
    void handleIdleState(int code)
    {
        switch (code)
        {
        case KEY_UP:
            bus.goToNextHalt();
            updateIdleUI();
            Serial.println("Next Halt: " + String(bus.getCurrentHaltName()));
            break;

        case KEY_DOWN:
            bus.goToPreviousHalt();
            updateIdleUI();
            Serial.println("Previous Halt: " + String(bus.getCurrentHaltName()));
            break;

        case KEY_MENU:
            transitionToState(SystemState::MENU);
            break;

        case KEY_F2:
            transitionToState(SystemState::SHOW_STATISTICS);
            break;

        case KEY_PRT:
            // Go directly to ticket selection
            // transitionToState(SystemState::TICKET_SELECTION);
            printTicket();
            break;
        }
    }

    // Handler for MENU state
    void handleMenuState(int code)
    {
        switch (code)
        {
        case KEY_UP:
            menuSelection = (menuSelection > 0) ? menuSelection - 1 : 0;
            updateMenuUI();
            break;

        case KEY_DOWN:
            menuSelection = (menuSelection < maxMenuItems - 1) ? menuSelection + 1 : maxMenuItems - 1;
            updateMenuUI();
            break;

        case KEY_ENTER:
            handleMenuSelection(menuSelection);
            break;

        case KEY_ESC:
            transitionToState(SystemState::IDLE);
            break;

        // Direct menu selection with number keys
        case KEY_1:
        case KEY_2:
        case KEY_3:
        case KEY_4:
            if (code - KEY_1 < maxMenuItems)
            {
                menuSelection = code - KEY_1;
                handleMenuSelection(menuSelection);
            }
            break;
        }
    }

    // Process menu selection and transition to appropriate state
    void handleMenuSelection(int selection)
    {
        switch (selection)
        {
        case 0: // Issue Ticket
            transitionToState(SystemState::TICKET_SELECTION);
            break;
        case 1: // Show Statistics
            transitionToState(SystemState::SHOW_STATISTICS);
            break;
        case 2: // Settings or other option
            // Placeholder for future feature
            Serial.println("Settings selected - Not implemented");
            break;
        case 3: // Return to main screen
            transitionToState(SystemState::IDLE);
            break;
        }
    }

    // Handler for TICKET_SELECTION state
    void handleTicketSelectionState(int code)
    {
        switch (code)
        {
        case KEY_ESC:
            transitionToState(SystemState::MENU);
            break;

        case KEY_RIGHT:
            // Toggle focus between boarding and destination
            if (currentFocus == BOARDING_HALT)
            {
                currentFocus = DESTINATION_HALT;
            }
            else
            {
                currentFocus = BOARDING_HALT;
            }
            updateTicketSelectionUI();
            break;

        case KEY_UP:
            if (currentFocus == BOARDING_HALT)
            {
                if (selectedBoardingHalt > 0)
                {
                    selectedBoardingHalt--;
                }
            }
            else
            { // DESTINATION_HALT
                if (selectedDestinationHalt < BusData::NUM_HALTS - 1)
                {
                    selectedDestinationHalt++;
                }
            }
            updateTicketSelectionUI();
            break;

        case KEY_DOWN:
            if (currentFocus == BOARDING_HALT)
            {
                if (selectedBoardingHalt < selectedDestinationHalt - 1)
                {
                    selectedBoardingHalt++;
                }
            }
            else
            { // DESTINATION_HALT
                if (selectedDestinationHalt > selectedBoardingHalt + 1)
                {
                    selectedDestinationHalt--;
                }
            }
            updateTicketSelectionUI();
            break;

        case KEY_ENTER:
            // Make sure destination is after boarding
            if (selectedDestinationHalt > selectedBoardingHalt)
            {
                // Update the bus data
                bus.currentHaltIndex = selectedBoardingHalt;
                bus.setDestinationHalt(selectedDestinationHalt);
                transitionToState(SystemState::PASSENGER_COUNT);
            }
            else
            {
                Serial.println("Invalid selection: Destination must be after boarding halt");
                // Flash error on display
            }
            break;
        }
    }

    // Handler for PASSENGER_COUNT state
    void handlePassengerCountState(int code)
    {
        switch (code)
        {
        case KEY_ESC:
            transitionToState(SystemState::TICKET_SELECTION);
            break;

        case KEY_0:
        case KEY_1:
        case KEY_2:
        case KEY_3:
        case KEY_4:
        case KEY_5:
        case KEY_6:
        case KEY_7:
        case KEY_8:
        case KEY_9:
            // Add digit to input (limit to reasonable number)
            if (numericInput.length() < 2)
            {
                numericInput += (code - KEY_0);
                passengerCount = numericInput.toInt();
                if (passengerCount < 1)
                    passengerCount = 1;
                if (passengerCount > 99)
                    passengerCount = 99;
                updatePassengerCountUI();
            }
            break;

        case KEY_UP:
            if (passengerCount < 99)
            {
                passengerCount++;
                numericInput = String(passengerCount);
                updatePassengerCountUI();
            }
            break;

        case KEY_DOWN:
            if (passengerCount > 1)
            {
                passengerCount--;
                numericInput = String(passengerCount);
                updatePassengerCountUI();
            }
            break;

        case KEY_ENTER:
            // Update bus data and move to preview
            bus.ticketCount = passengerCount;
            transitionToState(SystemState::TICKET_PREVIEW);
            break;
        }
    }

    // Handler for TICKET_PREVIEW state
    void handleTicketPreviewState(int code)
    {
        switch (code)
        {
        case KEY_ESC:
            transitionToState(SystemState::PASSENGER_COUNT);
            break;

        case KEY_PRT:
            // Print the ticket
            printTicket();
            transitionToState(SystemState::PRINT_CONFIRMATION);
            break;

        case KEY_ENTER:
            // Print the ticket
            printTicket();
            transitionToState(SystemState::PRINT_CONFIRMATION);
            break;
        }
    }

    // Handler for PRINT_CONFIRMATION state
    void handlePrintConfirmationState(int code)
    {
        switch (code)
        {
        case KEY_ESC:
        case KEY_ENTER:
            // Return to idle state after printing
            transitionToState(SystemState::IDLE);
            break;
        }
    }

    // Handler for SHOW_STATISTICS state
    void handleStatisticsState(int code)
    {
        switch (code)
        {
        case KEY_ESC:
        case KEY_ENTER:
            transitionToState(SystemState::IDLE);
            break;

        case KEY_PRT:
            // Print statistics report
            ticketPrinter.printHaltStats(bus, timeManager);
            Serial.println("Statistics report printed");
            break;
        }
    }

    // Print ticket with updated data
    void printTicket()
    {
        // Update the ticket data
        bus.issueTicket(passengerCount);

        // Set up printer with latest data
        ticketPrinter.setTicketDataFromBus(bus, timeManager);

        // Print the ticket
        ticketPrinter.printTicket();

        Serial.println("Ticket printed");
    }

    // Helper function to get state name for logging
    const char *getStateName(SystemState state)
    {
        switch (state)
        {
        case SystemState::IDLE:
            return "IDLE";
        case SystemState::MENU:
            return "MENU";
        case SystemState::TICKET_SELECTION:
            return "TICKET_SELECTION";
        case SystemState::PASSENGER_COUNT:
            return "PASSENGER_COUNT";
        case SystemState::TICKET_PREVIEW:
            return "TICKET_PREVIEW";
        case SystemState::PRINT_CONFIRMATION:
            return "PRINT_CONFIRMATION";
        case SystemState::SHOW_STATISTICS:
            return "SHOW_STATISTICS";
        default:
            return "UNKNOWN";
        }
    }

    // UI creation functions (placeholders as requested)
    void createIdleUI()
    {
        Serial.println("Creating idle UI display");
        // Will be replaced with actual UI creation cod
    }

    void updateIdleUI()
    {
        Serial.println("Updating idle UI with current halt: " + String(bus.getCurrentHaltName()));
        display.setLocation(bus.getCurrentHaltName());
    }

    void create_Menu_UI()
    {
        Serial.println("Creating menu UI display");
        // Create the menu UI
        display.createMenuUI();
    }

    void updateMenuUI()
    {
        Serial.println("Updating menu UI with selection: " + String(menuSelection));
        // Will be replaced with actual UI update code
    }

    void createTicketSelectionUI()
    {
        Serial.println("Creating ticket selection UI");
        // Will be replaced with actual UI creation code
    }

    void updateTicketSelectionUI()
    {
        Serial.println("Updating ticket selection UI");
        Serial.print("Boarding: ");
        Serial.print(BusData::haltNames[selectedBoardingHalt]);
        Serial.print(", Destination: ");
        Serial.println(BusData::haltNames[selectedDestinationHalt]);
        // Will be replaced with actual UI update code
    }

    void createPassengerCountUI()
    {
        Serial.println("Creating passenger count UI");
        // Will be replaced with actual UI creation code
    }

    void updatePassengerCountUI()
    {
        Serial.println("Updating passenger count UI: " + String(passengerCount));
        // Will be replaced with actual UI update code
    }

    void createTicketPreviewUI()
    {
        Serial.println("Creating ticket preview UI");
        Serial.print("From: ");
        Serial.print(BusData::haltNames[selectedBoardingHalt]);
        Serial.print(" To: ");
        Serial.print(BusData::haltNames[selectedDestinationHalt]);
        Serial.print(" Passengers: ");
        Serial.println(passengerCount);
        // Will be replaced with actual UI creation code
    }

    void createPrintConfirmationUI()
    {
        Serial.println("Creating print confirmation UI");
        // Will be replaced with actual UI creation code
    }

    void createStatisticsUI()
    {
        Serial.println("Creating statistics UI");
        bus.printHaltStats(); // Print to serial for debugging
        // Will be replaced with actual UI creation code
    }
};

// Define static instance
KeyReceiver *KeyReceiver::instance = nullptr;

#endif // KEY_RECEIVER_H