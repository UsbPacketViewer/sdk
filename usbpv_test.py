import ctypes,sys

class CapSpeed:
    HighSpeed = 0
    FullSpeed = 1
    LowSpeed = 2
    AutoSpeed = 3
class CapFlag:
    ACK    = 0x01
    ISO    = 0x02
    NAK    = 0x04
    STALL  = 0x08
    SOF    = 0x10
    PING   = 0x20
    INCOMP = 0x40
    ERROR  = 0x80
    ALL    = 0xFF

ANY_ADDR = 0xff
ANY_EP   = 0xff


upv_data_func_type = ctypes.CFUNCTYPE(ctypes.c_int32, ctypes.py_object, ctypes.c_int32, ctypes.c_int32, ctypes.POINTER(ctypes.c_ubyte), ctypes.c_int32, ctypes.c_int32)

def UPV_Callback(upv, ts, nano, data, data_len, status):
    if upv.on_packet:
        upv.on_packet(ts, nano, data, data_len, status)
    else:
        print(ts,nano,data_len,status)
    return 0

packet_handler = upv_data_func_type(UPV_Callback)

class UPV:
    def __init__(self):
        self.lib = ctypes.CDLL('./x64/usbpv_lib')
        self.lib.upv_list_devices.restype = ctypes.c_char_p

        self.lib.upv_open_device.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.py_object, upv_data_func_type]
        self.lib.upv_open_device.restype = ctypes.c_void_p

        self.lib.upv_close_device.argtypes = [ctypes.c_void_p]
        self.lib.upv_close_device.restype = ctypes.c_int

        self.lib.upv_get_last_error.restype = ctypes.c_int
        self.lib.upv_get_error_string.argtypes =  [ctypes.c_int]
        self.lib.upv_get_error_string.restype = ctypes.c_char_p

        self.lib.upv_get_monitor_speed.argtypes = [ctypes.c_void_p]
        self.lib.upv_get_monitor_speed.restype = ctypes.c_int

        def wrap_UPV_Callback(upv, ts, nano, data, data_len, status):
            if self.on_packet:
                self.on_packet(ts, nano, data, data_len, status)
            else:
                print("UPV_Callback", data_len)
            return 0
        self.c_upv_callback = upv_data_func_type(wrap_UPV_Callback)


        self.dev = None

    def list_devices(self):
        res = self.lib.upv_list_devices()
        devs = res.decode('ascii')
        if devs != '':
            return devs.split(',')
        else:
            return []

    def last_error(self):
        err = self.lib.upv_get_last_error()
        err = self.lib.upv_get_error_string(err)
        return err.decode('ascii')

    def close(self):
        r = self.lib.upv_close_device(self.dev)
        self.dev = None
        print("close", r)
        if r != 0:
            print("Fail to close device", self.last_error())
        return r == 0
    def isUSB30(self):
        r = self.lib.upv_get_monitor_speed(self.dev)
        return r == 1

    def open(self, sn, speed = CapSpeed.AutoSpeed, flag = CapFlag.ALL, accept = True, addr1 = ANY_ADDR, ep1 = ANY_EP, addr2 = ANY_ADDR, ep2 = ANY_EP, addr3 = ANY_ADDR, ep3 = ANY_EP, addr4 = ANY_ADDR, ep4 = ANY_EP):
        #print("open",sn, speed, flag, accept, addr1,ep1,addr2,ep2,addr3,ep3,addr4,ep4)
        opt_len = len(sn)+12
        param_idx = len(sn) + 1
        option = ctypes.create_string_buffer(sn.encode('ascii'), opt_len)
        option[param_idx+0] = speed
        option[param_idx+1] = flag
        option[param_idx+2] = 1 if accept else 0
        option[param_idx+3] = addr1
        option[param_idx+4] = ep1
        option[param_idx+5] = addr2
        option[param_idx+6] = ep2
        option[param_idx+7] = addr3
        option[param_idx+8] = ep3
        option[param_idx+9] = addr4
        option[param_idx+10] = ep4
        print(option.value)
        self.dev = self.lib.upv_open_device(option, opt_len, self, self.c_upv_callback)
        if not self.dev:
            print("Fail to open",sn,"  Reason:", self.last_error())
        return self.dev != None

speedStr = ['Xxxx','Low ','Full','High']
busEventStr = ['Packet', 'Reset begin', 'Reset end', 'Suspend begin', 
               'Suspend End', 'Unknown', 'Unknown', 'Unknown',
               'Unknown','Unknown','Unknown','Unknown',
               'Unknown','Unknown','Unknown','Overflow']
def on_packet(ts, nano, data, data_len, status):
    pkt_type = status & 0xf0
    speed = status & 0x0f
    if pkt_type == 0:
        # hex_string = ' '.join(hex(data[i]) for i in range(data_len))
        # print(hex_string)
        print( f"[{ts}.{nano}] PID: {data[0]:02x} {data[data_len-1]:02x} LEN:{data_len} Spd: {speedStr[speed&0x03]}")
    else:
        print( f"[{ts}.{nano}] bus event {busEventStr[pkt_type>>4]}")
    return 0

def run_test():
    upv = UPV()
    devs = upv.list_devices()
    if len(devs) < 1:
        print("No device connected")
        sys.exit(-1)
    for dev in devs:
        print(dev)

    upv.on_packet = on_packet
    res = upv.open(devs[0])
    if not res:
        print("Fail to open", devs[0], upv.last_error())
        return
    print("Monitor speed is ",'Super speed' if upv.isUSB30() else 'High speed')
    input("press any key to exit")
    upv.close()

run_test()
