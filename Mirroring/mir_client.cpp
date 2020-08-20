#include "mir_client.h"
#include "MIR_Common.h"

#include <QDebug>
#include <QThread>

#include <unistd.h>
#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>

MIR_Client::MIR_Client(QObject *parent) : QObject(parent)
{
    m_pRcvBuf = nullptr;
    memset(m_sendBuf, 0, SEND_BUFFER_SIZE);
    m_dwRcvPos = 0;

    m_isThreadRunning = false;

    m_isMirroring = false;
    m_isControl = false;

    m_mirroringRoutine = nullptr;

    ResetVars();

    connect(&m_hMirrorSocket, SIGNAL(readyRead()), this, SLOT(OnReadyRead()));
    connect(&m_hMirrorSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(OnSocketStateChanged(QAbstractSocket::SocketState)));
//    connect(&m_hMirrorSocket, SIGNAL(connected()), this, SLOT(OnSocketConnected()));
//    connect(&m_hMirrorSocket, SIGNAL(disconnected()), this, SLOT(OnSocketDisconnected()));
    connect(&m_hControlSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(OnSocketStateChanged(QAbstractSocket::SocketState)));
//    connect(&m_hControlSocket, SIGNAL(connected()), this, SLOT(OnSocketConnected()));
//    connect(&m_hControlSocket, SIGNAL(disconnected()), this, SLOT(OnSocketDisconnected()));
}

void MIR_Client::SetIsThreadRunning(bool isThreadRunning)
{
    m_isThreadRunning = isThreadRunning;
}

bool MIR_Client::IsThreadRunning()
{
    return m_isThreadRunning;
}

void MIR_Client::SetHpNo(int nHpNo)
{
    m_nHpNo = nHpNo;
}

int MIR_Client::GetHpNo()
{
    return m_nHpNo;
}

void MIR_Client::SetControlPort(int nControlPort)
{
    m_nControlPort = nControlPort;
}

int MIR_Client::GetControlPort()
{
    return m_nControlPort;
}

void MIR_Client::SetMirrorPort(int nMirrorPort)
{
    m_nMirrorPort = nMirrorPort;
}

int MIR_Client::GetMirrorPort()
{
    return m_nMirrorPort;
}

//void MIR_Client::SetRunClientThreadReady(bool isRunClientThreadReady)
//{
//    m_isRunClientThreadReady = isRunClientThreadReady;
//}

//bool MIR_Client::IsRunClientThreadReady()
//{
//    return m_isRunClientThreadReady;
//}

//void MIR_Client::SetDoExitRunClientThread(bool isDoExitRunClientThread)
//{
//    m_isDoExitRunClientThread = isDoExitRunClientThread;
//}

//bool MIR_Client::IsDoExitRunClientThread()
//{
//    return m_isDoExitRunClientThread;
//}

void MIR_Client::CleanUpRunClientThreadData()
{
//    qDebug() << "MIR_Client::CleanUpRunClientThreadData 1";
    m_mCleanUpRunClientThreadData.lock();

//    if(IsThreadRunning())
    {
//        qDebug() << "MIR_Client::CleanUpRunClientThreadData 2";
        if(m_isMirroring)
        {
            qDebug() << "MIR_Client::CleanUpRunClientThreadData Mirroring Port Open";
//            m_hMirrorSocket.close();
//            m_hMirrorSocket.waitForDisconnected();
            m_hMirrorSocket.disconnectFromHost();
//            m_hMirrorSocket.waitForDisconnected();
//            m_isMirroring = false;
        }

        if(m_isControl)
        {
            qDebug() << "MIR_Client::CleanUpRunClientThreadData Control Port Open";
//            m_hControlSocket.close();
//            m_hControlSocket.waitForDisconnected();
            m_hControlSocket.disconnectFromHost();
//            m_hControlSocket.waitForConnected();
//            m_isControl = false;
        }

//        SetIsThreadRunning(false);

//        qDebug() << "MIR_Client::CleanUpRunClientThreadData 3";
//        emit mirrorStopped(GetHpNo(), EXIT_FAILURE);
    }

    ResetVars();

    m_mCleanUpRunClientThreadData.unlock();

//    qDebug() << "MIR_Client::CleanUpRunClientThreadData 4";
}

void MIR_Client::ResetVars()
{
    m_nHpNo = 0;
    m_nControlPort = 0;
    m_nMirrorPort = 0;

    m_isFirstImage = true;

    // memory free
    if(m_pRcvBuf != nullptr)
    {
        free(m_pRcvBuf);
        m_pRcvBuf = nullptr;
    }
}

