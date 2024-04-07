const express = require("express");
const http = require("http");
const socketIo = require("socket.io");

const app = express();
const server = http.createServer(app);

const cors = require("cors");

// Allow CORS for all routes
app.use(cors());

// Configure Socket.IO to allow CORS
const io = socketIo(server, {
  cors: {
    origin: "http://localhost:3000", // URL of your React frontend
    methods: ["GET", "POST"],
  },
});

let watchNodes = {};
let nodeDevices = {};
let nodeConnections = {};

app.use(express.json());

function sendCurrentState(socket) {
  Object.entries(nodeDevices).forEach(([mac, node]) => {
    socket.emit("tag_update", {
      mac_address: mac,
      node_type: node.node_type,
      rssi: node.rssi,
      node_mac: node.node_mac,
    });
  });
}

app.post("/api/data", (req, res) => {
  const { mac, rssi, node_mac } = req.body;

  // Update nodeDevices
  nodeDevices[node_mac] = {
    node_type: "LoRa",
    rssi: rssi,
    node_mac: node_mac,
  };

  // Update watchNodes count
  if (!nodeConnections[node_mac]) {
    nodeConnections[node_mac] = new Set();
  }
  if (!nodeConnections[node_mac].has(mac)) {
    nodeConnections[node_mac].add(mac);
    watchNodes[node_mac] = {
      node_type: "LoRa",
      count: nodeConnections[node_mac].size,
    };
  }

  // Emit updates to clients with the count
  io.emit("tag_update", {
    node_mac: node_mac,
    node_type: "LoRa",
    rssi: rssi,
    count: watchNodes[node_mac].count,
  });

  res.json({ status: "success", message: "Data received" });
});

app.post("/api/ble_data", (req, res) => {
  const data = req.body;
  data.forEach((device) => {
    const { mac_address, rssi, boardName } = device;

    // Update nodeDevices
    nodeDevices[boardName] = {
      node_type: "BLE",
      rssi: rssi,
      node_mac: boardName,
    };

    // Update watchNodes count
    if (!nodeConnections[boardName]) {
      nodeConnections[boardName] = new Set();
    }
    if (!nodeConnections[boardName].has(mac_address)) {
      nodeConnections[boardName].add(mac_address);
      watchNodes[boardName] = {
        node_type: "BLE",
        count: nodeConnections[boardName].size,
      };
    }

    // Emit updates to clients with the count
    io.emit("tag_update", {
      node_mac: boardName,
      node_type: "BLE",
      rssi: rssi,
      count: watchNodes[boardName].count,
    });
  });

  res.json({ status: "success", message: "BLE data received" });
});

io.on("connection", (socket) => {
  console.log("A client connected");
  sendCurrentState(socket); // Send the current state to the newly connected client
});

server.listen(5000, () => {
  console.log("Server listening on port 5000");
});
