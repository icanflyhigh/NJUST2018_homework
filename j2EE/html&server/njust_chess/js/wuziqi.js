function playChess(){
    var chess=document.getElementById("chess");
    var context=chess.getContext('2d');
    chess.addEventListener("mouseup", domouseup, false);
    var canvasWidth=450;
    var canvasHeight=450;
    //每个格子的宽度
    //30为两侧棋盘距canvas画布的距离和，与每个格子的宽度值相等
    var bootWidth=(canvasWidth-30)/14;
    //每个格子的高度
    var bootHeight=(canvasHeight-30)/14;
    //true代表我方是黑棋，false代表我方是白棋
    var me=true;
    //存储棋子的位置的二维数组
    var chessBoard=[];
    //表示是否结束
    var over=false;
    for(var i=0;i<15;i++){
        chessBoard[i]=[];
        for(var j=0;j<15;j++){
            chessBoard[i][j]=0;
        }
    }
    chess.width=canvasWidth;
    chess.height=canvasHeight;
    var img=new Image();
    img.src="img/bg2.jpg";
    img.onload=function(){
        context.drawImage(img,0,0,canvasWidth,canvasHeight);
        drawChessBoard(bootWidth,bootHeight);
    }
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

//走一步棋的函数
    function oneStep(i,j,me){
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
        if(me){
            gradient.addColorStop(0,"#0a0a0a");
            gradient.addColorStop(1,"#636766");
        }else{
            gradient.addColorStop(0,"#d1d1d1");
            gradient.addColorStop(1,"#f9f9f9");
        }
        context.fillStyle=gradient;
        context.fill();
    }
    function domouseup(e){
        if(over){
            return;
        }
        var x= e.offsetX;
        var y= e.offsetY;
        var i=Math.floor(x/bootWidth);
        var j=Math.floor(y/bootHeight);
        if(chessBoard[i][j]==0){
            oneStep(i,j,me);
            //代表这个位置是我的棋子
            chessBoard[i][j]=1;
            for(var k=0;k<count;k++){
                if(wins[i][j][k]){
                    myWin[k]++;
                    //表示这种赢法计算机不可能赢了
                    computerWin[k]=6;
                    if(myWin[k]==5){
                        over=true;
                        //不加个定时器就会先alert再画第五颗棋
                        setTimeout(function(){
                            window.alert("你赢了");
                            //提示完之后清空并重绘棋盘
                            playChess();
                        },50);

                    }
                }
            }
            //电脑落子
            if(!over){
                me=!me;
                computerAI();
            }
        }
    }

//定义一个赢法数组，三维数组
    var wins=[];
    for(var i=0;i<15;i++){
        wins[i]=[];
        for(var j=0;j<15;j++){
            wins[i][j]=[];
        }
    }
    var count=0;          //赢法种类的索引

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
        oneStep(u,v,me);
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
                        playChess();
                    },50);

                }
            }
        }
        if(!over){
            me=!me;
        }
    }
}

// window.onload=function(){
//    playChess();
// }