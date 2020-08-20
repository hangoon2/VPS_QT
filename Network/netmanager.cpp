#include "netmanager.h"

#include <QDebug>

static NetManager* gs_netMgr = nullptr;

#define CMD_OFFSET 200

int g_iCMD_OFFSET = CMD_OFFSET;

static BYTE gs_pBufSendDataToClient[MAXCHCNT][512]  = {0,};

ushort SwapEndianU2(ushort value)
{
    return (ushort)((value >> 8) | (value << 8));
}

uint SwapEndianU4(uint value)
{
    BYTE tmp;
    tmp = *((BYTE*)&value + 3);
    *((BYTE*)&value + 3) = *((char*)&value + 0);
    *((BYTE*)&value + 0) = tmp;

    tmp = *((BYTE*)&value + 2);
    *((BYTE*)&value + 2) = *((BYTE*)&value + 1);
    *((BYTE*)&value + 1) = tmp;

    return value;
}

NetManager::NetManager(QObject *parent) : QObject(parent)
{
    m_bDCConnected = false;

    memset(m_iRefreshCH, 1, sizeof(m_iRefreshCH));

    memset(m_iCompressedCount, 0, sizeof(m_iCompressedCount));
    memset(m_iCompressedCountOld, 0, sizeof(m_iCompressedCountOld));

    memset(m_nCaptureCommandCountReceived, 0, sizeof(m_nCaptureCommandCountReceived));
    memset(m_nCaptureCompletionCountSent, 0, sizeof(m_nCaptureCompletionCountSent));
    memset(m_nRecordStartCommandCountReceived, 0, sizeof(m_nRecordStartCommandCountReceived));
    memset(m_nRecordStopCommandCountReceived, 0, sizeof(m_nRecordStopCommandCountReceived));
    memset(m_nRecordCompletionCountSent, 0, sizeof(m_nRecordCompletionCountSent));

    memset(m_isOnService, 0, sizeof(m_isOnService));
    memset(m_isReceivedRecordStartCommand, 0, sizeof(m_isReceivedRecordStartCommand));

    gs_netMgr = this;
}

void DoMirrorCallback(BYTE* pMirroringPacket)
{
    if(gs_netMgr == NULL)
    {
        return;
    }

    gs_netMgr->MIR_BypassPacket(pMirroringPacket);
}

void NetManager::OnServerModeStart()
{
    m_VPSServer.Init_Socket(10001);
}

void NetManager::MediaSend_Init()
{
    OnServerModeStart();

    connect(&m_VPSServer, SIGNAL(readEx(ClientObject*, BYTE*, int)), this, SLOT(OnReadEx(ClientObject*, BYTE*, int)));
    connect(&m_VPSServer, SIGNAL(closeClient(ClientObject*)), this,SLOT(OnClosingClient(ClientObject*)));

    QTimer::singleShot(1000, this, SLOT(OnTimer()));
    //connect(&m_timer, SIGNAL(timeout()), this, SLOT(OnTimer()));

    m_timer.start(1000);
}

void NetManager::MediaSend_Close()
{
    m_VPSServer.Dest_Socket();

    m_timer.stop();
}

bool NetManager::DuplicatedClientSend(int nHpNo, ClientObject *object)
{
//    DWORD dwTotLen = 0;
//    DWORD dwCmd = CMD_DUPLICATED_CLIENT;
//    BYTE* pSendData = MakeSendData2(dwCmd, nHpNo, 0, gs_pBufSendDataToClient[nHpNo-1], dwTotLen);

//    return Send(dwCmd, nHpNo, pSendData,dwTotLen, object);
    return true;
}

bool NetManager::DisconnectGuestSend(int nHpNo, ClientObject* object)
{
    DWORD dwTotLen = 0;
    DWORD dwCmd = CMD_DISCONNECT_GUEST;
    LPSTR szMbcs = (LPSTR)gs_pBufSendDataToClient[nHpNo-1] + g_iCMD_OFFSET;

    return false;
}

