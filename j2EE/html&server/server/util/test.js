var user_id = '001';
var name = 'admin';
var pass_word = '1234';
var pass_str = '1234';
var room_id = 0;
var ws ;
var is_black;
var have_op;
// 记得用cookie 把所有按钮 棋盘之类的状态都记录下来




window.onload=function()
{
    ws = new WebSocket("ws://localhost:1551",'echo-protocol');
    ws.onopen=function()
    {
        console.log("ws on");
        ws.send('107##'+"user_id"+"##"+pass_word+'##'+pass_str+"##"+name);
    };
    
    ws.onmessage=function(evt)
    {
        console.log(evt.data);
    };
    //playChess();

} 
