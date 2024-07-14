'use strict';
const loc = String(document.location);
const workerId = document.getElementById('workerId');
workerId.innerHTML = loc.substring(loc.indexOf('#') + 1);

const workers = JSON.parse(localStorage.getItem('userData')).workersData;
let workerName = '';

for (const worker of workers)
{
    if (worker.id == workerId.innerHTML)
    {
        workerName = worker.workerName;
        if (!workerName)
            workerName = (worker.id).substring(0, 8);
        break;
    }
}
document.title = `Worker ${workerName}`;