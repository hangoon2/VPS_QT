#ifndef MIRRORINGCALLBACK_H
#define MIRRORINGCALLBACK_H

#include "VPSCommon.h"

#include <functional>
using namespace std;

typedef function<void(BYTE*)> PMIRRORING_ROUTINE;
typedef function<void(int, int)> PMIRRORING_STOP_ROUTINE;

#endif // MIRRORINGCALLBACK_H
