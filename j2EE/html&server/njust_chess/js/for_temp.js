var user_id = '002';
var is_black = true;
function resetVal(ID, val)
{
    var obj = document.getElementById(ID);
    obj.value= val;
}
function set_focus(ID)
{
    console.log("focus")
    var obj = document.getElementById(ID);
    obj.focus();
}
function ready_focus(ID)
{
    console.log("focus")
    var obj = document.getElementById(ID);
    obj.focus();
}
function resetInnerHTML(ID, htm)
{
    var obj = document.getElementById(ID);
    obj.innerHTML = htm;
}

function ready_click(ID)//401事件
{
    
    var obj = document.getElementById(ID);
    if(obj.innerHTML == "准备")
    {
        WS_send(ws, "4011##"+user_id);        // 4表示game_ui界面，0表示click事件，1表示已经准备
        resetInnerHTML(ID,"取消准备");
    }
    else
    {
        WS_send(ws, "4010##"+user_id);
        resetInnerHTML(ID,"准备");
    }
}

function WS_send(websocket, message)
{
    if(websocket.readyState == 1)
    {
        websocket.send(message);
    }
    else
    {
        alert("网络连接失败");
    }
}

function process_message(message)
{
    if(message[0] == '4')
    {
        if(message.substring(1,2) == '01')
        {
            if(message.substring(3) == 'ERROR')
            {
                location.reload();
            }
        }
    }
}
var ws 
window.onload=function()
{
    ws = new WebSocket("ws://localhost:1551",'echo-protocol');
    ws.onopen=function()
    {
        console.log("ws on");
    };
    
    ws.onmessage=function(evt)
    {
        console.log(evt.data);
        var message = evt.data;

    };
    resetInnerHTML("user_id", "你的ID： "+user_id);
    //playChess();

} 
