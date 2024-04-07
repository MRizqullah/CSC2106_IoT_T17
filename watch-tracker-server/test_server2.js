const axios = require("axios");
const randomMac = require("random-mac");

const nodeServerUrl = "http://localhost:5000";

const nodes = [
  { mac: "11:22:33:44:55:66", rssiRange: [-50, -20], type: "LoRa" },
  { mac: "77:88:99:aa:bb:cc", rssiRange: [-60, -30], type: "LoRa" },
  { mac: "ff:ee:dd:cc:bb:aa", rssiRange: [-40, -10], type: "BLE" },
  { mac: "aa:bb:cc:dd:ee:ff", rssiRange: [-45, -15], type: "BLE" },
];

function randomInRange(min, max) {
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

function simulateWatchThroughNodes(watchMac) {
  let currentNodeIndex = 0;

  function connectToNextNode() {
    if (currentNodeIndex >= nodes.length) {
      currentNodeIndex = 0; // Loop back to the first node
    }

    const node = nodes[currentNodeIndex];
    const rssi = randomInRange(node.rssiRange[0], node.rssiRange[1]);
    const data =
      node.type === "LoRa"
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
      node.type === "LoRa"
        ? `${nodeServerUrl}/api/data`
        : `${nodeServerUrl}/api/ble_data`;

    axios
      .post(url, node.type === "BLE" ? [data] : data)
      .then((response) => {
        console.log(
          `Watch ${watchMac} connected to ${node.type} node ${node.mac}. RSSI: ${rssi}. Response: ${response.data.message}`
        );
        currentNodeIndex++;
        setTimeout(connectToNextNode, randomInRange(3000, 10000)); // Move to the next node after a random delay
      })
      .catch((error) => {
        console.error(`Error sending data to server: ${error.message}`);
        currentNodeIndex++;
        setTimeout(connectToNextNode, randomInRange(3000, 10000)); // Move to the next node even if there's an error
      });
  }

  connectToNextNode();
}

function main() {
  const watchMac = randomMac();
  simulateWatchThroughNodes(watchMac);
}

main();
