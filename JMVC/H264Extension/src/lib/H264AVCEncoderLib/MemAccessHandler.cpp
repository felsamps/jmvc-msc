
#include <stdlib.h>
#include <string>
#include <map>

#include "MemAccessHandler.h"

FILE* MemAccessHandler::fp_me;
FILE* MemAccessHandler::fp_de;
std::set<int> MemAccessHandler::acc;
std::set<int> MemAccessHandler::block_acc;
int MemAccessHandler::counter;
int MemAccessHandler::currMbX;
int MemAccessHandler::currMbY;
int MemAccessHandler::currPoc;
bool MemAccessHandler::bipred;
int MemAccessHandler::refView;
int MemAccessHandler::currView;
int MemAccessHandler::height;
int MemAccessHandler::width;
unsigned int MemAccessHandler::searchRange;
long long int MemAccessHandler::bw;
std::map<int,int> MemAccessHandler::usage_me;
std::map<int,int> MemAccessHandler::usage_de;
int MemAccessHandler::numRefFrames;
int MemAccessHandler::refsMe;
int MemAccessHandler::refsDe;

MemAccessHandler::MemAccessHandler() {
}

MemAccessHandler::~MemAccessHandler() {
}

void MemAccessHandler::openFile(unsigned int view) {
    if(view == 0) {
        fp_me = fopen("sw_usage_me.mat", "w");
        fp_de = fopen("sw_usage_de.mat", "w");
    }
    else {
        fp_me = fopen("sw_usage_me.mat", "a");
        fp_de = fopen("sw_usage_de.mat", "a");
    }
}

void MemAccessHandler::closeFile() {
    fclose(fp_me);
    fclose(fp_de);
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

void MemAccessHandler::insert(int x, int y) {
    int calc = y * width + x;
    acc.insert(calc);
}

void MemAccessHandler::insertBlock(int x, int y, int size) {
    //INSERT SAMPLE ACCESS!

    int posx = currMbX + x;
    int posy = currMbY + y;
    counter ++;
    
    for (int i = posx; i < posx+size; i++) {
        for (int j = posy; j < posy+size; j++) {
            insert(i, j);
        }
    }
    /*
    for (int i = x; i < x+size; i++) {
        for (int j = y; j < y+size; j++) {
            
            int block_x, block_y;
            //INSERT BLOCK ACCESS!
            if(i >= 0) {
                block_x = i/16;
            }
            else {
                block_x = -((-i)/16) - 1;
            }

            if(j >= 0) {
                block_y = j/16;
            }
            else {
                block_y = -((-j)/16) - 1;
            }

            block_acc.insert(block_y * H + block_x);

        }
    }*/

}

void MemAccessHandler::insertUsage() {
    if( !bipred ) {
           
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
                int usage = usage_me[y*width/16 + x];
                double swUsage = (double)usage/(double)(searchRange*refsMe);
                //fprintf(fp_me, "%d %d %d %d %.2f\n", currView, currPoc, usage, refsMe, swUsage * 100);
                fprintf(fp_me, "%.2f\n", swUsage * 100);
            }
            if(!usage_de.empty()) {
                int usage = usage_de[y*width/16 + x];
                double swUsage = (double)usage/(double)(searchRange*refsDe);
                //fprintf(fp_de, "%d %d %d %d %.2f\n", currView, currPoc, usage, refsDe, swUsage * 100);
                fprintf(fp_de, "%.2f\n", swUsage * 100);
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
    searchRange = (range*2+16) * (range*2+16);
}

void MemAccessHandler::setCurrPoc(int poc) {
    currPoc = poc;
}

void MemAccessHandler::setNumRefFrames(int num) {
    numRefFrames = num;
}