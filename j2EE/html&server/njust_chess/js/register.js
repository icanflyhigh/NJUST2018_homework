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

function register(){
    var obj1 = document.getElementById('account');
    var obj2 = document.getElementById('password');
    var obj3 = document.getElementById('password2');
    var obj4 = document.getElementById('correction');
    var obj5 = document.getElementById('name');
    if(obj1.value=='wrong'){
        alert("账号不可以为'wrong'")
    }
    else{
        if(obj2.value==obj3.value){
        WS_send(ws, "107##"+obj1.value+"##"+obj2.value+'##'+obj4.value+'##'+obj5.value);   
        ws.onmessage = function (evt) 
          { 
            var received_msg = evt.data;
            var rec = received_msg.split('##');
            if(rec[1]=='1')
                {
                    alert("恭喜你！账号已创建成功！")
                    window.location.href="login.html";
                }
            else alert("账号创建失败");
          }  
         }
         else alert('两次请输入相同的密码')
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