#include "stdio.h"
#ifdef _WIN32
#include "windows.h"
#else
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#define _strdup strdup
#endif
#include "usbpv_lib.h"


#if _WIN64
#define __SIZEOF_POINTER__ 8
#elif _WIN32
#define __SIZEOF_POINTER__ 4
#endif

#ifdef _WIN32
#if    __SIZEOF_POINTER__ == 8
#define  DLL_NAME "usbpv_lib.dll"
#elif  __SIZEOF_POINTER__ == 4
#define  DLL_NAME "usbpv_lib.dll"
#else
#error Wrong pointer size
#endif
#else
#if    __SIZEOF_POINTER__ == 8
#define  DLL_NAME "./libusbpv_lib.so"
#elif  __SIZEOF_POINTER__ == 4
#define  DLL_NAME "./libusbpv_lib.so"
#else
#error Wrong pointer size
#endif
#endif

#ifdef _MSC_VER
#define strcpy strcpy_s
static char* strtok_ptr = NULL;
#define strtok(s,d) strtok_s(s,d,&strtok_ptr)
#endif

#ifdef _WIN32
FARPROC get_proc(HMODULE mod, const char* name)
{
    FARPROC res = GetProcAddress(mod, name);
    return res;
}
#else
void* get_proc(void* mod, const char* name)
{
    return dlsym(mod, name);
}
#endif

#define GetFuncAddress(mod, name)\
    pfnt_##name func_##name = (pfnt_##name)get_proc(mod, #name)

#ifdef _WIN32
#define UPV_CB_API __cdecl
#else
#define UPV_CB_API
#endif

long UPV_CB_API packet_handler(void* context, unsigned long ts, unsigned long nano, const void* data, unsigned long len, long status);

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
#ifdef _WIN32
    char path[1024];
    ::GetCurrentDirectoryA(sizeof(path), path);
    strcat_s(path,"\\");
    strcat_s(path, DLL_NAME);
    HMODULE mod = LoadLibraryExA(path, NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
    //HMODULE mod = LoadLibraryA(DLL_NAME);
    DWORD err = GetLastError();
    if(mod == NULL){
        printf("Fail to load %s, %ld\n", DLL_NAME, err);
        return -100;
    }
#else
    void* mod = dlopen(DLL_NAME, RTLD_NOW);
    int err = 0;
    if(mod == NULL){
        err = -1;
        printf("Fail to load %s, %s", DLL_NAME, dlerror());
        return -100;
    }

#endif
    GetFuncAddress(mod, upv_list_devices);
    GetFuncAddress(mod, upv_open_device);
    GetFuncAddress(mod, upv_close_device);
    GetFuncAddress(mod, upv_get_last_error);
    GetFuncAddress(mod, upv_get_error_string);
    GetFuncAddress(mod, upv_get_monitor_speed);

    if(!(  func_upv_list_devices
      && func_upv_open_device
      && func_upv_close_device
      && func_upv_get_last_error
      && func_upv_get_error_string
      && func_upv_get_monitor_speed))
    {
        printf("Function load fail\n");
        return -101;
    }

    const char* devices = func_upv_list_devices();
    if(devices[0] == 0){
        printf("No device connected\n");
        return -2;
    }
    char* snList = _strdup(devices);
    int count = 0;
    const char* first = NULL;

    ;
    for (char* sn = strtok(snList, ","); sn; sn = strtok(NULL, ",")) {
        printf("SN: %s\n", sn);
        if (!first) {
            first = sn;
        }
        count++;
    }
    if(!first){
        printf("device sn not found\n");
        return -3;
    }
    printf("there are %d devices, open the first one %s\n", count, first);
    char option[128] = {0};
    int snLen = strlen(first);
    strcpy(option, first);
    unsigned char* pOpt = (unsigned char*)option;
    free(snList);
    int option_len = snLen+1;
    pOpt[option_len++] = UPV_Cap_Speed_Auto;
    pOpt[option_len++] = UPV_FLAG_ALL;      // UPV_FLAG_xxxx
    pOpt[option_len++] = UPV_FILTER_ACCEPT; // Accept or Drop
    pOpt[option_len++] = UPV_NO_ADDR;       // addr1
    pOpt[option_len++] = UPV_NO_EP;         // ep1
    pOpt[option_len++] = UPV_NO_ADDR;       // addr2
    pOpt[option_len++] = UPV_NO_EP;         // ep2
    pOpt[option_len++] = UPV_NO_ADDR;       // addr3
    pOpt[option_len++] = UPV_NO_EP;         // ep3
    pOpt[option_len++] = UPV_NO_ADDR;       // addr4
    pOpt[option_len++] = UPV_NO_EP;         // ep4

    UPV_HANDLE upv = func_upv_open_device(option, option_len, NULL, packet_handler);
    if (!upv) {
        int err = func_upv_get_last_error();
        const char* errStr = func_upv_get_error_string(err);
        printf("Open device fail %d %s\n", err, errStr);
        return -4;
    }

    int speed = func_upv_get_monitor_speed(upv);
    if (speed < 0) {
        printf("Fail to get capture monitor speed\n");
    }
    else {
        printf("Device monitor speed is %d %s\n", speed, speed ? "Super Speed" : "High Speed");
    }

    printf("Press any key to quit capture\n");
    int ch = getchar();
    (void)ch;
   
    int r = func_upv_close_device(upv);
    if (r == 0) {
        printf("Close device success\n");
    }
    else {
        printf("Close device fail %d %s", r, func_upv_get_error_string(r));
    }
    return 0;
}

const char* SPD_STR[] = { "Xxxx","Low ","Full","High" };

long UPV_CB_API packet_handler(void* context, unsigned long ts, unsigned long nano, const void* data, unsigned long len, long status)
{
    int speed = GetPacketSpeed(status);
    int pktType = GetPacketType(status);
    (void)context;
    (void)data;
    if (pktType == UPV_DATA_PACKET) {
        printf("[%ld.%9ld] T:%d S:%s", ts, nano, pktType, SPD_STR[speed]);
        for(unsigned long i=0;i<len;i++){
            printf(" %02x", ((unsigned char*)data)[i]);
        }
        printf("\n");
    }
    else {
        printf("[%ld.%9ld] T:%d Bus Event\n", ts, nano, pktType);
    }
    return 0;
}

