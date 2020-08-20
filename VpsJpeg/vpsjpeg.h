#ifndef VPSJPEG_H
#define VPSJPEG_H

#include "VPSCommon.h"

#include <QPixmap>
#include <QBuffer>
#include <QImage>

class VpsJpeg
{
public:
    VpsJpeg();

private:
    QImage m_rawImage[MAXCHCNT];

//    QBuffer m_buffer[MAXCHCNT];

    QMutex m_mJpg[MAXCHCNT];

public:
    bool VJ_OpenJpg(int nHpNo, BYTE* pJpgSrc, int nJpgSrcLen);
    bool VJ_RotateLeft(int nHpNo);
    bool VJ_RotateRight(int nHpNo);
    bool VJ_Encode(int nHpNo, BYTE* o_pJpgDst, int* o_pnJpgDstLen, int nJpgQuality);
    bool VJ_Copy(int nHpNo, QImage* o_image, int x, int y);
};

#endif // VPSJPEG_H
