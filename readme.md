# USB Packet Viewer API说明

## 列出已经连接的设备

函数名称：upv_list_devices

参数：无

返回值: 以半角逗号分隔的设备序列号，如果没有设备连接，返回一个空字符串

返回值示例:

  "1234567" 只有一个设备，序列号为 1234567
  "1234567,1234568" 有两个设备，序列号分别为 1234567 1234568

## 打开设备

函数名称：upv_open_device

参数：
1. option 设备参数，与服务端模式时的打开参数含义相同
2. option_len 参数数据长度，当参数长度小于option中序列号字符串长度时，表示只有序列号，其它参数均为默认值。option参数详细说明见附录
3. context 上下文，API中不会使用这个参数，这个参数会作为回调函数的第一个参数传入
4. callback 数据回调函数，当有数据或总线事件时，调用此函数

返回值： 非空为设备句柄，当返回为空时设备打开出错，通过upv_get_last_error函数获取错误原因。

## 打开设备回调函数

函数原型：typedef long(UPV_CB* pfn_packet_handler)(void* context, unsigned long ts, unsigned long nano, const void* data, unsigned long len, long status);

参数：
1. context 由upv_open_device函数传入的context参数
2. ts 以秒为单的时间戳
3. nano 以纳秒为单的时间戳
4. data 数据包内容
5. len 数据包数据长度
6. status 数据包状态，bit0-bit1 数据包速率  bit4-bi7 数据包类型， 其它位，内部使用

返回值： 目前没有使用，默认返回0

## 关闭设备

函数名称：upv_close_device

参数：
1. upv 由upv_open_device函数返回的设备句柄

返回值：0表示成功，其它值失败，通过upv_get_error_string获取出错的文字描述

## 获取最后一次错误

函数名称：upv_get_last_error

参数：无

返回值：最后一次错误原因

## 获取错误文字描述

函数名称：upv_get_error_string

参数：
1. errorCode 错误值

返回值： 错误描述

## 获取抓包器监听口的速率

函数名称：upv_get_monitor_speed

参数：
1. upv 由upv_open_device函数返回的设备句柄

返回值：0 高速模式  1 SuperSpeed模式

## 文件结构说明
```
|--- usbpv_lib.h         USBPV API头文件
|--- main.cpp            USBPV API示例代码
|--- Makefile            Linux 下的编译文件
|--- usbpv_test_lib.pro  Windows和Linux下的Qt示例工程文件
|--- test_lib.vcxproj    Visual Studio 工程文件
|--- x86                 32位程序相关目录
|    |--- *.dll               32位程序依赖的动态库
|    |--- test_lib_msvc.exe   MSVC编译的32位示例程序
|    |--- usbpv_test_lib.exe  Qt MingW编译的32位示例程序
|
|--- x64                 64位程序相关目录
|    |--- *.dll               64位程序依赖的动态库
|    |--- test_lib_msvc.exe   MSVC编译的64位Windows示例程序
|    |--- usbpv_test_lib.exe  Qt MingW编译的64位Windows示例程序
|    |--- libusbpv_lib.so     Linux动态库
|    |--- usbpv_test_lib      Qt 编译的64位Linux示例程序
|    |--- usbpv_test_gcc      Makefile编译的64位Linux示例程序
|
|--- test_lib_csharp    C#版本的测试程序
|    |--- test_lib_csharp.csproj    C#工程文件
|    |--- upvcsharp.cs              C#动态库封装程序
|    |--- Program.cs                C#动态库测试程序
|
|--- usbpv_test.py      64位Python版本的测试程序
```

## 附录：打开设备参数举例

### 使用默认参数打开设备

