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
  const nodeType = Math.random() > 0.5 ? "LoRa" : "BLE";
  const node =
    nodeType === "LoRa"
      ? loraNodes[Math.floor(Math.random() * loraNodes.length)]
      : bleNodes[Math.floor(Math.random() * bleNodes.length)];
  const rssi = randomInRange(node.rssiRange[0], node.rssiRange[1]);
  const data =
    nodeType === "LoRa"
      ? {
          mac: watchMac,
          rssi: rssi,
          node_mac: node.mac,
        }
      : {
          mac_address: watchMac,
          rssi: rssi,
          boardName: node.mac,
        };
  const url =
    nodeType === "LoRa"
      ? `${nodeServerUrl}/api/data`
      : `${nodeServerUrl}/api/ble_data`;

  axios
    .post(url, nodeType === "BLE" ? [data] : data)
    .then((response) => {
      console.log(
        `Watch ${watchMac} connected to ${nodeType} node ${node.mac}. RSSI: ${rssi}. Response: ${response.data.message}`
      );
      setTimeout(
        () => simulateWatchConnection(watchMac),
        randomInRange(3000, 10000)
      ); // Recursively call the function after a random delay
    })
    .catch((error) => {
      console.error(`Error sending data to server: ${error.message}`);
      setTimeout(
        () => simulateWatchConnection(watchMac),
        randomInRange(3000, 10000)
      ); // Recursively call the function even if there's an error
    });
}

function main() {
  // Simulate 4 watches
  for (let i = 0; i < 4; i++) {
    const watchMac = randomMac();
    simulateWatchConnection(watchMac);
  }
}

main();
