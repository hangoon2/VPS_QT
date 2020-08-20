#include "mirroring.h"
#include "VpsJpeg/VpsJpegLib.h"

#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QBuffer>

static Mirroring* gs_mirror = nullptr;

static int gs_nDeviceOrientation[MAXCHCNT] = {1, };  // 1: 세로, 0: 가로
static int gs_nKeyFrameW[MAXCHCNT] = {0, };
static int gs_nKeyFrameH[MAXCHCNT] = {0, };

static int gs_nJpgQuality[MAXCHCNT] = {VPS_DEFAULT_JPG_QUALITY, };

Mirroring::Mirroring(QObject *parent) : QObject(parent)
{
    m_recordRoutine = nullptr;

    m_isMirroringInited = false;

    gs_mirror = this;

    m_gtt.start();
}

void MIR_DoMirror(BYTE* pMirroringPacket)
{
    BYTE* pPacket = pMirroringPacket;

    uint iDataLen = ntohl(*(uint32_t*)&pPacket[1]);
    short usCmd = ntohs(*(short*)&pPacket[5]);
    int nHpNo = *((BYTE*)&pPacket[7]);
    if(nHpNo < 1 || nHpNo > MAXCHCNT) return;

    if(MIR_IsJpgPacket(pPacket))
    {
        gs_mirror->MIR_HandleJpegPacket(pPacket, iDataLen, usCmd, nHpNo);
    }
    else
    {
        gs_mirror->MIR_HandleJpegCaptureFailedPacket(pPacket, nHpNo);
    }
}

void MIR_DoEnqueue(BYTE* pMirroringPacket)
{
    BYTE* pPacket = pMirroringPacket;

    int nHpNo = *((BYTE*)&pPacket[7]);
    gs_mirror->MIR_EnQueue(nHpNo, pPacket);
}

bool Mirroring::MIR_InitializeMirroring(PMIRRORING_ROUTINE recordRoutine)
{
    for(int i = 0; i < MAXCHCNT; i++)
    {
        connect(&m_mirClient[i], SIGNAL(mirrorEnqueue(int, BYTE*)), this, SLOT(MIR_EnQueue(int, BYTE*)));
        connect(&m_mirClient[i], SIGNAL(mirrorStopped(int, int)), this, SLOT(MIR_OnMirrorStopped(int, int)));
        connect(&m_mirClient[i], SIGNAL(clientConnected(int)), this, SLOT(OnClientConnected(int)));

        connect(&m_mirQueHandler[i], SIGNAL(processPacket(BYTE*)), this, SLOT(MIR_DoMirror(BYTE*)), Qt::QueuedConnection);

        gs_nDeviceOrientation[i] = 1;
        gs_nKeyFrameW[i] = 0;
        gs_nKeyFrameH[i] = 0;
        gs_nJpgQuality[i] = VPS_DEFAULT_JPG_QUALITY;


//        m_mirQueHandler[i].StartThread(::MIR_DoMirror);
    }

    m_recordRoutine = recordRoutine;

    m_isMirroringInited = true;

    return true;
}

void Mirroring::MIR_DestroyMirroring()
{
//    for(int i = 0; i < MAXCHCNT; i++) {
//        m_mirQueHandler[i].StopThread();
//    }

    m_recordRoutine = nullptr;
}

void Mirroring::MIR_SetDeviceOrientation(int nHpNo, int iMode)
{
    gs_nDeviceOrientation[nHpNo-1] = iMode;
    qDebug() << "Mirroring::MIR_SetDeviceOrientation : " << iMode;
}

void Mirroring::MIR_SendKeyFrame(int nHpNo)
{
    m_mirClient[nHpNo-1].SendKeyFramePacket(nHpNo);
}