bool MIR_Client::StartMirroring(int nHpNo, int nControlPort, int nMirrorPort, PMIRRORING_ROUTINE pMirroringRoutine)
{
    bool ret = false;

//    if(!IsThreadRunning())
    {
//        SetIsThreadRunning(true);

        if(!m_isMirroring)
        {
            m_mirroringRoutine = pMirroringRoutine;

            qDebug() << "MIR_Client::StartRunClientThread : " << nHpNo << ", " << nControlPort << ", " << nMirrorPort;
            SetControlPort(nControlPort);
            SetMirrorPort(nMirrorPort);
            SetHpNo(nHpNo);

            m_pRcvBuf = (char*)malloc(MIR_READ_BUF_SIZE);

    //        m_hMirrorSocket.setSocketOption(QAbstractSocket::SocketOption::LowDelayOption, 0);

            struct linger lin = {1, 0};
            setsockopt(m_hMirrorSocket.socketDescriptor(), SOL_SOCKET, SO_LINGER, (const char*)&lin, sizeof(lin));

            connect(&m_hMirrorSocket, SIGNAL(connected()), this, SLOT(OnSocketConnected()));
            connect(&m_hMirrorSocket, SIGNAL(disconnected()), this, SLOT(OnSocketDisconnected()));
            connect(&m_hMirrorSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(OnSockecError(QAbstractSocket::SocketError)));

            m_hMirrorSocket.connectToHost("127.0.0.1", GetMirrorPort());
            if(!m_hMirrorSocket.waitForConnected(1000))
            {
                qDebug() << "MIR_Client::StartRunClient Mirroring Socket Connect Failed";
                CleanUpRunClientThreadData();
                return false;
            }
        }

        //usleep(300);

        if(!m_isControl)
        {
            connect(&m_hControlSocket, SIGNAL(connected()), this, SLOT(OnSocketConnected()));
            connect(&m_hControlSocket, SIGNAL(disconnected()), this, SLOT(OnSocketDisconnected()));
            connect(&m_hControlSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(OnSockecError(QAbstractSocket::SocketError)));

            m_hControlSocket.connectToHost("127.0.0.1", GetControlPort());
            if(!m_hControlSocket.waitForConnected(1000))
            {
                qDebug() << "MIR_Client::StartRunClient Control Socket Connect Failed";
                CleanUpRunClientThreadData();
                return false;
            }

            //usleep(300);
        }

        m_rxStreamOrder = RX_PACKET_POS_START;

        ret = true;
    }

    return ret;
}

void MIR_Client::StopMirroring()
{
    // cmd: 20012 (mirror off)
    SendOnOffPacket(GetHpNo(), false);

    CleanUpRunClientThreadData();

    m_mirroringRoutine = nullptr;
}

int MIR_Client::SendOnOffPacket(int nHpNo, bool onoff)
{
    if(!m_isControl)
    {
        return SOCKET_ERROR;
    }

    int size = 0;
    int sendBytes = 0;
    const ONYPACKET_UINT8* pstrData = MakeOnyPacketControl(m_sendBuf, nHpNo, onoff, size);
    if(size != 0)
    {
        qDebug() << "SendOnOffPacket : " << onoff;
        sendBytes = SendToControlSocket((const char*)pstrData, size);
    }

    return sendBytes;
}

int MIR_Client::SendKeyFramePacket(int nHpNo)
{
    if(!m_isControl)
    {
        return SOCKET_ERROR;
    }

    int size = 0;
    int sendBytes = 0;
    const ONYPACKET_UINT8* pstrData = MakeOnyPacketSendKeyFrame(m_sendBuf, nHpNo, size);
    if(size != 0)
    {
        sendBytes = SendToControlSocket((const char*)pstrData, size);
    }

    return sendBytes;
}

int MIR_Client::SendToControlSocket(const char *buf, int len)
{
    int ret = SOCKET_ERROR;

    m_mSendToControlSocket.lock();

    ret = m_hControlSocket.write(buf, len);
    m_hControlSocket.flush();

    m_mSendToControlSocket.unlock();

    return ret;
}

