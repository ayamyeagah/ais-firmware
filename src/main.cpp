#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <HardwareSerial.h>
#include <AIS.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

HardwareSerial RSSerial(1);

const int ledPin = 26;

void printDegrees(long min4);
void showMMSI(AIS &ais_msg);
void showSOG(AIS &ais_msg);
void showCOG(AIS &ais_msg);
void showLatitude(AIS &ais_msg);
void showLongitude(AIS &ais_msg);

void setup()
{
    Serial.begin(115200);
    RSSerial.begin(9600, SERIAL_8N1, 16, 17);

    lcd.init();
    lcd.backlight();

    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    lcd.setCursor(0, 0);
    lcd.print("Waiting for data");
}

void loop()
{
    if (RSSerial.available())
    {
        String data = RSSerial.readStringUntil('\n');
        data.trim();

        AIS ais_msg(data.c_str());
        if (ais_msg.get_mmsi() != 0)
        {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Data received:");

            showMMSI(ais_msg);
            showSOG(ais_msg);
            showCOG(ais_msg);
            showLatitude(ais_msg);
            showLongitude(ais_msg);

            digitalWrite(ledPin, HIGH);
            delay(500);
            digitalWrite(ledPin, LOW);

            Serial.println("Received: " + data);
        }
        else
        {
            Serial.println("Failed to decode AIS message");
        }
    }
}

void printDegrees(long min4)
{
    long intPart = min4 / 600000;
    long fracPart = abs(min4 % 600000);
    char frac[7];
    sprintf(frac, "%06ld", fracPart);
    Serial.print(intPart);
    Serial.print(".");
    Serial.print(frac);
}

void showMMSI(AIS &ais_msg)
{
    unsigned long mmsi = ais_msg.get_mmsi();
    lcd.setCursor(0, 1);
    lcd.print("MMSI: " + String(mmsi));
    Serial.print("Returned MMSI: ");
    Serial.print(mmsi);
    Serial.print(" (");
    Serial.print(mmsi, 16);
    Serial.print(" )");
    Serial.println("");
}

void showSOG(AIS &ais_msg)
{
    unsigned int SOG = ais_msg.get_SOG();
    lcd.setCursor(0, 2);
    lcd.print("SOG: " + String(SOG / 10.0, 1) + " nm");
    Serial.print("Returned SOG: ");
    Serial.print(SOG / 10.0);
    Serial.println(" nm");
}

void showCOG(AIS &ais_msg)
{
    unsigned int COG = ais_msg.get_COG();
    lcd.setCursor(0, 3);
    lcd.print("COG: " + String(COG / 10.0, 1) + " deg");
    Serial.print("Returned COG: ");
    Serial.print(COG / 10.0);
    Serial.println(" degrees");
}

void showLatitude(AIS &ais_msg)
{
    long LAT = ais_msg.get_latitude();
    Serial.print("Returned LAT: ");
    printDegrees(LAT);
    Serial.println(" degrees");
}

void showLongitude(AIS &ais_msg)
{
    long LONG = ais_msg.get_longitude();
    Serial.print("Returned LONG: ");
    printDegrees(LONG);
    Serial.println(" degrees");
}
