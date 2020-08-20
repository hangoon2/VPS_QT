#include "captureahybrid.h"
#include "VpsJpeg/VpsJpegLib.h"
//#include "Mirroring/MirroringCallback.h"

#include <QPainter>

static CaptureAHybrid* gs_pHY = nullptr;

static QImage* gs_recordImg[MAXCHCNT];

static VpsJpeg gs_jpgLib;

static QMutex gs_mRecLock[MAXCHCNT];

static uint gs_uCaptureInfoFPS[MAXCHCNT];

static bool gs_isRecReady[MAXCHCNT];
static bool gs_isRecFirstFrameInputed[MAXCHCNT];
static int gs_nRecFrameSkipCount[MAXCHCNT];

static int gs_nLongerKeyFrameLength[MAXCHCNT];
static int gs_nShorterKeyFrameLength[MAXCHCNT];

static int gs_nRecImgCenterX[MAXCHCNT];
static int gs_nRecImgCenterY[MAXCHCNT];
static int gs_nRecLongKeyFrameLength[MAXCHCNT];

static QSize gs_roiSize[MAXCHCNT];

//static QMutex gs_mRecLock[MAXCHCNT];

CaptureAHybrid::CaptureAHybrid(QObject *parent) : QObject(parent)
{
    connect(&m_netManager, SIGNAL(deviceStart(int, short, short)), parent, SLOT(OnDeviceStart(int, short, short)));
    connect(&m_netManager, SIGNAL(deviceStop(int, bool)), parent, SLOT(OnDeviceStop(int, bool)));
    connect(&m_netManager, SIGNAL(clientConnected(int, QString)), parent, SLOT(OnClientConnected(int, QString)));
    connect(&m_netManager, SIGNAL(clientDisconnected(int, QString)), parent, SLOT(OnClientDisconnected(int, QString)));
    connect(&m_netManager, SIGNAL(vpsCommandRotate(int, bool)), parent, SLOT(OnVpsCommand_Rotate(int, bool)));
    connect(&m_netManager, SIGNAL(vpsCommandRecord(int, bool)), parent, SLOT(OnVpsCommand_Record(int, bool)));

    connect(&m_netManager, SIGNAL(startMirroring(int, PMIRRORING_ROUTINE)), &m_mirror, SLOT(MIR_StartMirroring(int, PMIRRORING_ROUTINE)));
    connect(&m_netManager, SIGNAL(stopMirroring(int)), &m_mirror, SLOT(MIR_StopMirroring(int)));

    connect(&m_mirror, SIGNAL(mirroringRoutine(BYTE*)), &m_netManager, SLOT(OnMirrorCallback(BYTE*)), Qt::QueuedConnection);
    connect(&m_mirror, SIGNAL(mirroringStopRoutine(int, int)), &m_netManager, SLOT(OnMirrorStoppedCallback(int, int)));

    connect(&m_mirror, SIGNAL(mirroringRecordingRoutine(BYTE*)), this, SLOT(OnMirrorRecordCallback(BYTE*)), Qt::QueuedConnection);

    for(int i = 0; i < MAXCHCNT; i++)
    {
        gs_isRecReady[i] = false;

        gs_nLongerKeyFrameLength[i] = 0;
        gs_nShorterKeyFrameLength[i] = 0;

        gs_nRecImgCenterX[i] = 0;
        gs_nRecImgCenterY[i] = 0;
        gs_nRecLongKeyFrameLength[i] = 0;

        m_iMirrorVideoInputCount[i] = 0;

        gs_recordImg[i] = nullptr;

        m_isRecordingCH[i] = false;

        gs_uCaptureInfoFPS[i] = 16;

        m_dlLastCapTime[i] = 0;
        m_dlCaptureGap[i] = 1000/16;    //0;
    }

    gs_pHY = this;
}

void DoMirroringScreenCapture(int nHpNo, bool isKeyFrame)
{

}

