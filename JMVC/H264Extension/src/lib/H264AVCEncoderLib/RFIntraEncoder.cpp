#include <string>

#include "RFIntraEncoder.h"
#include "SearchMonitor.h"

UInt RFIntraEncoder::w, RFIntraEncoder::h;
FILE* RFIntraEncoder::traceFile;
FILE* RFIntraEncoder::costFile;
std::vector<Int> RFIntraEncoder::i4Modes;
Int RFIntraEncoder::i16Mode;
UInt RFIntraEncoder::i4Cost, RFIntraEncoder::i16Cost;
bool RFIntraEncoder::intraBlockSize; /* false: 16x16, true: 4x4 */
UInt** RFIntraEncoder::i4Costs;
Int*** RFIntraEncoder::i4ModeMatrix;


RFIntraEncoder::RFIntraEncoder() {
}

void RFIntraEncoder::init(std::string name, UInt view, UInt numFrames, UInt width, UInt heigth) {
	if(view == 0) {
		traceFile = fopen(name.c_str(), "w");
		costFile = fopen("cost_file.mat", "w");
	}
	else {
		traceFile = fopen(name.c_str(), "a");
		costFile = fopen("cost_file.mat", "a");
	}
	w = width;
	h = heigth;
	i4Costs = new UInt*[width/4];
	for (int i = 0; i < (width/4); i++) {
		i4Costs[i] = new UInt[heigth/4];
	}
        
        i4ModeMatrix = new Int**[numFrames];
        for (int i = 0; i < numFrames; i++) {
            i4ModeMatrix[i] = new Int*[width/4];
            for (int j = 0; j < width/4; j++) {
                i4ModeMatrix[i][j] = new Int[heigth/4];                        
            }
        }
	
}

void RFIntraEncoder::initMb() {
	i4Modes.clear();
	i4Cost = 0;
	i16Cost = 0x7FFFFFFF;
}

void RFIntraEncoder::insertI4Mode(Int predMode, UInt cost, Int framePoc, UInt mbX, UInt mbY, h264::LumaIdx idx) {
	
	i4Modes.push_back(predMode);
	i4Cost += cost;
	Int x = (idx.b4x4()%4);
	Int y = (idx.b4x4()/4);
	i4Costs[mbX*4 + x][mbY*4 + y] = cost;
        i4ModeMatrix[framePoc][mbX*4 + x][mbY*4 + y] = predMode;
	fprintf(costFile, "%d\n", cost);
	
}

Int RFIntraEncoder::getI4Mode(UInt framePoc, UInt x, UInt y) {
    return i4ModeMatrix[framePoc][x][y];
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

void RFIntraEncoder::reportI4Costs() {
	for (int y = 0; y < h/4; y++) {
		for (int x = 0; x < w/4; x++) {
			fprintf(traceFile, "%d ", i4Costs[x][y]);
		}
		fprintf(traceFile, "\n");
	}

}

void RFIntraEncoder::close() {
	fclose(traceFile);
	fclose(costFile);
}