#ifndef RFINTRACOMPRESSOR_H
#define	RFINTRACOMPRESSOR_H

#include "H264AVCEncoder.h"
#include "../../../include/H264AVCCommonLib/IntYuvPicBuffer.h"
#include "../H264AVCCommonLib/Debugger.h"
#include "NonLinearQuantizer.h"
#include "TestDefinitions.h"
#include "RFIntraEncoder.h"
#include "TestDefinitions.h"

enum IntraMode {
    VER_SMODE,
    HOR_SMODE,
    DC_SMODE,
    DL_SMODE,
    DR_SMODE,
    VR_SMODE,
    HD_SMODE,
    VL_SMODE,
    HU_SMODE
};

/* Neighbors naming:
 * M A B C D E F G H
 * I 
 * J
 * K
 * L
 */

enum PredNeighbors {
    L, K, J, I, M, A, B, C, D, E, F, G, H
};


class RFIntraCompressor {
private:
    static XPel*** pelRefFrames;
    static XPel*** pelCompressedFrames;
    static XPel*** pointers;
    static Int width, height;
    
    static NonLinearQuantizer *q32, *q16, *q8;    
       
public:
    RFIntraCompressor();
    
    static void init(Int numViews, Int numFrames, Int width, Int height);
    static void initRefFrame(Int viewId, Int framePoc);
    static void compressRefFrame(h264::IntYuvPicBuffer* refFrame, Int viewId, Int framePoc);
    static void recoverRefFrame(h264::IntYuvPicBuffer* refFrame, Int viewId, Int framePoc);
    
    static void xCompressBlock(Int viewId, Int framePoc, Int x, Int y);
    static XPel* xGetNeighbors(Int viewId, Int framePoc, Int x, Int y);
    static bool xIsUpperAvailable(Int x, Int y);
    static bool xIsLeftAvailable(Int x, Int y);
    static bool xIsRigthUpperAvailable(Int x, Int y);
    static void xHorizonalMode(XPel* n, XPel** pred);
    static void xVerticalMode(XPel* n, XPel** pred);
    static void xDCMode(XPel* n, XPel** pred, Int x, Int y);
    static void xDLMode(XPel* n, XPel** pred, Int x, Int y);
    static void xDRMode(XPel* n, XPel** pred);
    static void xVRMode(XPel* n, XPel** pred);
    static void xHDMode(XPel* n, XPel** pred);
    static void xVLMode(XPel* n, XPel** pred, Int x, Int y);
    static void xHUMode(XPel* n, XPel** pred);
    
};

#endif	/* RFINTRACOMPRESSOR_H */

