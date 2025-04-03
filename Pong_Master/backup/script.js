if (typeof socket === 'undefined') {
    const socket = new WebSocket('ws://192.168.1.222:81');

    socket.onopen = () => {
        console.log('WebSocket connection opened');
    };

    socket.onmessage = (event) => {
        try {
            const data = JSON.parse(event.data);
            if (data.message) {
                addLog(data.message);
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

    document.addEventListener('DOMContentLoaded', () => {
        document.getElementById('moveXForward').addEventListener('click', () => sendCommand({ X: 1 }));
        document.getElementById('moveXBackward').addEventListener('click', () => sendCommand({ X: -1 }));
        document.getElementById('moveYForward').addEventListener('click', () => sendCommand({ Y: 1 }));
        document.getElementById('moveYBackward').addEventListener('click', () => sendCommand({ Y: -1 }));
        document.getElementById('moveZForward').addEventListener('click', () => sendCommand({ Z: 1 }));
        document.getElementById('moveZBackward').addEventListener('click', () => sendCommand({ Z: -1 }));
        document.getElementById('testSolenoid').addEventListener('click', () => sendCommand({ TEST_SOLENOID: true }));
        document.getElementById('homeX').addEventListener('click', () => sendCommand({ HOME_X: true }));
        document.getElementById('homeY').addEventListener('click', () => sendCommand({ HOME_Y: true }));
        document.getElementById('homeZ').addEventListener('click', () => sendCommand({ HOME_Z: true }));

        const powerLevelSlider = document.getElementById('powerLevel');
        const powerLevelValue = document.getElementById('powerLevelValue');

        let tempPowerLevel = powerLevelSlider.value;

        powerLevelSlider.addEventListener('input', () => {
            tempPowerLevel = powerLevelSlider.value;
            powerLevelValue.textContent = tempPowerLevel;
        });

        document.getElementById('throwBall').addEventListener('click', () => {
            sendCommand({ THROW: true, POWER_LEVEL: parseInt(tempPowerLevel) });
        });
    });

    function sendCommand(command) {
        if (socket.readyState === WebSocket.OPEN) {
            socket.send(JSON.stringify(command));
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
        document.getElementById('powerLevel').value = data.powerLevel;
        document.getElementById('powerLevelValue').textContent = data.powerLevel;
        document.getElementById('loadingSpeed').value = data.loadingSpeed;
        document.getElementById('speedX').value = data.speedX;
        document.getElementById('speedY').value = data.speedY;
        document.getElementById('speedZ').value = data.speedZ;
        document.getElementById('homingSpeedX').value = data.homingSpeedX;
        document.getElementById('homingSpeedY').value = data.homingSpeedY;
        document.getElementById('homingSpeedZ').value = data.homingSpeedZ;
    }
}