"use strict";

const infoPanel = document.getElementById("infoPanel");
const ip_of_esp_set_manually = "192.168.1.249";
init();

/**
 *
 */
async function init() {
    console.log("### START ###");

    let IP = await get_IP_of_ESP();
    console.log(IP);

    // getIP(webSocketHandle);
    // createPlotPlotly();
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
async function get_IP_of_ESP() {
    /*
    Get the IP of the ESP.

    There are two possible cases:

    1. The web server used is the one on the ESP:
       In this case, we can know the ESP IP address
       by reading `location.hostname`.

    2. The files are served from another web server:
       In this case we cannot know the ESP IP address,
       the user has to enter it manually.

    If the file `info.json` can be found, then we are on the ESP,
    otherwise, we are on another server.
    */
    const file_url = `${location.origin}/info.json`;

    let response = await fetch(file_url);
    let ip_of_esp;
    if (response.status === 200) {
        // We are on the ESP.
        ip_of_esp = location.hostname;
        document.title = "ESP32—" + document.title;
    }
    else {
        // We are on the development server.
        // Manually set ESP IP address.
        ip_of_esp = ip_of_esp_set_manually;
        document.title = "DEV " + document.title;
    }

    return ip_of_esp;
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
