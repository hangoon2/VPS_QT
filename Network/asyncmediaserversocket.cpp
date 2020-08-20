#include "asyncmediaserversocket.h"

#include <QDebug>

AsyncMediaServerSocket::AsyncMediaServerSocket(QObject *parent) : QObject(parent)
{
    m_pMobileControlClient = nullptr;

    memset(m_bHostNodeConnected, 0, sizeof(m_bHostNodeConnected));
}

void AsyncMediaServerSocket::Init_Socket(int port)
{
    m_serverSock.listen(QHostAddress::Any, port);

    connect(&m_serverSock, SIGNAL(newConnection()), this, SLOT(OnNewConnection()));

    qDebug() << "VPS Server Socket Init";
}

void AsyncMediaServerSocket::Dest_Socket()
{
    qDebug() << "VPS Server Socket Close";
    if(!m_clientObjects.isEmpty())
    {
        for(int i = 0; i < m_clientObjects.size(); i++)
        {
            ClientObject* clientObject = m_clientObjects.at(i);
            if(clientObject != nullptr)
            {
                delete clientObject;
                clientObject = nullptr;
            }
        }

        m_clientObjects.clear();
    }
}

void AsyncMediaServerSocket::UpdateHostNodeInfo()
{
    bool bHostNodeConnected[MAXCHCNT];
    memset(bHostNodeConnected, false, sizeof(bHostNodeConnected));

    for(int i = 0; i < m_clientObjects.size(); i++)
    {
        ClientObject* object = m_clientObjects.at(i);
        if(object != nullptr
                && object->m_socket != nullptr
                && object->m_clientType == CLIENT_HOST)
        {
            // 연결된 것만 true로 설정
            bHostNodeConnected[object->m_iChannel - 1] = true;
        }
    }

    memcpy(m_bHostNodeConnected, bHostNodeConnected, sizeof(bHostNodeConnected));
}

int AsyncMediaServerSocket::IsHostNodeConnected(int nHpNo)
{
    return m_bHostNodeConnected[nHpNo-1];
}

int AsyncMediaServerSocket::GetHostNodeCount()
{
    int nHostNodeCount = 0;
    for(int i = 0; i < MAXCHCNT; i++) {
        if(m_bHostNodeConnected[i]) {
            ++nHostNodeCount;
        }
    }

    return nHostNodeCount;
}

ClientObject* AsyncMediaServerSocket::FindAbnormallyDisconnectedClient(int nHpNo, ClientType clientType)
{
    ClientObject* ret = nullptr;

    for(int i = 0; i < m_abnormallyDisconnectedClients.size(); i++) {
        ClientObject* object = m_abnormallyDisconnectedClients.at(i);
        if(object != nullptr && object->m_iChannel == nHpNo && object->m_clientType == clientType)
        {
            ret = object;
            break;
        }
    }

    return ret;
}

ClientObject* AsyncMediaServerSocket::FindAbnormallyDisconnectedClient(int nHpNo, ClientType clientType, QString strID, QString strIPAddr)
{
    ClientObject* ret = nullptr;

    for(int i = 0; i < m_abnormallyDisconnectedClients.size(); i++) {
        ClientObject* object = m_abnormallyDisconnectedClients.at(i);
        if(object != nullptr && object->IsSameClient(nHpNo, clientType, strID, strIPAddr))
        {
            ret = object;
            break;
        }
    }

    return ret;
}

void ::AsyncMediaServerSocket::OnClose(QTcpSocket* socket)
{
    socket->close();
    socket = nullptr;

    for(int i = 0; i < m_clientObjects.size(); i++)
    {
        ClientObject* object = m_clientObjects.at(i);
    }
}

bool AsyncMediaServerSocket::CheckAndRestoreOldState(ClientObject &r_pNewClientObj, int nHpNo, ClientType clientType, QString strID, QString strIPAddr)
{
    bool ret = false;

    m_mReconnection.lock();

    ClientObject* pOldClientObj = FindAbnormallyDisconnectedClient(nHpNo, clientType, strID, strIPAddr);
    if(pOldClientObj != nullptr)
    {
        // Apply State
        r_pNewClientObj.ApplyState(pOldClientObj);

        // Remove from list
        m_abnormallyDisconnectedClients.removeOne(pOldClientObj);
        delete pOldClientObj;
        ret = true;
    }

    m_mReconnection.unlock();

    return ret;
}

