'use strict';
const loc = String(document.location);
let workerId = document.getElementById('workerId');
workerId = loc.substring(loc.indexOf('#') + 1);

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
    for (const device of worker.devices)
    {
        console.log(device);
        const workerElement = document.createElement('div');
        workerElement.classList.add('workerElement');

        const gpuInfoDiv = document.createElement('div');
        gpuInfoDiv.classList.add('workerElement-gpuInfoDiv');

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

        // TEMP

        // FAN
        const fanDiv = document.createElement('div');
        fanDiv.classList.add("fanDiv");
        fanDiv.innerHTML = ('<img id="fanSvg" src="/src/cooler.svg" alt="fan svg" width="20" height="20"></img>');
        const fanValueDiv = document.createElement('div');
        fanValueDiv.classList.add('fanValueDiv');
        const fanValueTop = document.createElement('a');
        fanValueTop.innerHTML = `${Number(device.fanSpeed)} %`;
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
        workerElement.appendChild(rightBlock);

        gpuList.appendChild(workerElement);
    }
}