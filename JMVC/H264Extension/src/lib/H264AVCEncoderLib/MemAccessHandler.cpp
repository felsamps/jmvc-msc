
#include <stdlib.h>
#include <string>
#include <map>
#include <list>

#include "MemAccessHandler.h"
#include "TestDefinitions.h"

FILE* MemAccessHandler::fp_me;
FILE* MemAccessHandler::fp_de;
FILE* MemAccessHandler::fpSearchMap;
std::set<std::pair<int,int> > MemAccessHandler::acc;
std::set<int> MemAccessHandler::block_acc;
int MemAccessHandler::counter;
int MemAccessHandler::currMbX;
int MemAccessHandler::currMbY;
int MemAccessHandler::currPoc;
int MemAccessHandler::refPoc;
bool MemAccessHandler::bipred;
int MemAccessHandler::refView;
int MemAccessHandler::currView;
int MemAccessHandler::height;
int MemAccessHandler::width;
unsigned int MemAccessHandler::searchWindow, MemAccessHandler::searchRange, MemAccessHandler::swResolution;
long long int MemAccessHandler::bw;
std::map<int,int> MemAccessHandler::usage_me;
std::map<int,int> MemAccessHandler::usage_de;
int MemAccessHandler::numRefFrames;
int MemAccessHandler::refsMe;
int MemAccessHandler::refsDe;
std::pair<int,int> MemAccessHandler::mvPredictor;
std::map<std::pair<UInt,UInt>, long long**> MemAccessHandler::swMap;
std::list<std::pair<UInt, UInt> > MemAccessHandler::refs;

MemAccessHandler::MemAccessHandler() {
}

MemAccessHandler::~MemAccessHandler() {
}

void MemAccessHandler::openFile(unsigned int view, unsigned int range) {
    if(view == 0) {
        fp_me = fopen("sw_usage_me.mat", "w");
        fp_de = fopen("sw_usage_de.mat", "w");
#if SW_SEARCH_MAP
        fpSearchMap = fopen("search_map.mat", "w");
#endif
    }
    else {
        fp_me = fopen("sw_usage_me.mat", "a");
		fp_de = fopen("sw_usage_de.mat", "a");
#if SW_SEARCH_MAP
        fpSearchMap = fopen("search_map.mat", "a");
#endif
    }
	swResolution = (range/16) * 2 + 1;

	swMap.clear();
	refs.clear();
	

}

void MemAccessHandler::reportSearchOccurrences() {
	while(!refs.empty()) {
		std::pair<UInt,UInt> p = refs.front();
		for (int y = 0; y < swResolution; y++) {
			for (int x = 0; x < swResolution; x++) {
				fprintf(fpSearchMap, "%lld ", swMap[p][x][y]);
			}
			fprintf(fpSearchMap, "\n");
		}
		refs.pop_front();
	}
	swMap.clear();
	refs.clear();
}

void MemAccessHandler::closeAndReport() {
    fclose(fp_me);
    fclose(fp_de);
#if SW_SEARCH_MAP
	fclose(fpSearchMap);
#endif
}

void MemAccessHandler::init() {
    counter = 0;
    acc.clear();
    block_acc.clear();
}

void MemAccessHandler::initCurrMB() {
    usage_me.clear();
    usage_de.clear();
    refsMe = 0;
    refsDe = 0;
}

void MemAccessHandler::setMb(int x, int y) {
    currMbX = x*16;
    currMbY = y*16;
}

void MemAccessHandler::insert(std::pair<int, int> sp) {
    acc.insert(sp);
}

void MemAccessHandler::insertBlock(int x, int y, int size) {
    //INSERT SAMPLE ACCESS!

    int posx = x - mvPredictor.first;
    int posy = y - mvPredictor.second;
    
    counter ++;
    
#if SW_ANALYSIS
    for (int i = posx; i < posx+size; i++) {
        for (int j = posy; j < posy+size; j++) {
            std::pair<int, int> sp(i, j);
            insert(sp);
        }    
    }
#else
    
    std::pair<int,int> bp0(posx/size         , posy/size        );
    std::pair<int,int> bp1(posx/size         ,(posy+size)/size-1);
    std::pair<int,int> bp2((posx+size)/size-1, posy/size        );
    std::pair<int,int> bp3((posx+size)/size-1,(posy+size)/size-1);

    acc.insert(bp0);
    acc.insert(bp1);
    acc.insert(bp2);
    acc.insert(bp3);
#endif

}

