#include <map>

#include "SearchMonitor.h"
#include "ReferenceFrameComm.h"

FILE *SearchMonitor::file, *SearchMonitor::fileByFrame;
BestMatch ****SearchMonitor::video;
UInt SearchMonitor::w, SearchMonitor::h, SearchMonitor::nFrames, SearchMonitor::currViewId;
Int SearchMonitor::deRefCounter, SearchMonitor::meRefCounter;
std::vector<std::map<std::pair<UInt,UInt>, Int> > SearchMonitor::refFrames;

SearchMonitor::SearchMonitor() {
}

void SearchMonitor::init(UInt view, UInt width, UInt height, UInt numFrames) {
	if(view == 0) {
		file = fopen("search_monitor.mat", "w");
		fileByFrame = fopen("search_monitor_by_frame.mat", "w");
	}
	else {
		file = fopen("search_monitor.mat", "a");
		fileByFrame = fopen("search_monitor_by_frame.mat", "a");
	}

	currViewId = view;
	w = width/16;
	h = height/16;
	nFrames = numFrames;
	refFrames.resize(numFrames);

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
	UInt meBestChoices, deBestChoices;
	std::string report, reportByFrame;
	char temp[10];
	for (int f = 0; f < nFrames; f++) {
		meBestChoices = 0;
		deBestChoices = 0;
		for (int y = h-1; y >= 0; y--) {
			for (int x = 0; x < w; x++) {
				if(video[f][x][y]->refViewId != -1) {
					std::pair<UInt, UInt> p(video[f][x][y]->refViewId, video[f][x][y]->refFrameId);
					if(refFrames[f][p] < 0) {
						sprintf(temp, "%d;", refFrames[f][p]);
					}
					else {
						sprintf(temp, "%d;", refFrames[f][p]);
					}
					
					if(video[f][x][y]->refViewId == currViewId) {
						meBestChoices ++;
					}
					else {
						deBestChoices ++;
					}
					reportByFrame += temp;
				}
				else {
					reportByFrame += "0;";
				}
			}
			reportByFrame += "\n";
		}
		if(meBestChoices+deBestChoices != 0) {
			sprintf(temp,"%.2f;",meBestChoices/(double)(meBestChoices+deBestChoices));
		}
		else {
			sprintf(temp,"0.00;");
		}
		report += temp;
	}
	report += "\n";

	fprintf(file, "%s", report.c_str());
	fprintf(fileByFrame, "%s", reportByFrame.c_str());

}

void SearchMonitor::initCounters() {
	deRefCounter = -1;
	meRefCounter = +1;	
}

void SearchMonitor::insertRefFrame(UInt currFrame, UInt viewId, UInt framePOC) {
	std::pair<UInt,UInt> p(viewId,framePOC);
	if(viewId == currViewId) { //Motion Estimation
		refFrames[currFrame][p] = meRefCounter;
		meRefCounter ++;
	}
	else { //Disparity Estimation
		refFrames[currFrame][p] = deRefCounter;
		deRefCounter --;
	}
}