void DoMirrorVideoRecording(int nHpNo, int nLeft, int nTop, int iW, int iH, bool isKeyFrame)
{
    if(iW > gs_nLongerKeyFrameLength[nHpNo-1] || iH > gs_nLongerKeyFrameLength[nHpNo-1])
    {
        qDebug() << "[Mirror:Recording] Wrong Source Image";
        return;
    }

    // Recording 할 크기는 가로, 세로 모두 큰쪽 길이에 맞추므로, 더 긴 쪽만 신경쓰면 된다.
    if(gs_isRecReady[nHpNo-1] && gs_nLongerKeyFrameLength[nHpNo-1] != gs_nRecLongKeyFrameLength[nHpNo-1])
    {
        gs_isRecReady[nHpNo-1] = false;

        gs_nRecLongKeyFrameLength[nHpNo-1] = gs_nLongerKeyFrameLength[nHpNo-1];
    }

    if(!gs_isRecReady[nHpNo-1] && isKeyFrame)
    {
        gs_isRecReady[nHpNo-1] = true;
    }

    if(!gs_isRecReady[nHpNo-1])
    {
        return;
    }

    // set receive time
    //GetSystemTime

    if(isKeyFrame)
    {
        if(iW == gs_nLongerKeyFrameLength[nHpNo-1] && iH != gs_nLongerKeyFrameLength[nHpNo-1])
        {
            // 가로가 더 긴 경우 처리
            gs_nRecImgCenterX[nHpNo-1] = 0;
            gs_nRecImgCenterY[nHpNo-1] = (gs_nLongerKeyFrameLength[nHpNo-1] - gs_nShorterKeyFrameLength[nHpNo-1]) / 2;
        }
        else if(iW != gs_nLongerKeyFrameLength[nHpNo-1] && iH == gs_nLongerKeyFrameLength[nHpNo-1])
        {
            // 세로가 더 긴 경우 처리
            gs_nRecImgCenterX[nHpNo-1] = (gs_nLongerKeyFrameLength[nHpNo-1] - gs_nShorterKeyFrameLength[nHpNo-1]) / 2;
            gs_nRecImgCenterY[nHpNo-1] = 0;
        }
        else
        {
            // 가로/세로 같은 경우 처리
            gs_nRecImgCenterX[nHpNo-1] = 0;
            gs_nRecImgCenterY[nHpNo-1] = 0;
        }

        gs_recordImg[nHpNo-1]->fill(0);

//        memset(gs_recordMat[nHpNo-1]->data, 0, gs_recordMat[nHpNo-1]->total() * gs_recordMat[nHpNo-1]->elemSize());
    }

    int nDstX = nLeft + gs_nRecImgCenterX[nHpNo-1];
    int nDstY = nTop + gs_nRecImgCenterY[nHpNo-1];

//    gs_mRecLock[nHpNo-1].lock();

//    gs_jpgLib.VJ_Copy(nHpNo, gs_recordImg[nHpNo-1], nDstX, nDstY);
    int time1 = gs_pHY->GetTime();
    VJ_Copy(nHpNo, gs_recordImg[nHpNo-1], nDstX, nDstY);
    int time2 = gs_pHY->GetTime();

    gs_roiSize[nHpNo-1].setWidth(gs_nRecLongKeyFrameLength[nHpNo-1]);
    gs_roiSize[nHpNo-1].setHeight(gs_nRecLongKeyFrameLength[nHpNo-1]);

    if(gs_nRecFrameSkipCount[nHpNo-1] == 0)
    {
        gs_isRecFirstFrameInputed[nHpNo-1] = true;
    }
    else
    {
        --gs_nRecFrameSkipCount[nHpNo-1];
        gs_pHY->SendKeyFrame(nHpNo);
    }

//    gs_mRecLock[nHpNo-1].unlock();

    if(gs_pHY->IsRecording(nHpNo) && gs_isRecFirstFrameInputed[nHpNo-1] && gs_isRecReady[nHpNo-1])
    {
        int iRecWidth = gs_roiSize[nHpNo-1].width();
        int iRecHeight = gs_roiSize[nHpNo-1].height();

        if(iRecWidth > 0 && iRecHeight > 0)
        {
            gs_pHY->OnMirrorRecording(nHpNo, iW, iH, nullptr);
        }
    }
}

