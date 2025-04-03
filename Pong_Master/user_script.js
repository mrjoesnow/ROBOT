document.addEventListener('DOMContentLoaded', () => {
    const socket = new WebSocket('ws://192.168.1.226:81');

    socket.onopen = () => {
        console.log('WebSocket connection opened');
        enableButtons(); // Enable buttons when connected
    };

    let previousLog = {};
    let ballsThrown = 0; // Initialize the balls thrown counter
    let stepsX = 0; // Initialize the steps for X axis
    let stepsY = 0; // Initialize the steps for Y axis
    const stepsToDegreesX = 1 / 31; // Conversion factor from steps to degrees for X axis (31 steps per degree)
    const stepsToDegreesY = 1 / 52; // Conversion factor from steps to degrees for Y axis

    socket.onmessage = (event) => {
        try {
            const data = JSON.parse(event.data);
            if (data.message) {
                addLog(data.message);
            } else if (data.status === 'ready') {
                enableButtons();
            } else {
                updateLogs(data);
            }
        } catch (e) {
            console.log('Received plain text data:', event.data);
            addLog(event.data);
        }
    };

    socket.onclose = (event) => {
        console.log('WebSocket connection closed:', event);
    };

    socket.onerror = (error) => {
        console.error('WebSocket error:', error);
    };

    const powerLevelSlider = document.getElementById('powerLevel');
    const powerLevelValue = document.getElementById('powerLevelValue');
    const ballsThrownValue = document.getElementById('ballsThrownValue'); // Get the balls thrown element
    const panValue = document.getElementById('panValue'); // Get the pan value element
    const tiltValue = document.getElementById('tiltValue'); // Get the tilt value element

    if (powerLevelSlider && powerLevelValue && ballsThrownValue && panValue && tiltValue) {
        let tempPowerLevel = powerLevelSlider.value;

        powerLevelSlider.addEventListener('change', () => {
            tempPowerLevel = powerLevelSlider.value;
            powerLevelValue.textContent = tempPowerLevel;
            sendCommand({ POWER_LEVEL: parseInt(tempPowerLevel) });
        });

        const presetStepsSmallX = 31;
        const presetStepsBigX = 155;
        const presetStepsSmallY = 52;
        const presetStepsBigY = 260;
        const presetSpeedX = 1000;
        const presetSpeedY = 1000;
        const presetSpeedZ = 250;

        document.getElementById('moveXForwardSmall').addEventListener('click', () => {
            stepsX += presetStepsSmallX;
            updatePanTilt();
            sendCommand({ X: 1, STEPS: presetStepsSmallX, SPEED_X: presetSpeedX });
        });
        document.getElementById('moveXBackwardSmall').addEventListener('click', () => {
            stepsX -= presetStepsSmallX;
            updatePanTilt();
            sendCommand({ X: -1, STEPS: presetStepsSmallX, SPEED_X: presetSpeedX });
        });
        document.getElementById('moveYForwardSmall').addEventListener('click', () => {
            stepsY += presetStepsSmallY;
            updatePanTilt();
            sendCommand({ Y: 1, STEPS: presetStepsSmallY, SPEED_Y: presetSpeedY });
        });
        document.getElementById('moveYBackwardSmall').addEventListener('click', () => {
            stepsY -= presetStepsSmallY;
            updatePanTilt();
            sendCommand({ Y: -1, STEPS: presetStepsSmallY, SPEED_Y: presetSpeedY });
        });
        document.getElementById('moveXForwardBig').addEventListener('click', () => {
            stepsX += presetStepsBigX;
            updatePanTilt();
            sendCommand({ X: 1, STEPS: presetStepsBigX, SPEED_X: presetSpeedX });
        });
        document.getElementById('moveXBackwardBig').addEventListener('click', () => {
            stepsX -= presetStepsBigX;
            updatePanTilt();
            sendCommand({ X: -1, STEPS: presetStepsBigX, SPEED_X: presetSpeedX });
        });
        document.getElementById('moveYForwardBig').addEventListener('click', () => {
            stepsY += presetStepsBigY;
            updatePanTilt();
            sendCommand({ Y: 1, STEPS: presetStepsBigY, SPEED_Y: presetSpeedY });
        });
        document.getElementById('moveYBackwardBig').addEventListener('click', () => {
            stepsY -= presetStepsBigY;
            updatePanTilt();
            sendCommand({ Y: -1, STEPS: presetStepsBigY, SPEED_Y: presetSpeedY });
        });
        document.getElementById('throwBall').addEventListener('click', () => {
            sendCommand({
                THROW: true,
                POWER_LEVEL: parseInt(tempPowerLevel),
                BALL_LOADING_STEPS: 4250,
                SPEED_Z: presetSpeedZ
            });
            ballsThrown++; // Increment the balls thrown counter
            ballsThrownValue.textContent = ballsThrown; // Update the balls thrown display
        });
        document.getElementById('testSolenoid').addEventListener('click', () => sendCommand({ TEST_SOLENOID: true }));
        document.getElementById('homeX').addEventListener('click', () => {
            stepsX = 0; // Reset steps for X axis
            updatePanTilt();
            sendCommand({ HOME_X: true });
        });
        document.getElementById('homeY').addEventListener('click', () => {
            stepsY = 0; // Reset steps for Y axis
            updatePanTilt();
            sendCommand({ HOME_Y: true });
        });
        document.getElementById('homeZ').addEventListener('click', () => sendCommand({ HOME_Z: true }));

        function sendCommand(command) {
            if (socket.readyState === WebSocket.OPEN) {
                socket.send(JSON.stringify(command));
                if (command.X || command.Y || command.Z || command.THROW || command.TEST_SOLENOID || command.HOME_X || command.HOME_Y || command.HOME_Z) {
                    disableButtons();
                }
            } else {
                console.error('WebSocket is not open. Ready state:', socket.readyState);
            }
        }

        function addLog(message) {
            const logList = document.getElementById('logList');
            const logItem = document.createElement('li');

            try {
                const currentLog = JSON.parse(message);
                const diff = getDifference(previousLog, currentLog);
                logItem.textContent = diff;
                previousLog = currentLog;
            } catch (e) {
                logItem.textContent = message;
            }

            logList.appendChild(logItem);

            // Scroll to the bottom of the log box
            const logBox = document.getElementById('logBox');
            logBox.scrollTop = logBox.scrollHeight;
        }

        function getDifference(prev, curr) {
            const diff = [];
            for (const key in curr) {
                if (curr.hasOwnProperty(key)) {
                    if (!prev.hasOwnProperty(key) || prev[key] !== curr[key]) {
                        diff.push(`${key}: ${curr[key]}`);
                    }
                }
            }
            return diff.join(', ');
        }

        function updateLogs(data) {
            powerLevelSlider.value = data.powerLevel;
            powerLevelValue.textContent = data.powerLevel;
            document.getElementById('stepsX').textContent = -data.stepsX; // Flip sign for X
            document.getElementById('stepsY').textContent = data.stepsY;
            document.getElementById('stepsZ').textContent = -data.stepsZ; // Flip sign for Z
        }

        function updatePanTilt() {
            panValue.textContent = (stepsX * stepsToDegreesX).toFixed(2); // Update pan value
            tiltValue.textContent = (stepsY * stepsToDegreesY).toFixed(2); // Update tilt value
        }

        function disableButtons() {
            const buttons = document.querySelectorAll('.arrow-buttons button, .home-buttons button');
            buttons.forEach(button => {
                button.disabled = true;
            });
        }

        function enableButtons() {
            const buttons = document.querySelectorAll('.arrow-buttons button, .home-buttons button');
            buttons.forEach(button => {
                button.disabled = false;
            });
        }
    } else {
        console.error('One or more elements are missing from the DOM.');
    }
});