void MIR_Client::OnReadEx(BYTE* pPacket, int dwLen)
{
    int iReadPos = 0, iWrite = 0;

    while(iReadPos < dwLen)
    {
        if(m_rxStreamOrder == RX_PACKET_POS_START)
        {
            for(; iReadPos < dwLen; ++iReadPos)
            {
                if(pPacket[iReadPos] == CMD_START_CODE)
                {
                    m_rxStreamOrder = RX_PACKET_POS_HEAD;
                    m_dwRcvPos = 0;
                    break;
                }
            }
        }

        if(m_rxStreamOrder == RX_PACKET_POS_HEAD)
        {
            iWrite = min(CMD_HEAD_SIZE - m_dwRcvPos, dwLen - iReadPos);
            memcpy(m_pRcvBuf + m_dwRcvPos, pPacket + iReadPos, iWrite);

            iReadPos += iWrite;
            m_dwRcvPos += iWrite;

            if(m_dwRcvPos == CMD_HEAD_SIZE)
            {
                m_rxStreamOrder = RX_PACKET_POS_DATA;
            }
        }

        if(m_rxStreamOrder == RX_PACKET_POS_DATA)
        {
            int iDataLen = ntohl(*(uint32_t*)&m_pRcvBuf[1]);
            int iPacketPos = m_dwRcvPos - CMD_HEAD_SIZE;

            iWrite = min(iDataLen - iPacketPos, dwLen - iReadPos);
            memcpy(m_pRcvBuf + m_dwRcvPos, pPacket + iReadPos, iWrite);

            iReadPos += iWrite;
            m_dwRcvPos += iWrite;

            if(iPacketPos == iDataLen)
            {
                m_rxStreamOrder = RX_PACKET_POS_TAIL;
            }
        }

        if(m_rxStreamOrder ==RX_PACKET_POS_TAIL)
        {
            int iDataLen = ntohl(*(uint32_t*)&m_pRcvBuf[1]);
            int iPacketPos = m_dwRcvPos - CMD_HEAD_SIZE - iDataLen;

            iWrite = min(CMD_TAIL_SIZE - iPacketPos, dwLen - iReadPos);
            memcpy(m_pRcvBuf + m_dwRcvPos, pPacket + iReadPos, iWrite);

            iReadPos += iWrite;
            m_dwRcvPos += iWrite;

            if(iPacketPos == CMD_TAIL_SIZE)
            {
                m_rxStreamOrder = RX_PACKET_POS_START;
                m_dwRcvPos = 0;

//                if(m_mirroringRoutine != nullptr)
//                    m_mirroringRoutine(m_pRcvBuf);

                emit mirrorEnqueue(GetHpNo(), m_pRcvBuf);
            }
        }
    }
}

/* slots */
void MIR_Client::OnReadyRead()
{
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    QByteArray data = sender->readAll();

//    qDebug() << "OnReadData : " << data.size();
    OnReadEx(data.data(), data.size());
}

void MIR_Client::OnSocketStateChanged(QAbstractSocket::SocketState state)
{
    qDebug() << "OnSocketStateChanged ========= " << state;
    if(state == QAbstractSocket::UnconnectedState)
    {
        QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
        int port = sender->peerPort();

        if(port == GetMirrorPort())
        {
            qDebug() << "MIR_Client::OnSocketStateChanged Mirroring Socket Unconnected";
            m_isMirroring = false;
        }
        else if(port == GetControlPort())
        {
            qDebug() << "MIR_Client::OnSocketStateChanged Control Socket Unconnected";
            m_isControl = false;
        }
    }
    else if(state == QAbstractSocket::ConnectedState)
    {
//        QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
//        int port = sender->peerPort();

//        if(port == GetMirrorPort())
//        {
//            qDebug() << "MIR_Client::OnSocketStateChanged Mirroring Socket Connected";

//            QThread::msleep(300);

//            // sendme
//            m_hMirrorSocket.write("sendme", sizeof("sendme"));
//            QThread::msleep(300);

//            m_isMirroring = true;

//            m_hControlSocket.connectToHost("127.0.0.1", GetControlPort());
//            m_hControlSocket.waitForConnected(1000);
//        }
//        else if(port == GetControlPort())
//        {
//            qDebug() << "MIR_Client::OnSocketStateChanged Control Socket Connected";

//            // cmd: 21002 (mirror on)
//            SendOnOffPacket(GetHpNo(), true);

//            QThread::msleep(300);

//            m_isControl = true;
//        }
    }
}

void MIR_Client::OnSocketConnected()
{
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    int port = sender->peerPort();

    if(port == GetMirrorPort())
    {
        qDebug() << "Mirroring Socket Connected";
        m_isMirroring = true;

        // sendme
        m_hMirrorSocket.write("sendme", sizeof("sendme"));
        m_hMirrorSocket.flush();
//        m_hMirrorSocket.waitForBytesWritten(300);
    }
    else if(port == GetControlPort())
    {
        qDebug() << "Control Socket Connected";
        m_isControl = true;

        // cmd: 21002 (mirror on)
        SendOnOffPacket(GetHpNo(), true);
    }

    if(m_isMirroring && m_isControl)
    {
        emit clientConnected(GetHpNo());
    }
}

void MIR_Client::OnSocketDisconnected()
{
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    int port = sender->peerPort();

    if(port == GetMirrorPort())
    {
        qDebug() << "Mirroring Socket Disconnected";
        m_isMirroring = false;
    }
    else if(port == GetControlPort())
    {
        qDebug() << "Control Socket Disconnected";
        m_isControl = false;
    }

    if(!m_isMirroring && !m_isControl)
    {
        emit mirrorStopped(GetHpNo(), EXIT_FAILURE);
    }
}

void MIR_Client::OnSockecError(QAbstractSocket::SocketError error)
{
    qDebug() << "Socket Error : " << error;

//    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
//    int port = sender->peerPort();
//    if(port == GetMirrorPort())
//    {
//        qDebug() << "Mirroring Socket Error";
//    }
//    else if(port == GetControlPort())
//    {
//        qDebug() << "Control Socket Error";
//    }
}