void OnMirrorCallback(BYTE* pMirroringPacket)
{
//    BYTE* pJpgPacket = pMirroringPacket;

//    short usCmd = ntohs(*(short*)&pJpgPacket[5]);
//    int nHpNo = *((BYTE*)&pJpgPacket[7]);
//    short nLeft = ntohs(*(short*)&pJpgPacket[16]);
//    short nTop = ntohs(*(short*)&pJpgPacket[18]);
//    short nRight = ntohs(*(short*)&pJpgPacket[20]);
//    short nBottom = ntohs(*(short*)&pJpgPacket[22]);
//    bool isKeyFrame = *(BYTE*)&pJpgPacket[24];

//    if(isKeyFrame)
//    {
//        gs_nLongerKeyFrameLength[nHpNo-1] = nRight > nBottom ? nRight : nBottom;
//        gs_nShorterKeyFrameLength[nHpNo-1] = nRight < nBottom ? nRight : nBottom;
//    }

////    BYTE* pJpgData = pJpgPacket + 25;
////    int32_t nJpgDataSize = iDataLen - 17;

//    if(gs_pHY)
//    {
//        // FPS 장애처리를 위한 Count 기록
//        // 5초에 한번 KeyFrame이 들어오므로
//        // 장애처리 패킷을 보낼지 체크하는 주기인 10초 동안
//        // Count 값이 1 이상이어야 함
//        gs_pHY->MirrorIncreaseVideoInputCount(nHpNo);
//    }

//    // VPS 화면 업데이트를 위한 변수 업데이트
//    //theApp.m_BDCtrl.m_iCurVideoCaptureFPS[nHpNo-1]++;

////    if(gs_jpgLib.VJ_OpenJpg(nHpNo, pJpgData, nJpgDataSize))
////    if(VJ_OpenJpg(nHpNo, pJpgData, nJpgDataSize))
//    {
////        if(usCmd == CMD_JPG_DEV_VERT_IMG_HORI)
////        {
//////            gs_jpgLib.VJ_RotateLeft(nHpNo);
////            VJ_RotateLeft(nHpNo);

////            int nTempLeft = nLeft;
////            int nTempTop = nTop;
////            int nTempRight = nRight;
////            int nTempBottom = nBottom;

////            nLeft = nTempTop;
////            nTop = gs_nShorterKeyFrameLength[nHpNo-1] - nTempRight;
////            nRight = nTempBottom;
////            nBottom = gs_nShorterKeyFrameLength[nHpNo-1] - nTempLeft;
////        }
////        else if(usCmd == CMD_JPG_DEV_HORI_IMG_VERT)
////        {
//////            gs_jpgLib.VJ_RotateRight(nHpNo);
////            VJ_RotateRight(nHpNo);

////            int nTempLeft = nLeft;
////            int nTempTop = nTop;
////            int nTempRight = nRight;
////            int nTempBottom = nBottom;

////            nLeft = gs_nShorterKeyFrameLength[nHpNo-1] - nTempBottom;
////            nTop = nTempLeft;
////            nRight = gs_nShorterKeyFrameLength[nHpNo-1] - nTempTop;
////            nBottom = nTempRight;
////        }

//        gs_isRecReady[nHpNo-1] = true;

//        // Screen Capture
//        DoMirroringScreenCapture(nHpNo, isKeyFrame);

//        // Recording & Screen Display
//        DoMirrorVideoRecording(nHpNo, nLeft, nTop, nRight-nLeft, nBottom-nTop, isKeyFrame);
//    }
}

