#ifndef __USBPV_LIB_H__
#define __USBPV_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define UPV_CALL __cdecl
#define UPV_CB   __cdecl

#ifdef USBPV_LIB
#define UPV_API  __declspec(dllexport)
#else
#define UPV_API  __declspec(dllimport)
#endif

#else
#define UPV_CALL
#define UPV_CB

#define UPV_API  __attribute__((visibility("default")))
#endif

enum UPV_CaptureSpeed {
  UPV_Cap_Speed_High = 0,
  UPV_Cap_Speed_Full = 1,
  UPV_Cap_Speed_Low = 2,
  UPV_Cap_Speed_Auto = 3
};


// UPV capture type filter flag
#define UPV_FLAG_ACK      (0x01)
#define UPV_FLAG_ISO      (0x02)
#define UPV_FLAG_NAK      (0x04)
#define UPV_FLAG_STALL    (0x08)

#define UPV_FLAG_SOF      (0x10)
#define UPV_FLAG_PING     (0x20)
#define UPV_FLAG_INCOMP   (0x40)
#define UPV_FLAG_ERROR    (0x80)

#define UPV_FLAG_ALL      (0xff)

#define UPV_NO_ADDR       (0xff)
#define UPV_NO_EP         (0xff)

// UPV capture filter accept or drop
#define UPV_FILTER_DROP   (0)
#define UPV_FILTER_ACCEPT (1)


#define UPV_SPD_Unknown  0
#define UPV_SPD_LOW      1
#define UPV_SPD_FULL     2
#define UPV_SPD_HIGH     3
#define GetPacketSpeed(status)  ((status) & 0x03)


#define UPV_DATA_PACKET     0
#define UPV_RESET_BEGIN     1
#define UPV_RESET_END       2
#define UPV_SUSPEND_BEGIN   3
#define UPV_SUSPEND_END     4
#define UPV_OVERFLOW        0xf
#define GetPacketType(status)   (((status)>>4) & 0x0f)

typedef void* UPV_HANDLE;
typedef long(UPV_CB* pfn_packet_handler)(void* context, unsigned long ts, unsigned long nano, const void* data, unsigned long len, long status);

typedef const char* (UPV_CALL *pfnt_upv_list_devices)();
typedef UPV_HANDLE (UPV_CALL *pfnt_upv_open_device)(
        const char* option,
        int option_len,
        void* context,
        pfn_packet_handler callback);
typedef int (UPV_CALL *pfnt_upv_close_device)(UPV_HANDLE upv);
typedef int (UPV_CALL *pfnt_upv_get_last_error)();
typedef const char* (UPV_CALL *pfnt_upv_get_error_string)(int errorCode);
typedef int (UPV_CALL *pfnt_upv_get_monitor_speed)(UPV_HANDLE upv);

/**
 * List connected devices' SN
 * SN separate wiht ','
 * e.g.  "123456A,123456B,123456C"
 * if there is no devices found, return an empty string
 */
UPV_API const char* UPV_CALL upv_list_devices();

/**
 * Open device with specify SN
 * \param option open option,  "<sn>\x00 speed flag filter_type addr1 ep2 addr2 ep2 addr3 ep3 addr4 ep4"
 *        e.g.  "1234567\x00\x03\xff\x01\xff\xff\xff\xff\xff\xff\xff\xff"
 *           == "1234567\x00\x03\xff\x01"
 *           == "1234567\x00\x03\xff"
 *           == "1234567\x00\x03"
 *           == "1234567"
 *                  open device 1234567 with auto speed, accept all packet type, accept all addr and endpoint
 *              "1234567\x00\x01\xff\x01\x01\x01"
 *                  open device 1234567 with full speed, accept all packet type, and accept addr:1, endpoint:1
 *              "1234567\x00\x00\xff\x00\x01\x01"
 *                  open device 1234567 with high speed, accept all packet type, and drop addr:1, endpoint:1
 *
 * \param option_len length of the option. When option_len longer than SN length in option, means the option contains
 *                   more parameter
 * \param context context used in the callback function
 * \param callback packet data callback function
 * \returns the device handler
 */
UPV_API UPV_HANDLE UPV_CALL upv_open_device(
        const char* option,
        int option_len,
        void* context,
        pfn_packet_handler callback);

/**
 * Close the device
 * \param upv device handler open by upv_open_device
 * \returns 0 for succes, otherwise fail
 */
UPV_API int UPV_CALL upv_close_device(UPV_HANDLE upv);

/**
 * Get last error
 */
UPV_API int UPV_CALL upv_get_last_error();

/**
 * Convert error code to string
 */
UPV_API const char* UPV_CALL upv_get_error_string(int errorCode);

/**
 * Get monitor speed
 * \param upv device handler open by upv_open_device
 * \returns 0 high speed, 1 super speed, <0 error
 */
UPV_API int UPV_CALL upv_get_monitor_speed(UPV_HANDLE upv);

#ifdef __cplusplus
}
#endif

#endif
