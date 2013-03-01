#ifndef _RFINTRAENCODER_H
#define	_RFINTRAENCODER_H

#include "../../../include/Typedefs.h"
#include "H264AVCEncoder.h"
#include <string>
#include <vector>

class RFIntraEncoder {

private:
    static UInt w, h;

    static FILE* traceFile, *costFile;
    static std::vector<Int> i4Modes;
    static UInt** i4Costs;
    static Int*** i4ModeMatrix;
    static Int i16Mode;
    static UInt i4Cost, i16Cost;
    static bool intraBlockSize; /* false: 16x16, true: 4x4 */

public:
    RFIntraEncoder();

    static void init(std::string name, UInt view, UInt numFrames, UInt w, UInt h);
    static void initMb();
    static void insertI16Mode(Int predMode, UInt cost);
    static void insertI4Mode(Int predMode, UInt cost, Int framePoc, UInt mbX, UInt mbY, h264::LumaIdx idx);
    
    static Int getI4Mode(UInt framePoc, UInt x, UInt y);

    static bool isI4Mode();

    static void report();
    static void reportI4Costs();

    static void close();

};

#endif	/* _RFINTRAENCODER_H */

