document.addEventListener('DOMContentLoaded', () => {
    const socket = new WebSocket('ws://192.168.1.226:81');

    socket.onopen = () => {
        console.log('WebSocket connection opened');
        enableButtons(); // Enable buttons when connected
    };

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
    const stepSizeSlider = document.getElementById('stepSize');
    const stepSizeValue = document.getElementById('stepSizeValue');
    const speedXSlider = document.getElementById('speedX');
    const speedXValue = document.getElementById('speedXValue');
    const speedYSlider = document.getElementById('speedY');
    const speedYValue = document.getElementById('speedYValue');
    const speedZSlider = document.getElementById('speedZ');
    const speedZValue = document.getElementById('speedZValue');
    const ballLoadingStepsSlider = document.getElementById('ballLoadingSteps');
    const ballLoadingStepsValue = document.getElementById('ballLoadingStepsValue');

    let tempPowerLevel = powerLevelSlider.value;
    let tempStepSize = stepSizeSlider.value;
    let tempSpeedX = speedXSlider.value;
    let tempSpeedY = speedYSlider.value;
    let tempSpeedZ = speedZSlider.value;
    let tempBallLoadingSteps = ballLoadingStepsSlider.value;

    powerLevelSlider.addEventListener('input', () => {
        tempPowerLevel = powerLevelSlider.value;
        powerLevelValue.textContent = tempPowerLevel;
        sendCommand({ POWER_LEVEL: parseInt(tempPowerLevel) });
    });

    stepSizeSlider.addEventListener('input', () => {
        tempStepSize = stepSizeSlider.value;
        stepSizeValue.textContent = tempStepSize;
        sendCommand({ STEPS: parseInt(tempStepSize) });
    });

    speedXSlider.addEventListener('input', () => {
        tempSpeedX = speedXSlider.value;
        speedXValue.textContent = tempSpeedX;
        sendCommand({ SPEED_X: parseInt(tempSpeedX) });
    });

    speedYSlider.addEventListener('input', () => {
        tempSpeedY = speedYSlider.value;
        speedYValue.textContent = tempSpeedY;
        sendCommand({ SPEED_Y: parseInt(tempSpeedY) });
    });

    speedZSlider.addEventListener('input', () => {
        tempSpeedZ = speedZSlider.value;
        speedZValue.textContent = tempSpeedZ;
        sendCommand({ SPEED_Z: parseInt(tempSpeedZ) });
    });

    ballLoadingStepsSlider.addEventListener('input', () => {
        tempBallLoadingSteps = ballLoadingStepsSlider.value;
        ballLoadingStepsValue.textContent = tempBallLoadingSteps;
        sendCommand({ BALL_LOADING_STEPS: parseInt(tempBallLoadingSteps) });
    });

    document.getElementById('moveXForward').addEventListener('click', () => sendCommand({ X: 1, STEPS: parseInt(tempStepSize), SPEED_X: parseInt(tempSpeedX) }));
    document.getElementById('moveXBackward').addEventListener('click', () => sendCommand({ X: -1, STEPS: parseInt(tempStepSize), SPEED_X: parseInt(tempSpeedX) }));
    document.getElementById('moveYForward').addEventListener('click', () => sendCommand({ Y: 1, STEPS: parseInt(tempStepSize), SPEED_Y: parseInt(tempSpeedY) }));
    document.getElementById('moveYBackward').addEventListener('click', () => sendCommand({ Y: -1, STEPS: parseInt(tempStepSize), SPEED_Y: parseInt(tempSpeedY) }));
    document.getElementById('moveZForward').addEventListener('click', () => sendCommand({ Z: 1, STEPS: parseInt(tempStepSize), SPEED_Z: parseInt(tempSpeedZ) }));
    document.getElementById('moveZBackward').addEventListener('click', () => sendCommand({ Z: -1, STEPS: parseInt(tempStepSize), SPEED_Z: parseInt(tempSpeedZ) }));
    document.getElementById('throwBall').addEventListener('click', () => sendCommand({ THROW: true, POWER_LEVEL: parseInt(tempPowerLevel), BALL_LOADING_STEPS: parseInt(tempBallLoadingSteps), SPEED_Z: parseInt(tempSpeedZ) }));
    document.getElementById('testSolenoid').addEventListener('click', () => sendCommand({ TEST_SOLENOID: true }));
    document.getElementById('homeX').addEventListener('click', () => sendCommand({ HOME_X: true }));
    document.getElementById('homeY').addEventListener('click', () => sendCommand({ HOME_Y: true }));
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
        logItem.textContent = message;
        logList.appendChild(logItem);
        // Scroll to the bottom of the log box
        const logBox = document.getElementById('logBox');
        logBox.scrollTop = logBox.scrollHeight;
    }

    function updateLogs(data) {
        document.getElementById('stepSize').value = data.stepSize;
        document.getElementById('stepSizeValue').textContent = data.stepSize;
        document.getElementById('powerLevel').value = data.powerLevel;
        document.getElementById('powerLevelValue').textContent = data.powerLevel;
        document.getElementById('speedX').value = data.speedX;
        document.getElementById('speedXValue').textContent = data.speedX;
        document.getElementById('speedY').value = data.speedY;
        document.getElementById('speedYValue').textContent = data.speedY;
        document.getElementById('speedZ').value = data.speedZ;
        document.getElementById('speedZValue').textContent = data.speedZ;
        document.getElementById('ballLoadingSteps').value = data.ballLoadingSteps;
        document.getElementById('ballLoadingStepsValue').textContent = data.ballLoadingSteps;
        document.getElementById('stepsX').textContent = -data.stepsX; // Flip sign for X
        document.getElementById('stepsY').textContent = data.stepsY;
        document.getElementById('stepsZ').textContent = -data.stepsZ; // Flip sign for Z
    }

    function disableButtons() {
        const buttons = document.querySelectorAll('.controls button');
        buttons.forEach(button => {
            button.disabled = true;
        });
    }

    function enableButtons() {
        const buttons = document.querySelectorAll('.controls button');
        buttons.forEach(button => {
            button.disabled = false;
        });
    }
});