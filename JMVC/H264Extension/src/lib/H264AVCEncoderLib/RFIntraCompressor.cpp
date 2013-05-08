#include <unistd.h>

#include "RFIntraCompressor.h"

XPel*** RFIntraCompressor::pelRefFrames;
XPel*** RFIntraCompressor::pelCompressedFrames;
XPel*** RFIntraCompressor::pelOrigFrames;
XPel*** RFIntraCompressor::pointers;
XPel*** RFIntraCompressor::origPointers;
Int RFIntraCompressor::width;
Int RFIntraCompressor::height;
NonLinearQuantizer *RFIntraCompressor::q32, *RFIntraCompressor::q16, *RFIntraCompressor::q8; 

void RFIntraCompressor::init (Int numViews, Int numFrames, Int width, Int height) {
    
    pelRefFrames = new XPel**[numViews];
    pelCompressedFrames = new XPel**[numViews];
    pelOrigFrames = new XPel**[numViews];
    pointers = new XPel**[numViews];
    origPointers = new XPel**[numViews];
    for (int v = 0; v < numViews; v++) {
        pelRefFrames[v] = new XPel*[numFrames];
        pelCompressedFrames[v] = new XPel*[numFrames];
        pelOrigFrames[v] = new XPel*[numFrames];
        pointers[v] = new XPel*[numFrames];
        origPointers[v] = new XPel*[numFrames];
        for (int f = 0; f < numFrames; f++) {
            pelRefFrames[v][f] = NULL;
            pelCompressedFrames[v][f] = NULL;
            pelOrigFrames[v][f] = NULL;
            pointers[v][f] = NULL;
            origPointers[v][f] = NULL;
        }
    }
    
    RFIntraCompressor::width = width;
    RFIntraCompressor::height = height;
    
    q32 = new NonLinearQuantizer("quantizers/levels32.mat");
    q16 = new NonLinearQuantizer("quantizers/levels16.mat");
    q8 = new NonLinearQuantizer("quantizers/levels8.mat");
    
}

void RFIntraCompressor::initRefFrame(Int viewId, Int framePoc) {
    pelRefFrames[viewId][framePoc] = new XPel[width*height];
    pelCompressedFrames[viewId][framePoc] = new XPel[width*height];
    pelOrigFrames[viewId][framePoc] = new XPel[width*height];
}

void RFIntraCompressor::compressRefFrame(h264::IntYuvPicBuffer* refFrame, Int viewId, Int framePoc) {
    
    UInt yStride = refFrame->getLStride();
    
    if(pointers[viewId][framePoc] == NULL) {    
        XPel* p = refFrame->getMbLumAddr();
        XPel* pOrig = refFrame->getLumOrigin();
        
        initRefFrame(viewId, framePoc);
        
        pointers[viewId][framePoc] = p;
        origPointers[viewId][framePoc] = pOrig;
                
        /* Save the original reconstructed samples */
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                pelRefFrames[viewId][framePoc][x+y*width] = pointers[viewId][framePoc][x + y*yStride];             
                pelOrigFrames[viewId][framePoc][x+y*width] = origPointers[viewId][framePoc][x + y*yStride];             
            }
        }
        
        /* reference frame compression */
        for (int y = 0; y < height; y+=4) {
            for (int x = 0; x < width; x+=4) {                
                xCompressBlock(viewId, framePoc, x, y);
            }
        }        
    }
    
    /* Load the compressed reference samples (with losses) to be processed by the ME/DE */
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {                
            pointers[viewId][framePoc][x + y*yStride] = pelCompressedFrames[viewId][framePoc][x + y*width];
        }
    }
    
}

void RFIntraCompressor::recoverRefFrame(h264::IntYuvPicBuffer* refFrame, Int viewId, Int framePoc) {
    UInt yStride = refFrame->getLStride();
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pointers[viewId][framePoc][x + y*yStride] = pelRefFrames[viewId][framePoc][x+y*width];
        }
    }
}


