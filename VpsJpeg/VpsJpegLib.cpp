#include "VpsJpegLib.h"

#include <QDebug>
#include <QImage>
#include <QBuffer>

static QImage m_rawImage[MAXCHCNT];
static QMutex m_mJpg[MAXCHCNT];

bool VJ_OpenJpg(int nHpNo, BYTE* pJpgSrc, int nJpgSrcLen)
{
    m_mJpg[nHpNo-1].lock();

    m_rawImage[nHpNo-1].loadFromData((uchar*)pJpgSrc, nJpgSrcLen, "JPEG");

    m_mJpg[nHpNo-1].unlock();

    return true;
}

bool VJ_RotateLeft(int nHpNo)
{
    m_mJpg[nHpNo-1].lock();

    QSize size = m_rawImage[nHpNo-1].size();
    int width = size.width();
    int height = size.height();

    QTransform rotating;
    rotating.translate(width/2, height/2);
    rotating.rotate(-90);

    m_rawImage[nHpNo-1] = m_rawImage[nHpNo-1].transformed(rotating);

    m_mJpg[nHpNo-1].unlock();

    return true;
}

bool VJ_RotateRight(int nHpNo)
{
    m_mJpg[nHpNo-1].lock();

    QSize size = m_rawImage[nHpNo-1].size();
    int width = size.width();
    int height = size.height();

    QTransform rotating;
    rotating.translate(width/2, height/2);
    rotating.rotate(90);

    m_rawImage[nHpNo-1] = m_rawImage[nHpNo-1].transformed(rotating);

    m_mJpg[nHpNo-1].unlock();

    return true;
}

bool VJ_Encode(int nHpNo, BYTE *o_pJpgDst, int *o_pnJpgDstLen, int nJpgQuality)
{
    m_mJpg[nHpNo-1].lock();

    QByteArray arr;
    QBuffer buffer(&arr);
    buffer.open(QIODevice::WriteOnly);
    m_rawImage[nHpNo-1].save(&buffer, "JPEG", nJpgQuality);
    *o_pnJpgDstLen = arr.size();

    memcpy(o_pJpgDst, arr.data(), *o_pnJpgDstLen);

    buffer.close();

    m_mJpg[nHpNo-1].unlock();

    return true;
}

bool VJ_Copy(int nHpNo, QImage* o_image, int x, int y)
{
    m_mJpg[nHpNo-1].lock();

//    QPainter painter(o_image);
//    painter.drawImage(x, y, m_rawImage[nHpNo-1]);
//    painter.end();

    int lineSize = m_rawImage[nHpNo-1].bytesPerLine();
    BYTE* pSrc = (BYTE*)m_rawImage[nHpNo-1].bits();
    BYTE* pDest = (BYTE*)o_image->bits() + (x * 4) + (y * o_image->bytesPerLine());
    for(int i = 0; i < m_rawImage[nHpNo-1].height(); i++)
    {
        memcpy(pDest + (i * o_image->bytesPerLine()), pSrc + (i * lineSize), lineSize);
    }

    m_mJpg[nHpNo-1].unlock();

    return true;
}
