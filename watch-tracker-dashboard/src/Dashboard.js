import React, { useEffect, useState } from "react";
import io from "socket.io-client";

const socket = io("http://localhost:5000");

function Dashboard() {
  const [watchNodes, setWatchNodes] = useState({});
  const [connectionsLog, setConnectionsLog] = useState([]);
  const [activeTaggings, setActiveTaggings] = useState({});

  useEffect(() => {
    socket.on("connect", () => {
      console.log("Connected to server");
    });

    socket.on("tag_update", (data) => {
      setWatchNodes((prevWatchNodes) => {
        const updatedWatchNodes = {
          ...prevWatchNodes,
          [data.node_mac]: {
            type: data.node_type,
            rssi: data.rssi,
            nodeMac: data.node_mac,
            count: data.count,
          },
        };
        return updatedWatchNodes;
      });
    });

    const getNodePosition = (nodeMac) => {
      // Simulated grid positions for demonstration
      const positions = {
        "00:11:22:33:44:55": { row: 1, col: 1 },
        "11:22:33:44:55:66": { row: 2, col: 2 },
        // Add more nodeMac to grid positions mapping as needed
      };

      return positions[nodeMac] || { row: 1, col: 1 }; // Default position
    };

    socket.on("connection_update", (data) => {
      setConnectionsLog((prevLog) => [...prevLog, data]);
      setActiveTaggings(data);
    });

    return () => {
      socket.off("connect");
      socket.off("tag_update");
      socket.off("connection_update");
    };
  }, []);

  return (
    <div>
      <h1>Watch Tracker Dashboard</h1>
      <div id="map">
        {Object.entries(watchNodes).map(([nodeMac, node]) => (
          <div
            key={nodeMac}
            className={`node ${node.type.toLowerCase()}`}
            data-mac={nodeMac}
          >
            Node MAC: {nodeMac}
            <div className="device-count">Count: {node.count || 0}</div>
          </div>
        ))}
      </div>
      <div id="active-taggings">
        <h2>Active Taggings</h2>
        {Object.entries(activeTaggings).map(([mac, details]) => (
          <div key={mac}>
            MAC: {mac}, Node MAC: {details.node_mac}, RSSI: {details.rssi}
          </div>
        ))}
      </div>
      <h2>Connections Log</h2>
      <div
        id="connections-log"
        style={{ maxHeight: "200px", overflowY: "scroll" }}
      >
        {connectionsLog
          .slice()
          .reverse()
          .map((entry, index) => (
            <div key={index}>
              {Object.entries(entry).map(([mac, details]) => (
                <div key={mac}>
                  MAC: {mac}, Node MAC: {details.node_mac}, RSSI: {details.rssi}
                </div>
              ))}
            </div>
          ))}
      </div>
    </div>
  );
}

export default Dashboard;
