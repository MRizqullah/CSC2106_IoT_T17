const axios = require("axios");
const randomMac = require("random-mac");

const nodeServerUrl = "http://localhost:5000";

// Simulated LoRa nodes
const loraNodes = [
  { mac: "11:22:33:44:55:66", rssiRange: [-50, -20] },
  { mac: "77:88:99:aa:bb:cc", rssiRange: [-60, -30] },
];

// Simulated BLE Mesh nodes
const bleNodes = [
  { mac: "ff:ee:dd:cc:bb:aa", rssiRange: [-40, -10] },
  { mac: "aa:bb:cc:dd:ee:ff", rssiRange: [-45, -15] },
];

function randomInRange(min, max) {
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

function simulateWatchConnection(watchMac) {
  setInterval(async () => {
    const nodeType = Math.random() > 0.5 ? "LoRa" : "BLE";

    if (nodeType === "LoRa") {
      const node = loraNodes[Math.floor(Math.random() * loraNodes.length)];
      const rssi = randomInRange(node.rssiRange[0], node.rssiRange[1]);
      const loraData = {
        mac: watchMac,
        rssi: rssi,
        node_mac: node.mac,
      };
      try {
        const response = await axios.post(
          `${nodeServerUrl}/api/data`,
          loraData
        );
        console.log(
          `Watch ${watchMac} connected to LoRa node ${node.mac}. RSSI: ${rssi}. Response: ${response.data.message}`
        );
      } catch (error) {
        console.error(`Error sending data to server: ${error.message}`);
      }
    } else {
      const node = bleNodes[Math.floor(Math.random() * bleNodes.length)];
      const rssi = randomInRange(node.rssiRange[0], node.rssiRange[1]);
      const bleData = {
        mac_address: watchMac,
        rssi: rssi,
        boardName: node.mac,
      };
      try {
        const response = await axios.post(`${nodeServerUrl}/api/ble_data`, [
          bleData,
        ]);
        console.log(
          `Watch ${watchMac} connected to BLE node ${node.mac}. RSSI: ${rssi}. Response: ${response.data.message}`
        );
      } catch (error) {
        console.error(`Error sending data to server: ${error.message}`);
      }
    }
  }, randomInRange(3000, 10000));
}

function main() {
  // Simulate 4 watches
  for (let i = 0; i < 4; i++) {
    const watchMac = randomMac();
    simulateWatchConnection(watchMac);
  }
}

main();