bool NetManager::SendToClient(short cmd, int nHpNo, BYTE* data, int len, int iKeyFrameNo)
{
    return m_VPSServer.SendToClient(cmd, nHpNo, data, len, iKeyFrameNo);
}

void NetManager::MIR_BypassPacket(const void *pPacket)
{
    BYTE* pSendPacket = (BYTE*)pPacket;

    uint iDataLen = SwapEndianU4(*(uint32_t*)&pSendPacket[1]);
    short usCmd = SwapEndianU2(*(short*)&pSendPacket[5]);
    int nHpNo = *((BYTE*)&pSendPacket[7]);
    bool isKeyFrame = *(BYTE*)&pSendPacket[24];
    int dwTotLen = CMD_HEAD_SIZE + iDataLen + CMD_TAIL_SIZE;

    if(usCmd == CMD_MIRRORING_JPEG_CAPTURE_FAILED)
    {
        SendToClient(usCmd, nHpNo, pSendPacket, dwTotLen, 0);
    }
    else
    {
        if(SendToClient(usCmd, nHpNo, pSendPacket, dwTotLen, 0))
        {
            m_iCompressedCount[nHpNo-1]++;
        }
        else
        {
            //  전송 실패시 키프레임을 만들어준다.

        }
    }
}

bool NetManager::Send(short cmd, int nHpNo, BYTE* data, int len, ClientObject *object, bool force)
{
    return false;
}

bool NetManager::CloseClientManual(ClientObject *object)
{
    m_VPSServer.OnCloseSocket(object->m_socket);
    return true;
}

