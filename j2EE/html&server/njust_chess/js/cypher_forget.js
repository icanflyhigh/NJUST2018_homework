var ws ;

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
        console.log(typeof(message));
        process_message(message);
    };
} 

function forget(){
    var obj1 = document.getElementById('account');
    var obj2 = document.getElementById('correction');
    WS_send(ws, "103##"+obj1.value+"##"+obj2.value);   
    ws.onmessage = function (evt) 
      { 
        var received_msg = evt.data;
        var rec = received_msg.split('##');
        if(rec[1]=='1')
            {
                alert("密码已重置")
                window.location.href="login.html";
            }
        else alert("与服务器连接错误");
      }        
}

function back(){
    window.location.href="login.html";
}

function WS_send(websocket, message)
{
    if(websocket.readyState == 1)
    {
        websocket.send(message);
    }
    else
    {
        alert("服务器连接失败,请手动刷新");
    }
}