using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace test_lib_csharp
{
    internal class UPVCSharp
    {
        const string dll_name = "usbpv_lib";
        [DllImport(dll_name, EntryPoint = "upv_list_devices", CharSet =CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        extern static IntPtr upv_list_devices();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate Int32 CallbackDelegate(IntPtr context, UInt32 ts, UInt32 nano, IntPtr data, UInt32 len, UInt32 status);

        [DllImport(dll_name, EntryPoint = "upv_open_device", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        extern static IntPtr upv_open_device(IntPtr option, int option_len, IntPtr context, CallbackDelegate callbak);

        [DllImport(dll_name, EntryPoint = "upv_close_device", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        extern static int upv_close_device(IntPtr dev);

        [DllImport(dll_name, EntryPoint = "upv_get_last_error", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        extern static int upv_get_last_error();

        [DllImport(dll_name, EntryPoint = "upv_get_error_string", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        extern static IntPtr upv_get_error_string(int error);

        [DllImport(dll_name, EntryPoint = "upv_get_monitor_speed", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        extern static int upv_get_monitor_speed(IntPtr dev);

        public static string[] listDevices() {
            List<string> res = new List<string>();
            IntPtr devs = upv_list_devices();
            var r = Marshal.PtrToStringAnsi(devs);
            if (r != null && r != "") {
                return r.Split(',');
            }
            return Array.Empty<string>();
        }

        private IntPtr m_upv = IntPtr.Zero;
        private IntPtr m_this = IntPtr.Zero;

        public enum CaptureSpeed
        {
            UPV_Cap_Speed_High = 0,
            UPV_Cap_Speed_Full = 1,
            UPV_Cap_Speed_Low = 2,
            UPV_Cap_Speed_Auto = 3
        };

        public enum CaptureFlag{
            UPV_FLAG_ACK      = (0x01),
            UPV_FLAG_ISO      = (0x02),
            UPV_FLAG_NAK      = (0x04),
            UPV_FLAG_STALL    = (0x08),

            UPV_FLAG_SOF      = (0x10),
            UPV_FLAG_PING     = (0x20),
            UPV_FLAG_INCOMP   = (0x40),
            UPV_FLAG_ERROR    = (0x80),
            UPV_FLAG_ALL      = (0xff)
        };

        public enum DataSpeed {
            UPV_SPD_Unknown  = 0,
            UPV_SPD_LOW      = 1,
            UPV_SPD_FULL     = 2,
            UPV_SPD_HIGH     = 3
        };

        public enum EventType {
            UPV_DATA_PACKET      = 0,
            UPV_RESET_BEGIN      = 1,
            UPV_RESET_END        = 2,
            UPV_SUSPEND_BEGIN    = 3,
            UPV_SUSPEND_END      = 4,
            UPV_OVERFLOW         = 0xf
        };

        public const byte NO_EP = 0xff;
        public const byte NO_ADDR = 0xff;

        private CallbackDelegate callback_delegate = CallBackFunction;

        public bool Open(string sn, CaptureSpeed speed = CaptureSpeed.UPV_Cap_Speed_Auto, CaptureFlag flag = CaptureFlag.UPV_FLAG_ALL, bool accpet = true, 
            byte addr1 = NO_ADDR, byte ep1 = NO_EP, byte addr2 = NO_ADDR, byte ep2 = NO_EP, byte addr3 = NO_ADDR, byte ep3 = NO_EP, byte addr4 = NO_ADDR, byte ep4 = NO_EP)
        {
            IntPtr option = Marshal.AllocHGlobal(128);
            int option_len = 0;
            var res = true;
            try
            {
                byte[] opt = new byte[128];
                var snByte = Encoding.ASCII.GetBytes(sn);
                Array.Copy(snByte, 0, opt, option_len, snByte.Length);
                option_len = snByte.Length;
                opt[option_len] = 0;
                option_len++;
                opt[option_len++] = (byte)speed;
                opt[option_len++] = (byte)flag;
                opt[option_len++] = accpet ? (byte)1 : (byte)0;
                opt[option_len++] = addr1;
                opt[option_len++] = ep1;
                opt[option_len++] = addr2;
                opt[option_len++] = ep2;
                opt[option_len++] = addr3;
                opt[option_len++] = ep3;
                opt[option_len++] = addr4;
                opt[option_len++] = ep4;
                Marshal.Copy(opt, 0, option, option_len);
                if (m_this != IntPtr.Zero) {
                    var handle = GCHandle.FromIntPtr(m_this);
                    handle.Free();
                    m_this = IntPtr.Zero;
                }
                m_this = GCHandle.ToIntPtr(GCHandle.Alloc(this));
                m_upv = upv_open_device(option, option_len, m_this, callback_delegate);
                if (m_upv == IntPtr.Zero) {
                    res = false;
                }
            }
            finally {
                Marshal.FreeHGlobal(option);
            }
            return res;
        }

        public delegate void PacketDataEvent(uint ts, uint nano, byte[] data, int len, DataSpeed speed);
        public delegate void BusEvent(uint ts, uint nano, EventType type, DataSpeed speed);

        public event PacketDataEvent OnPacketData;
        public event BusEvent OnBusEvent;

        byte[] m_packetBuffer = new byte[8192];

        static Int32 CallBackFunction(IntPtr context, UInt32 ts, UInt32 nano, IntPtr data, UInt32 len, UInt32 status)
        {
            if (context != IntPtr.Zero) {
                UPVCSharp? upvObj = (UPVCSharp?)GCHandle.FromIntPtr(context).Target;
                var speed = (DataSpeed)(status & 0x03);
                var pktType = (EventType)((status >> 4) & 0xff);
                if (pktType == EventType.UPV_DATA_PACKET)
                {
                    Marshal.Copy(data, upvObj.m_packetBuffer, 0, (int)len);
                    upvObj?.OnPacketData?.Invoke(ts, nano, upvObj.m_packetBuffer, (int)len, speed);
                }
                else {
                    upvObj?.OnBusEvent?.Invoke(ts, nano, pktType, speed);
                }
            }
            return 0;
        }
        public string GetLastError()
        {
            var error = upv_get_error_string(upv_get_last_error());
            var res = Marshal.PtrToStringAnsi(error);
            if (res != null) {
                return res;
            }
            return "";
        }

        public int GetMonitorSpeed() {
            if (m_upv != IntPtr.Zero) {
                return upv_get_monitor_speed(m_upv);
            }
            return -1;
        }

        public bool Close() {
            var res = false;
            if (m_upv != IntPtr.Zero)
            {
                int r = upv_close_device(m_upv);
                res = r == 0;
            }
            if (m_this != IntPtr.Zero)
            {
                var handle = GCHandle.FromIntPtr(m_this);
                handle.Free();
                m_this = IntPtr.Zero;
            }
            return res;
        }
    }
}
