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
          [data.mac_address]: {
            ...data,
            type: data.node_type,
            rssi: data.rssi,
            nodeMac: data.node_mac,
          },
        };
        console.log("Updated watchNodes:", updatedWatchNodes);
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
        {Object.entries(watchNodes).map(([mac, node]) => (
          <div
            key={mac}
            className={`node ${node.type.toLowerCase()}`}
            data-mac={mac}
          >
            {mac}
            <div className="device-count">
              Count: {/* Add device count logic here */}
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}

export default Dashboard;