void Mirroring::MIR_HandleJpegPacket(BYTE *pPacket, uint iDataLen, short usCmd, int nHpNo)
{
    short nLeft = ntohs(*(short*)&pPacket[16]);
    short nTop = ntohs(*(short*)&pPacket[18]);
    short nRight = ntohs(*(short*)&pPacket[20]);
    short nBottom = ntohs(*(short*)&pPacket[22]);
    bool isKeyFrame = MIR_IsKeyFrame(pPacket);

    if(isKeyFrame)
    {
        gs_nKeyFrameW[nHpNo-1] = nRight;
        gs_nKeyFrameH[nHpNo-1] = nBottom;
    }

    if(gs_nKeyFrameW[nHpNo-1] < 1 && gs_nKeyFrameH[nHpNo-1] < 1) return;

    int nKeyFrameWidth = gs_nKeyFrameW[nHpNo-1];
    int nKeyFrameHeight = gs_nKeyFrameH[nHpNo-1];
    int nLongerKeyFrameLength = nKeyFrameWidth > nKeyFrameHeight ? nKeyFrameWidth : nKeyFrameHeight;
    int nShorterKeyFrameLength = nKeyFrameWidth < nKeyFrameHeight ? nKeyFrameWidth : nKeyFrameHeight;

    if(usCmd == 20001 || usCmd == 20002)
    {

    }
    else
    {
        bool isWideDevice = nKeyFrameWidth > nKeyFrameHeight;
        if(isWideDevice)
        {

        }

        int nDeviceOrientation = gs_nDeviceOrientation[nHpNo-1];
        if(usCmd == CMD_JPG_DEV_VERT_IMG_VERT && nDeviceOrientation == 0)
        {
            usCmd = CMD_JPG_DEV_HORI_IMG_VERT;
            *(short*)&pPacket[5] = htons(usCmd);
        }
        else if(usCmd == CMD_JPG_DEV_HORI_IMG_HORI && nDeviceOrientation == 1)
        {
            usCmd = CMD_JPG_DEV_VERT_IMG_HORI;
            *(short*)&pPacket[5] = htons(usCmd);
        }

        // JPEG 이미지 회전을 위한 코드
        BYTE* pJpgData = pPacket + 25;
        int nJpgDataSize = iDataLen - 17;
        int o_nJpgDstLen = 0;

        if(VJ_OpenJpg(nHpNo, pJpgData, nJpgDataSize))
        {
            if((usCmd == CMD_JPG_DEV_HORI_IMG_HORI
                || usCmd == CMD_JPG_DEV_HORI_IMG_VERT)
                    && !isWideDevice) {
                if(VJ_RotateLeft(nHpNo))
                {
                    *(short*)&pPacket[16] = htons(nTop);
                    *(short*)&pPacket[18] = htons(nShorterKeyFrameLength - nRight);
                    *(short*)&pPacket[20] = htons(nBottom);
                    *(short*)&pPacket[22] = htons(nShorterKeyFrameLength - nLeft);

                    if(VJ_Encode(nHpNo, pJpgData, &o_nJpgDstLen, gs_nJpgQuality[nHpNo-1]))
                    {
                        iDataLen = iDataLen - nJpgDataSize + o_nJpgDstLen;
                        *(uint32_t*)&pPacket[1] = htonl(iDataLen);

                        // checksum 다시 계산
                        *(short*)&pPacket[CMD_HEAD_SIZE + iDataLen] = htons(calChecksum((ONYPACKET_UINT16*)(pPacket+1), iDataLen+CMD_HEAD_SIZE-1));
                        *(BYTE*)&pPacket[CMD_HEAD_SIZE + iDataLen + 2] = (BYTE)CMD_END_CODE;
                    }
                }
            }
            else if(usCmd == CMD_JPG_DEV_VERT_IMG_VERT && isWideDevice)
            {
                if(VJ_RotateLeft(nHpNo))
                {
                    *(short*)&pPacket[16] = htons(nTop);
                    *(short*)&pPacket[18] = htons(nLongerKeyFrameLength - nRight);
                    *(short*)&pPacket[20] = htons(nBottom);
                    *(short*)&pPacket[22] = htons(nLongerKeyFrameLength - nLeft);

                    iDataLen = iDataLen - nJpgDataSize + o_nJpgDstLen;
                    *(uint32_t*)&pPacket[1] = htonl(iDataLen);

                    // checksum 다시 계산
                    *(short*)&pPacket[CMD_HEAD_SIZE + iDataLen] = htons(calChecksum((ONYPACKET_UINT16*)(pPacket+1), iDataLen+CMD_HEAD_SIZE-1));
                    *(BYTE*)&pPacket[CMD_HEAD_SIZE + iDataLen + 2] = (BYTE)CMD_END_CODE;
                }
            }
            else if(usCmd == CMD_JPG_DEV_VERT_IMG_HORI && isWideDevice)
            {
                if(VJ_RotateRight(nHpNo))
                {
                    *(short*)&pPacket[16] = htons(nShorterKeyFrameLength - nBottom);
                    *(short*)&pPacket[18] = htons(nLeft);
                    *(short*)&pPacket[20] = htons(nShorterKeyFrameLength - nTop);
                    *(short*)&pPacket[22] = htons(nRight);

                    iDataLen = iDataLen - nJpgDataSize + o_nJpgDstLen;
                    *(uint32_t*)&pPacket[1] = htonl(iDataLen);

                    // checksum 다시 계산
                    *(short*)&pPacket[CMD_HEAD_SIZE + iDataLen] = htons(calChecksum((ONYPACKET_UINT16*)(pPacket+1), iDataLen+CMD_HEAD_SIZE-1));
                    *(BYTE*)&pPacket[CMD_HEAD_SIZE + iDataLen + 2] = (BYTE)CMD_END_CODE;
                }
            }
        }
    }

    if(m_mirroringRoutine[nHpNo-1] != nullptr)
    {
        m_mirroringRoutine[nHpNo-1](pPacket);
    }

    if(m_recordRoutine != nullptr)
    {
        m_recordRoutine(pPacket);
    }
}

