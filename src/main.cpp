#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HardwareSerial.h>

#define RXD2 16
#define TXD2 17

const int baudRate = 115200;
const char *password = "0000"; // Replace with your actual password

HardwareSerial MySerial(2);

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Function declarations
bool sendPassword(const char *password);
void printConfig(byte *config, int length);
void printAsciiConfig(byte *config, int length);

void setup()
{
    Serial.begin(baudRate);
    MySerial.begin(baudRate, SERIAL_8N1, RXD2, TXD2);

    // Initialize the LCD
    lcd.init();
    lcd.backlight();

    delay(1000); // Allow some time for the serial connection to establish

    // Send initialization command with password
    if (!sendPassword(password))
    {
        Serial.println("Could not initialize with password.");
        while (true)
            ; // Halt the program
    }

    // Send command to read config
    byte readCommand[] = {0x51, 0x40}; // Assuming default length of 0x40
    MySerial.write(readCommand, sizeof(readCommand));

    delay(100);

    // Read response
    if (MySerial.available() > 0)
    {
        byte header[2];
        MySerial.readBytes(header, 2);

        if (header[0] == 0x25 && header[1] == 0x40)
        {
            byte config[0x40];
            MySerial.readBytes(config, 0x40);
            printConfig(config, 0x40);
            printAsciiConfig(config, 0x40);
        }
        else
        {
            Serial.println("Could not read config.");
        }
    }
    else
    {
        Serial.println("No response from device.");
    }
}

void loop()
{
    // Empty loop as all operations are done in setup
}

bool sendPassword(const char *password)
{
    byte passwordCommand[] = {0x59, 0x01, 0x42, 0x06}; // Command with length of 6
    MySerial.write(passwordCommand, sizeof(passwordCommand));

    // Prepare and send password bytes
    char passwordBuffer[6] = {'0', '0', '0', '0', '0', '0'};
    strncpy(passwordBuffer, password, 6);
    MySerial.write((uint8_t *)passwordBuffer, 6);

    delay(100);

    // Read response
    if (MySerial.available() > 0)
    {
        byte response[2];
        MySerial.readBytes(response, 2);
        return response[0] == 0x95 && response[1] == 0x20;
    }
    return false;
}

void printConfig(byte *config, int length)
{
    Serial.println("Config Data:");
    for (int i = 0; i < length; i++)
    {
        Serial.printf("0x%02x ", config[i]);
        if ((i + 1) % 16 == 0)
        {
            Serial.println();
        }
    }
    Serial.println();
}

void printAsciiConfig(byte *config, int length)
{
    // Convert specific sections to ASCII strings
    Serial.println("ASCII Representation:");

    // MMSI (bytes 1-4)
    int mmsi = config[1] + (config[2] << 8) + (config[3] << 16) + (config[4] << 24);
    Serial.print("MMSI: ");
    Serial.println(mmsi);

    // Name (bytes 5-24)
    Serial.print("Name: ");
    char name[21];
    for (int i = 5; i <= 24; i++)
    {
        Serial.print((char)config[i]);
        name[i - 5] = config[i];
    }
    name[20] = '\0';
    Serial.println();

    // Call Sign (bytes 32-37, 6-bit encoding)
    Serial.print("Call Sign: ");
    char callsign[7];
    for (int i = 32; i <= 37; i++)
    {
        Serial.print((char)((config[i] & 0x3F) + '0')); // Adjust the encoding as necessary
        callsign[i - 32] = (char)((config[i] & 0x3F) + '0');
    }
    callsign[6] = '\0';
    Serial.println();

    // Display on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MMSI:");
    lcd.setCursor(6, 0);
    lcd.print(mmsi);
    lcd.setCursor(0, 1);
    lcd.print(name);
    lcd.setCursor(11, 1);
    lcd.print(callsign);
}
