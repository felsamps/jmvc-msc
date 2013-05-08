#include <string>
#include <unistd.h>

#include "RFIntraEncoder.h"
#include "SearchMonitor.h"
#include "RFIntraCompressor.h"

UInt RFIntraEncoder::w, RFIntraEncoder::h, RFIntraEncoder::nf, RFIntraEncoder::viewId;
FILE* RFIntraEncoder::traceFile;
FILE* RFIntraEncoder::costFile;
FILE* RFIntraEncoder::tempFile;
std::vector<Int> RFIntraEncoder::i4Modes;
Int RFIntraEncoder::i16Mode;
UInt RFIntraEncoder::i4Cost, RFIntraEncoder::i16Cost;
bool RFIntraEncoder::intraBlockSize; /* false: 16x16, true: 4x4 */
UInt** RFIntraEncoder::i4Costs;
Int**** RFIntraEncoder::i4ModeMatrix;


RFIntraEncoder::RFIntraEncoder() {
}

void RFIntraEncoder::init(std::string name, UInt view, UInt numFrames, UInt width, UInt heigth) {
	
	Int ALLOC_VIEWS_ARRAY[8] = {1, 3, 3, 5, 5, 7, 7, 8};
	Int NUM_READ_VIEWS[8] = {0, 2, 1, 4, 3, 6, 5, 7};
	Int VIEW_ID[8] = {0, 2, 1, 4, 3, 6, 5, 7};
		
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
	nf = numFrames;
	viewId = view;
	
	i4Costs = new UInt*[width/4];
	for (int i = 0; i < (width/4); i++) {
		i4Costs[i] = new UInt[heigth/4];
	}

	/* TODO allocate one more dimension */
	Int allocViews = ALLOC_VIEWS_ARRAY[view];
	
	i4ModeMatrix = new Int***[allocViews];
	for (int v = 0; v < (allocViews); v++) {
		i4ModeMatrix[v] = new Int**[numFrames];
		for (int i = 0; i < numFrames; i++) {
			i4ModeMatrix[v][i] = new Int*[width/4];
			for (int j = 0; j < width/4; j++) {
				i4ModeMatrix[v][i][j] = new Int[heigth/4];                        
			}
		}
	}

	if(viewId != 0)	{
		tempFile = fopen("intra_file.tmp", "r");

		/* TODO read tmp file and fill the data structure with the intra modes of previous views */
		for (int v = 0; v < NUM_READ_VIEWS[view]; v++) {
			for (int f = 0; f < numFrames; f++) {
				for (int y = 0; y < heigth/4; y++) {
					for (int x = 0; x < width/4; x++) {
						Int intraMode;
						fscanf(tempFile, "%d", &intraMode);
						i4ModeMatrix[VIEW_ID[v]][f][x][y] = intraMode;
					}
				}
			}
		}
		fclose(tempFile);
		tempFile = fopen("intra_file.tmp", "a");
	}
	else {
		tempFile = fopen("intra_file.tmp", "w");
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
	i4ModeMatrix[viewId][framePoc][mbX*4 + x][mbY*4 + y] = predMode;
	fprintf(costFile, "%d\n", cost);
	
}

Int RFIntraEncoder::getI4Mode(UInt viewId, UInt framePoc, UInt x, UInt y) {
        return i4ModeMatrix[viewId][framePoc][x][y];
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

void RFIntraEncoder::reportI4Modes() {
	for (int f = 0; f < nf; f++) {
		for (int y = 0; y < h/4; y++) {
			for (int x = 0; x < w/4; x++) {
				fprintf(tempFile, "%d ", i4ModeMatrix[viewId][f][x][y]);
			}
			fprintf(tempFile, "\n");
		}
	}
}

void RFIntraEncoder::close() {
	fclose(traceFile);
	fclose(costFile);
	fclose(tempFile);
}