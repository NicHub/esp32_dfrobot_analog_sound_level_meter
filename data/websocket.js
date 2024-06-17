"use strict";

const infoPanel = document.getElementById("infoPanel");
const ip_of_esp_set_manually = [
    "192.168.1.249",
    "192.168.70.85",
    "192.168.70.93",
][1];

init();

/**
 *
 */
async function init() {
    console.log("### START ###");
    get_IP_of_ESP(webSocketHandle);
    createPlotPlotly();
}

/**
 * See https://plotly.com/javascript/
 */
function createPlotPlotly() {

    const layout = {
        title: "SOUND METER",
        yaxis: {
            range: [0, 100],
            tickmode: "linear",
            tick0: 0,
            dtick: 20
        }
    };

    const config = { responsive: true };

    const traces = [{
        y: [],
        mode: "lines+markers",
        opacity: 0.5,
        marker: { color: "orangered", size: 8 },
        name: "RAW SOUND LEVEL (dB)",
        line: { width: 4 }
    },
    {
        y: [],
        mode: "lines+markers",
        opacity: 0.5,
        marker: { color: "mediumseagreen", size: 8 },
        name: "MOVING AVERAGE FILTERED SOUND LEVEL (dB)",
        line: { width: 4 }
    },
    {
        y: [],
        mode: "lines",
        opacity: 1.0,
        marker: { color: "dodgerblue", size: 4 },
        name: "LOW PASS 1 FILTERED SOUND LEVEL (dB)",
        line: { width: 1 }
    },
    {
        y: [],
        mode: "lines",
        opacity: 1.0,
        marker: { color: "black", size: 4 },
        name: "LOW PASS 2 FILTERED SOUND LEVEL (dB)",
        line: { width: 1 }
    },
    {
        y: [],
        mode: "lines+markers",
        opacity: 0.5,
        marker: { color: "red", size: 4 },
        name: "KALMAN FILTERED SOUND LEVEL (dB)",
        line: { width: 16 }
    },
    {
        y: [],
        mode: "lines+markers",
        opacity: 0.5,
        marker: { color: "blue", size: 2 },
        name: "VU METER 1 FILTERED SOUND LEVEL (dB)",
        line: { width: 8 }
    },
    ];

    Plotly.newPlot("dbGraph", traces, layout, config);
}

/**
 *
 */
function get_IP_of_ESP(callback) {
    // Check if the HTML file is served from the ESP
    // or from a development server. This is done by
    // checking if the file `info.json` is present on
    // the server. If it is present, then we are on
    // the ESP. If we are on a development server, the
    // IP address of the ESP must be set manually below.
    const file_url = location.origin + "/info.json";
    const xhr = new XMLHttpRequest();
    xhr.open("GET", file_url, true);
    xhr.send();
    xhr.onload = function () {
        let ip;
        const ans = xhr.status;
        if (ans === 200) {
            // We are on the ESP.
            ip = location.hostname;
            document.title = "ESP32—" + document.title;
        }
        else {
            // We are on the development server.
            // Manually set ESP IP address.
            ip = ip_of_esp_set_manually;
            document.title = "DEV[" + ans + "]—" + document.title;
        }
        console.log("ip = " + ip);
        callback(ip);
    }
}

/**
 *
 */
function webSocketHandle(ip) {

    const ws = new WebSocket("ws://" + ip + "/ws", ["arduino"]);
    let T1 = +new Date();
    let deltaTmax = 0;

    ws.onmessage = function (evt) {

        // Calculate times.
        const T2 = +new Date();
        const deltaTms = T2 - T1;
        deltaTmax = (deltaTms > deltaTmax) ? deltaTms : deltaTmax;
        var refreshRate = 1000 / deltaTms;
        T1 = T2;

        // Parse raw data and display it.
        const data = JSON.parse(evt.data);
        infoPanel.innerHTML =
            `<pre>${evt.data}</pre>` +
            `<pre>Refresh rate = ${refreshRate.toFixed().padStart(2, " ")} Hz` +
            ` (ΔT = ${deltaTms.toFixed().padStart(3, " ")} ms` +
            ` | ΔTmax = ${deltaTmax.toFixed().padStart(3, " ")} ms)</pre>`;

        // Check that the needed properties are present in message.
        // If not display what is missing and leave.
        const needed_properties = ["sound_level_dB"];
        let success = true;
        let message = infoPanel.innerHTML;
        for (let index = 0; index < needed_properties.length; index++) {
            const needed_property = needed_properties[index];
            if (!data.hasOwnProperty(needed_property)) {
                success = false;
                message += `<pre>WebSocket message does not contain <span style="color: tomato; font-style:italic">${needed_property}</span> property</pre>`;
            }
        }
        if (!success) {
            infoPanel.innerHTML = message;
            return;
        }


        let maxPoints = 100;
        Plotly.extendTraces("dbGraph", {
            y: [
                [data.sound_level_dB.raw],
                [data.sound_level_dB.moving_average],
                [data.sound_level_dB.low_pass_1],
                [data.sound_level_dB.low_pass_2],
                [data.sound_level_dB.kalman],
                [data.sound_level_dB.vumeter_1]
            ]
        }, [0, 1, 2, 3, 4, 5], maxPoints);
    };

    ws.onclose = function () {
        infoPanel.innerHTML = "<pre>WebSocket connection is closed</pre>";
    };
}
