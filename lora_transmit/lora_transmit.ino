/*
   RadioLib SX128x Transmit with Interrupts Example

   This example transmits LoRa packets with one second delays
   between them. Each packet contains up to 256 bytes
   of data, in the form of:
    - Arduino String
    - null-terminated char array (C-string)
    - arbitrary binary data (byte array)

   Other modules from SX128x family can also be used.

   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx128x---lora-modem

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/


#include <RadioLib.h>
#include "boards.h"
#include <stdlib.h>
#include <WiFi.h>

int generateMACAddress(byte macAddress[]);

SX1280 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

// save transmission state between loops
int transmissionState = RADIOLIB_ERR_NONE;

// flag to indicate that a packet was sent
volatile bool transmittedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;

uint32_t counter = 0;

// this function is called when a complete packet
// is transmitted by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void setFlag(void)
{
    // check if the interrupt is enabled
    if (!enableInterrupt) {
        return;
    }

    // we sent a packet, set the flag
    transmittedFlag = true;
}

void setup()
{
    initBoard();
    // When the power is turned on, a delay is required.
    delay(1500);

    // initialize SX1280 with default settings
    Serial.print(F("[SX1280] Initializing ... "));
    int state = radio.begin();
#ifdef HAS_DISPLAY
    if (u8g2) {
        if (state != RADIOLIB_ERR_NONE) {
            u8g2->clearBuffer();
            u8g2->drawStr(0, 12, "Initializing: FAIL!");
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
        while (true);
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

    // you can transmit C-string or Arduino string up to
    // 256 characters long
    // transmissionState = radio.startTransmit("Hello Word!");
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
        delay(2);

        // send another one
        Serial.print(F("[SX1280] Sending another packet ... "));

        // you can transmit C-string or Arduino string up to
        // 256 characters long
        // transmissionState = radio.startTransmit("Hello World!");

        // you can also transmit byte array up to 256 bytes long
        // byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
        //                   0x89, 0xAB, 0xCD, 0xEF};
        byte byteArr[6];
        // generateMACAddress(byteArr);
        WiFi.macAddress(byteArr);
        transmissionState = radio.startTransmit(byteArr, 6);
        
        // transmissionState = radio.startTransmit("Hello Word!");

        counter++;
        // we're ready to send more packets,
        // enable interrupt service routine
        enableInterrupt = true;
    }
}




int generateMACAddress(byte macAddress[]) {
  // Seed the random number generator
  srand(time(NULL));

  // Ensure Multicast (bit 1 of the first byte is set) and Locally Administered (bit 7 of the first byte is set)
  macAddress[0] = (rand() % 256) | 0x02;  // Multicast bit
  macAddress[0] |= 0x80;               // Locally Administered bit

  // Generate random bytes for the remaining bytes (ensure uniqueness within the first three bytes)
  for (int i = 1; i < 6; i++) {
    // Avoid generating the same first three bytes for multiple addresses
    if (i < 3) {
      do {
        macAddress[i] = rand() % 256;
      } while (macAddress[i] == macAddress[i - 1] && macAddress[i] == macAddress[i - 2]);
    } else {
      macAddress[i] = rand() % 256;
    }
  }

  return 1;  // Function execution successful
}

