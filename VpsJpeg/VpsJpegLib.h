#ifndef VPSJPEGLIB_H
#define VPSJPEGLIB_H

#include "VPSCommon.h"

#include <QImage>

bool VJ_OpenJpg(int nHpNo, BYTE* pJpgSrc, int nJpgSrcLen);
bool VJ_RotateLeft(int nHpNo);
bool VJ_RotateRight(int nHpNo);
bool VJ_Encode(int nHpNo, BYTE *o_pJpgDst, int *o_pnJpgDstLen, int nJpgQuality);
bool VJ_Copy(int nHpNo, QImage* o_image, int x, int y);

#endif // VPSJPEGLIB_H