void NetManager::WebCommandDataParsing(ClientObject *object, char *pRcvData, int dataSize)
{
    uint dataLen = SwapEndianU4(*(uint*)&pRcvData[1]);
    short cmd = SwapEndianU2(*(short*)&pRcvData[5]);
    int nHpNo = (int)pRcvData[7];

    qDebug() << "Command : " << cmd;
    qDebug() << "Device No : " << nHpNo;

    QString strID = "";

    if(cmd == CMD_ID || cmd == CMD_ID_GUEST
            || cmd == CMD_ID_MONITOR || cmd == CMD_ID_AUDIO)
    {
        char cID[128] = {0,};
        memset(cID, 0, sizeof(cID));
        memcpy(cID, &pRcvData[8], dataLen);

        strID = cID;

        qDebug() << "ID : " << strID;

        if(!(cmd == CMD_ID && strID.compare("MOBILECONTROL") == 0)
                && cmd != CMD_ID_AUDIO && cmd != CMD_ID_MONITOR)
        {
            ClientType clientType = CLIENT_UNKNOWN;
            if(cmd == CMD_ID)
                clientType = CLIENT_HOST;
            else if(cmd == CMD_ID_GUEST)
                clientType = CLIENT_GUEST;

            // 연결/다시연결 되었으므로 기존에 CMD_PLAYER_EXIT(30010) 패킷을 받았더라도 초기화한다.
            object->m_isExitCommandReceived = false;

            ClientObject& r_pNewClientObj = *object;
            if(!m_VPSServer.CheckAndRestoreOldState(r_pNewClientObj, nHpNo, clientType, strID, object->m_strIPAddr))
            {
                ClientObject* pExistingClientObj = m_VPSServer.FindTheSameClient(nHpNo, clientType, strID, object->m_strIPAddr);
                if(pExistingClientObj != nullptr)
                {
                    // Apply State
                    r_pNewClientObj.ApplyState(pExistingClientObj);

                    // Remove the existing client
                    pExistingClientObj->m_isExitCommandReceived = true;
                    DuplicatedClientSend(nHpNo, pExistingClientObj);

                    m_VPSServer.OnCloseSocket(pExistingClientObj->m_socket);
                }
            }
        }
    }
    else
    {
        strID = object->m_strID;
    }

    if(dataLen > RECV_BUFFER_SIZE)
    {
        // 데이터 크기가 1메가를 넘음
        return;
    }

    if(dataSize < 11)
    {
        // 데이터 길이 작음
        return;
    }

    char* pData = &pRcvData[8];

    switch(cmd)
    {
    case CMD_ACK:
    case CMD_LOGCAT:
    case CMD_RESOURCE_USAGE_NETWROK:
    case CMD_RESOURCE_USAGE_CPU:
    case CMD_RESOURCE_USAGE_MEMORY:
    case CMD_TEST_RESULT:
    case CMD_AUTOTOOL_START_EVENT_INDEX:
    case CMD_AUTOTOOL_START_EVENT_PATH:
    case CMD_AUTOTOOL_START_SCRIPT_RESULT:
        SendToClient(cmd, nHpNo, pRcvData, dataSize, 0);
        break;

    case CMD_ID:
    {
        object->m_strID = strID;
        object->m_iChannel = nHpNo;

        if(strID.compare("MOBILECONTROL") == 0)
        {
            object->m_clientType = CLIENT_MC;
            m_VPSServer.m_pMobileControlClient = object;

            // client connected
            emit clientConnected(nHpNo, object->GetClientTypeString());
        }
        else
        {
            object->m_clientType = CLIENT_HOST;
            m_iRefreshCH[nHpNo-1] = 1;  // 전체 영상 전송

            // client connected
            emit clientConnected(nHpNo, object->GetClientTypeString());
        }
    }
        break;

    case CMD_ID_GUEST:
    {
        char cID[128] = {0,};
        memcpy(cID, pData, dataLen);
        object->m_strID = cID;
        object->m_iChannel = nHpNo;
        object->m_clientType = CLIENT_GUEST;
        m_iRefreshCH[nHpNo-1] = 1;  // 전체 영상 전송

        // Guest가 연결될 때 해당 채널의 Host가 연결되어 있지 않으면, 연결을 허락하지 않음.
        if(!m_VPSServer.IsHostNodeConnected(nHpNo)
                && m_VPSServer.FindAbnormallyDisconnectedClient(nHpNo, CLIENT_HOST)) {
            DisconnectGuestSend(nHpNo, object);
            return;
        }

        // start mirroring
        //MIR_SendKeyFrame(nHpNo);

        // client connected
        emit clientConnected(nHpNo, object->GetClientTypeString());
    }
        break;

    case CMD_ID_MONITOR:
        break;

    case CMD_ID_AUDIO:
        break;

    case CMD_PLAYER_EXIT:
    {
        object->m_isExitCommandReceived = true;

        int nClientType = object->m_clientType;
//        int iChannel = object->m_iChannel;

        if(object->m_socket != nullptr)
        {
            CloseClientManual(object);

            // Device Controller 커넥션이 끊기면....
            if(object == m_VPSServer.m_pMobileControlClient)
            {
                m_VPSServer.m_pMobileControlClient = nullptr;
            }

            if(nClientType == CLIENT_GUEST)
            {

            }
        }

        object = nullptr;
    }
        break;

    case CMD_DISCONNECT_GUEST:
        break;

    case CMD_UPDATE_SERVICE_TIME:
        break;

    case CMD_DEVICE_DISCONNECTED:
        break;

    case CMD_START:
    {
        qDebug() << "Start Command";
        short sVarHor = SwapEndianU2(*(short*)pData);
        short sVarVer = SwapEndianU2(*((short*)(pData + 2)));

        m_isOnService[nHpNo-1] = true;

        emit deviceStart(nHpNo, sVarHor, sVarVer);

        // Start Mirroring
        emit startMirroring(nHpNo, ::DoMirrorCallback);
        //emit startMirroring(nHpNo, nullptr);
    }
        break;

    case CMD_STOP:
    {
        emit deviceStop(nHpNo, true);

        emit stopMirroring(nHpNo);
    }
        break;

    case CMD_VERTICAL:
        emit vpsCommandRotate(nHpNo, true);
        break;

    case CMD_HORIZONTAL:
        emit vpsCommandRotate(nHpNo, false);
        break;

    case CMD_AVIRECORD:
    {
        BYTE bVal = pData[0];
        if(bVal)
        {
            emit vpsCommandRecord(nHpNo, true);
        }
        else
        {
            emit vpsCommandRecord(nHpNo, false);
        }
    }
        break;

    default:
        m_VPSServer.SendToMobileController(m_VPSServer.m_pMobileControlClient->m_socket, pRcvData, dataSize);
        break;
    }
}

