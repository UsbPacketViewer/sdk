// See https://aka.ms/new-console-template for more information

using System;
using test_lib_csharp;

var devices = UPVCSharp.listDevices();
Console.WriteLine($"There are {devices.Length} device connect");
for(var i = 0; i< devices.Length; i++)
{
    Console.WriteLine(devices[i]);
}

if (devices.Length < 1) {
    return;
}

var upv = new UPVCSharp();

string[] speedStr ={"Unknown", "Low ","Full","High"};

string[] eventStr = { "Data", "Reset Begin", "Reset End", "Suspend Begin", "Suspend End" };

upv.OnPacketData += (ts, nano, data, len, speed) =>
{
    Console.WriteLine($"[{ts}.{nano}]: Got {len} bytes data {speedStr[(int)speed]}, PID: {data[0].ToString("X")}");
};

upv.OnBusEvent += (ts, nano, type, speed) =>
{
    var evtStr = "Unknown";

    if ((int)type <= 4) {
        evtStr = eventStr[(int)type];
    }
    Console.WriteLine($"[{ts}.{nano}]: Got bus event {eventStr[(int)type]}");
};

Console.WriteLine($"Open first device {devices[0]}");
var res = upv.Open(devices[0]);
if (!res) {
    Console.WriteLine($"Open device fail, Reason: {upv.GetLastError()}");
    return;
}

var speed = "High";
var monSpd = upv.GetMonitorSpeed();
if (monSpd < 0) speed = "Error";
else if (monSpd > 0) speed = "Super";
Console.WriteLine($"Monitor speed is { speed } speed");

Console.WriteLine("Press any key to quit");
Console.ReadKey();
res = upv.Close();
if (!res)
{
    Console.WriteLine($"Close fail {upv.GetLastError()}");
}
else
{
    Console.WriteLine($"Close success");
}
