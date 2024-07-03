const refresh = document.getElementById("refresh");
const logout = document.getElementById("logout");
const addWorker = document.getElementById("addWorker");
const settings = document.getElementById("settings");

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

refresh.onclick = () => 
{
    const ws = new WebSocket("ws://localhost:9000");

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
        //console.log(JSON.stringify(data.content.workersData, null, 4));
    };
};

addWorker.onclick = () => 
{
    const ws = new WebSocket("ws://localhost:9000");

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

settings.onclick = () => 
{
    window.location.replace("./user_settings.html");
};