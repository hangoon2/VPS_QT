#ifndef MIR_CLIENT_H
#define MIR_CLIENT_H

#include "MIR_Common.h"
#include "MirroringCallback.h"

#include <QObject>
#include <QTcpSocket>

enum PacketPos {
    RX_PACKET_POS_START,
    RX_PACKET_POS_HEAD,
    RX_PACKET_POS_DATA,
    RX_PACKET_POS_TAIL
};

class MIR_Client : public QObject
{
    Q_OBJECT
public:
    explicit MIR_Client(QObject *parent = nullptr);

private:
    QTcpSocket m_hMirrorSocket;
    QTcpSocket m_hControlSocket;

    char* m_pRcvBuf;
    int m_dwRcvPos;

    ONYPACKET_UINT8 m_sendBuf[SEND_BUFFER_SIZE];

    QMutex m_mCleanUpRunClientThreadData;
    QMutex m_mSendToControlSocket;

    bool m_isThreadRunning;
//    bool m_isRunClientThreadReady;
//    bool m_isDoExitRunClientThread;

    int m_nHpNo;
    int m_nControlPort;
    int m_nMirrorPort;

    bool m_isFirstImage;

    bool m_isMirroring;
    bool m_isControl;

    PMIRRORING_ROUTINE m_mirroringRoutine;

    PacketPos m_rxStreamOrder;

private:
    void SetIsThreadRunning(bool isThreadRunning);
    bool IsThreadRunning();
    void SetHpNo(int nHpNo);
    int GetHpNo();
    void SetControlPort(int nControlPort);
    int GetControlPort();
    void SetMirrorPort(int nMirrorPort);
    int GetMirrorPort();
//    void SetRunClientThreadReady(bool isRunClientThreadReady);
//    bool IsRunClientThreadReady();
//    void SetDoExitRunClientThread(bool isDoExitRunClientThread);
//    bool IsDoExitRunClientThread();
    void CleanUpRunClientThreadData();
    void ResetVars();

    int SendToControlSocket(const char* buf, int len);

    void OnReadEx(BYTE* pPacket, int dwLen);

public:
    bool StartMirroring(int nHpNo, int nControlPort, int nMirrorPort, PMIRRORING_ROUTINE pMirroringRoutine);
    void StopMirroring();
    int SendOnOffPacket(int nHpNo, bool onoff);
    int SendKeyFramePacket(int nHpNo);

signals:
    void mirrorEnqueue(int, BYTE*);
    void mirrorStopped(int, int);
    void clientConnected(int);

public slots:
    void OnReadyRead();
    void OnSocketStateChanged(QAbstractSocket::SocketState state);
    void OnSocketConnected();
    void OnSocketDisconnected();
    void OnSockecError(QAbstractSocket::SocketError error);
};

#endif // MIR_CLIENT_H
