#include "painlessMesh.h"

// Directives for BLE scanning
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <string>

#define   MESH_PREFIX     "t17mesh" // Name of the mesh network
#define   MESH_PASSWORD   "password123" // Password for the mesh network
#define   MESH_PORT       5555
#define   ROOT_NODE_ID    634093945

/* GLOBAL VARIABLES */

int scanTime = 2; // BLE scan duration (seconds). It should not be greater than the first parameter of Task bleScan to avoid conflicts.
String jsonString;
Scheduler userScheduler;
painlessMesh mesh; // Mesh network object
String BOARD_NAME = String(mesh.getNodeId()); // Name/number of the board

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
  }
};

// Prototypes
void transmitData();
void scan();
void processScanData(const String& data);

// TASKS
Task transmit(TASK_SECOND * 3, TASK_FOREVER, &transmitData);
Task bleScan(TASK_SECOND * 3, TASK_FOREVER, &scan);

static void scanCompleteCB(BLEScanResults foundDevices) {
  printf("Scan complete!\n We found %d devices\n", foundDevices.getCount());

  StaticJsonDocument<1024> scanResultsJson;
  JsonArray devicesArray = scanResultsJson.to<JsonArray>();

  for (int i = 0; i < foundDevices.getCount(); i++) {
    // Save the i-th device (MAC, RSSI) in the i-th position of the array
    BLEAdvertisedDevice device = foundDevices.getDevice(i);
    JsonObject deviceJson = devicesArray.createNestedObject();
    deviceJson["mac_address"] = device.getAddress().toString();
    deviceJson["rssi"] = device.getRSSI();
    deviceJson["boardName"] = mesh.getNodeId(); // Include the board name to identify which board made the detection
  }

  // Convert the JSON to a string for sending via painlessMesh
  serializeJson(scanResultsJson, jsonString);
  devicesArray.clear();

  // printf("JsonString: ");
  // printf("%s\n", jsonString.c_str());

  foundDevices.dump();
}

static void run() {
  printf("Async Scanning sample starting\n");

  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), false);
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(200); // Time interval between two scans, in ms
  pBLEScan->setWindow(199); // Time window for scanning, between two scans. Should be less than or equal to the value above for obvious reasons

  printf("About to start scanning for %d seconds\n", scanTime);

  pBLEScan->start(scanTime, scanCompleteCB);
  printf("Now scanning in the background ... scanCompleteCB() will be called when done.\n");
}

void scan() {
  printf("Entered scan Task\n");
  run();
}

void transmitData() {
  if (mesh.getNodeId() != ROOT_NODE_ID) {
    // If not the root node, broadcast the scan results
    mesh.sendBroadcast(jsonString);
  } else {
    // If this is the root node, process the data
    processScanData(jsonString);
  }
}

void processScanData(const String& data) {
  // Process scan data at the root node
  Serial.println("Root node processing data:");
  Serial.println(data);
}

// Callbacks necessary to keep the mesh network up to date
void receivedCallback(uint32_t from, String &msg) {
  // This function is executed when a message is received
  if (mesh.getNodeId() == ROOT_NODE_ID) {
    // Only process the data if this is the root node
    processScanData(msg);
  }
}

// This function is executed every time a new node connects to the network
void newConnectionCallback(uint32_t nodeId) {
  printf("New node, nodeId = %u\n", nodeId);
}

// This function is executed when the connection with a node changes (a node enters or leaves the network)
void changedConnectionCallback() {
  printf("Network change\n");
}

// This function is executed when the network adjusts the timing, so everyone is in sync
void nodeTimeAdjustedCallback(int32_t offset) {
  printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup() {
  Serial.begin(115200);

  // Choose the debug message that pleases the developer's majesty
  // mesh.setDebugMsgTypes(ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE); // all types on
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION | GENERAL); // set before init() so that you can see mesh startup messages
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT); // initialize the mesh with the previously defined parameters
  
  // Assign each event to its corresponding function
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(transmit); // add the task of transmitting the message to the scheduler
  userScheduler.addTask(bleScan);
  bleScan.enable();
  transmit.enable();
}

void loop() {
  mesh.update(); // keep the mesh running and the scheduler as well
}
