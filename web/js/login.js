'use strict';
const loginBtn = document.getElementById("login");
const registrationBtn = document.getElementById("registration");
const reg = document.getElementById('reg');
let regState = false;

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

reg.onclick = () =>
{
    const auth = document.getElementById('auth-text');
    if (!regState)
    {
        auth.innerHTML = 'Регистрация';
        loginBtn.classList.add('cover-hidden');
        registrationBtn.classList.remove('cover-hidden');
        reg.classList.add('reg-active');
        regState = true;
    }
    else
    {
        auth.innerHTML = 'Авторизация';
        loginBtn.classList.remove('cover-hidden');
        registrationBtn.classList.add('cover-hidden');
        reg.classList.remove('reg-active');
        regState = false;
    }
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

loginBtn.onclick = () => 
{
    const loginInput = document.getElementById('loginInput');
    const passwordInput = document.getElementById('passwordInput');
    let resultText = "";

    const ws = new WebSocket(`ws://${location.hostname}:9000`);
    ws.onopen = () => 
    {
        const message = 
        {
            id: uuidv4(),
            command: "cab5e72d-b557-40e5-bd84-869d8c63eb36",
            content: 
            {
                login: loginInput.value,
                password: passwordInput.value,
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
            window.location.replace("./main.html");
            localStorage.setItem('id', data.content.id);
            localStorage.setItem('login', data.content.login);
            localStorage.setItem('password', data.content.password);
        }
        else
        {
            showMessageBox("Ошибка авторизации");
        }
    };
};

registrationBtn.onclick = () => 
{
    const loginInput = document.getElementById('loginInput');
    const passwordInput = document.getElementById('passwordInput');
    let resultText = "";

    const ws = new WebSocket(`ws://${location.hostname}:9000`);
    ws.onopen = () => 
    {
        const message = 
        {
            id: uuidv4(),
            command: "f8c9bf81-9062-4376-a33e-3c0bf7158501",
            content: 
            {
                login: loginInput.value,
                password: passwordInput.value,
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
            showMessageBox(`Пользователь ${data.content.login} успешно зарегестрирован`);
            reg.click();
        }
        else
        {
            showMessageBox(`Пользователь с именем ${data.content.login} уже существует`);
        }
    };
};