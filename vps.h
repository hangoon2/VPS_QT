#ifndef VPS_H
#define VPS_H

#include "captureahybrid.h"

#include <QMainWindow>
#include <QTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class VPS; }
QT_END_NAMESPACE

class VPS : public QMainWindow
{
    Q_OBJECT

public:
    VPS(QWidget *parent = nullptr);
    ~VPS();

private:
    Ui::VPS *ui;

    CaptureAHybrid m_pHY;

    int m_iRunRecordingCH[MAXCHCNT];

    bool m_isVpsVersionSend[MAXCHCNT];

    QString m_strPathMediaFiles[MAXCHCNT];

    bool m_isToSkipFailureReport[MAXCHCNT];

    QTextEdit *m_textUI;

public:
    void AddLog(int nHpNo, QString text);

public slots:
    void OnDeviceStart(int nHpNo, short width, short height);
    void OnDeviceStop(int nHpNo, bool force);
    void OnClientConnected(int nHpNo, QString type);
    void OnClientDisconnected(int nHpNo, QString type);
    void OnVpsCommand_Rotate(int nHpNo, bool vertical);
    void OnVpsCommand_Record(int nHpNo, bool start);
};
#endif // VPS_H