void Mirroring::MIR_HandleJpegCaptureFailedPacket(BYTE *pPacket, int nHpNo)
{
    if(m_mirroringRoutine[nHpNo-1] != nullptr)
    {
        m_mirroringRoutine[nHpNo-1](pPacket);
    }
}

/* slots */
void Mirroring::MIR_StartMirroring(int nHpNo, PMIRRORING_ROUTINE pMirroringRoutine)
{
    if(!m_isMirroringInited) return;

    int nControlPort = 8820 + nHpNo;
    int nMirrorPort = 8800 + nHpNo;

    m_mMirroringStopRoutine[nHpNo-1].lock();

    m_mirroringRoutine[nHpNo-1] = pMirroringRoutine;

//    bool isClientThreadStarted =
    m_mirClient[nHpNo-1].StartMirroring(nHpNo, nControlPort, nMirrorPort, ::MIR_DoEnqueue);
//    if(isClientThreadStarted)
//    {
//        gs_nDeviceOrientation[nHpNo-1] = 1;
//    }

    m_mMirroringStopRoutine[nHpNo-1].unlock();
}

void Mirroring::MIR_StopMirroring(int nHpNo)
{
    m_mirClient[nHpNo-1].StopMirroring();
}

void Mirroring::MIR_EnQueue(int nHpNo, BYTE* pPacket)
{
    //m_mirQueHandler[nHpNo - 1].EnQueue(nHpNo, pPacket);
    MIR_DoMirror(pPacket);

//    BYTE* pPacket = pPacket;

//    uint iDataLen = ntohl(*(uint32_t*)&pPacket[1]);
//    short usCmd = ntohs(*(short*)&pPacket[5]);
//    //int nHpNo = *((BYTE*)&pPacket[7]);
//    if(nHpNo < 1 || nHpNo > MAXCHCNT) return;

//    if(MIR_IsJpgPacket(pPacket))
//    {
//        MIR_HandleJpegPacket(pPacket, iDataLen, usCmd, nHpNo);
//    }
//    else
//    {
//        MIR_HandleJpegCaptureFailedPacket(pPacket, nHpNo);
//    }
}

void Mirroring::MIR_OnMirrorStopped(int nHpNo, int nStopCode)
{
    qDebug() << "Mirroring::MIR_OnMirrorStopped";

    m_mMirroringStopRoutine[nHpNo-1].lock();

    m_mirroringRoutine[nHpNo-1] = nullptr;

    emit mirroringStopRoutine(nHpNo, nStopCode);

    m_mMirroringStopRoutine[nHpNo-1].unlock();
}

void Mirroring::MIR_DoMirror(BYTE* pMirroringPacket)
{
    BYTE* pPacket = pMirroringPacket;

    uint iDataLen = ntohl(*(uint32_t*)&pPacket[1]);
    short usCmd = ntohs(*(short*)&pPacket[5]);
    int nHpNo = *((BYTE*)&pPacket[7]);
    if(nHpNo < 1 || nHpNo > MAXCHCNT) return;

    if(MIR_IsJpgPacket(pPacket))
    {
        MIR_HandleJpegPacket(pPacket, iDataLen, usCmd, nHpNo);
    }
    else
    {
        MIR_HandleJpegCaptureFailedPacket(pPacket, nHpNo);
    }
}

void Mirroring::OnClientConnected(int nHpNo)
{
    qDebug() << "Mirroring::OnClientConnected ===== " << nHpNo;
    gs_nDeviceOrientation[nHpNo-1] = 1;
}
