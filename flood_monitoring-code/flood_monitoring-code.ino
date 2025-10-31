#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>

/* OTAA Parameters */
uint8_t devEui[] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x24, 0x68};
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0x5F, 0x4A, 0x2B, 0x2A, 0x58, 0xC4, 0x27, 0x53, 0x71, 0x14, 0x1F, 0x11, 0xF2, 0x95, 0x8B, 0x17 };

/* ABP Parameters */
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda, 0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef, 0x67 };
uint32_t devAddr = (uint32_t)0x007e6ae1;

/* LoRaWAN Settings */
uint16_t userChannelsMask[6] = { 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;
DeviceClass_t loraWanClass = CLASS_A;
uint32_t appTxDutyCycle = 15000; // 
bool overTheAirActivation = true;
bool loraWanAdr = true;
bool isTxConfirmed = true;
uint8_t appPort = 2;
uint8_t confirmedNbTrials = 4;


/* Sensor Pins */
#define TRIG_PIN 7
#define ECHO_PIN 5
#define RAIN_SENSOR_PIN 2  // Using D0 (Digital Output)

float duration, distance;
uint8_t irStatus, rainStatus;

/*!
   \brief   Prepares the payload of the frame
*/
static void prepareTxFrame(uint8_t port) {
    // Ultrasonic Sensor Measurement
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(30);
    digitalWrite(TRIG_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH);
    distance = duration * 0.034 / 2;

    Serial.print("Ultrasonic Distance: ");
    Serial.println(distance);

    // Raindrop Sensor Measurement
    pinMode(RAIN_SENSOR_PIN, INPUT_PULLUP);
    rainStatus = digitalRead(RAIN_SENSOR_PIN);  // Read rain sensor (D0)

    Serial.print("Raindrop Sensor Status: ");
    if (rainStatus == 1) {
        Serial.println("No Rain");
    } else {
        Serial.println("Rain Detected");
    }

// Convert floats to bytes and copy into appData
    memcpy(&appData[0], &distance, sizeof(float));       // Bytes 0-3
    memcpy(&appData[4], &rainStatus, sizeof(float));    // Bytes 4-7

    appDataSize = 8;  // Ensure 8 bytes payload

    // Debug: print payload as HEX
    Serial.print("App Data (Hex): ");
    for (int i = 0; i < appDataSize; i++) {
        Serial.printf("%02X ", appData[i]);
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    Mcu.begin();
    pinMode(RAIN_SENSOR_PIN, INPUT);
    deviceState = DEVICE_STATE_INIT;
}

void loop() {
    switch (deviceState) {
        case DEVICE_STATE_INIT: {
#if (LORAWAN_DEVEUI_AUTO)
            LoRaWAN.generateDeveuiByChipID();
#endif
            LoRaWAN.init(loraWanClass, loraWanRegion);
            break;
        }
        case DEVICE_STATE_JOIN: {
            LoRaWAN.join();
            break;
        }
        case DEVICE_STATE_SEND: {
            prepareTxFrame(appPort);
            LoRaWAN.send();
            deviceState = DEVICE_STATE_CYCLE;
            break;
        }
        case DEVICE_STATE_CYCLE: {
            txDutyCycleTime = appTxDutyCycle + randr(0, APP_TX_DUTYCYCLE_RND);
            LoRaWAN.cycle(txDutyCycleTime);
            deviceState = DEVICE_STATE_SLEEP;
            break;
        }
        case DEVICE_STATE_SLEEP: {
            LoRaWAN.sleep(loraWanClass);
            break;
        }
        default: {
            deviceState = DEVICE_STATE_INIT;
            break;
}
    }
}