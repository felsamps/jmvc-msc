#ifndef RFINTRACOMPRESSOR_H
#define	RFINTRACOMPRESSOR_H

#include "H264AVCEncoder.h"
#include "../../../include/H264AVCCommonLib/IntYuvPicBuffer.h"
#include "../H264AVCCommonLib/Debugger.h"

class RFIntraCompressor {
private:
    h264::IntYuvPicBuffer* pelFrame;
    
    Int yStride;
    XPel* pelY;
    Int cStride;
    XPel* pelU;
    XPel* pelV;
    
public:
    RFIntraCompressor(h264::IntYuvPicBuffer* frame);
    
    void debug();
};

#endif	/* RFINTRACOMPRESSOR_H */