```C
// 下面的例子中，假设设备序列号为"1234567"
const char* sn = "1234567";
char option[256] = {0};
int optLen = 7;
// 打开设备，使用自动速率，接受所有类型的包，接受所有地址和端点的包
memset(option,0,256);
strcpy(option,sn);                      // 设置序列号
upv_open_device(option,0,0,cb);         // 打开序列号为sn的设备，全部使用默认参数
upv_open_device(option,optLen,0,cb);    // 与上一个open等效，长度小于等于序列号字符串长度
option[optLen++] = 0;                   // 序列号字符串结束
upv_open_device(option,optLen,0,cb);    // 与上一个open等效
option[optLen++] = UPV_Cap_Speed_Auto;  // 设置抓包速率为自动
upv_open_device(option,optLen,0,cb);    // 与上一个open等效
option[optLen++] = UPV_FLAG_ALL;        // 设置类型过滤标志为全部接收
upv_open_device(option,optLen,0,cb);    // 与上一个open等效
option[optLen++] = UPV_FILTER_ACCEPT;   // 设置地址端点过滤方式为接收
upv_open_device(option,optLen,0,cb);    // 与上一个open等效
option[optLen++] = UPV_NO_ADDR;         // 设置过滤地址1为无效
upv_open_device(option,optLen,0,cb);    // 与上一个open等效
option[optLen++] = UPV_NO_EP;           // 设置过滤端点1为无效
upv_open_device(option,optLen,0,cb);    // 与上一个open等效
option[optLen++] = UPV_NO_ADDR;         // 设置过滤地址2为无效
upv_open_device(option,optLen,0,cb);    // 与上一个open等效
option[optLen++] = UPV_NO_EP;           // 设置过滤端点2为无效
upv_open_device(option,optLen,0,cb);    // 与上一个open等效
option[optLen++] = UPV_NO_ADDR;         // 设置过滤地址3为无效
upv_open_device(option,optLen,0,cb);    // 与上一个open等效
option[optLen++] = UPV_NO_EP;           // 设置过滤端点3为无效
upv_open_device(option,optLen,0,cb);    // 与上一个open等效
option[optLen++] = UPV_NO_ADDR;         // 设置过滤地址4为无效
upv_open_device(option,optLen,0,cb);    // 与上一个open等效
option[optLen++] = UPV_NO_EP;           // 设置过滤端点4为无效
upv_open_device(option,optLen,0,cb);    // 与上一个open等效
```

### 使用指定速率打开设备
```c
// 下面的例子中，假设设备序列号为"1234567"
const char* sn = "1234567";
char option[256] = {0};
int optLen = 7;
// 打开设备，使用自动速率，接受所有类型的包，接受所有地址和端点的包
memset(option,0,256);
strcpy(option,sn);                     // 设置序列号
option[optLen++] = 0;                  // 序列号字符串结束
if(speed == 0){
    option[optLen++] = UPV_Cap_Speed_High;   // 抓包速率设置为高速
}else if(speed == 1){
    option[optLen++] = UPV_Cap_Speed_Full;   // 抓包速率设置为全速
}else if(speed = 2){
    option[optLen++] = UPV_Cap_Speed_Low;    // 抓包速率设置为低速
}else{
    option[optLen++] = UPV_Cap_Speed_Auto;    // 抓包速率设置为自动
}
upv_open_device(option,optLen,0,cb);    // 以上述参数打开设备
```

