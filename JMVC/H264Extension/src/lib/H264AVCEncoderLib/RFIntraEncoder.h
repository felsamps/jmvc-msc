#ifndef _RFINTRAENCODER_H
#define	_RFINTRAENCODER_H

#include "../../../include/Typedefs.h"

#include <string>
#include <vector>

class RFIntraEncoder {

private:
    static FILE* traceFile;
    static std::vector<Int> i4Modes;
    static Int i16Mode;
    static UInt i4Cost, i16Cost;
    static bool intraBlockSize; /* false: 16x16, true: 4x4 */

public:
    RFIntraEncoder();

    static void init(std::string name, UInt view);
    static void initMb();
    static void insertI16Mode(Int predMode, UInt cost);
    static void insertI4Mode(Int predMode, UInt cost);

    static bool isI4Mode();

    static void report();

    static void close();

};

#endif	/* _RFINTRAENCODER_H */

