var database= require('./database');
var asynch = require("async_hooks");
var room = [];
var room_info = {};
var room_max_num = 10;
for(var i = 0; i<=room_max_num; i++)
{
    room[i]= 0;
}

//  检查还有没有多样的房间
function chk_room()
{
    for(var i = 0; i<room_max_num;i++)
    {
        if(room[i] < 2)
        {
            room[i] ++;
            return i;
        }
    }
    console.log("room full");
    return -1;
}
function getuser(message)
{
    var temp= message.split("##");
    return temp[1];
}
async function update_room(idx, num)
{
    try
    {
        room[idx]+=num ;
        if(room[idx] > 0)
        {
            await database.db_update('room', 'play_num', ''+room[idx], ' room_id = '+idx);
        }
        else 
        {
            await database.db_del('room', 'room_idx', ''+idx);
        }
        
    }
    catch(err)
    {
        console.log("catch error :"+err);
    }
    
}
async function m401 (message) // 401事件 ：用户摁下准备按钮
{
    return new Promise(async (resolve, reject)=>
{
    var user_id = getuser(message);
    var room_id = 0;
    var ret={user_id: user_id, room_id:room_id} ;
    if(room_id == -1)
    {
        return false;
    }
    if(message[3] == '0') // 用户取消准备
    {
        
        try
        {
            await database.db_update('in_room', 'is_ready', '0', " user_id  "+"'"+user_id+"'");
            ret.val = true;
            console.log(ret);
            resolve(ret);
        }
        catch(err)
        {
            ret.val = false
            console.log("catch "+err);
            reject(ret);
        }
    }
    else if(message[3] == '1')
    {
        try 
        {
            await database.db_update('in_room', 'is_ready', '1', " user_id = "+"'"+user_id+"'");
            ret.val = true;
            resolve(ret);
        }
        catch(error)
        {
            ret.val = false
            console.log("catch "+error);
            reject(ret);
        }
    }
    
});
}

module.exports=
{
    m401,
    update_room,
    room
}