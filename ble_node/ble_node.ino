#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "";
const char* password = "";
const char* serverName = "";
const char* nodeMacAddress = "BLE";
const char* targetMacAddress = "dc:54:75:e4:7a:e1"; // The MAC address you're interested in

BLEScan* pBLEScan;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop() {
  BLEScanResults foundDevices = pBLEScan->start(5, false);
  
  bool targetDeviceFound = false;
  String payload = "[";

  for (int i = 0; i < foundDevices.getCount(); i++) {
    BLEAdvertisedDevice device = foundDevices.getDevice(i);
    
    if (String(device.getAddress().toString().c_str()).equalsIgnoreCase(targetMacAddress)) {
      targetDeviceFound = true;
      
      if (payload.length() > 1) { // If not the first device, add a comma
        payload += ",";
      }
      
      payload += "{";
      payload += "\"mac_address\":\"" + String(device.getAddress().toString().c_str()) + "\",";
      payload += "\"rssi\":" + String(device.getRSSI()) + ",";
      payload += "\"node_mac\":\"" + String(nodeMacAddress) + "\",";
      payload += "\"timestamp\":" + String(millis());
      payload += "}";
      break; // Since the target device is found, no need to check further
    }
  }
  
  payload += "]";

  if (targetDeviceFound) {
    WiFiClient client;
    HTTPClient http;
  
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");
  
    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response code: " + String(httpResponseCode));
      Serial.println(response);
    } else {
      Serial.println("Error sending HTTP POST: " + http.errorToString(httpResponseCode));
    }
  
    http.end();
  } else {
    Serial.println("Target device not found.");
  }

  pBLEScan->clearResults(); // Clear the results for the next scan cycle
  delay(5000); // Delay before the next scan cycle
}
