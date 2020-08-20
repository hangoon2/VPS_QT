#ifndef VPSCOMMON_H
#define VPSCOMMON_H

#define MAXCHCNT    10

#define RECV_BUFFER_SIZE            (1024 * 1024)

#define CMD_START_CODE  0x7F
#define CMD_END_CODE    0xEF

#define CMD_HEAD_SIZE   8
#define CMD_TAIL_SIZE   3

#define CMD_START   1
#define CMD_STOP    2

#define CMD_RESOURCE_MONITOR_START  14
#define CMD_RESOURCE_MONITOR_STOP   15

#define CMD_RESOURCE_USAGE_NETWROK  16
#define CMD_RESOURCE_USAGE_CPU      17
#define CMD_RESOURCE_USAGE_MEMORY   18

#define CMD_TEST_RESULT     309

#define CMD_AUTOTOOL_START_EVENT_INDEX      318
#define CMD_AUTOTOOL_START_EVENT_PATH       319
#define CMD_AUTOTOOL_START_SCRIPT_RESULT    320

#define CMD_VERTICAL        1003
#define CMD_HORIZONTAL      1004
#define CMD_AVIRECORD       1005
#define CMD_JPGCAPTURE      1006
#define CMD_STOP_RECORDING  1007

#define CMD_WAKEUP                  1008
#define CMD_USBMHL_SWITCHING        1009

#define CMD_ACK                     10001
#define CMD_LOGCAT                  10003

#define CMD_CAPTURE_COMPLETED       10004

#define CMD_JPG_DEV_VERT_IMG_VERT   20004
#define CMD_JPG_DEV_HORI_IMG_HORI   20005
#define CMD_JPG_DEV_VERT_IMG_HORI   20006
#define CMD_JPG_DEV_HORI_IMG_VERT   20007

#define CMD_ID          30000
#define CMD_ID_AUDIO    30005
#define CMD_ID_GUEST    30007
#define CMD_ID_MONITOR  30008

#define CMD_CONVERTER_MIRACAST_POWER_ONOFF  30006

#define CMD_PLAYER_QUALITY  30001
#define CMD_PLAYER_FPS      30002
#define CMD_PLAYER_EXIT     30010

#define CMD_DISCONNECT_GUEST    30011
#define CMD_UPDATE_SERVICE_TIME 30012
#define CMD_GUEST_UPDATED       31001

#define CMD_MIRRORING_STOPPED               22002
#define CMD_MIRRORING_JPEG_CAPTURE_FAILED   32401

#define CMD_DEVICE_DISCONNECTED     22003

#define CMD_MONITOR_VD_HEARTBEAT    32100

#define CMD_DUPLICATED_CLIENT       32201

#define CMD_SEND_KEY_FRAME          21003

#define CMD_CHANGE_RESOLUTION       21001
#define CMD_CHANGE_RESOLUTION_RATIO 21004

#define CMD_REQUEST_MAX_RESOLUTION  21005

#define SOCKET_ERROR    -1
#define EXIT_FAILURE    1

#define VPS_DEFAULT_JPG_QUALITY     70

#define FULLHD_IMAGE_SIZE (1382400)     //(960 * 960 * 4)


typedef char BYTE;
typedef unsigned char ONYPACKET_UINT8;
typedef unsigned short ONYPACKET_UINT16;
typedef int ONYPACKET_INT;
typedef long ONYPACKET_INT32;
typedef long long ONYPACKET_INT64;

typedef unsigned long DWORD;
typedef char* LPSTR;

typedef struct TAG_HDCAP
{
    int msec;
    int width;
    int height;
    int bytesPerLine;
    BYTE btImg[FULLHD_IMAGE_SIZE];
}HDCAP;

#endif // VPSCOMMON_H
