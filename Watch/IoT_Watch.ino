#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <RadioLib.h>
#include "boards.h"
#include <WiFi.h>

// BLE UUIDs
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Initialize the SX1280 radio module
SX1280 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

// Transmission state and transmitted flag for radio
int transmissionState = RADIOLIB_ERR_NONE;
volatile bool transmittedFlag = false;
volatile bool enableInterrupt = true;

// Counter for transmissions
uint32_t counter = 0;

void setFlag(void) {
    if (!enableInterrupt) return;
    transmittedFlag = true;
}

void setup() {

  initBoard();
  delay(1500);
  Serial.begin(115200);
  
  // Initialize BLE
  Serial.println("Starting BLE work!");
  BLEDevice::init("TESTING FOR MARK");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic->setValue("Hello World says Neil");
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("BLE Characteristic defined! Now you can read it in your phone!");

  // Setup for SX1280
  // When the power is turned on, a delay is required.
  Serial.print(F("[SX1280] Initializing ... "));
  int state = radio.begin();
#ifdef HAS_DISPLAY
    if (u8g2) {
      if (state != RADIOLIB_ERR_NONE) {
        u8g2->clearBuffer();
        u8g2->drawStr(0, 12, "Initialising: FAIL!");
        u8g2->sendBuffer();
      }
    }
#endif
#ifdef EDP_DISPLAY
if (state != RADIOLIB_ERR_NONE) {
    display.setRotation(1);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(0, 15);
    display.println("Initializing: FAIL!");
    display.update();
}
#endif
    if (state == RADIOLIB_ERR_NONE) {
      Serial.println(F("success!"));
    } else {
      Serial.print(F("failed, code "));
      Serial.println(state);
      while (true); // Infinite loop on failure
    }

#if defined(RADIO_RX_PIN) && defined(RADIO_TX_PIN)
    //Set ANT Control pins
    radio.setRfSwitchPins(RADIO_RX_PIN, RADIO_TX_PIN);
#endif

#ifdef LILYGO_T3_S3_V1_0
    // T3 S3 V1.1 with PA Version Set output power to 3 dBm    !!Cannot be greater than 3dbm!!
    int8_t TX_Power = 3;
#else
    // T3 S3 V1.2 (No PA) Version Set output power to 3 dBm    !!Cannot be greater than 3dbm!!
    int8_t TX_Power = 3;
#endif
    if (radio.setOutputPower(TX_Power) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
        Serial.println(F("Selected output power is invalid for this module!"));
        while (true);
    }


    // set carrier frequency to 2410.5 MHz
    if (radio.setFrequency(2400.0) == RADIOLIB_ERR_INVALID_FREQUENCY) {
        Serial.println(F("Selected frequency is invalid for this module!"));
        while (true);
    }

    // set bandwidth to 203.125 kHz
    if (radio.setBandwidth(203.125) == RADIOLIB_ERR_INVALID_BANDWIDTH) {
        Serial.println(F("Selected bandwidth is invalid for this module!"));
        while (true);
    }

    // set spreading factor to 10
    if (radio.setSpreadingFactor(10) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR) {
        Serial.println(F("Selected spreading factor is invalid for this module!"));
        while (true);
    }

    // set coding rate to 6
    if (radio.setCodingRate(6) == RADIOLIB_ERR_INVALID_CODING_RATE) {
        Serial.println(F("Selected coding rate is invalid for this module!"));
        while (true);
    }


    // set the function that will be called
    // when packet transmission is finished
    radio.setDio1Action(setFlag);

    // start transmitting the first packet
    Serial.print(F("[SX1280] Sending first packet ... "));

    // you can also transmit byte array up to 256 bytes long
    byte byteArr[6];
    WiFi.macAddress(byteArr);
    transmissionState = radio.startTransmit(byteArr, 6);
}

void loop()
{
// check if the previous transmission finished
    if (transmittedFlag) {
        // disable the interrupt service routine while
        // processing the data
        enableInterrupt = false;

        // reset flag
        transmittedFlag = false;

        if (transmissionState == RADIOLIB_ERR_NONE) {
            // packet was successfully sent
            Serial.println(F("transmission finished!"));

            // NOTE: when using interrupt-driven transmit method,
            //       it is not possible to automatically measure
            //       transmission data rate using getDataRate()
#ifdef HAS_DISPLAY
            if (u8g2) {
                u8g2->clearBuffer();
                u8g2->drawStr(0, 12, "Transmitting: OK!");
                u8g2->drawStr(0, 30, ("TX:" + String(counter)).c_str());
                // u8g2->drawStr(0, 48, ("Sent:" + String(byteArr)).c_str());
                u8g2->sendBuffer();
            }
#endif
#ifdef EDP_DISPLAY
            display.setRotation(1);
            display.fillScreen(GxEPD_WHITE);
            display.setTextColor(GxEPD_BLACK);
            display.setFont(&FreeMonoBold9pt7b);
            display.setCursor(0, 15);
            display.println("Transmitting: OK!");
            display.update();
#endif
        } else {
            Serial.print(F("failed, code "));
            Serial.println(transmissionState);
        }

        // wait a second before transmitting again
        delay(1000  );

        // send another one
        Serial.print(F("[SX1280] Sending another packet ... "));

  
        byte byteArr[6];
        // generateMACAddress(byteArr);
        WiFi.macAddress(byteArr);
        transmissionState = radio.startTransmit(byteArr, 6);

        counter++;
        // we're ready to send more packets,
        // enable interrupt service routine
        enableInterrupt = true;
    }
}