ClientObject* AsyncMediaServerSocket::FindTheSameClient(int nHpNo, ClientType clientType, QString strID, QString strIPAddr)
{
    ClientObject* ret = nullptr;

    for(int i = 0; i < m_clientObjects.size(); i++)
    {
        ClientObject* object = m_clientObjects.at(i);
        if(object != nullptr && object->IsSameClient(nHpNo, clientType, strID, strIPAddr))
        {
            ret = object;
            break;
        }
    }

    return ret;
}

void AsyncMediaServerSocket::OnCloseSocket(QTcpSocket* socket)
{
//    ClientObject* o_pClientObj = nullptr;
    int nIndexInArray = -1;

    for(int i = 0; i < m_clientObjects.size(); i++)
    {
        ClientObject* object = m_clientObjects.at(i);
        if(object != nullptr && object->m_socket == socket)
        {
            nIndexInArray = i;

            QString strIP = socket->peerAddress().toString();

            qDebug() << "클라이언트 연결 끊김 => IP: " << strIP;

            OnClose(socket);

            object->Close();

            break;
        }
    }

//    if(GetClientObjFromSocket(&o_pClientObj, &o_nIndexInArray, socket))
//    {
//        QString strIP = socket->peerAddress().toString();

//        qDebug() << "클라이언트 연결 끊김 => IP: " << strIP;

//        OnClose(socket);
//    }

//    socket->close();
}

bool AsyncMediaServerSocket::SendToClient(short cmd, int nHpNo, BYTE *data, int len, int iKeyFrameNo, bool force)
{
    int iClientCountSent = 0;
    for(int i = 0; i < m_clientObjects.size(); i++)
    {
        ClientObject* object = m_clientObjects.at(i);
        if(object != nullptr
                && object->m_iChannel == nHpNo
                && object->m_clientType != CLIENT_MC)
        {
            if(OnSend(nHpNo, object->m_socket, data, len, force))
            {
                iClientCountSent++;
            }
        }
    }

    return iClientCountSent > 0 ? true : false;
}

bool AsyncMediaServerSocket::SendToMobileController(QTcpSocket* socket, BYTE* data, int dwCnt) {
    if(socket != nullptr) {
        socket->write(data, dwCnt);
        socket->flush();
        return true;
    }

    return false;
}

bool AsyncMediaServerSocket::OnSend(int nHpNo, QTcpSocket *sock, BYTE *data, int dwCnt, bool force)
{
    if(sock != nullptr)
    {
        sock->write(data, dwCnt);
        sock->flush();
        return true;
    }

    return false;
}

/* slots */
void AsyncMediaServerSocket::OnNewConnection()
{
    QTcpSocket* clientSocket = m_serverSock.nextPendingConnection();

    // TCP_NODELAY
    clientSocket->setSocketOption(QAbstractSocket::SocketOption::LowDelayOption, 0);

    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(OnReadyRead()));
    connect(clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(OnSocketStateChanged(QAbstractSocket::SocketState)));

    ClientObject* clientObject = new ClientObject();

    clientObject->Init(clientSocket);
    clientObject->m_strIPAddr = clientSocket->peerAddress().toString();

    qDebug() << "Connected IP : " + clientObject->m_strIPAddr;

    m_clientObjects.append(clientObject);
}

void AsyncMediaServerSocket::OnReadyRead()
{
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    QByteArray data = sender->readAll();

    ClientObject* clientObject = nullptr;
    for(int i = 0; i < m_clientObjects.size(); i++)
    {
        ClientObject *pObj = m_clientObjects.at(i);
        if(pObj != nullptr)
        {
            if(pObj->m_socket == sender)
            {
                clientObject = pObj;
                break;
            }
        }
    }

    if(clientObject != nullptr)
        emit readEx(clientObject, data.data(), data.size());

    UpdateHostNodeInfo();
}

void AsyncMediaServerSocket::OnSocketStateChanged(QAbstractSocket::SocketState state)
{
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());

    if(state == QAbstractSocket::ClosingState)
    {
        for(int i = 0; i < m_clientObjects.size(); i++)
        {
            ClientObject *pObj = m_clientObjects.at(i);
            if(pObj != nullptr)
            {
                if(pObj->m_socket == sender)
                {
                    emit closeClient(pObj);
                    break;
                }
            }
        }
    }
}
