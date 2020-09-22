var mysql = require('mysql');
var asynch = require("async_hooks");
const { resolve } = require('path');
const { rejects } = require('assert');
var pool = mysql.createPool({
  host     : 'localhost',
  user     : 'root',
  password : '1234',
  database : 'keshe01',
  connectionLimit: 100
});

// function db_del(table, attri, val, callback)
// {
//   var secflag = new Boolean(false);
//   pool.getConnection(function(erro, connection)
//   {
//     var query = "delete from "+table+" where "+attri+" = "+val+";";
//     connection.query(query, function(err, result)
//     {
//       if(err)
//       {
//         console.log("[DELETE ERROR] - ", err.message);
//         secflag = false;
//       }
//       else
//       {
//         secflag =true;
//         console.log(query+"    OK");
//       }
//       pool.releaseConnection(connection);
//     });
//     callback(secflag);
//   });
  
// }
/** 十分十分十分重要 ： 数据读取的同步性！！！！！， 血的代价 */
let db_del = function(table, attri, val)
{
  return new Promise((resolve, reject)=>
  {
    pool.getConnection(function(err, connection)
    {
      if(err)
      {
        reject(err);
      }
      else
      {
        var query = "delete from "+table+" where "+attri+" = "+val+";";
        console.log(query);
        connection.query(query, (err,result)=>
        {
          if(err)
          {
            reject(err);
          }
          else
          {
            resolve(true);
          }
        })
      }
      pool.releaseConnection(connection);
    })
  })
}

let db_insert = function(table, val)
{
  return new Promise((resolve, reject)=>
  {
    pool.getConnection( function(err, connection)
    {
      if(err )
      {
        reject(false);
      }
      else
      {
        var query = "insert into "+table+" values("+val+");";
        connection.query(query, (err, result)=>
        {
          if(err)
          {
            reject(err)
          }
          else
          {
            resolve(true)
          }
        });
      }
      pool.releaseConnection(connection);
    })
  })
} 

let db_select = function(table, sel_attri, cmp_attri, val)
{
  return new Promise((resolve, reject)=>
  {
    pool.getConnection( function(err, connection)
    {
      if(err )
      {
        reject(false);
      }
      else
      {
        var query = "select "+sel_attri+" from "+table+" where "+cmp_attri+" = "+val+";";
        console.log(query);
        connection.query(query, (err, result)=>
        {
          if(err)
          {
            reject(err)
          }
          else
          {
            resolve(result)
          }
        });
      }
      pool.releaseConnection(connection);
    })
  })
}
let db_update = function(table, set_attri, set_val, condi)
{
  return new Promise((resolve, reject)=>
  {
    pool.getConnection( function(err, connection)
    {
      if(err )
      {
        reject(false);
      }
      else
      {
        var query = "update "+table+" set "+set_attri+" = "+set_val+" where "+condi+";";
        console.log(query);
        connection.query(query, (err, result)=>
        {
          if(err)
          {
            reject(err)
          }
          else
          {
            resolve(result)
          }
        });
      }
      pool.releaseConnection(connection);
    })
  })
}  
// function db_insert(table,  val, callback)
// {
//   var secflag = false;
//   await pool.getConnection(function(error, connection)
//   {
//     var query = "insert into "+table+" values("+val+");";
//     connection.query(query, function(err, result)
//     {
//       if(err)
//       {
//         console.log("[INSERT ERROR] - ", err.message);
//         secflag = false;
//       }
//       else
//       {
//         secflag =true;
//         console.log(query+"    OK");
//       }
//       pool.releaseConnection(connection);
//     });
//     callback(secflag);
//   });
  
// }

// var  sql = "insert into user values('002', 'mao', '1234', '2000-01-01', true);";
// // 选择的结果是一个list，每一个元素是一个dict装属性和属性的值
// connection.query(sql,function (err, result) {
//     if(err){
//         console.log('[SELECT ERROR] - ',err.message);
//         return;
//     }

//     console.log('--------------------------SELECT----------------------------');
//     console.log(result);
//     console.log('------------------------------------------------------------\n\n');
// });
module.exports=
{
  db_insert,
  db_del,
  db_select,
  db_update
}