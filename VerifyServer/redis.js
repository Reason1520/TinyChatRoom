const config_module = require('./config')
const Redis = require("ioredis");

// 创建Redis客户端实例
const RedisCli = new Redis({
  host: config_module.redis_host,       // Redis服务器主机名
  port: config_module.redis_port,        // Redis服务器端口号
  password: config_module.redis_passwd, // Redis密码
  enableOfflineQueue: false, // <--- 关键：禁止离线队列，没连上就报错
  lazyConnect: false         // 立即连接
});

RedisCli.on("connect", () => {
  console.log("📡 RedisCli: 正在尝试连接 6380...");
});

RedisCli.on("ready", () => {
  console.log("✅ RedisCli: 身份验证通过，连接已就绪！");
});

RedisCli.on("error", (err) => {
  console.log("❌ RedisCli 发生错误:", err.message);
});


/**
 * 监听错误信息
 */
RedisCli.on("error", function (err) {
  console.log("RedisCli connect error");
  RedisCli.quit();
});

/**
 * 根据key获取value
 * @param {*} key 
 * @returns 
 */
async function GetRedis(key) {
    
    try{
        const result = await RedisCli.get(key)
        if(result === null){
          console.log('result:','<'+result+'>', 'This key cannot be find...')
          return null
        }
        console.log('Result:','<'+result+'>','Get key success!...');
        return result
    }catch(error){
        console.log('GetRedis error is', error);
        return null
    }

  }

/**
 * 根据key查询redis中是否存在key
 * @param {*} key 
 * @returns 
 */
async function QueryRedis(key) {
    try{
        const result = await RedisCli.exists(key)
        //  判断该值是否为空 如果为空返回null
        if (result === 0) {
          console.log('result:<','<'+result+'>','This key is null...');
          return null
        }
        console.log('Result:','<'+result+'>','With this value!...');
        return result
    }catch(error){
        console.log('QueryRedis error is', error);
        return null
    }

  }

/**
 * 设置key和value，并过期时间
 * @param {*} key 
 * @param {*} value 
 * @param {*} exptime 
 * @returns 
 */
async function SetRedisExpire(key, value, exptime){
    console.log(`🚀 Trying to set Redis: Key=${key}, Value=${value}`); // 添加这一行
    try {
        const res1 = await RedisCli.set(key, value);
        console.log("Set Result:", res1); // 打印 set 结果
        const res2 = await RedisCli.expire(key, exptime);
        console.log("Expire Result:", res2); // 打印 expire 结果
        return true;
    } catch(error) {
        console.log('❌ SetRedisExpire error is', error);
        return false;
    }
}

/**
 * 退出函数
 */
function Quit(){
    RedisCli.quit();
}

module.exports = {GetRedis, QueryRedis, Quit, SetRedisExpire,}