/* slots */
void NetManager::OnReadEx(ClientObject* object, BYTE* pPacket, int dwLen)
{
    if(pPacket[0] == CMD_START_CODE
            && object->m_rcvCommandBuffer[0] != CMD_START_CODE)
    {
        memcpy(object->m_rcvCommandBuffer, pPacket, dwLen);
        object->m_dwRcvPos = dwLen;
    }
    else
    {
        if(object->m_rcvCommandBuffer[0] != CMD_START_CODE) {
            // 처음이 시작코드가 아니면 버린다. 항상 시작 코드부터 패킷을 만든다.
            return;
        }

        memcpy(&object->m_rcvCommandBuffer[object->m_dwRcvPos], pPacket, dwLen);
        object->m_dwRcvPos += dwLen;
    }

    while(1)
    {
        if(object->m_rcvCommandBuffer[0] != CMD_START_CODE) break;

        bool bPacketOK = false;
        uint dataLen = SwapEndianU4(*(int*)&object->m_rcvCommandBuffer[1]);
        int totalDataLen = CMD_HEAD_SIZE + dataLen + CMD_TAIL_SIZE;

        if(object->m_dwRcvPos >= totalDataLen
                && object->m_rcvCommandBuffer[totalDataLen - 1] == (BYTE)CMD_END_CODE) {
            bPacketOK = true;
        }

        if(!bPacketOK)
        {
            qDebug() << "OnReadEx : Packet Not OK";
            return;
        }

        WebCommandDataParsing(object, object->m_rcvCommandBuffer, totalDataLen);
        object->m_rcvCommandBuffer[0] = 0;

        int remain = object->m_dwRcvPos - totalDataLen;
        if(remain > 0)
        {
            if(object->m_rcvCommandBuffer[totalDataLen] == CMD_START_CODE)
            {
                memcpy(object->m_rcvCommandBuffer, &object->m_rcvCommandBuffer[totalDataLen], remain);
                object->m_dwRcvPos = remain;
            }
        }
        else
        {
            // 남은것이 있는데, 첫 바이트가 CMD_START_CODE가 아니면 버린다.
        }
    }
}

void NetManager::OnMirrorCallback(BYTE* pMirroringPacket)
{
    BYTE* pSendPacket = pMirroringPacket;

    uint iDataLen = SwapEndianU4(*(uint32_t*)&pSendPacket[1]);
    short usCmd = SwapEndianU2(*(short*)&pSendPacket[5]);
    int nHpNo = *((BYTE*)&pSendPacket[7]);
    bool isKeyFrame = *(BYTE*)&pSendPacket[24];
    int dwTotLen = CMD_HEAD_SIZE + iDataLen + CMD_TAIL_SIZE;

//    qDebug() << "DoMirrorCallback : " << nHpNo << ", " << usCmd << ", " << iDataLen;

    SendToClient(usCmd, nHpNo, pSendPacket, dwTotLen, 0);
}

void NetManager::OnMirrorStoppedCallback(int nHpNo, int nStopCode)
{
    qDebug() << "OnMirrorStoppedCallback : " << nHpNo << ", " << nStopCode;

    // 비정상적으로 미러링 소켓이 닫힌 경우, DC에 알린다.
}

void NetManager::OnClosingClient(ClientObject* object)
{
    qDebug() << "OnClosingClient : " << object->m_strID;
    emit clientDisconnected(object->m_iChannel, object->GetClientTypeString());
}

void NetManager::OnTimer()
{
    for(int i = 0; i < MAXCHCNT; i++)
    {
        m_iCompressedCountOld[i] = m_iCompressedCount[i];
        m_iCompressedCount[i] = 0;

        // 테스트 코드
        if(i == 0 || i == 1)
        {
            if(m_iCompressedCountOld[i] != 0)
                qDebug() << "Device_" << (i+1) << " Send FPS : " << m_iCompressedCountOld[i];
        }
    }
}
