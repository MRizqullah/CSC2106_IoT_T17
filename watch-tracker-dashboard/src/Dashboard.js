import React, { useEffect, useState } from "react";
import io from "socket.io-client";

const socket = io("http://localhost:5000");

function Dashboard() {
  const [watchNodes, setWatchNodes] = useState({});

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

    return () => {
      socket.off("connect");
      socket.off("tag_update");
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
    </div>
  );
}

export default Dashboard;
