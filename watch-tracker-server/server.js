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

app.use(express.json());

function sendCurrentState(socket) {
  Object.entries(nodeDevices).forEach(([nodeMac, node]) => {
    socket.emit("tag_update", {
      node_mac: nodeMac,
      node_type: node.node_type,
      rssi: node.rssi,
      count: watchNodes[nodeMac]?.count || 0, // Provide a default value of 0 for count
    });
  });
}

app.post("/api/data", (req, res) => {
  const { mac, rssi, node_mac } = req.body;

  if (
    !watchCurrentNode[mac] ||
    (watchCurrentNode[mac].node_type === "LoRa" &&
      rssi > watchCurrentNode[mac].rssi)
  ) {
    if (
      watchCurrentNode[mac] &&
      watchNodes[watchCurrentNode[mac].node_mac] &&
      watchCurrentNode[mac].node_mac !== node_mac
    ) {
      watchNodes[watchCurrentNode[mac].node_mac].count = Math.max(
        0,
        watchNodes[watchCurrentNode[mac].node_mac].count - 1
      );
    }
    watchCurrentNode[mac] = {
      node_type: "LoRa",
      rssi: rssi,
      node_mac: node_mac,
    };
    watchNodes[node_mac] = watchNodes[node_mac] || {
      count: 0,
      node_type: "LoRa",
    };
    watchNodes[node_mac].count++;
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
    count: watchNodes[node_mac]?.count || 0, // Provide a default value of 0 for count
  });

  res.json({ status: "success", message: "Data received" });
});

app.post("/api/ble_data", (req, res) => {
  const data = req.body;
  data.forEach((device) => {
    const { mac_address, rssi } = device;
    if (
      !watchCurrentNode[mac_address] ||
      watchCurrentNode[mac_address].node_type !== "BLE" ||
      rssi > watchCurrentNode[mac_address].rssi
    ) {
      if (
        watchCurrentNode[mac_address] &&
        watchNodes[watchCurrentNode[mac_address].node_mac]
      ) {
        watchNodes[watchCurrentNode[mac_address].node_mac].count = Math.max(
          0,
          (watchNodes[watchCurrentNode[mac_address].node_mac].count || 0) - 1
        );
      }
      watchCurrentNode[mac_address] = {
        node_type: "BLE",
        rssi: rssi,
        node_mac: mac_address,
      };
      watchNodes[mac_address] = watchNodes[mac_address] || {
        count: 0,
        node_type: "BLE",
      };
      watchNodes[mac_address].count++;
    }
    nodeDevices[mac_address] = {
      node_type: "BLE",
      rssi: rssi,
      node_mac: mac_address,
    };
    io.emit("tag_update", {
      node_mac: mac_address,
      node_type: "BLE",
      rssi: rssi,
      count: watchNodes[mac_address]?.count || 0, // Provide a default value of 0 for count
    });
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