void RFIntraCompressor::xCompressBlock(Int viewId, Int framePoc, Int blkX, Int blkY) {
    
    XPel **res, **pred, **recon8, **recon16, **recon32;
    res = new XPel*[4];
    pred = new XPel*[4];
    recon8 = new XPel*[4];
    recon16 = new XPel*[4];
    recon32 = new XPel*[4];
    
    for (int i = 0; i < 4; i++) {
        res[i] = new XPel[4];
        pred[i] = new XPel[4];
        recon8[i] = new XPel[4];
        recon16[i] = new XPel[4];
        recon32[i] = new XPel[4];
    }
    
    /* Symplified Intra Prediction*/
    Int predMode = RFIntraEncoder::getI4Mode(viewId, framePoc, blkX/4, blkY/4);
    
	XPel* n;
#if OPEN_LOOP
	n = xGetOrigNeighbors(viewId, framePoc, blkX, blkY);
#endif
	
#if CLOSE_LOOP
	n = xGetNeighbors(viewId, framePoc, blkX, blkY);
#endif
    
#if HYBRID_LOOP
	/* TODO implement it */
#endif
	
    switch(predMode) {
        case VER_SMODE:
                xVerticalMode(n, pred);
                break;
        case HOR_SMODE:
                xHorizonalMode(n, pred);
                break;
        case DC_SMODE:
                xDCMode(n, pred, blkX, blkY);
                break;
        case DL_SMODE:
                xDLMode(n, pred, blkX, blkY);
                break;
        case DR_SMODE:
                xDRMode(n, pred);
                break;
        case VR_SMODE:
                xVRMode(n, pred);
                break;
        case HD_SMODE:
                xHDMode(n, pred);
                break;
        case VL_SMODE:
                xVLMode(n, pred, blkX, blkY);
                break;
        case HU_SMODE:
                xHUMode(n, pred);
                break;
    }
    
    double mse8=0, mse16=0, mse32=0;
   
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            Int idX = blkX+x, idY = blkY+y;
            /* residual calculation */
            res[x][y] = pred[x][y] - pelRefFrames[viewId][framePoc][idX + (idY)*width];
            /* quantization */
            /* inverse quantization */
            /* reconstruction (iquanted + prediction)*/
            recon8[x][y] = pred[x][y] + ( q8->invQuantize( q8->quantize(res[x][y]) ) );
            recon16[x][y] = pred[x][y] + ( q16->invQuantize( q16->quantize(res[x][y]) ) );
            recon32[x][y] = pred[x][y] + ( q32->invQuantize( q32->quantize(res[x][y]) ) );
            
            mse8 += pow(recon8[x][y] - pelRefFrames[viewId][framePoc][idX + (idY)*width], 2);
            mse16 += pow(recon16[x][y] - pelRefFrames[viewId][framePoc][idX + (idY)*width], 2);
            mse32 += pow(recon32[x][y] - pelRefFrames[viewId][framePoc][idX + (idY)*width], 2);           
        }
    }
    
    mse8 /= 16;
    mse16 /= 16;
    mse32 /= 16;
    
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            Int idX = blkX+x, idY = blkY+y;
            Int MSE_TH = 4;
            pelCompressedFrames[viewId][framePoc][idX + (idY)*width] = (mse8 < MSE_TH) ? recon8[x][y] : 
                    (mse16 < MSE_TH) ? recon16[x][y] :
                    (mse32 < MSE_TH) ? recon32[x][y] :
                    pelRefFrames[viewId][framePoc][idX + (idY)*width];
        }
    }
}

bool RFIntraCompressor::xIsUpperAvailable(Int x, Int y) {
    return y != 0;
}

bool RFIntraCompressor::xIsLeftAvailable(Int x, Int y) {
    return x != 0;
}

bool RFIntraCompressor::xIsRigthUpperAvailable(Int x, Int y) {
    return (x+4 < width);
}

XPel* RFIntraCompressor::xGetOrigNeighbors(Int viewId, Int framePoc, Int x, Int y) {
    XPel* r = new XPel[13];
    XPel* frame = pelOrigFrames[viewId][framePoc];
    
    
    r[A] = (x!=0 && y!=0) ? frame[(x-1) + (y-1)*width] : -1;

    /* left samples */
    for (int j = 0; j < 4; j++) {
            r[3-j] = (x != 0) ? frame[(x-1) + (j+y)*width] : -1;
    }
    
    /* upper samples */
    for (int i = 0; i < 8; i++) {
            r[5+i] = (y == 0) ? -1 :
                     ((x+i) >= width) ? -1 :
                     frame[(i+x) + (y-1)*width];
    }
    
    return r;
}