void CaptureAHybrid::HybridInit()
{
    m_netManager.MediaSend_Init();
    m_mirror.MIR_InitializeMirroring(::OnMirrorCallback);

    m_gtt.start();
    int msec = m_gtt.elapsed();

    for(int i = 0; i < MAXCHCNT; i++)
    {
        gs_recordImg[i] = new QImage(960, 960, QImage::Format_RGB32);
        gs_recordImg[i]->fill(0);

//        gs_recordMat[i] = new Mat(960, 960, CV_8UC3);
//        memset(gs_recordMat[i]->data, 0, (gs_recordMat[i]->total() * gs_recordMat[i]->elemSize()));

        m_dlLastCapTime[i] = msec;
    }
}

void CaptureAHybrid::HybridExit()
{
    m_netManager.MediaSend_Close();
    m_mirror.MIR_DestroyMirroring();

    for(int i = 0; i < MAXCHCNT; i++)
    {
        delete gs_recordImg[i];
        gs_recordImg[i] = nullptr;

//        delete gs_recordMat[i];
//        gs_recordMat[i] = nullptr;
    }
}

void CaptureAHybrid::DeviceRotate(int nHpNo, bool vertical)
{
    qDebug() << "CaptureAHybrid::DeviceRotate : " << vertical;
    m_mirror.MIR_SetDeviceOrientation(nHpNo, vertical ? 1 : 0);
    m_mirror.MIR_SendKeyFrame(nHpNo);
}

void CaptureAHybrid::MirroRecordingCommand(int nHpNo, bool start)
{
    qDebug() << "CaptureAHybrid::MirroRecordingCommand : " << start;
    if(start)
    {
        gs_isRecReady[nHpNo-1] = false;

        // 단말기로 부터 첫 이미지를 받았을 때 부터 레코딩하기 위해, 우선 false로 초기화
        gs_isRecFirstFrameInputed[nHpNo-1] = false;

        // 레코딩시 몇 프레임은 이전 영상이 레코딩 될 수 있으므로 무시함
        static const int REC_FRAME_SKIP_COUNT = gs_uCaptureInfoFPS[nHpNo-1] / 5;
        gs_nRecFrameSkipCount[nHpNo-1] = REC_FRAME_SKIP_COUNT;

//        m_recorder[nHpNo-1].StartRecord(nHpNo);

        SendKeyFrame(nHpNo);
    }
    else
    {
//        m_recorder[nHpNo-1].StopRecord();
    }
}

void CaptureAHybrid::OnMirrorRecording(int nHpNo, int iW, int iH, BYTE *pSrc)
{
    if(IsCompressTime(nHpNo, m_dlCaptureGap[nHpNo-1]))
    {
//        m_recorder[nHpNo-1].EnQueue(m_gtt.elapsed(), gs_recordImg[nHpNo-1]);
    }
}

int CaptureAHybrid::MirrorGetVideoInputCount(int nHpNo)
{
    return m_iMirrorVideoInputCount[nHpNo-1];
}
void CaptureAHybrid::MirrorIncreaseVideoInputCount(int nHpNo)
{
    m_iMirrorVideoInputCount[nHpNo-1]++;
}

void CaptureAHybrid::MirrorClearVideoInputCount(int nHpNo)
{
    m_iMirrorVideoInputCount[nHpNo-1] = 0;
}

bool CaptureAHybrid::IsRecording(int nHpNo)
{
    return m_isRecordingCH[nHpNo-1];
}

void CaptureAHybrid::SetRecording(int nHpNo, bool start)
{
    m_isRecordingCH[nHpNo-1] = start;
}

void CaptureAHybrid::SendKeyFrame(int nHpNo)
{
    m_mirror.MIR_SendKeyFrame(nHpNo);
}

