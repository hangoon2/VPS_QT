#ifndef NETMANAGER_H
#define NETMANAGER_H

#include "asyncmediaserversocket.h"
#include "VPSCommon.h"
#include "Mirroring/MirroringCallback.h"

#include <QObject>
#include <QTimer>

class NetManager : public QObject
{
    Q_OBJECT
public:
    explicit NetManager(QObject *parent = nullptr);

private:
    AsyncMediaServerSocket m_VPSServer;

    bool m_bDCConnected;

    int m_iRefreshCH[MAXCHCNT];     // 클라이언트에서 영상 요구 채널 명령시 1, 전체 영상 전송 처리후 0

    uint m_iCompressedCount[MAXCHCNT];
    uint m_iCompressedCountOld[MAXCHCNT];

    int m_nCaptureCommandCountReceived[MAXCHCNT];
    int m_nCaptureCompletionCountSent[MAXCHCNT];
    int m_nRecordStartCommandCountReceived[MAXCHCNT];
    int m_nRecordStopCommandCountReceived[MAXCHCNT];
    int m_nRecordCompletionCountSent[MAXCHCNT];

    bool m_isOnService[MAXCHCNT];
    bool m_isReceivedRecordStartCommand[MAXCHCNT];

    QTimer m_timer;

private:
    void OnServerModeStart();
    bool DuplicatedClientSend(int nHpNo, ClientObject* object);
    bool DisconnectGuestSend(int nHpNo, ClientObject* object);
    bool Send(short cmd, int nHpNo, BYTE* data, int len, ClientObject* object, bool force = false);
    bool CloseClientManual(ClientObject* object);
    void WebCommandDataParsing(ClientObject* object, char* pRcvData, int dataSize);

public:
    void MediaSend_Init();
    void MediaSend_Close();

    bool SendToClient(short cmd, int nHpNo, BYTE* data, int len, int iKeyFrameNo);
    void MIR_BypassPacket(const void* pPacket);

signals:
    void deviceStart(int, short, short);
    void deviceStop(int, bool);
    void startMirroring(int, PMIRRORING_ROUTINE);
    void stopMirroring(int);
    void clientConnected(int, QString);
    void clientDisconnected(int, QString);
    void vpsCommandRotate(int, bool);
    void vpsCommandRecord(int, bool);

public slots:
    void OnReadEx(ClientObject* object, BYTE* pPacket, int dwLen);
    void OnMirrorCallback(BYTE* pMirroringPacket);
    void OnMirrorStoppedCallback(int nHpNo, int nStopCode);
    void OnClosingClient(ClientObject* object);

    void OnTimer();
};

#endif // NETMANAGER_H
