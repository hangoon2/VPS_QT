#include "vps.h"
#include "ui_vps.h"

#include <QDebug>
#include <QVBoxLayout>

VPS::VPS(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::VPS)
    , m_pHY(this)
{
    memset(m_isVpsVersionSend, 0, sizeof(m_isVpsVersionSend));
    memset(m_isToSkipFailureReport, 0, sizeof(m_isToSkipFailureReport));

    ui->setupUi(this);

    m_pHY.HybridInit();

    m_textUI = findChild<QTextEdit*>("logView");
    if(m_textUI != nullptr) {
        QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        policy.setVerticalStretch(2);
        m_textUI->setSizePolicy(policy);
        m_textUI->setReadOnly(true);

        AddLog(0, "UI 초기화 완료");
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    if(layout != nullptr)
    {
        layout->setSpacing(0);
        layout->setMargin(0);
        layout->setContentsMargins(0, 0, 0, 0);

        for(int i = 0; i < MAXCHCNT/2; i++)
        {
            QWidget *edit = new QWidget();

            QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            policy.setVerticalStretch(1);
            edit->setSizePolicy(policy);

            layout->addWidget(edit);
        }

        layout->addWidget(m_textUI, 0);

        centralWidget()->setLayout(layout);
    }
}

VPS::~VPS()
{
    m_pHY.HybridExit();

    delete ui;
}

void VPS::AddLog(int nHpNo, QString text)
{
    if(m_textUI != nullptr) {
        QString newLine = "";
        if(!m_textUI->toPlainText().isEmpty())
        {
            newLine = "\n";
        }

        QDateTime time = QDateTime::currentDateTime();
        QString log = newLine + time.toString("yyyy/MM/dd, HH:mm:ss:zzz") + " : [" + QString::number(nHpNo) + ":VPS] " + text;

        m_textUI->moveCursor(QTextCursor::End);
        m_textUI->textCursor().insertText(log);
        m_textUI->moveCursor(QTextCursor::End);
    }
}

/* slots */
void VPS::OnDeviceStart(int nHpNo, short width, short height)
{
    // 세로모드 기준으로 바꿔준다.
    if(width > height)
    {
        short tmp = height;
        height = width;
        width = tmp;
    }

    qDebug() << "OnDeviceStart : " << width << ", " << height;

    m_isVpsVersionSend[nHpNo-1] = true;

    QString text = "Start Command 명령 받음 : LCD Width=" + QString::number(width) + ", Height=" + QString::number(height);
    AddLog(nHpNo, text);
}

void VPS::OnDeviceStop(int nHpNo, bool force)
{
    AddLog(nHpNo, "Stop Command 명령 받음 : 채널 서비스 종료 처리 수행");
}

void VPS::OnClientConnected(int nHpNo, QString type)
{
    m_isToSkipFailureReport[nHpNo-1] = true;

    qDebug() << type << " is connected.";

    AddLog(nHpNo, type + " is connected.");
}

void VPS::OnClientDisconnected(int nHpNo, QString type)
{
    m_isToSkipFailureReport[nHpNo-1] = false;

    qDebug() << type << " is disconnected.";

    AddLog(nHpNo, type + " is disconnected.");
}

void VPS::OnVpsCommand_Rotate(int nHpNo, bool vertical)
{
    qDebug() << "VPS::OnVpsCommand_Rotate : " << vertical;
    m_pHY.DeviceRotate(nHpNo, vertical);

    AddLog(nHpNo, vertical ? "영상 세로 모드 출력" : "영상 가로 모드 출력");
}

void VPS::OnVpsCommand_Record(int nHpNo, bool start)
{
//    if(!m_pHY.IsRecording(nHpNo) && )
//    {

//    }

    if(m_pHY.IsRecording(nHpNo) == start)
    {
        return;
    }

    m_pHY.SetRecording(nHpNo, start);

    qDebug() << "VPS::OnVpsCommand_Record : " << start;

    m_pHY.MirroRecordingCommand(nHpNo, start);
}
