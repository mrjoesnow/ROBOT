document.addEventListener('DOMContentLoaded', () => {
    console.log('DOM loaded, initializing');
    const logList = document.getElementById('logList');
    let socket;
    let streamKey;
    let stepsX = 0;
    let stepsY = 0;
    let powerLevel = 2500;
    let ballsThrown = 0;
    let previousData = {};

    let droXValue = document.getElementById('droXValue');
    let droYValue = document.getElementById('droYValue');
    if (droXValue) {
        console.log('droXValue found');
        addLog('droXValue found');
    } else {
        console.error('droXValue not found');
        addLog('Error: droXValue not found');
    }
    if (droYValue) {
        console.log('droYValue found');
        addLog('droYValue found');
    } else {
        console.error('droYValue not found');
        addLog('Error: droYValue not found');
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

    function addLog(message) {
        const li = document.createElement('li');
        li.textContent = message;
        logList.appendChild(li);
        logList.scrollTop = logList.scrollHeight;
    }

    function updateValues() {
        const tilt = stepsY * (1 / 52);
        const pan = stepsX * (1 / 31);
        console.log(`Updating values: tilt=${tilt}, pan=${pan}`);

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
            }
            document.getElementById('ballsThrownValue').textContent = ballsThrown;
            addLog(`Balls thrown reset to: ${ballsThrown}`);
            droXValue = document.getElementById('droXValue');
            droYValue = document.getElementById('droYValue');
            if (droXValue) {
                droXValue.textContent = '0';
                addLog('droX initialized to 0');
            }
            if (droYValue) {
                droYValue.textContent = '0';
                addLog('droY initialized to 0');
            }
            updateValues();
        };

        socket.onmessage = (event) => {
            console.log('WebSocket message:', event.data);
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
                    document.getElementById('ballsThrownValue').textContent = data.ballsThrown;
                    addLog(`Balls thrown updated: ${ballsThrown}`);
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

    function loadHlsStream(streamKey) {
        console.log('Loading HLS stream with key:', streamKey);
        const video = document.getElementById('videoStream');
        if (!video || !(video instanceof HTMLVideoElement)) {
            console.error('Video element not found or invalid');
            addLog('Error: Video element not found');
            return;
        }
        if (Hls.isSupported()) {
            const hls = new Hls({
                lowLatencyMode: true,
                maxBufferLength: 2,           // Minimize buffering
                liveSyncDurationCount: 2,     // Tighter sync
                liveMaxLatencyDurationCount: 4,
                enableWorker: false,          // Avoid eval
                enableSoftwareAES: false,     // Disable AES to test CSP
                backBufferLength: 4,          // Match playlist
                maxFragLookUpTolerance: 0.1,
                liveBackBufferLength: 0,
                abrEwmaFastLive: 1.0,
                abrEwmaSlowLive: 3.0,
                debug: true                   // Log hls.js issues
            });
            const hlsUrl = `https://pongrobot.com/hls/${streamKey}.m3u8`;
            console.log('HLS URL:', hlsUrl);
            hls.loadSource(hlsUrl);
            hls.attachMedia(video);
            hls.on(Hls.Events.MANIFEST_PARSED, () => {
                console.log('HLS manifest parsed');
                video.play().catch(err => {
                    console.error('Video play error:', err);
                    addLog(`Video play error: ${err.message}`);
                });
            });
            hls.on(Hls.Events.ERROR, (event, data) => {
                console.error('HLS error:', data);
                addLog(`HLS error: ${data.details}`);
                if (data.type === 'networkError' && !data.fatal) {
                    console.log('Retrying segment load');
                    hls.startLoad();
                }
                if (data.details === 'cspError') {
                    console.error('HLS CSP violation:', data);
                    addLog('HLS CSP violation detected');
                }
            });
        } else if (video.canPlayType('application/vnd.apple.mpegurl')) {
            video.src = `https://pongrobot.com/hls/${streamKey}.m3u8`;
            video.play().catch(err => {
                console.error('Video play error:', err);
                addLog(`Video play error: ${err.message}`);
            });
        } else {
            console.error('HLS not supported');
            addLog('Error: HLS not supported');
        }
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
            document.getElementById('ballsThrownValue').textContent = ballsThrown;
            const ballsThrownSpan = document.querySelector('#throwBall .balls-thrown-value');
            if (ballsThrownSpan) {
                ballsThrownSpan.textContent = ballsThrown;
                addLog(`Balls thrown: ${ballsThrown}`);
            }
        } else {
            console.error('WebSocket not open for throwBall');
            addLog('WebSocket not connected');
        }
    }

    fetch('https://pongrobot.com/api/stream-key')
        .then(response => {
            console.log('Stream key response status:', response.status);
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            console.log('Stream key data:', data);
            if (data.error) {
                throw new Error(data.error);
            }
            streamKey = data.streamKey;
            addLog(`Fetched stream key: ${streamKey}`);
            initializeWebSocket();
            loadHlsStream(streamKey);
        })
        .catch(error => {
            console.error('Error fetching stream key:', error);
            addLog(`Failed to fetch stream key: ${error.message}`);
            streamKey = '23672E6t';
            addLog(`Using fallback stream key: ${streamKey}`);
            initializeWebSocket();
            loadHlsStream(streamKey);
        });

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
            button.addEventListener('click', () => {
                console.log(`Button ${id} clicked`);
                if (socket && socket.readyState === WebSocket.OPEN) {
                    action();
                } else {
                    console.error(`WebSocket not open for ${id} click`);
                    addLog('WebSocket not connected');
                }
            });
        } else {
            console.error(`Button ${id} not found`);
            addLog(`Error: Button ${id} not found`);
        }
    });

    const powerLevelSlider = document.getElementById('powerLevelSlider');
    if (powerLevelSlider) {
        console.log('Attaching listener to powerLevelSlider');
        powerLevelSlider.addEventListener('input', () => {
            powerLevel = parseInt(powerLevelSlider.value);
            document.getElementById('powerLevelValue').textContent = powerLevel;
            addLog(`Power level set to: ${powerLevel}`);
        });
    } else {
        console.error('Power level slider not found');
        addLog('Error: Power level slider not found');
    }
});