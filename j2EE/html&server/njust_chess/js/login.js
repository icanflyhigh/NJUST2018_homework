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

function setCookie(cname){
	document.cookie = cname;
}

function getCookie(name){
	var ca = document.cookie.split(';');
	for(var i=0; i<ca.length; i++) {
		var c = ca[i].trim();
		if (c.indexOf(name)==0) { return c.substring(name.length,c.length); }
	}
	return "";
}                                                   //若有cookie则返回，否则为""

function login(){
    var obj1 = document.getElementById('account');
    var obj2 = document.getElementById('password');
    WS_send(ws, "101##"+obj1.value+"##"+obj2.value);                              
    ws.onmessage = function (evt) 
      { 
        var received_msg = evt.data;
        var rec = received_msg.split('##');
        if(rec[1]=="wrong")
            alert("账号与密码不匹配");
        else{
            var temp=obj1.value+'#'+rec[1];
            document.cookie=temp;                       //将账号和返回的用户名记录在cookie
            window.location.href="main_ui.html";
        }
      }                                              //若回传字符串为用户序号则允许跳转，否则不允许
}                                                    //请求服务器验证是否为已注册账号

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
