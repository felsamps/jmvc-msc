
#include <stdlib.h>
#include <string>
#include <map>
#include <list>

#include "MemAccessHandler.h"
#include "TestDefinitions.h"

FILE* MemAccessHandler::fp_me;
FILE* MemAccessHandler::fp_de;
FILE* MemAccessHandler::fpMeSearchMap;
FILE* MemAccessHandler::fpDeSearchMap;
FILE* MemAccessHandler::fpPerPrediction;
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
bool MemAccessHandler::firstSearchTZ;
std::map<std::pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >, long long> MemAccessHandler::accPerPrediction;
std::set<std::pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> > > MemAccessHandler::refsPrediction;
std::map<std::pair<UInt, UInt>, char> MemAccessHandler::translateFrameToChar;

MemAccessHandler::MemAccessHandler() {
}

MemAccessHandler::~MemAccessHandler() {
}

void MemAccessHandler::openFile(unsigned int view, unsigned int range) {
    if(view == 0) {
        fp_me = fopen("sw_usage_me.mat", "w");
        fp_de = fopen("sw_usage_de.mat", "w");
		fpPerPrediction = fopen("acc_per_prediction", "w");
#if SW_SEARCH_MAP
        fpMeSearchMap = fopen("search_map_me.mat", "w");
		fpDeSearchMap = fopen("search_map_de.mat", "w");
#endif
    }
    else {
        fp_me = fopen("sw_usage_me.mat", "a");
		fp_de = fopen("sw_usage_de.mat", "a");
		fpPerPrediction = fopen("acc_per_prediction", "a");
#if SW_SEARCH_MAP
        fpMeSearchMap = fopen("search_map_me.mat", "a");
		fpDeSearchMap = fopen("search_map_de.mat", "a");
#endif
    }
	swResolution = (range/16) * 2 + 1;
	firstSearchTZ = false;
	swMap.clear();
	refs.clear();
	
	initRefsPrediction();

}


void MemAccessHandler::reportSearchOccurrences() {
	
	while(!refs.empty()) {
		std::pair<UInt,UInt> p = refs.front();
		FILE* targetFile = (p.first == currView) ? 
			fpMeSearchMap : /*Motion Estimation*/
			fpDeSearchMap ;


		for (int y = 0; y < swResolution; y++) {
			for (int x = 0; x < swResolution; x++) {
				fprintf(targetFile, "%lld ", swMap[p][x][y]);
			}
			fprintf(targetFile, "\n");
		}
		refs.pop_front();
	}
	swMap.clear();
	refs.clear();
}