void MemAccessHandler::reportSearchMap() {
    
    int offset = searchRange/16;
    int minX = -offset;
    int minY = -offset;

    //fprintf(fpSearchMap,"%d %d %d %d %d %d %c %d %d \n",  currView, currPoc, currMbX, currMbY, refView, refPoc, (refView==currView) ? '0' : '1', mvPredictor.first, mvPredictor.second);
    for(int i = minY; i < (minY + 2*offset+1); i++) {
        for(int j = minX; j < (minX + 2*offset+1); j++) {
            std::pair<int,int> p(j, i);
			/*fprintf(fpSearchMap, "%c ", (acc.find(p)!=acc.end()) ? ((i == 0 and j == 0) ? '*' : '1') :	'0');*/
			if(acc.find(p)!=acc.end()) {
				std::pair<UInt,UInt> ref(refView, refPoc);
				swMap[ref][j+offset][i+offset] += 1;
			}
		}
    }
}

void MemAccessHandler::insertUsage() {
    if( !bipred ) {

#if SW_SEARCH_MAP
        reportSearchMap();
#endif
        int idx = ((currMbY/16) * width/16) + (currMbX/16);
        
        if(currView == refView)  {/* motion estimation */
            
            /* Incrementing the number of reference frames (ME) */
            refsMe += (idx == 0) ? 1 : 0;
             
            if(usage_me.find(idx) != usage_me.end()) {
                usage_me[idx] += acc.size();
            }
            else {
                
                usage_me[idx] = acc.size();
            }
        }
        else { /* disparity estimation */
            
            /* Incrementing the number of reference frames (ME) */
            refsDe += (idx == 0) ? 1 : 0;
            
            if(usage_de.find(idx) != usage_de.end()) {
                usage_de[idx] += acc.size();
            }
            else {
                usage_de[idx] = acc.size();
            }
        }
    }
}

void MemAccessHandler::report() {
    
    for(int x=0; x < width/16; x++) {
        for (int y = 0; y < height/16; y++) {
                       
            
            if(!usage_me.empty()) {
#if SW_ANALYSIS
                int usage = usage_me[y*width/16 + x];
                double swUsage = (double)usage/(double)(searchWindow*refsMe);
                //fprintf(fp_me, "%d %d %d %d %.2f\n", currView, currPoc, usage, refsMe, swUsage * 100);
                fprintf(fp_me, "%.2f\n", swUsage * 100);
#else
                int usage = usage_me[y*width/16 + x];
                double swUsage = (double)usage/(double)((searchWindow*refsMe)/256);
                //fprintf(fp_me, "%d %d %d %d %.2f\n", currView, currPoc, usage, refsMe, swUsage * 100);
                fprintf(fp_me, "%.2f\n", swUsage * 100);
#endif
            }
            if(!usage_de.empty()) {
#if SW_ANALYSIS
                int usage = usage_de[y*width/16 + x];
                double swUsage = (double)usage/(double)(searchWindow*refsDe);
                //fprintf(fp_de, "%d %d %d %d %.2f\n", currView, currPoc, usage, refsDe, swUsage * 100);
                fprintf(fp_de, "%.2f\n", swUsage * 100);
#else
                int usage = usage_de[y*width/16 + x];
                double swUsage = (double)usage/(double)((searchWindow*refsDe)/256);
                //fprintf(fp_de, "%d %d %d %d %.2f\n", currView, currPoc, usage, refsDe, swUsage * 100);
                fprintf(fp_de, "%.2f\n", swUsage * 100);
#endif
            }
        }
    }
    incBW(acc.size());
}

void MemAccessHandler::setHeight(int h) {
    height = h;
}

void MemAccessHandler::setWidth(int w) {
    width = w;
}

void MemAccessHandler::setBiPred(bool flag) {
    bipred = flag;
}

void MemAccessHandler::setCurrView(int view) {
    currView = view;
}

void MemAccessHandler::setRefView(int view) {
    refView = view;
}

void MemAccessHandler::initBW() {
    bw = 0;
}

void MemAccessHandler::incBW(int value) {
    bw += value;
}

void MemAccessHandler::setSearchRange(unsigned int range) {
	searchWindow = (range*2+16) * (range*2+16);
	searchRange = range;
	
}

void MemAccessHandler::setCurrPoc(int poc) {
    currPoc = poc;
}

void MemAccessHandler::setNumRefFrames(int num) {
    numRefFrames = num;
}

void MemAccessHandler::setMvPredictor(int h, int v) {
    mvPredictor.first = h;
    mvPredictor.second = v;
}

void MemAccessHandler::setRefPoc(int poc) {
	refPoc = poc;
}

void MemAccessHandler::insertRefFrame(UInt view, UInt poc) {
	std::pair<UInt, UInt> p(view,poc);
	setRefView(view);
	setRefPoc(poc);

	long long** sw = new long long* [swResolution];
	for (int i = 0; i < swResolution; i++) {
		sw[i] = new long long[swResolution];
		memset(sw[i], 0, sizeof(long long) * swResolution);
	}
	refs.push_back(p);
	swMap[p] = sw;
}