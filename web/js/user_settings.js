const changePassword = document.getElementById("changePassword");
const deleteUserttings = document.getElementById("deleteUser");
const goToMain = document.getElementById("goToMain");

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

changePassword.onclick = () => 
{
    const ws = new WebSocket("ws://localhost:9000");

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
            localStorage.setItem('password', data.content.password);
            alert("Пароль успешно обновлен");
        }
        else
        {
            alert("Ошибка обновления пароля");
        }
    };
};

deleteUserttings.onclick = () => 
{
    const ws = new WebSocket("ws://localhost:9000");

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
            alert(`Пользовател ${data.content.login} успешно удален`);
        }
        else
        {
            alert(`Ошибка удаления пользователя ${data.content.login}`);
        }
    };
};

goToMain.onclick = () => 
{
    window.location.replace("./main.html");
}