### 只抓取特定类型的包
```c
// 下面的例子中，假设设备序列号为"1234567"
const char* sn = "1234567";
char option[256] = {0};
int optLen = 7;
// 打开设备，使用自动速率，接受所有类型的包，接受所有地址和端点的包
memset(option,0,256);
strcpy(option,sn);                     // 设置序列号
option[optLen++] = 0;                  // 序列号字符串结束
option[optLen++] = UPV_Cap_Speed_Auto; // 抓包速率设置为自动
option[optLen] = 0;
if(cap_ack){
    option[optLen] |= UPV_FLAG_ACK;    // 抓取类型为ACK的包
}
if(cap_iso){
    option[optLen] |= UPV_FLAG_ISO;    // 抓取类型为ISO的包
}
if(cap_nak){
    option[optLen] |= UPV_FLAG_NAK;    // 抓取类型为NAK的包
}
if(cap_stall){
    option[optLen] |= UPV_FLAG_STALL;  // 抓取类型为STALL的包
}
if(cap_sof){
    option[optLen] |= UPV_FLAG_SOF;    // 抓取类型为SOF的包
}
if(cap_ping){
    option[optLen] |= UPV_FLAG_PING;    // 抓取Ping包和全速hub的低速前导包
}
if(cap_incomp){
    option[optLen] |= UPV_FLAG_INCOMP;  // 抓取未完成的包
}
if(cap_error){
    option[optLen] |= UPV_FLAG_ERROR;   // 抓取错误包
}
optLen++;
upv_open_device(option,optLen,0,cb);    // 以上述参数打开设备
```
### 按地址和端点过滤包，接受满足条件的包
```c
// 下面的例子中，假设设备序列号为"1234567"
const char* sn = "1234567";
char option[256] = {0};
int optLen = 7;
// 打开设备，使用自动速率，接受所有类型的包，接受所有地址和端点的包
memset(option,0,256);
strcpy(option,sn);                     // 设置序列号
option[optLen++] = 0;                  // 序列号字符串结束
option[optLen++] = UPV_Cap_Speed_Auto; // 抓包速率设置为自动
option[optLen++] = UPV_FLAG_ALL;       // 抓取所有类型的包
option[optLen++] = UPV_FILTER_ACCEPT;  // 抓取地址和端点满足条件的包
if(xx0){
    // 不设置地址和端点相关信息，表示任意地址和端点都满足要求，相当于抓取所有包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，抓取地址任意值端点为任意值的包
}else if(xx1){
    option[optLen++] = 0;                  // 抓取地址为0的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，抓取地址为0端点为任意值的包
}else if(xx2){
    option[optLen++] = 0;                  // 抓取地址为0的包
    option[optLen++] = 0;                  // 抓取端点为0的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，抓取地址为0并且端点为0的包
}else if(xx3){
    option[optLen++] = 3;                  // 抓取地址为3的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，抓取地址为3并且端点为任意值的包
}else if(xx4){
    option[optLen++] = 3;                  // 抓取地址为3的包
    option[optLen++] = 1;                  // 抓取端点为1的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，抓取地址为3并且端点为1的包
}else if(xx4){
    option[optLen++] = 3;                  // 抓取地址为3的包
    option[optLen++] = 1;                  // 抓取端点为1的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，抓取地址为3并且端点为1的包
}else if(xx5){
    option[optLen++] = UPV_NO_ADDR;        // 抓取地址为任意的包
    option[optLen++] = 1;                  // 抓取端点为1的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，抓取地址为任意值端点为1的包
}else if(xx6){
    option[optLen++] = UPV_NO_ADDR;        // 抓取地址为任意的包
    option[optLen++] = 1;                  // 抓取端点为1的包
    option[optLen++] = UPV_NO_ADDR;        // 抓取地址为任意的包
    option[optLen++] = 2;                  // 抓取端点为2的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，抓取地址为任意值端点为1或2的包
}else if(xx7){
    option[optLen++] = UPV_NO_ADDR;        // 抓取地址为任意的包
    option[optLen++] = 1;                  // 抓取端点为1的包
    option[optLen++] = UPV_NO_ADDR;        // 抓取地址为任意的包
    option[optLen++] = 2;                  // 抓取端点为2的包
    option[optLen++] = UPV_NO_ADDR;        // 抓取地址为任意的包
    option[optLen++] = 3;                  // 抓取端点为3的包
    option[optLen++] = UPV_NO_ADDR;        // 抓取地址为任意的包
    option[optLen++] = 4;                  // 抓取端点为4的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，抓取地址为任意值端点为1、2、3或4的包
}else if(xx8){
    option[optLen++] = 13;                 // 抓取地址为13的包
    option[optLen++] = UPV_NO_EP;          // 抓取端点为任意值的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，抓取地址为13值端点为任意值的包
}else if(xx9){
    option[optLen++] = 2;                  // 抓取地址为2的包
    option[optLen++] = UPV_NO_EP;          // 抓取端点为任意值的包
    option[optLen++] = 3;                  // 抓取地址为3的包
    option[optLen++] = UPV_NO_EP;          // 抓取端点为任意值的包
    option[optLen++] = 4;                  // 抓取地址为4的包
    option[optLen++] = UPV_NO_EP;          // 抓取端点为任意值的包
    option[optLen++] = 5;                  // 抓取地址为5的包
    option[optLen++] = UPV_NO_EP;          // 抓取端点为任意值的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，抓取地址为2、3、4、或5且端点为任意值的包
}else if(xx10){
    option[optLen++] = 2;                  // 抓取地址为2的包
    option[optLen++] = 3;                  // 抓取端点为3的包
    option[optLen++] = 3;                  // 抓取地址为3的包
    option[optLen++] = 2;                  // 抓取端点为2的包
    option[optLen++] = 4;                  // 抓取地址为4的包
    option[optLen++] = 1;                  // 抓取端点为1的包
    option[optLen++] = 5;                  // 抓取地址为5的包
    option[optLen++] = 3;                  // 抓取端点为3的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，抓取地址和端点满足2.3或3.2或4.1或5.3的包
}
```

### 按地址和端点过滤包，丢弃满足条件的包

