function changeTemp(delta) {
            const newTemp = deviceStatus.targetTemp + delta;
            if (newTemp >= 0 && newTemp <= 300) {
                setTargetTemp(newTemp);
            }