XPel* RFIntraCompressor::xGetNeighbors(Int viewId, Int framePoc, Int x, Int y) {
    XPel* r = new XPel[13];
    XPel* frame = pelCompressedFrames[viewId][framePoc];
    
    r[A] = (x!=0 && y!=0) ? frame[(x-1) + (y-1)*width] : -1;

    /* left samples */
    for (int j = 0; j < 4; j++) {
            r[3-j] = (x != 0) ? frame[(x-1) + (j+y)*width] : -1;
    }
    
    /* upper samples */
    for (int i = 0; i < 8; i++) {
            r[5+i] = (y == 0) ? -1 :
                     ((x+i) >= width) ? -1 :
                     frame[(i+x) + (y-1)*width];
    }
    
    return r;
}

void RFIntraCompressor::xVerticalMode(XPel* n, XPel** pred) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int predSample = n[(4+1) + x];
            pred[x][y] = predSample;
	}
    }
}

void RFIntraCompressor::xHorizonalMode(XPel* n, XPel** pred) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int predSample = n[3-y];
            pred[x][y] = predSample;
        }
    }
}

void RFIntraCompressor::xDCMode(XPel* n, XPel** pred, Int blkX, Int blkY) {
    unsigned int predSample = 0;
    int validNeigh = 0;

    if (xIsLeftAvailable(blkX,blkY)) {
        predSample = n[L] + n[K] + n[J] + n[I];
        validNeigh +=4;
    }
    if (xIsUpperAvailable(blkX,blkY)) {
        predSample = n[A] + n[B] + n[C] + n[D];
        validNeigh +=4;
    }
    if(validNeigh != 0) {
        predSample /= validNeigh;
    }
    else {
        predSample = 0x80;
    }
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            pred[x][y] = predSample;
        }
    }
}

//TODO refactor
void RFIntraCompressor::xDRMode(XPel* n, XPel** pred) {
    pred[3][0] = (n[6] + n[7]*2 + n[8] + 2) >> 2;
    pred[2][0] = pred[3][1] = (n[5] + n[6]*2 + n[7] + 2) >> 2;
    pred[1][0] = pred[2][1] = pred[3][2] = (n[4] + n[5]*2 + n[6] + 2) >> 2;
    pred[0][0] = pred[1][1] = pred[2][2] = pred[3][3] = (n[3] + n[4]*2 + n[5] + 2) >> 2;
                              pred[0][1] = pred[1][2] = pred[2][3] = (n[2] + n[3]*2 + n[4] + 2) >> 2;
                                           pred[0][2] = pred[1][3] = (n[1] + n[2]*2 + n[3] + 2) >> 2;
                                                        pred[0][3] = (n[0] + n[1]*2 + n[2] + 2) >> 2;
}

void RFIntraCompressor::xDLMode(XPel* n, XPel** pred, Int x, Int y) {
    
    if(xIsRigthUpperAvailable(x,y)) {
            n[E] = n[D];
            n[F] = n[D];
            n[G] = n[D];
            n[H] = n[D];		
    }

    pred[0][0] =                                                                  (n[A] + n[B]*2 + n[C] + 2) >> 2;
    pred[1][0] = pred[0][1] =                                                     (n[B] + n[C]*2 + n[D] + 2) >> 2;
    pred[2][0] = pred[1][1] = pred[0][2] =                                        (n[C] + n[D]*2 + n[E] + 2) >> 2;
    pred[3][0] = pred[2][1] = pred[1][2] = pred[0][3] =                           (n[D] + n[E]*2 + n[F] + 2) >> 2;
                              pred[3][1] = pred[2][2] = pred[1][3] =              (n[E] + n[F]*2 + n[G] + 2) >> 2;
                                                        pred[3][2] = pred[2][3] = (n[F] + n[G]*2 + n[H] + 2) >> 2;
                                                                     pred[3][3] = (n[G] + n[H]*2 + n[I] + 2) >> 2;
}

