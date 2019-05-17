#ifndef _AMINER_GROUND_LAYER_H_
#define _AMINER_GROUND_LAYER_H_

#include <ace/utils/extview.h>

void groundLayerCreate(tVPort *pVp);

void groundLayerProcess(UWORD uwCameraY);

void groundLayerReset(UBYTE ubLowerLayer);

#endif // _AMINER_GROUND_LAYER_H_