
#include <stdlib.h>

#include "MemAccessHandler.h"

std::set<int> MemAccessHandler::acc;
std::set<int> MemAccessHandler::block_acc;
int MemAccessHandler::counter;
FILE* MemAccessHandler::fp;
int MemAccessHandler::currMbX;
int MemAccessHandler::currMbY;
bool MemAccessHandler::bipred;
int MemAccessHandler::refView;
int MemAccessHandler::currView;
int MemAccessHandler::height;
int MemAccessHandler::width;
long long int MemAccessHandler::bw;

MemAccessHandler::MemAccessHandler() {
}

MemAccessHandler::~MemAccessHandler() {
}

void MemAccessHandler::init() {
    counter = 0;
    acc.clear();
    block_acc.clear();
}

void MemAccessHandler::setMb(int x, int y) {
    currMbX = x*16;
    currMbY = y*16;
}

void MemAccessHandler::insert(int x, int y) {
    int calc = y * + x;
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

void MemAccessHandler::report() {
    if( !bipred ) {
        //fprintf(fp, "%d %d %d %d %c\n", currMbY/16, currMbX/16, acc.size(), counter,
        //        (currView == refView) ? '0' : '1');

        //fprintf(fp, "%d %d\n", block_acc.size(), (currView == refView) ? '0' : '1');
        incBW(acc.size());
    }
}

void MemAccessHandler::setHeight(int height) {
    height = height;
}


void MemAccessHandler::setWidth(int width) {
    width = width;
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

