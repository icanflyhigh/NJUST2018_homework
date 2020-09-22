console.log(document.cookie);
var ck = document.cookie.split('#');                 //ck[0]记录了账号，ck[1]记录了用户名,ck[2]记录了房间号
var user_id =ck[0];
var name = ck[1];
var room_id = ck[2];
var ws ;
var is_black;
var have_op;
// 记得用cookie 把所有按钮 棋盘之类的状态都记录下来

var chess=document.getElementById("chess");
var context=chess.getContext('2d');
var oneStep_out;
var canvasWidth=450;    //每个格子的宽度
var canvasHeight=450;   //每个格子的高度
var chessBoard=[];
chess.width=canvasWidth;
chess.height=canvasHeight;

//30为两侧棋盘距canvas画布的距离和，与每个格子的宽度值相等
var bootWidth=(canvasWidth-30)/14;
//每个格子的高度
var bootHeight=(canvasHeight-30)/14;
//绘制棋盘方法
function drawChessBoard(bootWidth,bootHeight){
    context.beginPath();
    context.strokeStyle="#000000";
    for(var i=0;i<15;i++){
        context.moveTo(i*bootWidth+15,15);
        context.lineTo(i*bootWidth+15,canvasWidth-15);
        context.moveTo(15,i*bootHeight+15);
        context.lineTo(canvasHeight-15,i*bootHeight+15);
    }
    context.stroke();
    context.closePath();
}
function playChess(is_AI)
{
    //定义一个赢法数组，三维数组
    var wins=[];
    for(var i=0;i<15;i++){
        wins[i]=[];
        for(var j=0;j<15;j++){
            wins[i][j]=[];
        }
    }
    var count=0;          //赢法种类的索引
    var me_play = is_black;
    //所有连成竖线的赢法
    for(var i=0;i<15;i++){
        for(var j=0;j<11;j++){
            for(var k=0;k<5;k++){
                wins[i][j+k][count]=true;
            }
            count++;
        }
    }
    //所有连成横线的赢法
    for(var i=0;i<11;i++){
        for(var j=0;j<15;j++){
            for(var k=0;k<5;k++){
                wins[i+k][j][count]=true;
            }
            count++;
        }
    }
    //所有连成斜线的赢法
    for(var i=0;i<11;i++){
        for(var j=0;j<11;j++){
            for(var k=0;k<5;k++){
                wins[i+k][j+k][count]=true;
            }
            count++;
        }
    }
    //所有连成反斜线的赢法
    for(var i=0;i<11;i++){
        for(var j=14;j>3;j--){
            for(var k=0;k<5;k++){
                wins[i+k][j-k][count]=true;
            }
            count++;
        }
    }
    //赢法统计数组
    var myWin=[];
    var computerWin=[];
    for(var i=0;i<count;i++){
        myWin[i]=0;
        computerWin[i]=0;
    }
    var img=new Image();
    img.src="img/bg2.jpg";
    img.onload=function()
    {
        console.log("drawpic");
        context.drawImage(img,0,0,canvasWidth,canvasHeight);
        drawChessBoard(bootWidth,bootHeight);
    }


    //true代表我方是黑棋，false代表我方是白棋
    //存储棋子的位置的二维数组

    //表示是否结束
    var over=false;
    for(var i=0;i<15;i++)
    {
        chessBoard[i]=[];
        for(var j=0;j<15;j++)
        {
            chessBoard[i][j]=0;
        }
    }
    function ini()
    {
        context.drawImage(img,0,0,canvasWidth,canvasHeight);
        drawChessBoard(bootWidth,bootHeight);
        over=false;
        me_play = false;
        for(var i=0;i<15;i++)
        {
            for(var j=0;j<15;j++)
            {
                chessBoard[i][j]=0;
            }
        }
        count=0;          //赢法种类的索引
        //所有连成竖线的赢法
        for(var i=0;i<15;i++){
            for(var j=0;j<11;j++){
                for(var k=0;k<5;k++){
                    wins[i][j+k][count]=true;
                }
                count++;
            }
        }
        //所有连成横线的赢法
        for(var i=0;i<11;i++){
            for(var j=0;j<15;j++){
                for(var k=0;k<5;k++){
                    wins[i+k][j][count]=true;
                }
                count++;
            }
        }
        //所有连成斜线的赢法
        for(var i=0;i<11;i++){
            for(var j=0;j<11;j++){
                for(var k=0;k<5;k++){
                    wins[i+k][j+k][count]=true;
                }
                count++;
            }
        }
        //所有连成反斜线的赢法
        for(var i=0;i<11;i++){
            for(var j=14;j>3;j--){
                for(var k=0;k<5;k++){
                    wins[i+k][j-k][count]=true;
                }
                count++;
            }
        }
        //赢法统计数组
        for(var i=0;i<count;i++)
        {
            myWin[i]=0;
            computerWin[i]=0;
        }
    }

//走一步棋的函数
    function oneStep(i,j, is_AI)
    {
        //i、j是索引，me表示黑棋还是白棋,我方是黑棋
        context.beginPath();
        context.arc(15+i*bootWidth,15+j*bootWidth,13,0,2*Math.PI);
        context.closePath();
        //设置圆形渐变色的位置及半径
        var gradient=context.createRadialGradient(
            15+i*bootWidth+2,
            15+j*bootWidth-2,
            13,
            15+i*bootWidth+2,
            15+j*bootWidth-2,
            0);
        if((is_black&&me_play )|| (!is_black&&!me_play))
        {
            gradient.addColorStop(0,"#0a0a0a");
            gradient.addColorStop(1,"#636766");
        }
        else
        {
            gradient.addColorStop(0,"#d1d1d1");
            gradient.addColorStop(1,"#f9f9f9");
        }
        context.fillStyle=gradient;
        context.fill();
        if(is_AI) return ;
        //代表这个位置是我的棋子
        if((is_black&&me_play )|| (!is_black&&!me_play) )
        chessBoard[i][j] = 1;
        else
        chessBoard[i][j] = 2;
        if(me_play)
        {
            
            for(var k=0;k<count;k++){
                if(wins[i][j][k]){
                    myWin[k]++;
                    //表示这种赢法计算机不可能赢了
                    computerWin[k]=6;
                    if(myWin[k]==5)
                    {
                        over=true;
                        //不加个定时器就会先alert再画第五颗棋
                        setTimeout(function()
                        {
                            window.alert("你赢了");
                            //提示完之后清空并重绘棋盘
                            //playChess();

                        },50);
                    }
                    
                }
            }
            if(!over)ws.send("402|"+i+"|"+j+"|##"+user_id);
            else ws.send("402|"+i+"|"+j+"|.##"+user_id);
        }
        else
        {
            for(var k=0;k<count;k++){
                if(wins[i][j][k]){
                    computerWin[k]++;
                    //表示这种赢法我不可能赢了
                    myWin[k]=6;
                    if(computerWin[k]==5){
                        over=true;
                        //不加个定时器就会先alert再画第五颗棋
                        setTimeout(function(){
                            window.alert("你的对手赢了");
                            //提示完之后清空并重绘棋盘
                            //playChess();
                            ini();
                            after_match();
                        },50);
    
                    }
                }
            }
        }
        if(!over)
        {
            me_play = !me_play;
        }
        else
        {
            ini();
            after_match();
        }
    }
    oneStep_out = oneStep;
    chess.addEventListener("mouseup", domouseup, false);
    function domouseup(e)
    {
        if(over || !me_play)
        {
            return;
        }
        var x = e.offsetX;
        var y = e.offsetY;
        var i = Math.floor(x/bootWidth);
        var j = Math.floor(y/bootHeight);
        if(chessBoard[i][j] == 0 && is_AI)
        {
            oneStep(i, j, true);
            //代表这个位置是我的棋子
            if(me_play)
            chessBoard[i][j] = 1;
            else
            chessBoard[i][j] = 2;
            for(var k=0;k<count;k++){
                if(wins[i][j][k]){
                    myWin[k]++;
                    //表示这种赢法计算机不可能赢了
                    computerWin[k]=6;
                    if(myWin[k]==5){
                        over=true;
                        //不加个定时器就会先alert再画第五颗棋
                        setTimeout(function()
                        {
                            window.alert("你赢了");
                            //提示完之后清空并重绘棋盘
                            //playChess();
                            ini();
                            after_match();
                        },50);

                    }
                }
            }
            if(!over)
            {
                me_play=!me_play;
                computerAI();
            }
        }
        else if(chessBoard[i][j] == 0)
        {
            oneStep(i, j, false);
        }
    }





    function computerAI(){
        //我方得分的二维数组
        var myScore=[];
        //电脑得分的二维数组
        var computerScore=[];
        //保存最高的分数
        var max=0;
        //保存最高分的点的坐标
        var u= 0,v=0;
        for(var i=0;i<15;i++){
            myScore[i]=[];
            computerScore[i]=[];
            for(var j=0;j<15;j++){
                myScore[i][j]=0;
                computerScore[i][j]=0;
            }
        }
        for(var i=0;i<15;i++){
            for(var j=0;j<15;j++){
                if(chessBoard[i][j]==0){
                    //遍历所有赢法
                    for(var k=0;k<count;k++){
                        if(wins[i][j][k]){
                            //判断我方在该位置落子好不好，加的分数越多代表越好
                            if(myWin[k]==1){
                                myScore[i][j]+=1;
                            }else if(myWin[k]==2){
                                myScore[i][j]+=100;
                            }else if(myWin[k]==3){
                                myScore[i][j]+=10000;
                            }else if(myWin[k]==4){
                                myScore[i][j]+=1000000;
                            }
                            //判断电脑在该位置落子好不好，加的分数越多代表越好
                            if(computerWin[k]==1){
                                computerScore[i][j]+=2;
                            }else if(computerWin[k]==2){
                                computerScore[i][j]+=220;
                            }else if(computerWin[k]==3){
                                computerScore[i][j]+=22000;
                            }else if(computerWin[k]==4){
                                computerScore[i][j]+=2200000;
                            }
                        }
                    }
                    //判断我的最高分位置
                    if(myScore[i][j]>max){
                        max=myScore[i][j];
                        u=i;
                        v=j;
                    }else if(myScore[i][j]==max){
                        if(computerScore[i][j]>computerScore[u][v]){
                            u=i;
                            v=j;
                        }
                    }
                    //判断电脑的最高分位置
                    if(computerScore[i][j]>max){
                        max=computerScore[i][j];
                        u=i;
                        v=j;
                    }else if(computerScore[i][j]==max){
                        if(myScore[i][j]>myScore[u][v]){
                            u=i;
                            v=j;
                        }
                    }
                }
            }
        }
        oneStep(u, v, true);
        //代表这个位置是计算机的棋子
        chessBoard[u][v]=2;
        for(var k=0;k<count;k++){
            if(wins[u][v][k]){
                computerWin[k]++;
                //表示这种赢法我不可能赢了
                myWin[k]=6;
                if(computerWin[k]==5){
                    over=true;
                    //不加个定时器就会先alert再画第五颗棋
                    setTimeout(function(){
                        window.alert("电脑赢了");
                        //提示完之后清空并重绘棋盘
                        ini();
                        after_match();
                    },50);

                }
            }
        }
        if(!over){
            me_play=!me_play;
        }
    }
}

