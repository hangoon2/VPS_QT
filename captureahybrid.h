#ifndef CAPTUREAHYBRID_H
#define CAPTUREAHYBRID_H

#include "Mirroring/mirroring.h"
#include "Network/netmanager.h"

#include <QObject>
#include <QTime>

class CaptureAHybrid : public QObject
{
    Q_OBJECT
public:
    explicit CaptureAHybrid(QObject *parent = nullptr);

private:
    Mirroring m_mirror;
    NetManager m_netManager;

    QMutex m_mRecordLock[MAXCHCNT];

    int m_iMirrorVideoInputCount[MAXCHCNT];

    bool m_isRecordingCH[MAXCHCNT];

    QTime m_gtt;

    double m_dlLastCapTime[MAXCHCNT];
    double m_dlCaptureGap[MAXCHCNT];

public:
    void HybridInit();
    void HybridExit();

    void DeviceRotate(int nHpNo, bool vertical);
    void MirroRecordingCommand(int nHpNo, bool start);

    void OnMirrorRecording(int nHpNo, int iW, int iH, BYTE* pSrc);

    void MirrorIncreaseVideoInputCount(int nHpNo);
    bool IsRecording(int nHpNo);
    void SetRecording(int nHpNo, bool start);

    void SendKeyFrame(int nHpNo);

    int GetTime();

private:
    int MirrorGetVideoInputCount(int nHpNo);
    void MirrorClearVideoInputCount(int nHpNo);
    bool IsCompressTime(int nHpNo, double dlCaptureGap);

signals:

public slots:
    void OnMirrorRecordCallback(BYTE* pMirroringPacket);
};

#endif // CAPTUREAHYBRID_H
