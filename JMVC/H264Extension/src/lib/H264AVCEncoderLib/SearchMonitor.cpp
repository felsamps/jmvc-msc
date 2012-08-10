#include "SearchMonitor.h"

FILE *SearchMonitor::file;
BestMatch ****SearchMonitor::video;
UInt SearchMonitor::w, SearchMonitor::h, SearchMonitor::nFrames, SearchMonitor::currViewId;

SearchMonitor::SearchMonitor() {
}

void SearchMonitor::init(std::string name, UInt view, UInt width, UInt height, UInt numFrames) {
	if(view == 0) {
		file = fopen(name.c_str(), "w");
	}
	else {
		file = fopen(name.c_str(), "a");
	}

	currViewId = view;
	w = width/16;
	h = height/16;
	nFrames = numFrames;

	video = new BestMatch***[numFrames];
	for (int f = 0; f < numFrames; f++) {
		video[f] = new BestMatch**[w];
		for (int x = 0; x < w; x++) {
			video[f][x] = new BestMatch*[h];
			for (int y = 0; y < h; y++) {
				video[f][x][y] = new BestMatch();
			}
		}
	}
}

void SearchMonitor::insert(UInt poc, UInt xMb, UInt yMb, h264::Mv& vec, UInt frameId, UInt viewId, UInt cost, UInt bits) {
	BestMatch* bm = video[poc][xMb][yMb];
	bm->set(vec, frameId, viewId, cost, bits);
	
}

void SearchMonitor::reportAndClose() {
	fprintf(file,"%s",xReportSummary().c_str());
	//xReportByFrame();
}

std::string SearchMonitor::xReportSummary() {
	UInt meBestChoices, deBestChoices;
	std::string report;
	char temp[10];
	for (int f = 0; f < nFrames; f++) {
		meBestChoices = 0;
		deBestChoices = 0;
		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				if(video[f][x][y]->refViewId != -1) {
					if(video[f][x][y]->refViewId == currViewId) {
						meBestChoices ++;
					}
					else {
						deBestChoices ++;
					}
				}
			}
		}
		if(meBestChoices+deBestChoices != 0) {
			sprintf(temp,"%.6f ",meBestChoices/(double)(meBestChoices+deBestChoices));
		}
		else {
			sprintf(temp,"0.00 ");
		}
		report += temp;
	}
	report += "\n";
	return report;

}