#ifndef MIRRORING_H
#define MIRRORING_H

#include "mir_client.h"
#include "mir_queuehandler.h"
#include "VpsJpeg/vpsjpeg.h"

#include <QObject>
#include <QTime>

class Mirroring : public QObject
{
    Q_OBJECT
public:
    explicit Mirroring(QObject *parent = nullptr);

private:
    QMutex m_mMirroringStopRoutine[MAXCHCNT];

    MIR_Client m_mirClient[MAXCHCNT];
    MIR_QueueHandler m_mirQueHandler[MAXCHCNT];
//    MIR_QueueHandler m_mirQueHandler;

    bool m_isMirroringInited;

    VpsJpeg m_vpsJpeg;

    PMIRRORING_ROUTINE m_mirroringRoutine[MAXCHCNT];
    PMIRRORING_ROUTINE m_recordRoutine;

    QTime m_gtt;

public:
    bool MIR_InitializeMirroring(PMIRRORING_ROUTINE recordRoutine);
    void MIR_DestroyMirroring();
    void MIR_SetDeviceOrientation(int nHpNo, int iMode);
    void MIR_SendKeyFrame(int nHpNo);

//private:
    void MIR_HandleJpegPacket(BYTE* pPacket, uint iDataLen, short usCmd, int nHpNo);
    void MIR_HandleJpegCaptureFailedPacket(BYTE* pPacket, int nHpNo);

signals:
    void mirroringRoutine(BYTE*);
    void mirroringStopRoutine(int, int);
    void mirroringRecordingRoutine(BYTE*);

public slots:
   void MIR_StartMirroring(int nHpNo, PMIRRORING_ROUTINE pMirroringRoutine);
   void MIR_StopMirroring(int nHpNo);
   void MIR_EnQueue(int nHpNo, BYTE* pPacket);
   void MIR_OnMirrorStopped(int nHpNo, int nStopCode);
   void MIR_DoMirror(BYTE* pMirroringPacket);
   void OnClientConnected(int nHpNo);
};

#endif // MIRRORING_H