void MemAccessHandler::closeAndReport() {
    fclose(fp_me);
    fclose(fp_de);
	fclose(fpPerPrediction);
#if SW_SEARCH_MAP
	fclose(fpMeSearchMap);
	fclose(fpDeSearchMap);
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
    incBW(size*size);
    
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

void MemAccessHandler::handleAccPerPrediction() {
	std::pair<UInt,UInt> curr(currView, currPoc);
	std::pair<UInt,UInt> ref(refView, refPoc);
	std::pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> > p(curr,ref);
	
	if(accPerPrediction.find(p) != accPerPrediction.end()) {
		accPerPrediction[p] += acc.size();
	}
	else {
		accPerPrediction[p] = acc.size();
	}
}

void MemAccessHandler::insertUsage() {
    if( !bipred ) {
		
		handleAccPerPrediction();

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

void MemAccessHandler::reportPerPrediction() {
	for (std::map<std::pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >, long long>::iterator it = accPerPrediction.begin(); it != accPerPrediction.end(); it++) {
		std::pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> > p = (*it).first;			
		std::pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> > pTest(p);
		
		if(pTest.second.second %8 == 0) {
			if(p.first.second < p.second.second) { //anchor from the right side
				pTest.second.second -= ((pTest.second.second-1)/8)*8;
			}
			else {
				pTest.second.second -= ((pTest.second.second/8))*8;
			}
		}
		else {
			pTest.second.second -= (pTest.first.second/8)*8;
		}
		
		
		pTest.first.second -= (pTest.first.second!=0 and pTest.first.second%8==0) ? ((pTest.first.second-1)/8)*8 : (pTest.first.second/8)*8;
		//pTest.second.second -= (pTest.second.second!=0 and pTest.second.second%8==0) ? ((pTest.second.second-1)/8)*8 : (pTest.second.second/8)*8;
		long long l = (*it).second;
		if(refsPrediction.find(pTest) != refsPrediction.end()) {
//			std::cout << p.first.first << " " << p.first.second << " " << p.second.first << " " << p.second.second << " " << l << std::endl;
			fprintf(fpPerPrediction, "(%d %d, %d %d) ", p.first.first, p.first.second, p.second.first, p.second.second);
			fprintf(fpPerPrediction, "(%d %d, %d %d) ", pTest.first.first, pTest.first.second, pTest.second.first, pTest.second.second);
			fprintf(fpPerPrediction, "%c %c %d\n", translateFrameToChar[pTest.first], translateFrameToChar[pTest.second], l);
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
    //incBW(acc.size());
}

void MemAccessHandler::reportTotalBW() {
    printf("     ME memory bandwidth: %lld bytes\n", bw);
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

void MemAccessHandler::setFirstSearchTZ(bool b) {
	firstSearchTZ = b;
}

bool MemAccessHandler::isFirstSearchTZ() {
	return firstSearchTZ;

}

void MemAccessHandler::initRefsPrediction() {
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(0,4), std::make_pair<UInt,UInt>(0,0)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(0,4), std::make_pair<UInt,UInt>(0,8)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(0,2), std::make_pair<UInt,UInt>(0,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(0,2), std::make_pair<UInt,UInt>(0,0)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(0,1), std::make_pair<UInt,UInt>(0,0)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(0,1), std::make_pair<UInt,UInt>(0,2)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(0,3), std::make_pair<UInt,UInt>(0,2)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(0,3), std::make_pair<UInt,UInt>(0,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(0,6), std::make_pair<UInt,UInt>(0,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(0,6), std::make_pair<UInt,UInt>(0,8)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(0,5), std::make_pair<UInt,UInt>(0,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(0,5), std::make_pair<UInt,UInt>(0,6)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(0,7), std::make_pair<UInt,UInt>(0,6)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(0,7), std::make_pair<UInt,UInt>(0,8)));
	
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(2,2), std::make_pair<UInt,UInt>(2,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(2,2), std::make_pair<UInt,UInt>(2,0)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(2,1), std::make_pair<UInt,UInt>(2,0)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(2,1), std::make_pair<UInt,UInt>(2,2)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(2,3), std::make_pair<UInt,UInt>(2,2)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(2,3), std::make_pair<UInt,UInt>(2,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(2,6), std::make_pair<UInt,UInt>(2,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(2,6), std::make_pair<UInt,UInt>(2,8)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(2,5), std::make_pair<UInt,UInt>(2,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(2,5), std::make_pair<UInt,UInt>(2,6)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(2,7), std::make_pair<UInt,UInt>(2,6)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(2,7), std::make_pair<UInt,UInt>(2,8)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(2,0), std::make_pair<UInt,UInt>(0,0)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(2,8), std::make_pair<UInt,UInt>(0,0))); //work around
	
	
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,2), std::make_pair<UInt,UInt>(1,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,2), std::make_pair<UInt,UInt>(1,0)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,1), std::make_pair<UInt,UInt>(1,0)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,1), std::make_pair<UInt,UInt>(1,2)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,3), std::make_pair<UInt,UInt>(1,2)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,3), std::make_pair<UInt,UInt>(1,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,6), std::make_pair<UInt,UInt>(1,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,6), std::make_pair<UInt,UInt>(1,8)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,5), std::make_pair<UInt,UInt>(1,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,5), std::make_pair<UInt,UInt>(1,6)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,7), std::make_pair<UInt,UInt>(1,6)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,7), std::make_pair<UInt,UInt>(1,8)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,0), std::make_pair<UInt,UInt>(2,0)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,1), std::make_pair<UInt,UInt>(2,1)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,2), std::make_pair<UInt,UInt>(2,2)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,3), std::make_pair<UInt,UInt>(2,3)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,4), std::make_pair<UInt,UInt>(2,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,5), std::make_pair<UInt,UInt>(2,5)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,6), std::make_pair<UInt,UInt>(2,6)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,7), std::make_pair<UInt,UInt>(2,7)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,8), std::make_pair<UInt,UInt>(2,0)));  //work around
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,0), std::make_pair<UInt,UInt>(0,0)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,1), std::make_pair<UInt,UInt>(0,1)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,2), std::make_pair<UInt,UInt>(0,2)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,3), std::make_pair<UInt,UInt>(0,3)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,4), std::make_pair<UInt,UInt>(0,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,5), std::make_pair<UInt,UInt>(0,5)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,6), std::make_pair<UInt,UInt>(0,6)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,7), std::make_pair<UInt,UInt>(0,7)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(1,8), std::make_pair<UInt,UInt>(0,0))); //work around
	
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,2), std::make_pair<UInt,UInt>(4,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,2), std::make_pair<UInt,UInt>(4,0)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,1), std::make_pair<UInt,UInt>(4,0)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,1), std::make_pair<UInt,UInt>(4,2)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,3), std::make_pair<UInt,UInt>(4,2)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,3), std::make_pair<UInt,UInt>(4,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,6), std::make_pair<UInt,UInt>(4,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,6), std::make_pair<UInt,UInt>(4,8)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,5), std::make_pair<UInt,UInt>(4,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,5), std::make_pair<UInt,UInt>(4,6)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,7), std::make_pair<UInt,UInt>(4,6)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,7), std::make_pair<UInt,UInt>(4,8)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,0), std::make_pair<UInt,UInt>(2,0)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,1), std::make_pair<UInt,UInt>(2,1)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,2), std::make_pair<UInt,UInt>(2,2)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,3), std::make_pair<UInt,UInt>(2,3)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,4), std::make_pair<UInt,UInt>(2,4)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,5), std::make_pair<UInt,UInt>(2,5)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,6), std::make_pair<UInt,UInt>(2,6)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,7), std::make_pair<UInt,UInt>(2,7)));
	refsPrediction.insert(std::make_pair<std::pair<UInt,UInt>,std::pair<UInt,UInt> >(std::make_pair<UInt,UInt>(4,8), std::make_pair<UInt,UInt>(2,0))); //work around
	
	translateFrameToChar[std::make_pair<UInt,UInt>(0,0)] = 'I';
	translateFrameToChar[std::make_pair<UInt,UInt>(0,1)] = 'C';
	translateFrameToChar[std::make_pair<UInt,UInt>(0,2)] = 'A';
	translateFrameToChar[std::make_pair<UInt,UInt>(0,3)] = 'D';
	translateFrameToChar[std::make_pair<UInt,UInt>(0,4)] = '1';
	translateFrameToChar[std::make_pair<UInt,UInt>(0,5)] = 'E';
	translateFrameToChar[std::make_pair<UInt,UInt>(0,6)] = 'B';
	translateFrameToChar[std::make_pair<UInt,UInt>(0,7)] = 'F';
	translateFrameToChar[std::make_pair<UInt,UInt>(0,8)] = 'I';
	
	translateFrameToChar[std::make_pair<UInt,UInt>(1,0)] = '4';
	translateFrameToChar[std::make_pair<UInt,UInt>(1,1)] = 'O';
	translateFrameToChar[std::make_pair<UInt,UInt>(1,2)] = 'M';
	translateFrameToChar[std::make_pair<UInt,UInt>(1,3)] = 'P';
	translateFrameToChar[std::make_pair<UInt,UInt>(1,4)] = '5';
	translateFrameToChar[std::make_pair<UInt,UInt>(1,5)] = 'Q';
	translateFrameToChar[std::make_pair<UInt,UInt>(1,6)] = 'N';
	translateFrameToChar[std::make_pair<UInt,UInt>(1,7)] = 'R';
	translateFrameToChar[std::make_pair<UInt,UInt>(1,8)] = '4';
	
	translateFrameToChar[std::make_pair<UInt,UInt>(2,0)] = '2';
	translateFrameToChar[std::make_pair<UInt,UInt>(2,1)] = 'I';
	translateFrameToChar[std::make_pair<UInt,UInt>(2,2)] = 'G';
	translateFrameToChar[std::make_pair<UInt,UInt>(2,3)] = 'J';
	translateFrameToChar[std::make_pair<UInt,UInt>(2,4)] = '3';
	translateFrameToChar[std::make_pair<UInt,UInt>(2,5)] = 'K';
	translateFrameToChar[std::make_pair<UInt,UInt>(2,6)] = 'H';
	translateFrameToChar[std::make_pair<UInt,UInt>(2,7)] = 'L';
	translateFrameToChar[std::make_pair<UInt,UInt>(2,8)] = '2';
	
	translateFrameToChar[std::make_pair<UInt,UInt>(4,0)] = '6';
	translateFrameToChar[std::make_pair<UInt,UInt>(4,1)] = 'U';
	translateFrameToChar[std::make_pair<UInt,UInt>(4,2)] = 'S';
	translateFrameToChar[std::make_pair<UInt,UInt>(4,3)] = 'V';
	translateFrameToChar[std::make_pair<UInt,UInt>(4,4)] = '7';
	translateFrameToChar[std::make_pair<UInt,UInt>(4,5)] = 'W';
	translateFrameToChar[std::make_pair<UInt,UInt>(4,6)] = 'T';
	translateFrameToChar[std::make_pair<UInt,UInt>(4,7)] = 'X';
	translateFrameToChar[std::make_pair<UInt,UInt>(4,8)] = '6';
}