bool CaptureAHybrid::IsCompressTime(int nHpNo, double dlCaptureGap)
{
    double msec = (double)m_gtt.elapsed();

    if(msec - m_dlLastCapTime[nHpNo-1] >= dlCaptureGap)
    {
        m_dlLastCapTime[nHpNo-1] = msec;
        return true;
    }

    return false;
}

int CaptureAHybrid::GetTime()
{
    return m_gtt.elapsed();
}

/* slots */
void CaptureAHybrid::OnMirrorRecordCallback(BYTE* pMirroringPacket)
{
    BYTE* pJpgPacket = pMirroringPacket;

    uint iDataLen = ntohl(*(uint32_t*)&pJpgPacket[1]);
    short usCmd = ntohs(*(short*)&pJpgPacket[5]);
    int nHpNo = *((BYTE*)&pJpgPacket[7]);
    short nLeft = ntohs(*(short*)&pJpgPacket[16]);
    short nTop = ntohs(*(short*)&pJpgPacket[18]);
    short nRight = ntohs(*(short*)&pJpgPacket[20]);
    short nBottom = ntohs(*(short*)&pJpgPacket[22]);
    bool isKeyFrame = *(BYTE*)&pJpgPacket[24];

    if(isKeyFrame)
    {
        gs_nLongerKeyFrameLength[nHpNo-1] = nRight > nBottom ? nRight : nBottom;
        gs_nShorterKeyFrameLength[nHpNo-1] = nRight < nBottom ? nRight : nBottom;
    }

//    BYTE* pJpgData = pJpgPacket + 25;
//    int32_t nJpgDataSize = iDataLen - 17;

    if(gs_pHY)
    {
        // FPS 장애처리를 위한 Count 기록
        // 5초에 한번 KeyFrame이 들어오므로
        // 장애처리 패킷을 보낼지 체크하는 주기인 10초 동안
        // Count 값이 1 이상이어야 함
        gs_pHY->MirrorIncreaseVideoInputCount(nHpNo);
    }

    // VPS 화면 업데이트를 위한 변수 업데이트
    //theApp.m_BDCtrl.m_iCurVideoCaptureFPS[nHpNo-1]++;

//    if(gs_jpgLib.VJ_OpenJpg(nHpNo, pJpgData, nJpgDataSize))
//    if(VJ_OpenJpg(nHpNo, pJpgData, nJpgDataSize))
    {
        if(usCmd == CMD_JPG_DEV_VERT_IMG_HORI)
        {
//            gs_jpgLib.VJ_RotateLeft(nHpNo);
            VJ_RotateLeft(nHpNo);

            int nTempLeft = nLeft;
            int nTempTop = nTop;
            int nTempRight = nRight;
            int nTempBottom = nBottom;

            nLeft = nTempTop;
            nTop = gs_nShorterKeyFrameLength[nHpNo-1] - nTempRight;
            nRight = nTempBottom;
            nBottom = gs_nShorterKeyFrameLength[nHpNo-1] - nTempLeft;
        }
        else if(usCmd == CMD_JPG_DEV_HORI_IMG_VERT)
        {
//            gs_jpgLib.VJ_RotateRight(nHpNo);
            VJ_RotateRight(nHpNo);

            int nTempLeft = nLeft;
            int nTempTop = nTop;
            int nTempRight = nRight;
            int nTempBottom = nBottom;

            nLeft = gs_nShorterKeyFrameLength[nHpNo-1] - nTempBottom;
            nTop = nTempLeft;
            nRight = gs_nShorterKeyFrameLength[nHpNo-1] - nTempTop;
            nBottom = nTempRight;
        }

        gs_isRecReady[nHpNo-1] = true;

        // Screen Capture
        DoMirroringScreenCapture(nHpNo, isKeyFrame);

        // Recording & Screen Display
        //DoMirrorVideoRecording(nHpNo, nLeft, nTop, nRight-nLeft, nBottom-nTop, isKeyFrame);
    }
}
