const express = require("express");
const http = require("http");
const socketIo = require("socket.io");
const app = express();
const server = http.createServer(app);
const cors = require("cors");
app.use(cors());

const io = socketIo(server, {
  cors: {
    origin: "http://localhost:3000",
    methods: ["GET", "POST"],
  },
});

let watchNodes = {};
let nodeDevices = {};
let watchCurrentNode = {};
let ongoingConnections = {};

app.use(express.json());

function sendCurrentState(socket) {
  Object.entries(nodeDevices).forEach(([nodeMac, node]) => {
    socket.emit("tag_update", {
      node_mac: nodeMac,
      node_type: node.node_type,
      rssi: node.rssi,
      count: watchNodes[nodeMac]?.count || 0,
    });
  });
  io.emit("connection_update", ongoingConnections);
}

function updateNodeCounts(watchMac, newNodeMac) {
  const currentNodeMac = watchCurrentNode[watchMac];
  if (currentNodeMac && currentNodeMac !== newNodeMac) {
    watchNodes[currentNodeMac].count = Math.max(
      0,
      watchNodes[currentNodeMac].count - 1
    );
    if (watchNodes[currentNodeMac].count === 0) {
      delete watchNodes[currentNodeMac]; // Remove node if no devices are connected
    } else {
      io.emit("tag_update", {
        node_mac: currentNodeMac,
        node_type: watchNodes[currentNodeMac].node_type,
        rssi: nodeDevices[currentNodeMac]?.rssi || 0,
        count: watchNodes[currentNodeMac].count,
      });
    }
  }
  if (newNodeMac) {
    watchNodes[newNodeMac] = watchNodes[newNodeMac] || {
      count: 0,
      node_type: nodeDevices[newNodeMac]?.node_type || "Unknown",
    };
    watchNodes[newNodeMac].count++;
    watchCurrentNode[watchMac] = newNodeMac;
    io.emit("tag_update", {
      node_mac: newNodeMac,
      node_type: watchNodes[newNodeMac].node_type,
      rssi: nodeDevices[newNodeMac]?.rssi || 0,
      count: watchNodes[newNodeMac].count,
    });
  }
}

function handleDisconnect(mac, node_mac) {
  delete ongoingConnections[mac];
  if (watchCurrentNode[mac] === node_mac) {
    delete watchCurrentNode[mac];
    updateNodeCounts(mac, null);
  }
  io.emit("connection_update", ongoingConnections);
}

app.post("/api/data", (req, res) => {
  const { mac, rssi, node_mac, timestamp } = req.body;
  console.log(
    `Data received from ${mac} at ${timestamp}: RSSI ${rssi}, Node MAC ${node_mac}`
  );
  if (rssi == 0) {
    // Assuming rssi == 0 indicates a disconnect
    handleDisconnect(mac, node_mac);
  } else {
    ongoingConnections[mac] = { rssi, node_mac, timestamp };
    io.emit("connection_update", ongoingConnections);
    if (
      !watchCurrentNode[mac] ||
      (watchCurrentNode[mac] !== node_mac &&
        rssi > nodeDevices[watchCurrentNode[mac]]?.rssi)
    ) {
      updateNodeCounts(mac, node_mac);
    }
    nodeDevices[node_mac] = {
      node_type: "LoRa",
      rssi: rssi,
      node_mac: node_mac,
    };
    io.emit("tag_update", {
      node_mac: node_mac,
      node_type: "LoRa",
      rssi: rssi,
      count: watchNodes[node_mac]?.count || 0,
    });
  }
  res.json({ status: "success", message: "Data received" });
});

app.post("/api/ble_data", (req, res) => {
  const data = req.body;
  data.forEach((device) => {
    const { mac_address, rssi, node_mac, timestamp } = device;
    console.log(
      `BLE data received from ${mac_address} at ${timestamp}: RSSI ${rssi}, Node MAC ${node_mac}`
    );
    if (rssi == 0) {
      // Assuming rssi == 0 indicates a disconnect
      handleDisconnect(mac_address, node_mac);
    } else {
      ongoingConnections[mac_address] = { rssi, node_mac, timestamp };
      io.emit("connection_update", ongoingConnections);
      if (
        !watchCurrentNode[mac_address] ||
        (watchCurrentNode[mac_address] !== node_mac &&
          rssi > nodeDevices[watchCurrentNode[mac_address]]?.rssi)
      ) {
        updateNodeCounts(mac_address, node_mac);
      }
      nodeDevices[node_mac] = {
        node_type: "BLE",
        rssi: rssi,
        node_mac: node_mac,
      };
      io.emit("tag_update", {
        node_mac: node_mac,
        node_type: "BLE",
        rssi: rssi,
        count: watchNodes[node_mac]?.count || 0,
      });
    }
  });
  res.json({ status: "success", message: "BLE data received" });
});

io.on("connection", (socket) => {
  console.log("A client connected");
  sendCurrentState(socket);
});

server.listen(5000, () => {
  console.log("Server listening on port 5000");
});
