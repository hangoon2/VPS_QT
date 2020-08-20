#include "clientobject.h"

#include <QDebug>

ClientObject::ClientObject()
{
    m_rcvCommandBuffer = new BYTE[RECV_BUFFER_SIZE];

    Init(nullptr);
}

ClientObject::~ClientObject()
{
    if(m_socket != nullptr)
    {
        delete m_socket;
        m_socket = nullptr;
    }

    delete[] m_rcvCommandBuffer;
    m_rcvCommandBuffer = nullptr;

    qDebug() << "Client Object delete";
}

void ClientObject::Init(QTcpSocket *socket)
{
    m_bFirstImage = true;

    m_iKeyFrameLength = 0;

    m_iCurrentSentFPS = 0;
    m_iSentBytes = 0;
    m_iSentBytesOld = 0;

    m_socket = socket;
    memset(m_rcvCommandBuffer, 0, RECV_BUFFER_SIZE);
    m_dwRcvPos = 0;

    m_strID = "";
    m_strIPAddr = "";

    m_iChannel = -1;
    m_iImageFullSizeDiv = 4;    // 이미지 사이즈 모드
    m_iCompressQuality = 30;    // 압축율
    m_iFrameRate = 15;          // frame rate
    m_iVideoJPGorH264 = 0;      // 영상 포맷 종류
    m_iAudioOnOff = 0;          // 오디오 전송 여부
//    m_connectedTime =

    m_isAudioChannelExist = false;

    m_clientType = CLIENT_UNKNOWN;

    m_isExitCommandReceived = false;
}

void ClientObject::Close()
{
    Init(nullptr);
}

QString ClientObject::GetClientTypeString()
{
    QString type;

    switch(m_clientType)
    {
    case CLIENT_MC:
        type = "Mobile Controller";
        break;

    case CLIENT_HOST:
        type = "Host";
        break;

    case CLIENT_GUEST:
        type = "Guest";
        break;

    case CLIENT_MONITOR:
        type = "Monitor";
        break;

    case CLIENT_AUDIO:
        type = "Audio";
        break;

    default:
        type = "Unknown";
        break;
    }

    return type;
}

ClientObject* ClientObject::Clone()
{
    ClientObject* pDst = new ClientObject();

    pDst->m_bFirstImage = m_bFirstImage;

    pDst->m_iKeyFrameLength = m_iKeyFrameLength;

    pDst->m_iCurrentSentFPS = m_iCurrentSentFPS;
    pDst->m_iSentBytes = m_iSentBytes;
    pDst->m_iSentBytesOld = m_iSentBytesOld;

    pDst->m_socket = m_socket;
    memcpy(pDst->m_rcvCommandBuffer, m_rcvCommandBuffer, RECV_BUFFER_SIZE);
    pDst->m_dwRcvPos = m_dwRcvPos;

    pDst->m_strID = m_strID;
    pDst->m_strIPAddr = m_strIPAddr;

    pDst->m_iChannel = m_iChannel;
    pDst->m_iImageFullSizeDiv = m_iImageFullSizeDiv;
    pDst->m_iCompressQuality = m_iCompressQuality;
    pDst->m_iFrameRate = m_iFrameRate;
    pDst->m_iVideoJPGorH264 = m_iVideoJPGorH264;
    pDst->m_iAudioOnOff = m_iAudioOnOff;
//    pDst->m_connectedTime = m_connectedTime;

    pDst->m_isAudioChannelExist = m_isAudioChannelExist;

    pDst->m_clientType = m_clientType;

    pDst->m_isExitCommandReceived = m_isExitCommandReceived;
//    pDst->m_abnormalDisconnectedTime = m_abnormalDisconnectedTime;

    return pDst;
}

void ClientObject::ApplyState(ClientObject *pSrc)
{
    m_bFirstImage = pSrc->m_bFirstImage;

    m_iKeyFrameLength = pSrc->m_iKeyFrameLength;

    m_iCurrentSentFPS = pSrc->m_iCurrentSentFPS;
    m_iSentBytes = pSrc->m_iSentBytes;
    m_iSentBytesOld = pSrc->m_iSentBytesOld;

    memcpy(m_rcvCommandBuffer, pSrc->m_rcvCommandBuffer, RECV_BUFFER_SIZE);
    m_dwRcvPos = pSrc->m_dwRcvPos;

    m_strID = pSrc->m_strID;
    m_strIPAddr = pSrc->m_strIPAddr;

    m_iChannel = pSrc->m_iChannel;
    m_iImageFullSizeDiv = pSrc->m_iImageFullSizeDiv;
    m_iCompressQuality = pSrc->m_iCompressQuality;
    m_iFrameRate = pSrc->m_iFrameRate;
    m_iVideoJPGorH264 = pSrc->m_iVideoJPGorH264;
    m_iAudioOnOff = pSrc->m_iAudioOnOff;
//    m_connectedTime = pSrc->m_connectedTime;

    m_isAudioChannelExist = pSrc->m_isAudioChannelExist;

    m_clientType = pSrc->m_clientType;

//    m_abnormalDisconnectedTime = pSrc->m_abnormalDisconnectedTime;
}

bool ClientObject::IsSameClient(int nHpNo, ClientType clientType, QString strID, QString strIPAddr)
{
    if(m_iChannel == nHpNo
            && m_clientType == clientType
            && m_strID.compare(strID) == 0
            && m_strID.compare(strIPAddr) == 0) {
        return true;
    }

    return false;
}
