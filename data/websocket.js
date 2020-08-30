"use strict"
var scene;
var camera;
var renderer;
var axis3d;
var scene3dcontainer = document.getElementById("scene3dcontainer");
var infoPanel = document.getElementById("infoPanel");

{
    console.log("### START ###");
    getIP(webSocketHandle);
    createPlot();
}

function createPlot() {

    var layout = {
        yaxis: {
            range: [0, 100],
            tickmode: "linear",
            tick0: 0,
            dtick: 20
        }
    };

    var config = { responsive: true };

    var traces = [{
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

function getIP(callback) {
    // Check if the HTML file is served from the ESP
    // or from a development server. This is done by
    // checking if the file `info.json` is present on
    // the server. If it is present, then we are on
    // the ESP. If we are on a development server, the
    // IP address of the ESP must be set manually below.
    var file_url = location.origin + "/info.json";
    var xhr = new XMLHttpRequest();
    xhr.open("GET", file_url, true);
    xhr.send();
    xhr.onload = function () {
        var ip;
        var ans = xhr.status;
        if (ans === 200) {
            // We are on the ESP.
            ip = location.hostname;
            document.title = "ESP32—" + document.title;
        }
        else {
            // We are on the development server.
            // Manually set ESP IP address.
            ip = "192.168.88.253";
            document.title = "DEV[" + ans + "]—" + document.title;
        }
        console.log("ip = " + ip);
        callback(ip);
    }
}

function webSocketHandle(ip) {

    var ws = new WebSocket("ws://" + ip + "/ws", ["arduino"]);
    var T1 = +new Date();
    var deltaTmax = 0;

    ws.onmessage = function (evt) {
        var T2 = +new Date();
        var deltaTms = T2 - T1;
        deltaTmax = (deltaTms > deltaTmax) ? deltaTms : deltaTmax;
        var refreshRate = 1000 / deltaTms;
        T1 = T2;

        var data = JSON.parse(evt.data);
        infoPanel.innerHTML =
            `<pre>${evt.data}</pre>` +
            `<pre>Refresh rate = ${refreshRate.toFixed().padStart(2, " ")} Hz` +
            ` (ΔT = ${deltaTms.toFixed().padStart(3, " ")} ms}` +
            ` | ΔTmax = ${deltaTmax.toFixed().padStart(3, " ")} ms)</pre>`;

        if (data.hasOwnProperty("sound_level_dB")) {
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
        }
    };

    ws.onclose = function () {
        infoPanel.innerHTML = "<pre>WebSocket connection is closed</pre>";
    };
}