丢弃与抓取的逻辑刚好相反，当设置过滤标志为UPV_FILTER_DROP后，满足条件的包会丢弃，不满足条件的包会接收。
```c
// 下面的例子中，假设设备序列号为"1234567"
const char* sn = "1234567";
char option[256] = {0};
int optLen = 7;
// 打开设备，使用自动速率，接受所有类型的包，接受所有地址和端点的包
memset(option,0,256);
strcpy(option,sn);                     // 设置序列号
option[optLen++] = 0;                  // 序列号字符串结束
option[optLen++] = UPV_Cap_Speed_Auto; // 抓包速率设置为自动
option[optLen++] = UPV_FLAG_ALL;       // 抓取所有类型的包
option[optLen++] = UPV_FILTER_DROP;    // 丢弃地址和端点满足条件的包
if(xx0){
    ////////////////////////////////////////////
    //// 警告 警告 警告 警告 警告 警告 警告 警告
    //// 警告 警告 警告 警告 警告 警告 警告 警告
    //// 不设置地址和端点相关信息，表示任意地址和端点都满足要求，相当于丢弃所有包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，丢弃地址任意值端点为任意值的包
}else if(xx1){
    option[optLen++] = 0;                  // 丢弃地址为0的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，丢弃地址为0端点为任意值的包
}else if(xx2){
    option[optLen++] = 0;                  // 丢弃地址为0的包
    option[optLen++] = 0;                  // 丢弃端点为0的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，丢弃地址为0并且端点为0的包
}else if(xx3){
    option[optLen++] = 3;                  // 丢弃地址为3的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，丢弃地址为3并且端点为任意值的包
}else if(xx4){
    option[optLen++] = 3;                  // 丢弃地址为3的包
    option[optLen++] = 1;                  // 丢弃端点为1的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，丢弃地址为3并且端点为1的包
}else if(xx4){
    option[optLen++] = 3;                  // 丢弃地址为3的包
    option[optLen++] = 1;                  // 丢弃端点为1的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，丢弃地址为3并且端点为1的包
}else if(xx5){
    option[optLen++] = UPV_NO_ADDR;        // 丢弃地址为任意的包
    option[optLen++] = 1;                  // 丢弃端点为1的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，丢弃地址为任意值端点为1的包
}else if(xx6){
    option[optLen++] = UPV_NO_ADDR;        // 丢弃地址为任意的包
    option[optLen++] = 1;                  // 丢弃端点为1的包
    option[optLen++] = UPV_NO_ADDR;        // 丢弃地址为任意的包
    option[optLen++] = 2;                  // 丢弃端点为2的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，丢弃地址为任意值端点为1或2的包
}else if(xx7){
    option[optLen++] = UPV_NO_ADDR;        // 丢弃地址为任意的包
    option[optLen++] = 1;                  // 丢弃端点为1的包
    option[optLen++] = UPV_NO_ADDR;        // 丢弃地址为任意的包
    option[optLen++] = 2;                  // 丢弃端点为2的包
    option[optLen++] = UPV_NO_ADDR;        // 丢弃地址为任意的包
    option[optLen++] = 3;                  // 丢弃端点为3的包
    option[optLen++] = UPV_NO_ADDR;        // 丢弃地址为任意的包
    option[optLen++] = 4;                  // 丢弃端点为4的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，丢弃地址为任意值端点为1、2、3或4的包
}else if(xx8){
    option[optLen++] = 13;                 // 丢弃地址为13的包
    option[optLen++] = UPV_NO_EP;          // 丢弃端点为任意值的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，丢弃地址为13值端点为任意值的包
}else if(xx9){
    option[optLen++] = 2;                  // 丢弃地址为2的包
    option[optLen++] = UPV_NO_EP;          // 丢弃端点为任意值的包
    option[optLen++] = 3;                  // 丢弃地址为3的包
    option[optLen++] = UPV_NO_EP;          // 丢弃端点为任意值的包
    option[optLen++] = 4;                  // 丢弃地址为4的包
    option[optLen++] = UPV_NO_EP;          // 丢弃端点为任意值的包
    option[optLen++] = 5;                  // 丢弃地址为5的包
    option[optLen++] = UPV_NO_EP;          // 丢弃端点为任意值的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，丢弃地址为2、3、4、或5且端点为任意值的包
}else if(xx10){
    option[optLen++] = 2;                  // 丢弃地址为2的包
    option[optLen++] = 3;                  // 丢弃端点为3的包
    option[optLen++] = 3;                  // 丢弃地址为3的包
    option[optLen++] = 2;                  // 丢弃端点为2的包
    option[optLen++] = 4;                  // 丢弃地址为4的包
    option[optLen++] = 1;                  // 丢弃端点为1的包
    option[optLen++] = 5;                  // 丢弃地址为5的包
    option[optLen++] = 3;                  // 丢弃端点为3的包
    upv_open_device(option,optLen,0,cb);   // 抓取所有类型的包，丢弃地址和端点满足2.3或3.2或4.1或5.3的包
}
```

