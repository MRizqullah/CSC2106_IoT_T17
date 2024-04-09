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
        const updatedWatchNodes = { ...prevWatchNodes };
        if (data.count === 0) {
          delete updatedWatchNodes[data.node_mac]; // Remove node if no devices are connected
        } else {
          updatedWatchNodes[data.node_mac] = {
            type: data.node_type,
            rssi: data.rssi,
            nodeMac: data.node_mac,
            count: data.count,
          };
        }
        return updatedWatchNodes;
      });
    });

    socket.on("connection_update", (data) => {
      setConnectionsLog((prevLog) => [...prevLog, data]);
      setActiveTaggings((prevTaggings) => {
        const updatedTaggings = { ...prevTaggings };
        Object.entries(data).forEach(([mac, details]) => {
          if (details.rssi === 0) {
            // Remove disconnected watch from active taggings
            delete updatedTaggings[mac];
          } else {
            updatedTaggings[details.node_mac] = { mac, ...details };
          }
        });
        return updatedTaggings;
      });
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
        {Object.entries(activeTaggings).map(([nodeMac, details]) => (
          <div key={nodeMac}>
            MAC: {details.mac}, Node MAC: {nodeMac}, RSSI: {details.rssi}
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
