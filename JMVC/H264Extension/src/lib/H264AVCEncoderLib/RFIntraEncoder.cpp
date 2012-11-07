#include <string>

#include "RFIntraEncoder.h"
#include "SearchMonitor.h"

FILE* RFIntraEncoder::traceFile;
std::vector<Int> RFIntraEncoder::i4Modes;
Int RFIntraEncoder::i16Mode;
UInt RFIntraEncoder::i4Cost, RFIntraEncoder::i16Cost;
bool RFIntraEncoder::intraBlockSize; /* false: 16x16, true: 4x4 */

RFIntraEncoder::RFIntraEncoder() {
}

void RFIntraEncoder::init(std::string name, UInt view) {
	if(view == 0) {
		traceFile = fopen(name.c_str(), "w");
	}
	else {
		traceFile = fopen(name.c_str(), "a");
	}
}

void RFIntraEncoder::initMb() {
	i4Modes.clear();
	i4Cost = 0;
	i16Cost = 0x7FFFFFFF;
}

void RFIntraEncoder::insertI4Mode(Int predMode, UInt cost) {
	i4Modes.push_back(predMode);
	i4Cost += cost;	
}

void RFIntraEncoder::insertI16Mode(Int predMode, UInt cost) {
	i16Mode = predMode;
	i16Cost = cost;
}

bool RFIntraEncoder::isI4Mode() { /* false: 16x16, true: 4x4 */
	return (i16Cost > i4Cost);
}

void RFIntraEncoder::report() {
	
	fprintf(traceFile,"S %d ", i4Cost);
	for (int i = 0; i < 16; i++) {
		fprintf(traceFile,"%d ", i4Modes[i]);
	}
	fprintf(traceFile,"\n");
	
	fprintf(traceFile,"B %d %d\n", i16Cost, i16Mode);
	
}

void RFIntraEncoder::close() {
	fclose(traceFile);
}