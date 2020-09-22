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
        if(message[2] == '4')
        {

        }
        else if(message[2] == '5')
        {

        }
        
    };
} 

var ck = document.cookie.split('#');                                       //ck[0]记录了账号，ck[1]记录了用户名

function createroom(){
    WS_send(ws, "104##"+ck[0]);                               
    ws.onmessage = function (evt) 
      { 
        var received_msg = evt.data;                   //创建房间，返回房间号
        var rec = received_msg.split('##');
        if(rec[1]=='?')
           {
                alert("创建房间失败")
               
           }
        else {
            var temp=document.cookie;
            temp=temp+'#'+rec[1];
            setCookie(temp);
            window.location.href="game_ui.html";
        }
      }                                             
}          

function quickgame(){
    WS_send(ws, "105##"+ck[0]);                              
    ws.onmessage = function (evt) 
      { 
        var received_msg = evt.data;                   //返回服务器已有的一个房间号
        var rec = received_msg.split('##');
        if(rec[1]=='?')
          {
                alert("没有已创建的房间")  
         }
     else {
         var temp=document.cookie;
         temp=temp+'#'+rec[1];
         setCookie(temp);
         window.location.href="game_ui.html";
        }
      }                                             
}  

function joinroom(){
    var obj1 = document.getElementById('find');
    WS_send(ws, "106##"+ck[0]+'##'+obj1.value);                            
    ws.onmessage = function (evt) 
      { 
        var received_msg = evt.data;                   //加入房间，找到则返回found
        var rec = received_msg.split('##');
        if(rec[1]=='1')
            {
                var temp=document.cookie;
                temp=temp+'#'+obj1.value;
                setCookie(temp);
                window.location.href="game_ui.html";
            }
        else alert("加入房间失败");
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
        alert("服务器连接失败,请手动刷新");
    }
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
