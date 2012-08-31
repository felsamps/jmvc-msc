#include <map>

#include "SearchMonitor.h"
#include "ReferenceFrameComm.h"
#include "TestDefinitions.h"

FILE *SearchMonitor::file, *SearchMonitor::fileByFrame;
FILE *SearchMonitor::fileMvMe, *SearchMonitor::fileMvDe;
FILE *SearchMonitor::fileMvdMe, *SearchMonitor::fileMvdDe;

BestMatch ****SearchMonitor::video;
UInt SearchMonitor::w, SearchMonitor::h, SearchMonitor::nFrames, SearchMonitor::currViewId;
Int SearchMonitor::deRefCounter, SearchMonitor::meRefCounter;
std::vector<std::map<std::pair<UInt,UInt>, Int> > SearchMonitor::refFrames;

SearchMonitor::SearchMonitor() {
}

void SearchMonitor::init(UInt view, UInt width, UInt height, UInt numFrames) {
	if(view == 0) {
#if REF_DIRECTION_EN
		file = fopen("search_monitor.mat", "w");
		fileByFrame = fopen("search_monitor_by_frame.mat", "w");
#endif
#if PR_ANALYSIS_EN
		fileMvMe = fopen("mv_tracing_me.mat", "w");
		fileMvDe = fopen("mv_tracing_de.mat", "w");
		fileMvdMe = fopen("mvd_me.mat", "w");
		fileMvdDe = fopen("mvd_de.mat", "w");
		
#endif
	}
	else {
#if REF_DIRECTION_EN
		file = fopen("search_monitor.mat", "a");
		fileByFrame = fopen("search_monitor_by_frame.mat", "a");
#endif
#if PR_ANALYSIS_EN
		fileMvMe = fopen("mv_tracing_me.mat", "a");
		fileMvDe = fopen("mv_tracing_de.mat", "a");
		fileMvdMe = fopen("mvd_me.mat", "a");
		fileMvdDe = fopen("mvd_de.mat", "a");
#endif
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
	std::pair<UInt, UInt> p(viewId, frameId);
	bm->set(vec, frameId, viewId, refFrames[poc][p], cost, bits);
	
}

void SearchMonitor::xReportRefFrame() {
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

h264::Mv* SearchMonitor::xCalcMvd(UInt f, UInt x, UInt y, Int idx) {
	h264::Mv *mv = video[f][x][y]->mvList[idx];
	Int xMvp, yMvp;
	/* simple case */
	if(x != 0)  { /* x!=0 and y==X */
		xMvp = x-1;
		yMvp = y;
	}
	else {
		if(y != 0) { /* x==0 and y!=0 */
			xMvp = x;
			yMvp = y-1;
		}
		else { /* x==0 and y==0 */
			xMvp = x;
			yMvp = y;
		}
	}
	h264::Mv *mvp = video[f][xMvp][yMvp]->mvList[idx];
	h264::Mv *mvd = new h264::Mv( (mvp->getHor() - mv->getHor()) , (mvp->getVer() - mv->getVer()) );
	return mvd;
	
}

void SearchMonitor::xReportMvTracing() {
	std::string reportMv, reportMvd;
	char temp[15];
	for (int f = 0; f < nFrames; f++) {
		std::map<std::pair<UInt,UInt>, Int> refs = refFrames[f];
		for(std::map<std::pair<UInt,UInt>, Int>::iterator it = refs.begin(); it != refs.end(); it ++) {
			std::pair<UInt,UInt> p = (*it).first;
			Int idx = (*it).second;
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++) {
					/* MotionDisparity Vector Printing */
					h264::Mv *mv = video[f][x][y]->mvList[idx];
					sprintf(temp, "%d %d\n", mv->getHor(), mv->getVer());
					reportMv += temp;
					/* Differential Vector Printing */
					h264::Mv *mvd = xCalcMvd(f, x, y, idx);
					sprintf(temp, "%d %d\n", mvd->getHor(), mvd->getVer());
					reportMvd += temp;
				}
			}
			
			if(currViewId == p.first) { //Motion Estimation
				fprintf(fileMvMe, "%s", reportMv.c_str());
				fprintf(fileMvdMe, "%s", reportMvd.c_str());
			}
			else { //Disparity Estimation
				fprintf(fileMvDe, "%s", reportMv.c_str());
				fprintf(fileMvdDe, "%s", reportMvd.c_str());
			}
			reportMv.clear();
		}		
	}
}

void SearchMonitor::reportAndClose() {
#if PR_ANALYSIS_EN
	xReportMvTracing();
	fclose(fileMvMe);
	fclose(fileMvDe);
	fclose(fileMvdMe);
	fclose(fileMvdDe);
#endif
#if REF_DIRECTION_EN
	xReportRefFrame();
	fclose(file);
	fclose(fileByFrame);
#endif

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
