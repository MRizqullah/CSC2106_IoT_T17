/*
   RadioLib SX128x Receive with Interrupts Example

   This example listens for LoRa transmissions and tries to
   receive them. Once a packet is received, an interrupt is
   triggered. To successfully receive data, the following
   settings have to be the same on both transmitter
   and receiver:
    - carrier frequency
    - bandwidth
    - spreading factor
    - coding rate
    - sync word

   Other modules from SX128x family can also be used.

   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx128x---lora-modem

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/

#include <RadioLib.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "boards.h"

// WiFi credentials and server URL
const char *ssid = "Pixel_2658";
const char *password = "IhateIOT";
const char *serverUrl = "http://192.168.243.241:5000/api/data"; // Adjust to your server's address

SX1280 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

#ifdef HAS_DISPLAY
void displayMessage(const char* message, const byte* data, float rssi, float snr);
#endif

// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;

// FIFO buffer to store received data
#define BUFFER_SIZE 30
byte buffer[BUFFER_SIZE][6]; // Array of byte arrays to store 6-byte packets
int head = 0;                // Index of the oldest packet in the buffer
int tail = 0;                // Index where the next packet will be written

// Timeout management
unsigned long lastReceivedTime = 0;
const unsigned long TIMEOUT = 10000; // 10 seconds timeout

// this function is called when a complete packet
// is received by the module
void setFlag(void)
{
  // check if the interrupt is enabled
  if (!enableInterrupt)
  {
    return;
  }

  // we got a packet, set the flag
  receivedFlag = true;
  lastReceivedTime = millis();
}

void setup()
{
  Serial.begin(115200);
  Serial.println("testing 123");

  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  String gatewayMac = WiFi.macAddress();
  Serial.println("Gateway MAC Address: " + gatewayMac);

  initBoard();
  // When the power is turned on, a delay is required.
  delay(1500);

  // initialize SX1280 with default settings
  Serial.print("[SX1280] Initializing ... ");
  int state = radio.begin();
#ifdef HAS_DISPLAY
  if (u8g2)
  {
    if (state != RADIOLIB_ERR_NONE)
    {
      u8g2->clearBuffer();
      u8g2->drawStr(0, 12, "Initializing: FAIL!");
      u8g2->sendBuffer();
    }
  }
#endif

  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println("success!");
  }
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }

#if defined(RADIO_RX_PIN) && defined(RADIO_TX_PIN)
  // Set ANT Control pins
  radio.setRfSwitchPins(RADIO_RX_PIN, RADIO_TX_PIN);
#endif

#ifdef LILYGO_T3_S3_V1_0
  // T3 S3 V1.1 with PA Version Set output power to 3 dBm    !!Cannot be greater than 3dbm!!
  int8_t TX_Power = 3;
#else
  // T3 S3 V1.2 (No PA) Version Set output power to 3 dBm    !!Cannot be greater than 3dbm!!
  int8_t TX_Power = 3;
#endif
  if (radio.setOutputPower(TX_Power) == RADIOLIB_ERR_INVALID_OUTPUT_POWER)
  {
    Serial.println(F("Selected output power is invalid for this module!"));
    while (true)
      ;
  }

  // set carrier frequency to 2410.5 MHz
  if (radio.setFrequency(2400.0) == RADIOLIB_ERR_INVALID_FREQUENCY)
  {
    Serial.println(F("Selected frequency is invalid for this module!"));
    while (true)
      ;
  }

  // set bandwidth to 203.125 kHz
  if (radio.setBandwidth(203.125) == RADIOLIB_ERR_INVALID_BANDWIDTH)
  {
    Serial.println(F("Selected bandwidth is invalid for this module!"));
    while (true)
      ;
  }

  // set spreading factor to 10
  if (radio.setSpreadingFactor(10) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR)
  {
    Serial.println(F("Selected spreading factor is invalid for this module!"));
    while (true)
      ;
  }

  // set coding rate to 6
  if (radio.setCodingRate(6) == RADIOLIB_ERR_INVALID_CODING_RATE)
  {
    Serial.println(F("Selected coding rate is invalid for this module!"));
    while (true)
      ;
  }
  // set the function that will be called
  // when packet transmission is finished
  radio.setDio1Action(setFlag);

  // start listening for LoRa packets
  Serial.print(F("[SX1280] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }
  
  lastReceivedTime = millis();
}

void displayMessage(const char* message, const byte* data, float rssi, float snr) {
  #ifdef HAS_DISPLAY
    char buf[256];
    u8g2->clearBuffer();
    u8g2->drawStr(0, 12, message);
    snprintf(buf, sizeof(buf), "Data: %02x:%02x:%02x:%02x:%02x:%02x", 
            data[0], data[1], data[2], data[3], data[4], data[5]);
    u8g2->drawStr(0, 26, buf);
    snprintf(buf, sizeof(buf), "RSSI: %.2f dBm", rssi);
    u8g2->drawStr(0, 40, buf);
    snprintf(buf, sizeof(buf), "SNR: %.2f dB", snr);
    u8g2->drawStr(0, 54, buf);
    u8g2->sendBuffer();
  #endif
}

void loop()
{
  byte byteArr[6];
  // check if the flag is set
  if (receivedFlag)
  {
    // disable the interrupt service routine while
    // processing the data
    enableInterrupt = false;

    // reset flag
    receivedFlag = false;

    // read received data as byte array
    int state = radio.readData(byteArr, 6);

    if (state == RADIOLIB_ERR_NONE)
    {
      // packet was successfully received
      Serial.println(F("[SX1280] Received packet!"));

      // get rssi to determine if device is in range
      int rssi = radio.getRSSI();

      if (rssi >= -100)
      { // TODO: change to appropriate range

        // calculate next tial position
        int nextTail = (tail + 1) % BUFFER_SIZE;

        // check if buffer is full
        if (nextTail == head)
        {
          Serial.println("[SX1280] Buffer full, dropping oldest packet.");
          // update head to point to next packet to be overwritten
          head = (head + 1) % BUFFER_SIZE;
        }

        // copy received data to buffer
        memcpy(buffer[tail], byteArr, sizeof(byteArr));
        tail = nextTail;

        // print to check
        Serial.println(F("[SX1280] Buffer contents:"));
        for (int i = head; i != tail; i = (i + 1) % BUFFER_SIZE)
        {
          Serial.print(F("  Packet "));
          Serial.print(i);
          Serial.print(F(": "));
          for (int j = 0; j < 6; j++)
          {
            Serial.print(F("0x"));
            Serial.print(buffer[i][j], HEX);
            Serial.print(F(" "));
          }
          Serial.println();
        }

        // TODO: send buffer to server via mqtt
        // Convert received byteArr to HEX string for JSON payload
        String macAddress;
        for (int i = 0; i < 6; i++)
        {
          if (i > 0)
            macAddress += ":";
          macAddress += String(byteArr[i], HEX);
        }

        // Create JSON object string
        String jsonData = "{\"mac\": \"" + macAddress + "\", \"rssi\": " + String(rssi) + ", \"node_mac\": \"" + WiFi.macAddress() + "\"}";

        // Send JSON data to server
        if (WiFi.status() == WL_CONNECTED)
        {
          HTTPClient http;
          http.begin(serverUrl);
          http.addHeader("Content-Type", "application/json");
          int httpResponseCode = http.POST(jsonData);

          if (httpResponseCode > 0)
          {
            String response = http.getString();
            Serial.println("Server responded: " + response);
          }
          else
          {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
          }

          http.end();
        }

        // print data of the packet
        Serial.print(F("[SX1280] Data:\t\t"));
        for (int i = 0; i < sizeof(byteArr); i++)
        {
          Serial.println(byteArr[i]);
        }

        // print RSSI (Received Signal Strength Indicator)
        Serial.print(F("[SX1280] RSSI:\t\t"));
        Serial.print(radio.getRSSI());
        Serial.println(F(" dBm"));

        // print SNR (Signal-to-Noise Ratio)
        Serial.print(F("[SX1280] SNR:\t\t"));
        Serial.print(radio.getSNR());
        Serial.println(F(" dB"));

        if (u8g2)
        {
          displayMessage("Received OK!", byteArr, radio.getRSSI(), radio.getSNR());
        }
      }
      else
      {
        // Packet is outside desired range, ignore it
        Serial.println(F("[SX1280] Received packet outside range, ignoring."));

        if (u8g2)
        {
          displayMessage("Received IGNORE!", byteArr, radio.getRSSI(), radio.getSNR());
        }
      }
    }
    else if (state == RADIOLIB_ERR_CRC_MISMATCH)
    {
      // packet was received, but is malformed
      Serial.println(F("[SX1280] CRC error!"));
    }
    else
    {
      // some other error occurred
      Serial.print(F("[SX1280] Failed, code "));
      Serial.println(state);
    }

    // put module back to listen mode
    radio.startReceive();

    // we're ready to receive more packets,
    // enable interrupt service routine
    enableInterrupt = true;
  }

  if (millis() - lastReceivedTime > TIMEOUT) {
    Serial.println(F("[SX1280] Timeout on packet received."));
    displayMessage("No Packets!", byteArr, radio.getRSSI(), radio.getSNR());
    lastReceivedTime = millis();
  }
}