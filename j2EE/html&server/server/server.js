#!/usr/bin/env node
var WebSocketServer = require('websocket').server;
var http = require('http');
var fs = require('fs');
var database = require('./util/database');
var game_ui_server = require('./util/game_ui_server');
const { del_room } = require('./util/game_ui_server');
var root_path = 'D:/Spring/html/njust_chess'
var port = 1551;
var IP = '0.0.0.0';
var home_page = '/login.html';
var op_client = {};
var room_list = {};
var connection_list = {};



//  下面是响应http请求返回网页，css，js，图片等
var server = http.createServer(function(request, res) 
{
    console.log((new Date()) + ' Received request for ' + request.url);
    var url = request.url;
    file = root_path+url;
    fs.readFile(file,function(err,data)
    {
        if(err)
        {
            res.writeHeader(404,
            {
                'content-type' : 'charset="utf-8"'
            });
            res.write('<h1>404错误</h1><p>你要找的页面不存在</p>');
            res.end();
        }
        else
        {
            if(url.indexOf("html") != -1)
            {
                res.writeHeader(200,{
                    'content-type' : 'text/html;charset="utf-8"'
                });
            }
            else if(url.indexOf("css") != -1)
            {
                res.writeHeader(200,{
                    'content-type' : 'text/css;charset="utf-8"'
                });
            }
            else if(url.indexOf("js") != -1)
            {
                res.writeHeader(200,{
                    'content-type' : 'text/js;charset="utf-8"'
                });
                
            }
            else if(url.indexOf("png") != -1)
            {
                res.writeHeader(200,{
                    'content-type' : 'img/png;charset="utf-8"'
                });
            }
            else if(url.indexOf("jpg") != -1)
            {
                res.writeHeader(200,{
                    'content-type' : 'img/jpg;charset="utf-8"'
                });
            }
            else
            {

            }
            res.end(data);
            
        }
    });
    //response.writeHead(404);
});

//开始侦听
server.listen(port, IP,function() 
{
    console.log((new Date()) + ' Server is listening on port '+port);
});

//  websocket服务器
wsServer = new WebSocketServer({
    httpServer: server,
    autoAcceptConnections: false
});

//  决定是否响应请求
function originIsAllowed(origin) 
{
  return true;
}

