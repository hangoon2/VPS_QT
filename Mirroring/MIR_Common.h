#ifndef MIR_COMMON_H
#define MIR_COMMON_H

#include "VPSCommon.h"

#define ONY_CMD_AUDIO           20003
#define ONY_CMD_RESOL           21001
#define ONY_CMD_ONOFF           21002
#define ONY_CMD_SEND_KEY_FRAME  21003
#define ONY_COMD_RESOL_RATIO    21004

#define MIR_READ_BUF_SIZE           (2 * 960 * 960)
#define SEND_BUFFER_SIZE    512

ONYPACKET_UINT16 calChecksum(unsigned short *ptr, int nbytes);
bool MIR_IsJpgPacket(const BYTE* pPacket);
bool MIR_IsJpgPacket(short nCmd);
bool MIR_IsKeyFrame(const BYTE* pPacket);
const ONYPACKET_UINT8* MakeOnyPacketControl(ONYPACKET_UINT8* sendBuf, int nHpNo, BYTE onoff, int &size);
const ONYPACKET_UINT8* MakeOnyPacketSendKeyFrame(ONYPACKET_UINT8* sendBuf, int nHpNo, int &size);
const ONYPACKET_UINT8* MakeOnyPacketRecordFrame(ONYPACKET_UINT8* sendBuf, int nHpNo, BYTE* pData, int dataSize, int &size);

#endif // MIR_COMMON_H
