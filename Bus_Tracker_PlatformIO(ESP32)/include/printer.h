#ifndef PRINTER_H
#define PRINTER_H

#include <Arduino.h>
#include "Adafruit_Thermal.h"
#include "Config.h"

class Printer
{
private:
    HardwareSerial *SerialPort;
    Adafruit_Thermal *printer;

public:
    // Constructor
    Printer()
    {
        SerialPort = new HardwareSerial(0);
        printer = new Adafruit_Thermal(SerialPort);
    }

    void begin() {
        // Initialize the printer
        SerialPort->begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
        printer->begin();
        
        // Optional: Reduce the default line spacing
        printer->setLineHeight(30); // Default is 32
        
        // Force a reset of printer settings without paper feed
        printer->wake();
        printer->setDefault();
        
        // This helps reduce the initial paper feed on some printers
        printer->writeBytes(27, 74, 0); // ESC J 0 - Feed 0 lines
    }

    void printBill(String regNum, String comName, String rtNo, String date, String time,
                   String refNo, String route, String from, String to, bool type,
                   uint8_t numOfTicks, float unitPrize, String number, String hotline)
    {

        printer->writeBytes(27, 64); // ESC @ - Initialize/reset printer

        // Optional: Set smaller top margin
        printer->writeBytes(27, 79); // ESC O - Cancel top/bottom margins

        printer->justify('C');
        printer->setFont('B');
        printer->setSize('S');
        printer->println(regNum);

        printer->setSize('M');
        printer->println(comName);

        printer->setSize('S');
        printer->justify('L');
        printer->print(F("Route No: "));
        printer->print(rtNo);
        printer->print(F("  "));
        printer->justify('R');
        printer->print(F("Ref: "));
        printer->println(refNo);

        printer->justify('C');
        printer->print(date);
        printer->print(F("   "));
        printer->println(time);

        printer->println(route);

        printer->setSize('M');
        printer->justify('L');
        printer->print(F("From: "));
        printer->print(from);
        printer->justify('R');
        printer->print(F("  "));
        printer->print(F("To: "));
        printer->println(to);

        printer->setSize('S');
        printer->justify('C');

        if (type)
        {
            printer->print(F("Full"));
        }
        else
        {
            printer->print(F("Half"));
        }

        printer->print(F("  "));
        printer->print(numOfTicks);
        printer->print(F("x"));
        printer->print(unitPrize);
        printer->print(F(" = "));
        float total = numOfTicks * unitPrize;
        printer->println(total);

        printer->justify('C');
        printer->setSize('M');
        printer->boldOn();
        printer->print(F("Total : "));
        printer->println(total);
        printer->boldOff();

        printer->justify('C');
        printer->setSize('S');
        printer->print(F("Contact : "));
        printer->println(number);

        printer->print(F("Contact : "));
        printer->println(hotline);
        printer->justify('C');
        printer->println(F("___________________________"));

        printer->sleep();
    }
};

// Global instance declaration
extern Printer printer;

#endif