#ifndef CLIENTOBJECT_H
#define CLIENTOBJECT_H

#include "VPSCommon.h"

#include <QTcpSocket>

enum ClientType
{
    CLIENT_UNKNOWN = 0,
    CLIENT_MC,
    CLIENT_HOST,
    CLIENT_GUEST,
    CLIENT_MONITOR,
    CLIENT_AUDIO
};

class ClientObject
{
public:
    ClientObject();
    ~ClientObject();

    bool m_bFirstImage;

    int m_iKeyFrameLength;

    int m_iCurrentSentFPS;
    int m_iSentBytes;
    int m_iSentBytesOld;

    QTcpSocket *m_socket;
    BYTE* m_rcvCommandBuffer;
    int m_dwRcvPos;

    QString m_strID;
    QString m_strIPAddr;

    int m_iChannel;
    int m_iImageFullSizeDiv;
    int m_iCompressQuality;
    int m_iFrameRate;
    int m_iVideoJPGorH264;
    volatile bool m_iAudioOnOff;
//    QTime m_connectedTime;

    bool m_isAudioChannelExist;

    ClientType m_clientType;

//    VarLock m_vlSend;

    bool m_isExitCommandReceived;
//    QTime m_abnormalDisconnectedTime;

public:
    void Init(QTcpSocket* socket);
    void Close();
    QString GetClientTypeString();
    ClientObject* Clone();
    void ApplyState(ClientObject* pSrc);
    bool IsSameClient(int nHpNo, ClientType clientType, QString strID, QString strIPAddr);
};

#endif // CLIENTOBJECT_H