//  响应请求部分
wsServer.on('request', function(request) 
{
    if (!originIsAllowed(request.origin)) 
    {
      request.reject();
      console.log((new Date()) + ' Connection from origin ' + request.origin + ' rejected.');
      return;
    }
    var connection = request.accept('echo-protocol', request.origin);
    var user_id;
    console.log((new Date()) + ' Connection accepted.');
    connection.on('message', function(message) 
    {
        console.log(message);
        if (message.type === 'utf8')
         {
            if(message.utf8Data[0] != '1')
            {
                user_id = getuser(message.utf8Data);
            }
            else
            {
                user_id = ""+connection.remoteAddress+new Date();
            }
            connection_list[user_id] = connection;
            console.log('\nReceived Message: ' + message.utf8Data);
            message_judge(message.utf8Data);
            //connection.sendUTF(message.utf8Data);
        }
        else if (message.type === 'binary') 
        {
            console.log("ERROR");
        }
    });
    
    
    connection.on('close', function(reasonCode, description) 
    {
        delete connection_list[user_id];
        console.log((new Date()) + ' Peer ' + connection.remoteAddress +connection.remoteport+ ' disconnected.');
    });
    
    
    //  判断用户发送的报文类型
    function message_judge(message)
    {
        switch(message[0])
        {
            case '1':
                message1(message);
            break;
            case '2':

            break;
            case '3':

            break;
            case '4':
                message4(message);
            break;
            case '5':

            break;
            default:
                console.log("Message error");
                connection.sendUTF("message error");
                console.log(message);
        }
    }

    async function message4(message)
    {
        
        
        if (message[1] == '0' && message[2] == '0')  // 400事件  发送房间信息 
        {
            var temp = message.split('##');
            var user_id = temp[1];
            var room_id = temp[2];
            try
            {
                var sel = await database.db_select('in_room', 'user_id', 'room_id', room_id+' and user_id != "'+user_id+'"');
                await database.db_insert('in_room', "'"+user_id+"', "+room_id+", "+'0');
                //game_ui_server.update_room(room_id, 1);
                if(sel.length == 1)
                {
                    connection.sendUTF("400##"+sel[0].user_id);
                    connection_list[sel[0].user_id].sendUTF("400##"+user_id);
                }
                else
                {
                    connection.sendUTF("400");
                }
            }
            catch(err)
            {
                console.log(err);
                connection.sendUTF("400ERROR");
            }

        }
        else if(message[1] == '0'&& message[2] == '1')  // 401事件 准备按钮
        {
            var ret ={}
            console.log('401 message');
            try
            {
                ret = await game_ui_server.m401(message);
                console.log(ret);
                if(ret.val == true)
                {connection.sendUTF("401accepted");}
                else
                {connection.sendUTF("401ERROR");}
                console.log("try end");
            }
            catch
            {
                connection.sendUTF("401ERROR");
            }
            try
            {
                var sel = await database.db_select('in_room', 'user_id', 'is_ready', '1 AND room_id = '+ret.room_id);
                console.log(sel);
                if(sel.length == 2)
                {
                    op_client[sel[0].user_id] = sel[1].user_id;
                    op_client[sel[1].user_id] = sel[0].user_id;
                    try
                    {
                        var name0 = await database.db_select('user', 'name', 'user_id', "'"+sel[0].user_id+"'");
                        var name1 = await database.db_select('user', 'name', 'user_id', "'"+sel[1].user_id+"'");
                        name0 = name0[0].name;
                        name1 = name1[0].name;
                        console.log(name0[0]);
                    }catch(err)
                    {
                        console.log(err);
                    }
                    
                    if(Math.random() > 0.5) 
                    {
                        connection_list[sel[0].user_id].sendUTF("4021##b##"+sel[1].user_id+"##"+name1);
                        connection_list[sel[1].user_id].sendUTF("4021##w##"+sel[0].user_id+"##"+name0);
                    }
                    else
                    {
                        connection_list[sel[0].user_id].sendUTF("4021##w##"+sel[1].user_id+"##"+name1);
                        connection_list[sel[1].user_id].sendUTF("4021##b##"+sel[0].user_id+"##"+name0);
                    }
                }
            }
            catch(err)
            {
                console.log(err);
                connection.sendUTF("4021ERROR");
            }
            console.log("over 401");
        }
       else if(message[1] == '0' && message[2] == '2') // 402 事件 PvP
       {
           try
           {
                var user_id = getuser(message);
                var temp = message.split('|');
                if(temp[3][0] == '.' )
                {
                    database.db_update('in_room', 'is_ready', '0', "user_id = "+user_id);
                    database.db_update('in_room', 'is_ready', '0', "user_id = "+op_client[user_id]);
                }
                connection_list[op_client[user_id]].sendUTF('402|'+temp[1]+'|'+temp[2]+'|');
                connection.sendUTF("402accepted");
           }
           catch(err)
           {
                console.log(err);
                connection.sendUTF("402ERROR");
           }
            
       } 
       else if(message[1] == '0' && message[2] == '3') // 403 事件 离开
       {
           var user_id = getuser(message);
           try
           {
                var sel = await database.db_select('in_room', 'room_id','user_id', "'"+user_id+"'");
                var ret = await database.db_del('in_room','user_id', "'"+user_id+"'" );
                var op = await database.db_select('in_room', 'user_id','room_id', "'"+sel[0].room_id+"'");
                if(op.length > 0)
                {
                    connection_list[op[0].user_id].sendUTF("400");
                }
                game_ui_server.update_room(sel[0].room_id, -1);
           }
           catch(err)
           {
                console.log(err);
           }
       }
       else if(message[1] == '0' && message[2] == '4')  // 404事件  PvE
       {
           var temp = message.split('##');
           if(message[3] == '1')
            try
            {
                await database.db_update('room', 'max_num', '1', 'room_id = '+temp[2]);
                connection.sendUTF("404accepted");
            }
            catch(err)
            {
                console.log(err);
            }
            else if(message[3] == '2')
            {
                try 
                {
                    await database.db_update('room', 'max_num', '2', 'room = '+temp[2]);
                } 
                catch (err) 
                {
                    console.log(err);
                }
                
            }
            else
            {
                console.log("404EROR");
            }
       }

    }

    async function message1(message)
    {
       
            if(message[1] == '0' && message[2] == '1') // 101事件
            { 
                try
                {
                    var temp = message.split("##");
                    var user_id = temp[1];
                    var password = temp[2];
                    var sel = await database.db_select('user', 'password, name', 'user_id', "'"+user_id+"'");
                    if(sel.length == 0 || password != sel[0].password)
                    {
                        connection.sendUTF("101#wrong");
                    }
                    else
                    {
                        connection.sendUTF("101##"+sel[0].name);
                    }
                }
                catch(err)
                {
                    console.log(err);
                    connection.sendUTF("101#wrong");
                }
            
        }
        else if(message[1] == '0' && message[2] == '2') // 102事件
        {
            try
            {
                var temp = message.split("##");
                var user_id = temp[1];
                var password = temp[2];
                var new_password = temp[3];
                var sel = await database.db_select('user', 'password', 'user_id', "'"+user_id+"'");
                if(password != sel[0].password)
                {
                    connection.sendUTF('102##0');
                }
                else
                {
                    await database.db_update('user', 'password', "'"+new_password+"'", " user_id = "+"'"+user_id+"'");
                    connection.sendUTF('102##1');
                }

            }
            catch(err)
            {
                console.log(err);
                connection.sendUTF("102##0");
            }
        }
        else if(message[1] == '0' && message[2] == '3') // 103 事件
        {
            try
            {
                var temp = message.split("##");
                var user_id = temp[1];
                var pass_str = temp[2];
                var sel = await database.db_select('user', 'pass_str', 'user_id', "'"+user_id+"'");
                if(sel[0].pass_str != pass_str)
                {
                    connection.sendUTF('103##0');
                }
                else
                {
                    await database.db_update('user', 'password', "'1234'", "user_id = "+"'"+user_id+"'")
                    connection.sendUTF("103##1");
                }

            }
            catch(err)
            {
                console.log(err);
                connection.sendUTF("103##0");
            }
        }
        else if(message[1] == '0' && message[2] == '4') // 104 事件
        {
            try
            {
                var temp = message.split("##");
                var user_id = temp[1];
                var ret  = -1;
                for(var i = 0; i<10;i++)
                {
                    if(game_ui_server.room[i] == 0)
                    {
                        game_ui_server.room[i]++;
                        ret = i;
                        break;
                    }
                }
                if(ret == -1)
                {
                    connection.sendUTF("104##?");
                }
                else
                {
                    await database.db_insert('room',''+ret+',1,1,2');
                    connection.sendUTF("104##"+ret);
                }
                
            }
            catch(err)
            {
                console.log(err);
                connection.sendUTF("104##?");
            }
            

        }
        else if(message[1] == '0' && message[2] == '5') // 105 事件
        {
            try
            {
                var temp = message.split("##");
                var user_id = temp[1];
                var ret  = -1;
                for(var i = 0; i<10;i++)
                {
                    if(game_ui_server.room[i] == 1)
                    {
                        ret = i;
                        break;
                    }
                }
                if(ret == -1)
                {
                    connection.sendUTF("105##?");
                }
                else
                {
                    game_ui_server.update_room(ret, 1);
                    connection.sendUTF("105##"+ret);
                }
                
            }
            catch(err)
            {
                console.log(err);
                connection.sendUTF("105##??");
            }
        }
        else if(message[1] == '0' && message[2] == '6') // 106 事件
        {
            try
            {
                var temp = message.split("##");
                var user_id = temp[1];
                var room_id = parseInt(temp[2]);
                if(game_ui_server.room[room_id] == 1)
                {
                    connection.sendUTF("106##?");
                }
                else
                {
                    await  game_ui_server.update_room(room_id,1);
                    connection.sendUTF("106##1");
                }
                
            }
            catch(err)
            {
                console.log(err);
                connection.sendUTF("106##?");
            }
        }
        else if(message[1] == '0' && message[2] == '7') // 107 事件
        {
            try
            {
                var temp = message.split("##");
                var user_id = temp[1];
                var pass_word = temp[2];
                var pass_str = temp[3];
                var name = temp[4];
                var ban_date =" '2000_01_01'";
                var sel = await database.db_select('user', '*', 'user_id', "'"+user_id+"'");
                if(sel.length > 0)
                {
                    console.log("107 event");
                    connection.sendUTF("107##0");
                }
                else
                {
                    console.log("107 event");
                    await database.db_insert('user', "'"+user_id+"',"+"'"+name+"',"+"'"+pass_word+"',"+ban_date+',0,'+"'"+pass_str+"'");
                    connection.sendUTF("107##1");
                }
            }
            catch(err)
            {
                console.log(err);
                connection.sendUTF("107#0");
            }            
        }
    }
   
});



function getuser(message)
{
    var temp= message.split("##");
    return temp[1];
}
