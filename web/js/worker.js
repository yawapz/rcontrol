'use strict';

const loc = String(document.location);
let workerId = document.getElementById('workerId');
workerId = loc.substring(loc.indexOf('#') + 1);

function normalaizeSpeed(speed)
{
    let transformSpeed = speed;
    let value;

    if(speed >= 1000 && speed < 1000000)
    {
        transformSpeed /= 1000;
        value = "KH";
    }
    else if(speed >= 100000 && speed < 1000000000)
    {
        transformSpeed /= 1000000;
        value = "MH";
    }
    else if(speed >= 100000000 && speed < 1000000000000)
    {
        transformSpeed /= 1000000000;
        value = "GH";
    }
    else
        value = "H";

    if (transformSpeed)
        return String(`${transformSpeed.toFixed(1)} ${value}`);
    else 
        return '-/-'; 
}

function uuidv4() 
{
    return "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx".replace(
    /[xy]/g,
    function (c) {
        var r = (Math.random() * 16) | 0,
        v = c === "x" ? r : (r & 0x3) | 0x8;
        return v.toString(16);
    }
    );
}

function load()
{
    const workers = JSON.parse(localStorage.getItem('userData')).workersData;
    let workerName = '';
    let worker;

    for (const w of workers)
    {
        if (w.id == workerId)
        {
            workerName = w.workerName;
            worker = w;
            if (!workerName)
                workerName = (w.id).substring(0, 8);
            break;
        }
    }
    document.title = `Worker ${workerName}`;
    const workerNameBlock = document.getElementById('workerName');
    workerNameBlock.innerHTML = workerName;

    function clipboard(event)
    {
        navigator.clipboard.writeText(event.target.value);
    }

    if (worker)
    {
        // Status
        const workerStatusBlock = document.getElementById('workerStatus');
        worker.status ? workerStatusBlock.innerHTML = 'online' : workerStatusBlock.innerHTML = 'offline'
        if (worker.status)
            workerStatusBlock.classList.add('online-text')
        else
            workerStatusBlock.classList.add('offline-text')

        // Uptime
        const workerUpTimeBlock = document.getElementById('workerUpTime');
        if (!worker.status)
        {
            let timeSeconds = parseInt(((new Date).getTime() - worker.lastOnline ) / 1000);
            let seconds = parseInt(timeSeconds % 60);
            let minutes = parseInt(timeSeconds / 60) % 60;
            let hours   = parseInt(timeSeconds / 60 /  60);
            let days    = parseInt(timeSeconds / 60 /  60 / 24);
            
            let lastOnlineText = '';
            if (days)
                lastOnlineText = (days + 'd ') + (hours % 24 + 'h');
            if (!days && hours)
                lastOnlineText = (hours + 'h ') + (minutes % 60 + 'm');
            if (!days && !hours)
                lastOnlineText = (minutes + 'm ') + (seconds % 60 + 's');
            if (!days && !hours && !minutes)
                lastOnlineText = seconds + 's';

            if (days > 1000)
                workerUpTimeBlock.innerHTML = 'never';
            else
                workerUpTimeBlock.innerHTML = lastOnlineText;

                    
            const workerLA = document.getElementById('workerInfo-workerLA');
            workerLA.classList.add('cover-hidden');
            const workerPower = document.getElementById('workerInfo-power');
            workerPower.classList.add('cover-hidden');
        }
        else
        {
            let seconds = parseInt(worker.sysUptime % 60);
            let minutes = parseInt(worker.sysUptime / 60) % 60;
            let hours   = parseInt(worker.sysUptime / 60 /  60);
            let days    = parseInt(worker.sysUptime / 60 /  60 / 24);

            let uptimeText = '';
            if (days)
                uptimeText = (days + 'd ') + (hours % 24 + 'h');
            if (!days && hours)
                uptimeText = (hours + 'h ') + (minutes % 60 + 'm');
            if (!days && !hours)
                uptimeText = (minutes + 'm ') + (seconds % 60 + 's');
            if (!days && !hours && !minutes)
                uptimeText = seconds + 's';

            workerUpTimeBlock.innerHTML = `uptime ${uptimeText}`;

            const minerUpTimeBlock = document.getElementById('minerUpTime');
            let minerUptimeText = worker.minerUptime == 0 ? '' : worker.minerUptime;
            if (worker.minerUptime > 0)
            {
                let seconds = parseInt(worker.minerUptime % 60);
                let minutes = parseInt(worker.minerUptime / 60) % 60;
                let hours   = parseInt(worker.minerUptime / 60 /  60);
                let days    = parseInt(worker.minerUptime / 60 /  60 / 24);
                if (days)
                    minerUptimeText = (days + 'd ') + (hours % 24 + 'h');
                if (!days && hours)
                    minerUptimeText = (hours + 'h ') + (minutes % 60 + 'm');
                if (!days && !hours)
                    minerUptimeText = (minutes + 'm ') + (seconds % 60 + 's');
                if (!days && !hours && !minutes)
                    minerUptimeText = seconds + 's';
            }
            minerUpTimeBlock.innerHTML = minerUptimeText ? `miner ${minerUptimeText}` : '';

        // Load Average
        const workerLA1 = document.getElementById('workerLA-value1');
        workerLA1.innerHTML = worker.la1;
        const workerLA5 = document.getElementById('workerLA-value5');
        workerLA5.innerHTML = worker.la5;
        const workerLA15 = document.getElementById('workerLA-value15');
        workerLA15.innerHTML = worker.la15;

        // Power Usage
        const powerValue = document.getElementById('workerInfo-power-value');
        let totalPower = 0;
        for (let gpu of worker.devices)
            totalPower+=gpu.powerUsage;

        totalPower = parseInt(totalPower);
        powerValue.innerHTML = `${totalPower}w`;
        }

        // CPU
        const cpuModel = document.getElementById('cpu-div-text-model-value');
        cpuModel.innerHTML = worker.cpuInfo ? worker.cpuInfo : '--/--';
        const cpuTemp = document.getElementById('cpu-div-text-temp-value');
        cpuTemp.innerHTML = worker.cpuTemp ? `${worker.cpuTemp} °C` : '--/--';
        const motherboardInfo = document.getElementById('cpu-div-text-motherboard-value');
        motherboardInfo.innerHTML = worker.motherboardInfo ? worker.motherboardInfo : '--/--';

        // Disk
        const diskModel = document.getElementById('disk-model-value');
        diskModel.innerHTML = worker.diskModel ? worker.diskModel : '--/--';
        const diskSize = document.getElementById('disk-size-value');
        diskSize.innerHTML = worker.diskSize ? worker.diskSize : '--/--';
        const freeSpace = document.getElementById('disk-space-value');
        freeSpace.innerHTML = worker.diskFreeSpace ? worker.diskFreeSpace : '--/--';

        // RAM
        const ramTotal = document.getElementById('ram-total-value');
        ramTotal.innerHTML = worker.ramTotal ? worker.ramTotal : '--/--';
        const ramUsed = document.getElementById('ram-used-value');
        let ramUsedCalc = (Number.parseFloat(worker.ramTotal) - Number.parseFloat(worker.ramFree));
        ramUsed.innerHTML = ramUsedCalc ? `${ramUsedCalc.toFixed(2)} Gb` : '--/--';
        const ramFree = document.getElementById('ram-free-value');
        ramFree.innerHTML = worker.ramFree ? worker.ramFree : '--/--';

        // Network
        const remoteIP = document.getElementById('remoteIP-value');
        remoteIP.innerHTML = worker.extIp ? worker.extIp : '--/--';
        const localIP = document.getElementById('localIP-value');
        localIP.innerHTML = worker.localIp ? worker.localIp : '--/--';
        const mac = document.getElementById('mac-value');
        mac.innerHTML = worker.mac ? worker.mac : '--/--';
        const remoteIPSvg = document.getElementById('remoteIPSvg');
        remoteIPSvg.value = worker.extIp;
        if (worker.extIp)
            remoteIPSvg.addEventListener('click', clipboard);
        const localIPSvg = document.getElementById('localIPSvg');
        localIPSvg.value = worker.localIp;
        if (worker.localIp)
            localIPSvg.addEventListener('click', clipboard);

        // System
        const kernel = document.getElementById('system-div-text-kernel-value');
        kernel.innerHTML = worker.kernelVersion ? worker.kernelVersion : '--/--';
        const nvidiaVersion = document.getElementById('system-div-text-nvidia-value');
        nvidiaVersion.innerHTML = worker.nvidiaVersion ? worker.nvidiaVersion : '--/--';
        const amdVersion = document.getElementById('system-div-text-amd-value');
        amdVersion.innerHTML = worker.amdVersion ? worker.amdVersion : '--/--';

        // Создание виджетов GPU
        const gpuList = document.getElementById('gpuList');
        gpuList.innerHTML = '';
        const gpuWidgetList = document.getElementById('workerMinerStat-rightBlock');
        gpuWidgetList.innerHTML = '';

        let totalSpeed1 = 0;
        let totalSpeed2 = 0;
        let totalSpeedZil = 0;

        for (const device of worker.devices)
        {
            totalSpeed1 += device.speed1;
            totalSpeed2 += device.speed2;
            totalSpeedZil += device.speedZil;

            const workerElement = document.createElement('div');
            workerElement.classList.add('workerElement');

            const gpuInfoDiv = document.createElement('div');
            gpuInfoDiv.classList.add('workerElement-gpuInfoDiv');

            const gpuStatDiv = document.createElement('div');
            gpuStatDiv.classList.add('workerElement-gpuStatDiv');

            const gpuInfoDivTopDiv = document.createElement('div');
            gpuInfoDivTopDiv.classList.add('workerElement-gpuInfoDivTopDiv');

            const gpuID = document.createElement('div');
            gpuID.innerHTML = `GPU ${device.gpuId}`;
            gpuID.classList.add('gpuID');
            const gpuName = document.createElement('div');
            gpuName.innerHTML = device.name;
            gpuName.classList.add('gpuName');
            const gpuMemSize = document.createElement('div');
            gpuMemSize.innerHTML = `${device.totalMemory} MB`;
            const vendor = document.createElement('div');
            vendor.innerHTML = device.vendor;

            gpuInfoDivTopDiv.appendChild(gpuID);
            gpuInfoDivTopDiv.appendChild(gpuName);
            gpuInfoDivTopDiv.appendChild(gpuMemSize);
            gpuInfoDivTopDiv.appendChild(vendor);

            const gpuInfoDivBottomDiv = document.createElement('div');
            gpuInfoDivBottomDiv.classList.add('workerElement-gpuInfoDivBottomDiv');

            const busID = document.createElement('div');
            busID.innerHTML = device.busId;
            const bios = document.createElement('div');
            bios.innerHTML = device.vbiosVersion;
            const PLMinMaxInfo = document.createElement('div');
            PLMinMaxInfo.innerHTML = `PL ${Number(device.minPl)} w, ${Number(device.defaultPl)} w, ${Number(device.maxPl)} w`;

            gpuInfoDivBottomDiv.appendChild(busID);
            gpuInfoDivBottomDiv.appendChild(bios);
            gpuInfoDivBottomDiv.appendChild(PLMinMaxInfo);
            
            gpuInfoDiv.appendChild(gpuInfoDivTopDiv);
            gpuInfoDiv.appendChild(gpuInfoDivBottomDiv);

            workerElement.appendChild(gpuInfoDiv);

            const rightBlock = document.createElement('div');
            rightBlock.classList.add("workerElement-rightBlock");

            // SPEED
            const speedDiv = document.createElement('div');
            speedDiv.classList.add("speedDiv");
            const speedValue1 = document.createElement('a');
            const speedValue2 = document.createElement('a');

            const speed1 = normalaizeSpeed(device.speed1);
            const speed2 = normalaizeSpeed(device.speed2);
            const speedZil = normalaizeSpeed(device.speedZil);

            speedValue1.innerHTML = device.speed1 ? `${worker.algorithm1} ${speed1}` : "";
            speedValue2.innerHTML = device.speed2 ? `${worker.algorithm2} ${speed2}` : "";
            if (!device.speed2 && device.speedZil)
                speedValue2.innerHTML = `${worker.algorithmZil} ${speedZil}`;

            speedDiv.appendChild(speedValue1);
            speedDiv.appendChild(speedValue2);
            rightBlock.appendChild(speedDiv);

            // TEMP
            const tempDiv = document.createElement('div');
            tempDiv.classList.add("tempDiv");
            tempDiv.innerHTML = ('<img id="tempSvg" src="/src/temperature.svg" alt="temp svg" width="20" height="20"></img>');
            const tempValueDiv = document.createElement('div');
            tempValueDiv.classList.add('tempValueDiv');
            const tempValueCore = document.createElement('a');
            tempValueCore.innerHTML = device.coreTemp ? `Core ${Number(device.coreTemp)}°` : "Core -/-";
            const tempValueMem = document.createElement('a');
            tempValueMem.innerHTML = device.memoryTemp ? `Mem ${Number(device.memoryTemp)}°` : "Mem -/-";

            tempValueDiv.appendChild(tempValueCore);
            tempValueDiv.appendChild(tempValueMem);
            tempDiv.appendChild(tempValueDiv);
            rightBlock.appendChild(tempDiv);

            // FAN
            const fanDiv = document.createElement('div');
            fanDiv.classList.add("fanDiv");
            fanDiv.innerHTML = ('<img id="fanSvg" src="/src/cooler.svg" alt="fan svg" width="20" height="20"></img>');
            const fanValueDiv = document.createElement('div');
            fanValueDiv.classList.add('fanValueDiv');
            const fanValueTop = document.createElement('a');
            fanValueTop.innerHTML = device.fanSpeed ? `${Number(device.fanSpeed)} %` : ' 0%';
            const fanValueBottom = document.createElement('a');
            fanValueBottom.innerHTML = `${Number(device.setFanSpeed)} %`;

            fanValueDiv.appendChild(fanValueTop);
            fanValueDiv.appendChild(fanValueBottom);
            fanDiv.appendChild(fanValueDiv);
            rightBlock.appendChild(fanDiv);

            // CORE
            const coreDiv = document.createElement('div');
            coreDiv.classList.add("coreDiv");
            const coreValueTop = document.createElement('a');
            coreValueTop.innerHTML = `${Number(device.coreClock).toFixed(0)} Mhz`;
            const coreValueBottom = document.createElement('a');
            coreValueBottom.innerHTML = `${Number(device.setCore)} Mhz`;
            coreDiv.appendChild(coreValueTop);
            coreDiv.appendChild(coreValueBottom);
            rightBlock.appendChild(coreDiv);

            // PL
            const PLDiv = document.createElement('div');
            PLDiv.classList.add("PLDiv");
            PLDiv.innerHTML = ('<img id="powerSvg" src="/src/consumption.svg" alt="power svg" width="20" height="20"></img>');
            const PLValueDiv = document.createElement('div');
            PLValueDiv.classList.add('PLValueDiv');
            const PLValueTop = document.createElement('a');
            PLValueTop.innerHTML = `${Number(device.powerUsage).toFixed(0)} w`;
            const PLValueBottom = document.createElement('a');
            PLValueBottom.innerHTML = `${Number(device.setPl)} w`;

            PLValueDiv.appendChild(PLValueTop);
            PLValueDiv.appendChild(PLValueBottom);
            PLDiv.appendChild(PLValueDiv);
            rightBlock.appendChild(PLDiv);

            // SVG SPEED
            const speedSvgDiv = document.createElement('div');
            speedSvgDiv.classList.add("speedSvgDiv");
            speedSvgDiv.innerHTML = ('<img id="speedSvg" src="/src/speed.svg" alt="Разгон svg" width="25" height="25"></img>');
            rightBlock.appendChild(speedSvgDiv);
            
            // ACCEPTED SHARES
            const A = document.createElement('a');
            A.classList.add("A");
            const totalA = device.acceptedShares1 + device.acceptedShares2 + device.acceptedSharesZil;
            A.innerHTML = totalA ? `A ${totalA}` : "";
            // STALE SHARES
            const S = document.createElement('a');
            S.classList.add("S");
            const totalS = device.staleShares1 + device.staleShares2 + device.staleSharesZil;
            S.innerHTML = totalS ? `S ${totalS}` : "";
            // REJECT SHARES
            const R = document.createElement('a');
            R.classList.add("R");
            const totalR = device.rejectedShares1 + device.rejectedShares2 + device.rejectedSharesZil;
            R.innerHTML = totalR ? `R ${totalR}` : "";
            // INVALID SHARES
            const I = document.createElement('a');
            I.classList.add("I");
            const totalI = device.invalidShares1 + device.invalidShares2 + device.invalidSharesZil;
            I.innerHTML = totalI ? `I ${totalI}` : "";

            gpuStatDiv.appendChild(A);
            gpuStatDiv.appendChild(S);
            gpuStatDiv.appendChild(R);
            gpuStatDiv.appendChild(I);

            workerElement.appendChild(gpuStatDiv);
            workerElement.appendChild(rightBlock);
            gpuList.appendChild(workerElement);

            // GPU WIDGET ITEM
            if (worker.status)
            {
                const rect = document.createElement('div');
                const wTemp = document.createElement('a');
                wTemp.innerHTML = `${Number(device.coreTemp)}°`;
                const wFanSpeed = document.createElement('a');
                wFanSpeed.innerHTML = `${Number(device.fanSpeed)}%`;

                if (Number(device.fanSpeed) > 80)
                    wFanSpeed.classList.add("gpu-widget-item-fan-red");
                else if (Number(device.fanSpeed) > 65 && Number(device.fanSpeed) <= 80)
                    wFanSpeed.classList.add("gpu-widget-item-fan-yellow");
                else if (Number(device.fanSpeed) > 35 && Number(device.fanSpeed) <= 65)
                    wFanSpeed.classList.add("gpu-widget-item-fan-blue");
                else if (Number(device.fanSpeed) >= 0 && Number(device.fanSpeed) <= 35)
                    wFanSpeed.classList.add("gpu-widget-item-fan-purp");

                const wSpeed1 = document.createElement('a');
                wSpeed1.innerHTML = worker.algorithm1 ? `${speed1}` : "-/-";
                // if (device.speed1 == 0)
                //     wSpeed1.innerHTML = 'n/a';
                    
                const wSpeed2 = document.createElement('a');
                wSpeed2.innerHTML = worker.algorithm2 ? `${speed2}` : "";
                if (device.speed2 == 0 && device.speedZil > 0)
                    wSpeed2.innerHTML = speedZil;

                if (wSpeed2.innerHTML != "")
                {
                    wSpeed1.classList.add("gpu-widget-item-speed");
                    wSpeed2.classList.add("gpu-widget-item-speed");
                    rect.classList.add("gpu-widget-item");
                }
                else
                {
                    wSpeed1.classList.add("gpu-widget-item-speed");
                    rect.classList.add("gpu-widget-item");
                }

                rect.appendChild(wTemp);
                rect.appendChild(wFanSpeed);
                rect.appendChild(wSpeed1);
                rect.appendChild(wSpeed2);
                gpuWidgetList.appendChild(rect);
            }
        } // for device

        const minerName = document.getElementById('minerName');
        minerName.innerHTML = worker.minerName ? `Miner: ${worker.minerName}` : '';
        const alg1Name = document.getElementById('algo1Name');
        alg1Name.innerHTML = worker.algorithm1 ? `${worker.algorithm1}` : '';
        const alg2Name = document.getElementById('algo2Name');
        alg2Name.innerHTML = worker.algorithm2 ? `${worker.algorithm2}` : '';
        const alg1Speed = document.getElementById('algo1Speed');
        alg1Speed.innerHTML = totalSpeed1 ? `${normalaizeSpeed(totalSpeed1)}` : '';
        const alg2Speed = document.getElementById('algo2Speed');
        alg2Speed.innerHTML = totalSpeed2 ? `${normalaizeSpeed(totalSpeed2)}` : '';
        const pool1 = document.getElementById('pool1-value');
        pool1.innerHTML = worker.server1 ? `${worker.server1}` : '';
        const pool2 = document.getElementById('pool2-value');
        pool2.innerHTML = worker.server2 ? `${worker.server2}` : '';

        const totalAcceptedShares = document.getElementById('totalAcceptedShares');
        const totalAccept = Number(worker.totalAcceptedShares1) + Number(worker.totalAcceptedShares2 + Number(worker.totalAcceptedSharesZil));
        totalAcceptedShares.innerHTML = totalAccept ? `A ${totalAccept}` : '';
        const totalStaleShares = document.getElementById('totalStaleShares');
        const totalStale = Number(worker.totalStaleShares1) + Number(worker.totalStaleShares2 + Number(worker.totalStaleSharesZil));
        totalStaleShares.innerHTML = totalStale ? `S ${totalStale}` : '';
        const totalRejectShares = document.getElementById('totalRejectedShares');
        const totalRej = Number(worker.totalRejectedShares1) + Number(worker.totalRejectedShares2 + Number(worker.totalRejectedSharesZil));
        totalRejectShares.innerHTML =  totalRej ?`R ${totalRej}` : '';
        const totalInvalidShares = document.getElementById('totalInvalidShares');
        const totalInv = Number(worker.totalInvalidShares1) + Number(worker.totalInvalidShares2 + Number(worker.totalInvalidSharesZil));
        totalInvalidShares.innerHTML = totalInv ? `I ${totalInv}` : '';
    } // worker
} // load

load();

const refreshTimerId = setInterval(() => 
{
    const currMSecsSience = new Date().valueOf();
    const localMSecsSience = localStorage.getItem('lastDataUpdate') ? localStorage.getItem('lastDataUpdate') : 0;

    if ((currMSecsSience - localMSecsSience) >= 4000)
    {
        const ws = new WebSocket(`ws://${location.hostname}:9000`);
        ws.onopen = () => 
        {
            const message = 
            {
                id: uuidv4(),
                command: "620b83ac-0a73-4d18-b5f8-3ed7a7440b94",
                content: 
                {
                    id: localStorage.getItem('id'),
                },
                webFlags: 
                {
                    type: "command",
                    execStatus: "unknown",
                    priority: "normal",
                    contentFormat: "json",
                },
                tags: [],
            };
            ws.send(JSON.stringify(message));
        };

        ws.onclose = () => 
        {
            //console.log("closed");
        };

        ws.onmessage = (message) => 
        {
            const data = JSON.parse(message.data);
            ws.close();
            localStorage.setItem('userData', JSON.stringify(data.content, null, 4));
            localStorage.setItem('lastDataUpdate', currMSecsSience);
        }
    }
    load();
}, 5000);