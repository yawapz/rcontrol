'use strict';

let deleteState = false;

const refresh = document.getElementById("refresh");
const logout = document.getElementById("logout");
const addWorker = document.getElementById("addWorker");
const settings = document.getElementById("settings");
const whoami = document.getElementById("whoami");
const login = localStorage.getItem('login');
whoami.innerHTML = login;
document.title = `Main ${login}`;
document.getElementById("settings-login").innerHTML = login;
const changePassword = document.getElementById("changse-password");
const deleteUser = document.getElementById("delete-user");

const refreshTimerId = setInterval(() => {refresh.click()}, 5000);

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

function addTab()
{
    window.open("./main.html");
}

function showMessageBox(text)
{
    let msg = document.getElementById('messageBox-text');
    msg.innerHTML = text;
    messageBox.classList.remove("cover-hidden");
}

function closeMessageBox()
{
    let msg = document.getElementById('messageBox-text');
    msg.innerHTML = '';
    const messageBox = document.getElementById("messageBox");
    messageBox.classList.add("cover-hidden");
}

function closeDialog()
{
    let msg = document.getElementById('dialog-text');
    msg.innerHTML = '';
    const dialog = document.getElementById("dialog");
    dialog.classList.add("cover-hidden");

    let dialogYes = document.getElementById('dialog-yes');
    dialogYes.removeEventListener('click', deleteWorker);
    dialogYes.removeEventListener('click', deleteUserActivete);
    dialogYes.removeEventListener('click', closeDialog);
}

function workerShow(event)
{
    if (!event.target.workerId)
        return;

    window.history.pushState({}, "", './main.html');
    document.location = `./worker.html#${event.target.workerId}`;
}