void RFIntraCompressor::xVRMode(XPel* n, XPel** pred) {
    pred[0][0] = pred[1][2] = (n[A] + n[M] + 1) >> 1;
    pred[1][0] = pred[2][2] = (n[B] + n[A] + 1) >> 1;
    pred[2][0] = pred[3][2] = (n[C] + n[B] + 1) >> 1;
    pred[3][0] =              (n[D] + n[C] + 1) >> 1;

    pred[0][1] = pred[1][3] = (n[I] + n[M]*2 + n[A] + 2) >> 2;
    pred[1][1] = pred[2][3] = (n[M] + n[A]*2 + n[B] + 2) >> 2;
    pred[2][1] = pred[3][3] = (n[A] + n[B]*2 + n[C] + 2) >> 2;
    pred[3][1] =              (n[B] + n[C]*2 + n[D] + 2) >> 2;

    pred[0][2] = (n[J] + n[I]*2 + n[M] + 2) >> 2;
    pred[0][3] = (n[K] + n[J]*2 + n[I] + 2) >> 2;
}

//TODO refactor
void RFIntraCompressor::xHDMode(XPel* n, XPel** pred) {
    pred[0][0] = pred[2][1] = (n[4] + n[3] + 1) >> 1;
    pred[1][0] = pred[3][1] = (n[3] + n[4]*2 + n[5] + 2) >> 2;
    pred[2][0] =              (n[4] + n[5]*2 + n[6] + 2) >> 2;
    pred[3][0] =              (n[5] + n[6]*2 + n[7] + 2) >> 2;

    pred[0][1] = pred[2][2] = (n[3] + n[2] + 1) >> 1;
    pred[1][1] = pred[3][2] = (n[4] + n[3]*2 + n[2] + 2) >> 2;
    pred[0][2] = pred[2][3] = (n[2] + n[1] + 1) >> 1;
    pred[1][2] = pred[3][3] = (n[3] + n[2]*2 + n[1] + 2) >> 2;

    pred[0][3] = (n[0] + n[1] + 1) >> 1;
    pred[1][3] = (n[2] + n[1]*2 + n[0] + 2) >> 2;
}

//TODO refactor
void RFIntraCompressor::xVLMode(XPel* n, XPel** pred, Int x, Int y) {
    if(xIsRigthUpperAvailable(x,y)) {
        n[E] = n[D];
        n[F] = n[D];
        n[G] = n[D];
        n[H] = n[D];
    }

    pred[0][0] = (n[5] + n[6] + 1) >> 1;

    pred[1][0] = pred[0][2] = (n[6] + n[7] + 1) >> 1;
    pred[2][0] = pred[1][2] = (n[7] + n[8] + 1) >> 1;
    pred[3][0] = pred[2][2] = (n[8] + n[9] + 1) >> 1;
                 pred[3][2] = (n[9] + n[10] + 1) >> 1;

    pred[0][1] =              (n[5] + n[6]*2 + n[7] + 2) >> 2;
    pred[1][1] = pred[0][3] = (n[6] + n[7]*2 + n[8] + 2) >> 2;
    pred[2][1] = pred[1][3] = (n[7] + n[8]*2 + n[9] + 2) >> 2;
    pred[3][1] = pred[2][3] = (n[8] + n[9]*2 + n[10] + 2) >> 2;
                 pred[3][3] = (n[9] + n[10]*2 + n[11] + 2) >> 2;
}

//TODO refactor
void RFIntraCompressor::xHUMode(XPel* n, XPel** pred) {
    pred[0][0] =              (n[3] + n[2] + 1) >> 1;
    pred[1][0] =              (n[3] + n[2]*2 + n[1] + 2) >> 2;
    pred[2][0] = pred[0][1] = (n[2] + n[1] + 1) >> 1;
    pred[3][0] = pred[1][1] = (n[2] + n[1]*2 + n[0] + 2) >> 2;

    pred[2][1] = pred[0][2] = (n[1] + n[0] + 1) >> 1;
    pred[3][1] = pred[1][2] = (n[1] + n[0]*2 + n[0] + 2) >> 2;

    pred[2][2] = pred[3][2] = pred[0][3] = pred[2][3] = pred[1][3] = pred[3][3] = n[0];
}