// window.onload=function(){
//    playChess();
// }


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

function leave_click(ID)
{
    console.log("leave");
    WS_send(ws, "403##"+user_id);
    window.location.href="choose_ui.html";
}

function AI_click(ID)
{
    console.log("AI");
    // var obj = document.getElementById("op_player_name");
    resetInnerHTML('op_player_name', '简单的电脑');
    WS_send(ws, '4041##'+user_id+'##'+room_id);
    is_black = true;
    playChess(true);
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

function match_start()
{
    var ready_but = document.getElementById("ready_but");
    ready_but.disabled = true;
    ready_but.innerHTML = "";
    var leave_but = document.getElementById("leave_but");
    leave_but.disabled = true;
    leave_but.innerHTML = "";
}
function after_match()
{
    var ready_but = document.getElementById("ready_but");
    ready_but.disabled = false;
    ready_but.innerHTML = "准备";
    var leave_but = document.getElementById("leave_but");
    leave_but.disabled = false;
    leave_but.innerHTML = "离开";
}

function process_message(message)
{
    if(message[0] == '0')
    {
        if(message.substring(1,2) == '01')
        {

        }
    }
    else if(message[0] == '4')
    {
        if(message[1] == '0' && message[2] == '0')
        {
            if(message[3] == '#')
            {
                var temp = message.split('##')
                resetInnerHTML("op_player_name", 'ID：'+temp[1]);
                var obj = document.getElementById('AI_but');
                have_op = true;
                obj.disabled = true;
                resetInnerHTML('AI_but', '');
            }
            else
            {
                resetInnerHTML("op_player_name", '');
                var obj = document.getElementById('AI_but')
                obj.disabled = false;
                have_op = false;
                resetInnerHTML('AI_but', '与电脑<br>对战');
            }

        }
        if(message[1] == '0' && message[2] == '1')
        {
            if(message.substring(3) == 'ERROR')
            {
                //location.reload();
            }
        }
        if(message[1] == '0' && message[2] == '2') //  402事件
        {
            if(message[3] == '1')
            {

                if(message[4] != 'E')
                {
                    if(message[6] == 'b')
                    {
                        is_black = 1;
                    }
                    else
                    {
                        is_black = 0;
                    }
                    resetInnerHTML("op_player_name", "昵称： "+message.split("##")[3]+"<br>"+"ID： "+message.split("##")[2]);
                    playChess(false);
                    match_start();
                }
                else
                {
                    console.log(message);
                }
            }
            else if(message[3] == '|')
            {
                var temp = message.split('|');
                var x = parseInt(temp[1]), y = parseInt(temp[2]);
                oneStep_out(x, y,  false, false);

            }
        }
    }
}


window.onload=function()
{
    ws = new WebSocket("ws://localhost:1551",'echo-protocol');
    ws.onopen=function()
    {
        console.log("ws on");
        ws.send('400##'+user_id+"##"+room_id);
    };
    
    ws.onmessage=function(evt)
    {
        console.log(evt.data);
        var message = evt.data;
        console.log(typeof(message));
        process_message(message);
    };
    resetInnerHTML("user_id", "昵称 ："+name+"<br>"+"ID ："+user_id);
    //playChess();

} 