refresh.onclick = () => 
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
        let divList = document.getElementById("worker-list");
        divList.innerHTML = '';

        let workers = data.content.workersData;
        for (let worker of workers)
        {
            const elem = document.createElement('div');
            elem.set
            elem.classList.add("worker-element");
            elem.id = worker.id;
            const elemName = document.createElement('div');
            elemName.classList.add("worker-element-name");
            const elemStatus = document.createElement('div');
            elemStatus.classList.add("worker-element-status");
            if (worker.status)
                elemStatus.classList.add("worker-element-online");
            else
                elemStatus.classList.add("worker-element-offline");
            const elemUptime = document.createElement('div');
            elemUptime.classList.add("worker-element-uptime");
            const elemSpeed = document.createElement('div');
            elemSpeed.classList.add("worker-element-speed");
            const elemMiner = document.createElement('div');
            elemMiner.classList.add("worker-element-miner");
            const elemGPUS = document.createElement('div');
            elemGPUS.classList.add("worker-element-GPUS");
            for (let i = 0; i < worker.devices.length; ++i)
            {
                const block = document.createElement('div');
                if (worker.devices[i].coreTemp > 70)
                    block.classList.add("red-rect");
                else
                    block.classList.add("green-rect");
                elemGPUS.appendChild(block);
            }

            const elemLA = document.createElement('div');
            elemLA.classList.add("worker-element-la");
            if (worker.la1 > 0.8)
                elemLA.classList.add("red-text");
            const elemFan = document.createElement('div');
            elemFan.classList.add("worker-element-fan");
            const elemPower = document.createElement('div');
            elemPower.classList.add("worker-element-power");

            elemName.appendChild(document.createTextNode(
                worker.workerName ? worker.workerName : (worker.id).substring(0, 8)));
            elemStatus.appendChild(document.createTextNode(worker.status ? 'online' : 'offline'));
            let minerUptimeText = worker.minerUptime == 0 ? '' : worker.minerUptime;
            if (worker.minerUptime)
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

            let lastOnlineText = '';
            let timeSeconds = parseInt(((new Date).getTime() - worker.lastOnline ) / 1000);
            let seconds = parseInt(timeSeconds % 60);
            let minutes = parseInt(timeSeconds / 60) % 60;
            let hours   = parseInt(timeSeconds / 60 /  60);
            let days    = parseInt(timeSeconds / 60 /  60 / 24);
            
            if (days)
                lastOnlineText = (days + 'd ') + (hours % 24 + 'h');
            if (!days && hours)
                lastOnlineText = (hours + 'h ') + (minutes % 60 + 'm');
            if (!days && !hours)
                lastOnlineText = (minutes + 'm ') + (seconds % 60 + 's');
            if (!days && !hours && !minutes)
                lastOnlineText = seconds + 's';

            if (days > 1000)
            {
                elemUptime.appendChild(document.createTextNode('never'));
                elemUptime.classList.add("worker-element-uptime-never");
            }
            else
                elemUptime.appendChild(document.createTextNode(worker.status ? minerUptimeText : (lastOnlineText)));

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

            let speed1 = 0;
            let speed2 = 0;
            let speedZil = 0;
            for (let gpu of worker.devices)
            {
                speed1 += gpu.speed1;
                speed2 += gpu.speed2;
                speedZil += gpu.speedZil;
            }
            speed1 = normalaizeSpeed(speed1);
            speed2 = normalaizeSpeed(speed2);
            speedZil = normalaizeSpeed(speedZil);

            let speedString = '';
            if (worker.algorithm1 != null)
            {
                speedString = worker.algorithm1;
                if (worker.algorithm2 != null)
                    speedString = `${worker.algorithm1} ${speed1} ${worker.algorithm2} ${speed2}`;
                else if (worker.algorithm2 == null && worker.algorithmZil != null)
                    speedString = `${worker.algorithm1} ${speed1} ${worker.algorithmZil} ${speed2}`;
            }

            elemSpeed.appendChild(document.createTextNode(speedString));
            elemMiner.appendChild(document.createTextNode(worker.minerName == null ? '' : worker.minerName));
            elemLA.appendChild(document.createTextNode(worker.status ? worker.la1 : ''));
            let maxFanSpeed = 0;
            let totalPower = 0;
            for (let gpu of worker.devices)
            {
                if (gpu.fanSpeed > maxFanSpeed)
                    maxFanSpeed = gpu.fanSpeed

                totalPower+=gpu.powerUsage;
            }
            totalPower = parseInt(totalPower);

            if (worker.status)
            {
                const elemFanDivSvg = document.createElement('div');
                elemFanDivSvg.classList.add("worker-element-fan-svg");
                elemFanDivSvg.innerHTML = ('<object type="image/svg+xml" data="../src/cooler.svg"></object>');
                elemFan.appendChild(elemFanDivSvg);
            }

            const elemFanValue = document.createElement('div');
            elemFanValue.classList.add("worker-element-fan-value");
            if (maxFanSpeed > 80)
                elemFan.classList.add("red-text");
            else if (maxFanSpeed > 65 && maxFanSpeed <= 80)
                elemFan.classList.add("yellow-text");
            elemFanValue.appendChild(document.createTextNode(worker.status ? maxFanSpeed+='%' : ''));
            elemFan.appendChild(elemFanValue);

            if (worker.status)
            {
                const elemPowerDivSvg = document.createElement('div');
                elemPowerDivSvg.classList.add("worker-element-power-svg");
                elemPowerDivSvg.innerHTML = ('<object type="image/svg+xml" data="../src/consumption.svg"></object>');
                elemPower.appendChild(elemPowerDivSvg);
            }

            const elemPowerValue = document.createElement('div');
            elemPowerValue.classList.add("worker-element-power-value");
            elemPowerValue.appendChild(document.createTextNode(worker.status ? totalPower+='w' : ''));
            elemPower.appendChild(elemPowerValue);

            elem.appendChild(elemName);
            elem.appendChild(elemStatus);
            elem.appendChild(elemUptime);
            elem.appendChild(elemSpeed);
            elem.appendChild(elemMiner);
            elem.appendChild(elemGPUS);
            elem.appendChild(elemLA);
            elem.appendChild(elemFan);
            elem.appendChild(elemPower);

            elem.workerId = worker.id;
            elem.addEventListener('click', workerShow)
            for (let div of elem.children)
            {
                div.workerId = worker.id;
                div.addEventListener('click', workerShow);
            }

            divList.appendChild(elem);
        } // for workers

        let workerList = (document.getElementById('worker-list')).children;
        if (deleteState)
        {
            for (let i = 0; i < workerList.length; ++i)
            {
                workerList[i].addEventListener('mouseenter', redTrash);
            }
        }
    }; // onmessage
};

