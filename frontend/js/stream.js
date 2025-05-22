document.addEventListener("DOMContentLoaded", function() {
    let janus = null;
    let streaming = null;
    let socket;
    let stepsX = 0;
    let stepsY = 0;
    let powerLevel = 2500;
    let ballsThrown = 0;
    let previousData = {};
    const videoElement = document.getElementById("videoStream");
    const logList = document.getElementById('logList');
    let droXValue = document.getElementById('droXValue');
    let droYValue = document.getElementById('droYValue');

    if (!videoElement || !logList) {
        console.error("Video element or log list not found in DOM");
        return;
    }

    function addLog(message) {
        const li = document.createElement('li');
        li.textContent = message;
        logList.appendChild(li);
        logList.scrollTop = logList.scrollHeight;
    }

    const stepSizesX = {
        big: 620,
        medium: 155,
        small: 31
    };

    const stepSizesY = {
        big: 1040,
        medium: 260,
        small: 52
    };

    const limits = {
        tilt: { min: -45, max: 45 },
        pan: { min: -1, max: 89 }
    };

    let playTimeout = null;
    function playVideo() {
        if (videoElement.paused && !playTimeout) {
            console.log("Attempting to play video");
            addLog("Attempting to play video");
            playTimeout = setTimeout(() => {
                videoElement.play().then(() => {
                    console.log("Video playback started");
                    addLog("Video playback started");
                }).catch(e => {
                    console.error("Video play error: ", e);
                    addLog(`Video play error: ${e}`);
                });
                playTimeout = null;
            }, 100);
        } else {
            console.log("Video already playing or play request pending");
            addLog("Video already playing or play request pending");
        }
    }

    function updateValues() {
        const tilt = stepsY * (1 / 52);
        const pan = stepsX * (1 / 31);
        console.log(`Updating values: tilt=${tilt}, pan=${pan}`);
        addLog(`Updating values: tilt=${tilt}, pan=${pan}`);

        document.getElementById('homeX').innerHTML = `Home<br>X: ${Math.round(pan)}°`;
        document.getElementById('homeY').innerHTML = `Home<br>Y: ${Math.round(tilt)}°`;

        const remainingStepsX = {
            forward: Math.max(0, Math.floor((limits.pan.max - pan) * 31)),
            backward: Math.max(0, Math.floor((pan - limits.pan.min) * 31))
        };

        const remainingStepsY = {
            forward: Math.max(0, Math.floor((limits.tilt.max - tilt) * 52)),
            backward: Math.max(0, Math.floor((tilt - limits.tilt.min) * 52))
        };

        document.getElementById('moveXBackwardBig2').style.backgroundColor = remainingStepsX.backward >= stepSizesX.big ? 'rgb(43, 139, 194)' : 'grey';
        document.getElementById('moveXBackwardBig2').disabled = remainingStepsX.backward < stepSizesX.big;
        document.getElementById('moveXBackwardMedium2').style.backgroundColor = remainingStepsX.backward >= stepSizesX.medium ? 'rgb(43, 139, 194)' : 'grey';
        document.getElementById('moveXBackwardMedium2').disabled = remainingStepsX.backward < stepSizesX.medium;
        document.getElementById('moveXBackwardSmall2').style.backgroundColor = remainingStepsX.backward >= stepSizesX.small ? 'rgb(43, 139, 194)' : 'grey';
        document.getElementById('moveXBackwardSmall2').disabled = remainingStepsX.backward < stepSizesX.small;

        document.getElementById('moveXForwardSmall2').style.backgroundColor = remainingStepsX.forward >= stepSizesX.small ? 'rgb(43, 139, 194)' : 'grey';
        document.getElementById('moveXForwardSmall2').disabled = remainingStepsX.forward < stepSizesX.small;
        document.getElementById('moveXForwardMedium2').style.backgroundColor = remainingStepsX.forward >= stepSizesX.medium ? 'rgb(43, 139, 194)' : 'grey';
        document.getElementById('moveXForwardMedium2').disabled = remainingStepsX.forward < stepSizesX.medium;
        document.getElementById('moveXForwardBig2').style.backgroundColor = remainingStepsX.forward >= stepSizesX.big ? 'rgb(43, 139, 194)' : 'grey';
        document.getElementById('moveXForwardBig2').disabled = remainingStepsX.forward < stepSizesX.big;

        document.getElementById('moveYBackwardBig').style.backgroundColor = remainingStepsY.backward >= stepSizesY.big ? 'rgb(43, 139, 194)' : 'grey';
        document.getElementById('moveYBackwardBig').disabled = remainingStepsY.backward < stepSizesY.big;
        document.getElementById('moveYBackwardMedium').style.backgroundColor = remainingStepsY.backward >= stepSizesY.medium ? 'rgb(43, 139, 194)' : 'grey';
        document.getElementById('moveYBackwardMedium').disabled = remainingStepsY.backward < stepSizesY.medium;
        document.getElementById('moveYBackwardSmall').style.backgroundColor = remainingStepsY.backward >= stepSizesY.small ? 'rgb(43, 139, 194)' : 'grey';
        document.getElementById('moveYBackwardSmall').disabled = remainingStepsY.backward < stepSizesY.small;

        document.getElementById('moveYForwardSmall').style.backgroundColor = remainingStepsY.forward >= stepSizesY.small ? 'rgb(43, 139, 194)' : 'grey';
        document.getElementById('moveYForwardSmall').disabled = remainingStepsY.forward < stepSizesY.small;
        document.getElementById('moveYForwardMedium').style.backgroundColor = remainingStepsY.forward >= stepSizesY.medium ? 'rgb(43, 139, 194)' : 'grey';
        document.getElementById('moveYForwardMedium').disabled = remainingStepsY.forward < stepSizesY.medium;
        document.getElementById('moveYForwardBig').style.backgroundColor = remainingStepsY.forward >= stepSizesY.big ? 'rgb(43, 139, 194)' : 'grey';
        document.getElementById('moveYForwardBig').disabled = remainingStepsY.forward < stepSizesY.big;
    }

    function initializeWebSocket() {
        console.log('Attempting WebSocket connection to wss://pongrobot.com/ws');
        addLog('Attempting WebSocket connection');
        socket = new WebSocket('wss://pongrobot.com/ws');

        socket.onopen = () => {
            console.log('WebSocket opened');
            addLog('WebSocket connected');
            powerLevel = 2500;
            ballsThrown = 0;
            const powerLevelSlider = document.getElementById('powerLevelSlider');
            if (powerLevelSlider) {
                powerLevelSlider.value = powerLevel;
                document.getElementById('powerLevelValue').textContent = powerLevel;
                addLog(`Power level set to: ${powerLevel}`);
            } else {
                console.error('Power level slider not found');
                addLog('Power level slider not found');
            }
            const ballsThrownElement = document.getElementById('ballsThrownValue');
            if (ballsThrownElement) {
                ballsThrownElement.textContent = ballsThrown;
                addLog(`Balls thrown reset to: ${ballsThrown}`);
            } else {
                console.error('Balls thrown element not found');
                addLog('Balls thrown element not found');
            }
            droXValue = document.getElementById('droXValue');
            droYValue = document.getElementById('droYValue');
            if (droXValue) {
                droXValue.textContent = '0';
                addLog('droX initialized to 0');
            } else {
                console.error('droX element not found');
                addLog('droX element not found');
            }
            if (droYValue) {
                droYValue.textContent = '0';
                addLog('droY initialized to 0');
            } else {
                console.error('droY element not found');
                addLog('droY element not found');
            }
            updateValues();
        };

        socket.onmessage = (event) => {
            console.log('WebSocket message:', event.data);
            addLog(`WebSocket message: ${event.data}`);
            try {
                const data = JSON.parse(event.data);
                addLog(`Received: ${event.data}`);

                if (data.droX !== undefined) {
                    stepsX = data.droX;
                    if (!droXValue) droXValue = document.getElementById('droXValue');
                    if (droXValue) {
                        droXValue.textContent = data.droX;
                        addLog(`droX updated: ${data.droX}`);
                    }
                }
                if (data.droY !== undefined) {
                    stepsY = data.droY;
                    if (!droYValue) droYValue = document.getElementById('droYValue');
                    if (droYValue) {
                        droYValue.textContent = data.droY;
                        addLog(`droY updated: ${data.droY}`);
                    }
                }
                if (data.powerLevel !== undefined) {
                    powerLevel = data.powerLevel;
                    const powerLevelSlider = document.getElementById('powerLevelSlider');
                    if (powerLevelSlider) {
                        powerLevelSlider.value = powerLevel;
                        document.getElementById('powerLevelValue').textContent = powerLevel;
                        addLog(`Power level updated: ${powerLevel}`);
                    }
                }
                if (data.ballsThrown !== undefined) {
                    ballsThrown = data.ballsThrown;
                    const ballsThrownElement = document.getElementById('ballsThrownValue');
                    if (ballsThrownElement) {
                        ballsThrownElement.textContent = data.ballsThrown;
                        addLog(`Balls thrown updated: ${ballsThrown}`);
                    }
                }
                if (data.status === 'ready') {
                    addLog('Robot ready');
                }

                updateValues();

                for (const key in data) {
                    if (data[key] !== previousData[key]) {
                        addLog(`${key}: ${data[key]}`);
                    }
                }
                previousData = { ...data };
            } catch (e) {
                console.error('Error parsing WebSocket message:', e);
                addLog(`Error parsing WebSocket message: ${e.message}`);
            }
        };

        socket.onerror = (error) => {
            console.error('WebSocket error:', error);
            addLog('WebSocket error');
        };

        socket.onclose = () => {
            console.log('WebSocket closed');
            addLog('WebSocket disconnected');
            setTimeout(initializeWebSocket, 5000);
        };
    }

    function moveAxis(axis, direction, stepSize) {
        const message = {
            [axis]: direction * stepSize,
            STEPS: stepSize
        };
        if (socket && socket.readyState === WebSocket.OPEN) {
            socket.send(JSON.stringify(message));
            addLog(`Sent: ${JSON.stringify(message)}`);
        } else {
            console.error(`WebSocket not open for moveAxis ${axis}`);
            addLog('WebSocket not connected');
        }
    }

    function homeAxis(axis) {
        const message = {
            [`HOME_${axis}`]: true
        };
        if (socket && socket.readyState === WebSocket.OPEN) {
            socket.send(JSON.stringify(message));
            addLog(`Sent: ${JSON.stringify(message)}`);
        } else {
            console.error(`WebSocket not open for homeAxis ${axis}`);
            addLog('WebSocket not connected');
        }
    }

    function throwBall() {
        const powerLevelSlider = document.getElementById('powerLevelSlider');
        const powerLevelValue = parseInt(powerLevelSlider.value, 10);
        const ballLoadingSteps = 4200;
        const message = {
            THROW: true,
            POWER_LEVEL: powerLevelValue,
            BALL_LOADING_STEPS: ballLoadingSteps,
            SPEED_Z: 280
        };
        if (socket && socket.readyState === WebSocket.OPEN) {
            socket.send(JSON.stringify(message));
            addLog(`Sent: ${JSON.stringify(message)}`);
            ballsThrown++;
            const ballsThrownElement = document.getElementById('ballsThrownValue');
            if (ballsThrownElement) {
                ballsThrownElement.textContent = ballsThrown;
                addLog(`Balls thrown: ${ballsThrown}`);
            }
            const ballsThrownSpan = document.querySelector('#throwBall .balls-thrown-value');
            if (ballsThrownSpan) {
                ballsThrownSpan.textContent = ballsThrown;
                addLog(`Balls thrown: ${ballsThrown}`);
            }
            const ballsThrownCount = document.getElementById('ballsThrownCount');
            if (ballsThrownCount) {
                ballsThrownCount.textContent = ballsThrown;
            }
        } else {
            console.error('WebSocket not open for throwBall');
            addLog('WebSocket not connected');
        }
    }

    Janus.init({
        debug: "all",
        callback: function() {
            if (!Janus.isWebrtcSupported()) {
                console.error("WebRTC not supported in this browser");
                addLog("WebRTC not supported in this browser");
                return;
            }
            console.log("Janus initialized successfully");
            addLog("Janus initialized successfully");

            function connectJanus() {
                if (janus) {
                    janus.destroy();
                    janus = null;
                }
                janus = new Janus({
                    server: "https://pongrobot.com/janus",
                    success: function() {
                        console.log("Connected to Janus server");
                        addLog("Connected to Janus server");
                        janus.attach({
                            plugin: "janus.plugin.streaming",
                            success: function(pluginHandle) {
                                console.log("Attached to streaming plugin");
                                addLog("Attached to streaming plugin");
                                streaming = pluginHandle;
                                sendWatchRequest();
                            },
                            error: function(error) {
                                console.error("Error attaching to plugin: ", error);
                                addLog(`Error attaching to plugin: ${error}`);
                                setTimeout(connectJanus, 5000); // Changed from 1000ms to 5000ms
                            },
                            onmessage: function(msg, jsep) {
                                console.log("Received message: ", JSON.stringify(msg));
                                addLog(`Received message: ${JSON.stringify(msg)}`);
                                if (msg.error) {
                                    console.error("Plugin error: ", msg.error, "Code: ", msg.error_code);
                                    addLog(`Plugin error: ${msg.error} (Code: ${msg.error_code})`);
                                    if (msg.error_code === 453 || msg.error_code === 458) {
                                        console.warn("Retrying connection due to error: ", msg.error);
                                        addLog(`Retrying connection due to error: ${msg.error}`);
                                        setTimeout(connectJanus, 5000); // Changed from 1000ms to 5000ms
                                    }
                                    return;
                                }
                                if (msg.streaming === "event" && msg.result && msg.result.status) {
                                    console.log("Streaming status: ", msg.result.status);
                                    addLog(`Streaming status: ${msg.result.status}`);
                                }
                                if (jsep) {
                                    console.log("Handling JSEP: ", JSON.stringify(jsep));
                                    addLog(`Handling JSEP: ${JSON.stringify(jsep)}`);
                                    streaming.createAnswer({
                                        jsep: jsep,
                                        media: { audioSend: false, videoSend: false, audioRecv: true, videoRecv: true },
                                        tracks: [
                                            { type: "audio", recv: true, mid: "0", simulcast: false },
                                            { type: "video", recv: true, mid: "1", simulcast: false }
                                        ],
                                        customizeSdp: function(jsep) {
                                            // Optimize SDP for low latency
                                            jsep.sdp = jsep.sdp.replace(/a=rtcp-fb:.*\n/g, ''); // Disable RTCP feedback
                                            jsep.sdp = jsep.sdp.replace(/a=fmtp:.*\n/g, 'a=fmtp:111 minptime=10;useinbandfec=1\n'); // Low-latency Opus settings
                                        },
                                        success: function(jsep) {
                                            console.log("Created answer: ", JSON.stringify(jsep));
                                            addLog(`Created answer: ${JSON.stringify(jsep)}`);
                                            streaming.send({
                                                message: { request: "start", id: 1 },
                                                jsep: jsep,
                                                success: function() {
                                                    console.log("Stream started");
                                                    addLog("Stream started");
                                                },
                                                error: function(error) {
                                                    console.error("Start error: ", error);
                                                    addLog(`Start error: ${error}`);
                                                }
                                            });
                                        },
                                        error: function(error) {
                                            console.error("Error creating answer: ", error);
                                            addLog(`Error creating answer: ${error}`);
                                        }
                                    });
                                }
                            },
                            onremotestream: function(stream) {
                                console.log("Received remote stream: ", stream);
                                addLog("Received remote stream");
                                Janus.attachMediaStream(videoElement, stream);
                                console.log("Attached stream to video element");
                                addLog("Attached stream to video element");

                                const tracks = stream.getTracks();
                                tracks.forEach(track => {
                                    console.log(`${track.kind} track: id=${track.id}, enabled=${track.enabled}, muted=${track.muted}, readyState=${track.readyState}`);
                                    addLog(`${track.kind} track: id=${track.id}, enabled=${track.enabled}, muted=${track.muted}, readyState=${track.readyState}`);
                                    if (track.muted) {
                                        console.warn(`${track.kind} track still muted after 2s, retrying playback`);
                                        addLog(`${track.kind} track muted, retrying in 2s`);
                                        setTimeout(() => {
                                            if (track.muted) playVideo();
                                        }, 2000);
                                    }
                                });

                                // Reduce WebRTC jitter buffer
                                videoElement.bufferedAmountLowThreshold = 0.01; // Experimental, reduces client-side buffering

                                const prompt = document.createElement('div');
                                prompt.style.position = 'absolute';
                                prompt.style.top = '50%';
                                prompt.style.left = '50%';
                                prompt.style.transform = 'translate(-50%, -50%)';
                                prompt.style.background = 'rgba(0, 0, 0, 0.7)';
                                prompt.style.color = 'white';
                                prompt.style.padding = '10px';
                                prompt.style.borderRadius = '5px';
                                prompt.textContent = 'Click video to start playback';
                                videoElement.parentElement.appendChild(prompt);
                                videoElement.addEventListener('click', () => {
                                    prompt.remove();
                                    playVideo();
                                }, { once: true });

                                videoElement.addEventListener('loadedmetadata', function () {
                                    const videoWidth = videoElement.videoWidth;
                                    const videoHeight = videoElement.videoHeight;
                                    const aspectRatio = videoWidth / videoHeight;

                                    const streamBox = videoElement.parentElement;
                                    streamBox.style.aspectRatio = `${videoWidth}/${videoHeight}`;
                                    streamBox.style.width = '100%';
                                    streamBox.style.maxWidth = `${videoWidth}px`;
                                    videoElement.style.width = '100%';
                                    videoElement.style.height = 'auto';

                                    console.log(`Stream resolution: ${videoWidth}x${videoHeight}, Aspect ratio: ${aspectRatio}`);
                                    addLog(`Stream resolution: ${videoWidth}x${videoHeight}`);
                                }, { once: true });

                                window.addEventListener('resize', function () {
                                    const streamBox = videoElement.parentElement;
                                    streamBox.style.width = '100%';
                                    videoElement.style.width = '100%';
                                    videoElement.style.height = 'auto';
                                });

                                playVideo();

                                stream.getTracks().forEach(track => {
                                    track.onmute = () => {
                                        console.log(`${track.kind} track muted`);
                                        addLog(`${track.kind} track muted`);
                                        if (videoElement.paused) {
                                            console.warn("Track muted and video paused, awaiting user interaction");
                                            addLog("Track muted and video paused");
                                        }
                                    };
                                    track.onunmute = () => {
                                        console.log(`${track.kind} track unmuted`);
                                        addLog(`${track.kind} track unmuted`);
                                        if (videoElement.paused) {
                                            console.log("Retrying playback after unmute");
                                            addLog("Retrying playback after unmute");
                                            playVideo();
                                        }
                                    };
                                    track.onended = () => {
                                        console.log(`${track.kind} track ended`);
                                        addLog(`${track.kind} track ended`);
                                    };
                                });
                            },
                            oncleanup: function() {
                                console.log("Stream stopped");
                                addLog("Stream stopped");
                                videoElement.srcObject = null;
                                setTimeout(connectJanus, 5000); // Changed from 1000ms to 5000ms
                            },
                            iceState: function(state) {
                                console.log("ICE connection state: ", state);
                                addLog(`ICE connection state: ${state}`);
                                if (state === "disconnected" || state === "failed") {
                                    console.warn("ICE failure, retrying connection");
                                    addLog("ICE failure, retrying connection");
                                    setTimeout(connectJanus, 5000); // Changed from 1000ms to 5000ms
                                }
                            },
                            mediaState: function(medium, on, mid) {
                                console.log(`Media ${medium} ${on ? 'started' : 'stopped'} for mid: ${mid}`);
                                addLog(`Media ${medium} ${on ? 'started' : 'stopped'} for mid: ${mid}`);
                                if (!on) {
                                    console.warn(`Media ${medium} stopped, retrying watch request`);
                                    addLog(`Media ${medium} stopped, retrying watch request`);
                                    sendWatchRequest();
                                }
                            },
                            webrtcState: function(on) {
                                console.log("WebRTC PeerConnection is ", on ? "up" : "down");
                                addLog(`WebRTC PeerConnection is ${on ? "up" : "down"}`);
                                if (!on) {
                                    console.warn("WebRTC down, retrying connection");
                                    addLog("WebRTC down, retrying connection");
                                    setTimeout(connectJanus, 5000); // Changed from 1000ms to 5000ms
                                }
                            },
                            onicecandidate: function(candidate) {
                                console.log("Local ICE candidate: ", candidate);
                                addLog(`Local ICE candidate: ${JSON.stringify(candidate)}`);
                            },
                            oniceconnectionstatechange: function(state) {
                                console.log("ICE connection state changed to: ", state);
                                addLog(`ICE connection state changed to: ${state}`);
                                if (state === "failed" || state === "disconnected" || state === "closed") {
                                    console.warn("ICE connection state critical: ", state, ", retrying connection");
                                    addLog(`ICE connection state critical: ${state}, retrying connection`);
                                    setTimeout(connectJanus, 5000); // Changed from 1000ms to 5000ms
                                }
                            },
                            ontrack: function(event) {
                                console.log("WebRTC ontrack event: ", event);
                                addLog(`WebRTC ontrack event: ${JSON.stringify(event)}`);
                            }
                        });
                    },
                    error: function(error) {
                        console.error("Error connecting to Janus: ", error);
                        addLog(`Error connecting to Janus: ${error}`);
                        setTimeout(connectJanus, 5000);
                    }
                });
            }

            function sendWatchRequest() {
                if (!streaming) {
                    console.error("Streaming handle not initialized");
                    addLog("Streaming handle not initialized");
                    return;
                }
                console.log("Sending watch request for stream ID 1");
                addLog("Sending watch request for stream ID 1");
                streaming.send({
                    message: { 
                        request: "watch", 
                        id: 1,
                        secret: "23672E6t"
                    },
                    success: function() {
                        console.log("Watch request sent successfully");
                        addLog("Watch request sent successfully");
                    },
                    error: function(error) {
                        console.error("Watch request error: ", error);
                        addLog(`Watch request error: ${error}`);
                        setTimeout(sendWatchRequest, 5000);
                    }
                });
                setInterval(() => {
                    if (janus) { // Changed to janus to avoid sending keep-alives with invalid handle
                        console.log("Sending keep-alive");
                        addLog("Sending keep-alive");
                        janus.getInfo({ // Changed to getInfo for proper session keep-alive
                            success: () => {
                                console.log("Keep-alive sent");
                                addLog("Keep-alive sent");
                            },
                            error: (error) => {
                                console.error("Keep-alive error: ", error);
                                addLog(`Keep-alive error: ${error}`);
                                setTimeout(connectJanus, 5000);
                            }
                        });
                    }
                }, 30000); // Changed from 5000ms to 30000ms
            }

            connectJanus();
            initializeWebSocket();

            const buttons = [
                { id: 'moveXForwardBig2', action: () => moveAxis('X', 1, stepSizesX.big) },
                { id: 'moveXForwardMedium2', action: () => moveAxis('X', 1, stepSizesX.medium) },
                { id: 'moveXForwardSmall2', action: () => moveAxis('X', 1, stepSizesX.small) },
                { id: 'moveXBackwardBig2', action: () => moveAxis('X', -1, stepSizesX.big) },
                { id: 'moveXBackwardMedium2', action: () => moveAxis('X', -1, stepSizesX.medium) },
                { id: 'moveXBackwardSmall2', action: () => moveAxis('X', -1, stepSizesX.small) },
                { id: 'moveYForwardBig', action: () => moveAxis('Y', 1, stepSizesY.big) },
                { id: 'moveYForwardMedium', action: () => moveAxis('Y', 1, stepSizesY.medium) },
                { id: 'moveYForwardSmall', action: () => moveAxis('Y', 1, stepSizesY.small) },
                { id: 'moveYBackwardBig', action: () => moveAxis('Y', -1, stepSizesY.big) },
                { id: 'moveYBackwardMedium', action: () => moveAxis('Y', -1, stepSizesY.medium) },
                { id: 'moveYBackwardSmall', action: () => moveAxis('Y', -1, stepSizesY.small) },
                { id: 'homeX', action: () => homeAxis('X') },
                { id: 'homeY', action: () => homeAxis('Y') },
                { id: 'throwBall', action: throwBall }
            ];

            buttons.forEach(({ id, action }) => {
                const button = document.getElementById(id);
                if (button) {
                    console.log(`Attaching listener to button: ${id}`);
                    addLog(`Attaching listener to button: ${id}`);
                    button.addEventListener('click', () => {
                        console.log(`Button ${id} clicked`);
                        addLog(`Button ${id} clicked`);
                        if (socket && socket.readyState === WebSocket.OPEN) {
                            action();
                        } else {
                            console.error(`WebSocket not open for ${id} click`);
                            addLog('WebSocket not connected');
                        }
                    });
                } else {
                    console.error(`Button ${id} not found`);
                    addLog(`Button ${id} not found`);
                }
            });

            const powerLevelSlider = document.getElementById('powerLevelSlider');
            if (powerLevelSlider) {
                console.log('Attaching listener to powerLevelSlider');
                addLog('Attaching listener to powerLevelSlider');
                powerLevelSlider.addEventListener('input', () => {
                    powerLevel = parseInt(powerLevelSlider.value);
                    document.getElementById('powerLevelValue').textContent = powerLevel;
                    addLog(`Power level set to: ${powerLevel}`);
                });
            } else {
                console.error('Power level slider not found');
                addLog('Power level slider not found');
            }
        }
    });
});