const axios = require("axios");
const nodeServerUrl = "http://localhost:5000";

const nodes = [
  { mac: "11:22:33:44:55:66", rssiRange: [-50, -20], type: "LoRa" },
  { mac: "77:88:99:aa:bb:cc", rssiRange: [-60, -30], type: "LoRa" },
  { mac: "ff:ee:dd:cc:bb:aa", rssiRange: [-40, -10], type: "BLE" },
  { mac: "aa:bb:cc:dd:ee:ff", rssiRange: [-45, -15], type: "BLE" },
];

const watchMac = "00:11:22:33:44:55";

function simulateWatchThroughNodes() {
  const sequence = [0, 1, 2, 3, 2, 1, 0];
  let currentIndex = 0;

  function connectToNextNode() {
    const nodeIndex = sequence[currentIndex];
    const node = nodes[nodeIndex];
    const rssi = node.rssiRange[0];
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
            node_mac: node.mac,
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
        currentIndex = (currentIndex + 1) % sequence.length;
        setTimeout(connectToNextNode, 5000);
      })
      .catch((error) => {
        console.error(`Error sending data to server: ${error.message}`);
        currentIndex = (currentIndex + 1) % sequence.length;
        setTimeout(connectToNextNode, 5000);
      });
  }

  connectToNextNode();
}

function main() {
  simulateWatchThroughNodes();
}

main();