addWorker.onclick = () => 
{
    const ws = new WebSocket(`ws://${location.hostname}:9000`);
    ws.onopen = () => 
    {
        const message = 
        {
            id: uuidv4(),
            command: "72b2b855-0094-44a5-9348-3cb4e2d63acf",
            content: 
            {
                id: localStorage.getItem('id'),
                resault: "false",
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
        refresh.click();
    };
};

logout.onclick = () => 
{
    localStorage.clear();
    window.location.replace("./login.html");
};

function toggleSettingsPopup() 
{
    const settings_form = document.getElementById("settings-form");

    if (settings_form.classList.contains("cover-hidden"))
        settings_form.classList.remove("cover-hidden");
    else
        settings_form.classList.add("cover-hidden");
}

function deleteWorker(event)
{
    const ws = new WebSocket(`ws://${location.hostname}:9000`);
    ws.onopen = () => 
    {
        const message = 
        {
            id: uuidv4(),
            command: "f74a0363-1ae7-4ee0-96ad-3edb9080cd0a",
            content: 
            {
                userId: localStorage.getItem('id'),
                workerId: event.target.workerId,
                workerName: event.target.workerName,
                resault: "false",
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
        let resault = JSON.stringify(data.content.resault);
        if (resault === 'true')
        {
            showMessageBox(`Воркер ${event.target.workerName} успешно удален`)
            refresh.click();
        }
        else
        {
            showMessageBox(`Ошибка удаления воркера ${event.target.workerName}`);
        }
    };
}

function deleteWorkerDialog(event)
{
    const workers = JSON.parse(localStorage.getItem('userData')).workersData;
    let workerName = '';

    for (const worker of workers)
    {
        if (worker.id == event.target.id)
        {
            workerName = worker.workerName;
            if (!workerName)
                workerName = (worker.id).substring(0, 8);
            break;
        }
    }
    
    let dialogForm = document.getElementById('dialog');
    dialogForm.classList.remove("cover-hidden");
    let dialogText = document.getElementById('dialog-text');
    dialogText.innerHTML = `Удалить воркер ${workerName}?`;
    let dialogYes = document.getElementById('dialog-yes');
    dialogYes.workerId = event.target.id;
    dialogYes.workerName = workerName;
    dialogYes.addEventListener('click', deleteWorker);
    dialogYes.addEventListener('click', closeDialog);
}

function deleteUserDialog(event)
{

}

function redTrash(event)
{
    function stateUp(event2)
    {
        event2.target.innerHTML = lastMouseStateUp;
        event2.target.classList.add("worker-element");
        event2.target.classList.remove("worker-element-redTrash");
        event2.target.removeEventListener('click', deleteWorkerDialog);
        event.target.addEventListener("click", workerShow);
    }
    var lastMouseStateUp = event.target.innerHTML;
    event.target.innerHTML = `<img id="${event.target.id}" src="/src/trash_red.svg" alt="trash svg" width="20" height="20">`;
    event.target.classList.remove("worker-element");
    event.target.classList.add("worker-element-redTrash");
    event.target.removeEventListener('click', workerShow);
    event.target.addEventListener('mouseleave', stateUp);
    event.target.addEventListener("click", deleteWorkerDialog);
}

function toggleDelSvg() 
{
    const delBtn= document.getElementById("delWorker");
    const delSvg = document.getElementById("delSvg");
    let workerList = (document.getElementById('worker-list')).children;
    delSvg.remove();
    if (!deleteState)
    {
        delBtn.innerHTML = ('<img id="delSvg" src="/src/minus_red.svg" alt="Уд.Воркер svg" width="20" height="20">');
        deleteState = true;
        for (let i = 0; i < workerList.length; ++i)
        {
            workerList[i].addEventListener('mouseenter', redTrash);
        }
    }
    else
    {
        for (let i = 0; i < workerList.length; ++i)
        {
            workerList[i].removeEventListener('mouseenter', redTrash);
        }
        delBtn.innerHTML = ('<img id="delSvg" src="/src/minus.svg" alt="Уд.Воркер svg" width="20" height="20">');
        
        deleteState = false;
    }
}

changePassword.onclick = () => 
{
    const ws = new WebSocket(`ws://${location.hostname}:9000`);
    const newPassword = document.getElementById('new-password-input');
    ws.onopen = () => 
    {
        const message = 
        {
            id: uuidv4(),
            command: "1218ee85-b56e-4529-99c5-a9e8c4abe8dd",
            content: 
            {
                id: localStorage.getItem('id'),
                login: localStorage.getItem('login'),
                password: newPassword.value,
                resault: "false",
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
        let resault = JSON.stringify(data.content.resault);
        if (resault === 'true')
        {
            localStorage.setItem('password', data.content.password);
            showMessageBox("Пароль успешно обновлен");
        }
        else
        {
            showMessageBox("Ошибка обновления пароля");
        }
    };
};

function deleteUserActivete()
{
    const ws = new WebSocket(`ws://${location.hostname}:9000`);
    ws.onopen = () => 
    {
        const message = 
        {
            id: uuidv4(),
            command: "5ea5e390-5f7d-4cb6-a671-f25db9d35a95",
            content: 
            {
                id: localStorage.getItem('id'),
                login: localStorage.getItem('login'),
                password: localStorage.getItem('password'),
                resault: "false",
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
        let resault = JSON.stringify(data.content.resault);
        if (resault === 'true')
        {
            localStorage.clear();
            window.location.replace("./login.html");
            showMessageBox(`Пользовател ${data.content.login} успешно удален`);
        }
        else
        {
            showMessageBox(`Ошибка удаления пользователя ${data.content.login}`);
        }
    };
}

deleteUser.onclick = () => 
{
    let dialogForm = document.getElementById('dialog');
    dialogForm.classList.remove("cover-hidden");
    let dialogText = document.getElementById('dialog-text');
    dialogText.innerHTML = `Удалить пользователя ${localStorage.getItem('login')}?`;
    let dialogYes = document.getElementById('dialog-yes');
    dialogYes.addEventListener('click', deleteUserActivete);
    dialogYes.addEventListener('click', closeDialog);
};