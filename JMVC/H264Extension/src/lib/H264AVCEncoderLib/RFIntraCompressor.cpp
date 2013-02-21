#include <unistd.h>

#include "RFIntraCompressor.h"

RFIntraCompressor::RFIntraCompressor(h264::IntYuvPicBuffer* frame) {
    this->pelFrame = frame;
    
    yStride = pelFrame->getLStride();
    pelY = pelFrame->getLumBlk ();
    cStride = pelFrame->getCStride();
    pelU = pelFrame->getCbBlk  ();
    pelV = pelFrame->getCrBlk  ();
    
}

void RFIntraCompressor::debug() {
    
    XPel* p = pelFrame->getMbCbAddr();    
    
    for (int y = 0; y < 480/2; y++) {
        for (int x = 0; x < 640/2; x++) {
            Debugger::print("%d ", p[x + y*cStride]);
        }
        Debugger::print("\n");
    }
    
    p = pelFrame->getMbCrAddr();
    for (int y = 0; y < 480/2; y++) {
        for (int x = 0; x < 640/2; x++) {
            Debugger::print("%d ", p[x + y*cStride]);
        }
        Debugger::print("\n");
    }
    
}
