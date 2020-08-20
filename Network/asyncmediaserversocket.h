#ifndef ASYNCMEDIASERVERSOCKET_H
#define ASYNCMEDIASERVERSOCKET_H

#include "clientobject.h"
#include "VPSCommon.h"

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

class AsyncMediaServerSocket : public QObject
{
    Q_OBJECT
public:
    explicit AsyncMediaServerSocket(QObject *parent = nullptr);

public:
    ClientObject* m_pMobileControlClient;

private:
    QTcpServer m_serverSock;

    QList<ClientObject*> m_clientObjects;
    QList<ClientObject*> m_abnormallyDisconnectedClients;

    bool m_bHostNodeConnected[MAXCHCNT];

    QMutex m_mReconnection;

private:
    void UpdateHostNodeInfo();

public:
    int IsHostNodeConnected(int nHpNo);
    int GetHostNodeCount();
    ClientObject* FindAbnormallyDisconnectedClient(int nHpNo, ClientType clientType);
    ClientObject* FindAbnormallyDisconnectedClient(int nHpNo, ClientType clientType, QString strID, QString strIPAddr);
    void OnClose(QTcpSocket* socket);

    void Init_Socket(int port);
    void Dest_Socket();
    bool CheckAndRestoreOldState(ClientObject& r_pNewClientObj, int nHpNo, ClientType clientType, QString strID, QString strIPAddr);
    ClientObject* FindTheSameClient(int nHpNo, ClientType clientType, QString strID, QString strIPAddr);
    void OnCloseSocket(QTcpSocket* socket);
    bool SendToClient(short cmd, int nHpNo, BYTE* data, int len, int iKeyFrameNo, bool force = false);
    bool SendToMobileController(QTcpSocket* socket, BYTE* data, int dwCnt);
    bool OnSend(int nHpNo, QTcpSocket* sock, BYTE* data, int dwCnt, bool force = false);

signals:
    void readEx(ClientObject*, BYTE*, int);
    void closeClient(ClientObject*);

public slots:
    void OnNewConnection();
    void OnReadyRead();
    void OnSocketStateChanged(QAbstractSocket::SocketState state);
};

#endif // ASYNCMEDIASERVERSOCKET_H
