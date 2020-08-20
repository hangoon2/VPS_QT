#include "MIR_Common.h"
#include "VPSCommon.h"

#include <QThread>

ONYPACKET_UINT16 calChecksum(unsigned short *ptr, int nbytes)
{
    ONYPACKET_INT32 sum;
    ONYPACKET_UINT16 answer;

    sum = 0;
    while(nbytes > 1)
    {
        sum += *ptr++;
        nbytes -= 2;
    }

    if(nbytes == 1)
    {
        sum += *(ONYPACKET_UINT8*)ptr;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = (ONYPACKET_UINT16)~sum;

    return answer;
}

bool MIR_IsJpgPacket(const BYTE* pPacket)
{
    short nCmd = ntohs(*(short*)&pPacket[5]);
    if(nCmd == CMD_JPG_DEV_VERT_IMG_VERT ||
            nCmd == CMD_JPG_DEV_HORI_IMG_HORI ||
            nCmd == CMD_JPG_DEV_VERT_IMG_HORI ||
            nCmd == CMD_JPG_DEV_HORI_IMG_VERT)
    {
        return true;
    }

    return false;
}


bool MIR_IsJpgPacket(short nCmd)
{
    if(nCmd == CMD_JPG_DEV_VERT_IMG_VERT ||
            nCmd == CMD_JPG_DEV_HORI_IMG_HORI ||
            nCmd == CMD_JPG_DEV_VERT_IMG_HORI ||
            nCmd == CMD_JPG_DEV_HORI_IMG_VERT)
    {
        return true;
    }

    return false;
}

bool MIR_IsKeyFrame(const BYTE* pPacket)
{
    BYTE isKeyFrame = *(BYTE*)&pPacket[24];
    if(isKeyFrame)
    {
        return true;
    }

    return false;
}

const ONYPACKET_UINT8* MakeOnyPacketControl(ONYPACKET_UINT8* sendBuf, int nHpNo, BYTE onoff, int &size)
{
    int dataSum = 0;
    int sizeofData = 0;

    memset(sendBuf, 0, SEND_BUFFER_SIZE);

    ONYPACKET_UINT8 mStartFlag = CMD_START_CODE;
    sizeofData = sizeof(mStartFlag);
    memcpy(sendBuf, (char*)(&mStartFlag), sizeofData);
    dataSum = sizeofData;

    ONYPACKET_INT mDataSize = htonl(1);
    sizeofData = sizeof(mDataSize);
    memcpy(sendBuf + dataSum, (char*)(&mDataSize), sizeofData);
    dataSum += sizeofData;

    ONYPACKET_UINT16 mCommandCode = htons(ONY_CMD_ONOFF);
    sizeofData = sizeof(mCommandCode);
    memcpy(sendBuf + dataSum, (char*)(&mCommandCode), sizeofData);
    dataSum += sizeofData;

    ONYPACKET_UINT8 mDeviceNo = nHpNo;
    sizeofData = sizeof(mDeviceNo);
    memcpy(sendBuf + dataSum, (char*)(&mDeviceNo), sizeofData);
    dataSum += sizeofData;

    ONYPACKET_UINT8 mOnOff = onoff;
    sizeofData = sizeof(mOnOff);
    memcpy(sendBuf + dataSum, (char*)(&mOnOff), sizeofData);
    dataSum += sizeofData;

    ONYPACKET_UINT16 mCheckSum = htons(calChecksum((ONYPACKET_UINT16*)(sendBuf + 1), ntohl(mDataSize) + CMD_HEAD_SIZE - 1));
    sizeofData = sizeof(mCheckSum);
    memcpy(sendBuf + dataSum, (char*)(&mCheckSum), sizeofData);
    dataSum += sizeofData;

    ONYPACKET_UINT8 mEndFlag = (ONYPACKET_UINT8)CMD_END_CODE;
    sizeofData = sizeof(mEndFlag);
    memcpy(sendBuf + dataSum, (char*)(&mEndFlag), sizeofData);
    dataSum += sizeofData;

    size = dataSum;

    return sendBuf;
}

const ONYPACKET_UINT8* MakeOnyPacketSendKeyFrame(ONYPACKET_UINT8* sendBuf, int nHpNo, int &size)
{
    int dataSum = 0;
    int sizeofData = 0;

    memset(sendBuf, 0, SEND_BUFFER_SIZE);

    ONYPACKET_UINT8 mStartFlag = CMD_START_CODE;
    sizeofData = sizeof(mStartFlag);
    memcpy(sendBuf, (char*)(&mStartFlag), sizeofData);
    dataSum = sizeofData;

    ONYPACKET_INT mDataSize = htonl(0);
    sizeofData = sizeof(mDataSize);
    memcpy(sendBuf + dataSum, (char*)(&mDataSize), sizeofData);
    dataSum += sizeofData;

    ONYPACKET_UINT16 mCommandCode = htons(ONY_CMD_SEND_KEY_FRAME);
    sizeofData = sizeof(mCommandCode);
    memcpy(sendBuf + dataSum, (char*)(&mCommandCode), sizeofData);
    dataSum += sizeofData;

    ONYPACKET_UINT8 mDeviceNo = nHpNo;
    sizeofData = sizeof(mDeviceNo);
    memcpy(sendBuf + dataSum, (char*)(&mDeviceNo), sizeofData);
    dataSum += sizeofData;

    ONYPACKET_UINT16 mCheckSum = htons(calChecksum((ONYPACKET_UINT16*)(sendBuf + 1), ntohl(mDataSize) + CMD_HEAD_SIZE - 1));
    sizeofData = sizeof(mCheckSum);
    memcpy(sendBuf + dataSum, (char*)(&mCheckSum), sizeofData);
    dataSum += sizeofData;

    ONYPACKET_UINT8 mEndFlag = (ONYPACKET_UINT8)CMD_END_CODE;
    sizeofData = sizeof(mEndFlag);
    memcpy(sendBuf + dataSum, (char*)(&mEndFlag), sizeofData);
    dataSum += sizeofData;

    size = dataSum;

    return sendBuf;
}

const ONYPACKET_UINT8* MakeOnyPacketRecordFrame(ONYPACKET_UINT8* sendBuf, int nHpNo, BYTE* pData, int dataSize, int &size)
{
    int dataSum = 0;
    int sizeofData = 0;

    memset(sendBuf, 0, 1024 * 1024 * 3);

    ONYPACKET_UINT8 mStartFlag = CMD_START_CODE;
    sizeofData = sizeof(mStartFlag);
    memcpy(sendBuf, (char*)(&mStartFlag), sizeofData);
    dataSum = sizeofData;

    ONYPACKET_INT mDataSize = htonl(dataSize);
    sizeofData = sizeof(mDataSize);
    memcpy(sendBuf + dataSum, (char*)(&mDataSize), sizeofData);
    dataSum += sizeofData;

    ONYPACKET_UINT16 mCommandCode = htons(20004);
    sizeofData = sizeof(mCommandCode);
    memcpy(sendBuf + dataSum, (char*)(&mCommandCode), sizeofData);
    dataSum += sizeofData;

    ONYPACKET_UINT8 mDeviceNo = nHpNo;
    sizeofData = sizeof(mDeviceNo);
    memcpy(sendBuf + dataSum, (char*)(&mDeviceNo), sizeofData);
    dataSum += sizeofData;

    sizeofData = dataSize;
    memcpy(sendBuf + dataSum, pData, sizeofData);
    dataSum += sizeofData;

    ONYPACKET_UINT16 mCheckSum = htons(calChecksum((ONYPACKET_UINT16*)(sendBuf + 1), ntohl(mDataSize) + CMD_HEAD_SIZE - 1));
    sizeofData = sizeof(mCheckSum);
    memcpy(sendBuf + dataSum, (char*)(&mCheckSum), sizeofData);
    dataSum += sizeofData;

    ONYPACKET_UINT8 mEndFlag = (ONYPACKET_UINT8)CMD_END_CODE;
    sizeofData = sizeof(mEndFlag);
    memcpy(sendBuf + dataSum, (char*)(&mEndFlag), sizeofData);
    dataSum += sizeofData;

    size = dataSum;

    return sendBuf;
}
