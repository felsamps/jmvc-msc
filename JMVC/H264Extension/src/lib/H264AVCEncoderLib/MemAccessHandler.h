#ifndef _MEMACCESSHANDLER_H
#define	_MEMACCESSHANDLER_H

#include<set>
#include<cstdio>
#include<string>
#include<cstdlib>
#include<map>
#include<utility>
#include<cmath>

class MemAccessHandler {
private:
    static FILE* fp_me;
    static FILE* fp_de;
    static FILE* fpSearchMap;
    static std::set<std::pair<int,int> > acc;
    static std::set<int> block_acc;
    static int counter;
    static bool bipred;
    static int width, height;
    static int refView, currView, currPoc, refPoc;
    static int currMbX, currMbY;
    static unsigned int searchWindow, searchRange;
    static int numRefFrames, refsMe, refsDe;
    static long long int bw;
    static std::map<int, int> usage_me, usage_de;
    static std::pair<int,int> mvPredictor;
    
public:
    
    
    MemAccessHandler();
    virtual ~MemAccessHandler();
    
    static void openFile(unsigned int view);
    static void closeFile();
    static void init();
    static void initBW();
    static void initCurrMB();
    static void incBW(int value);
    static void setMb(int x, int y);
    static void setBiPred(bool flag);
    static void setCurrView(int view);
    static void setRefView(int view);
    static void setRefPoc(int poc);
    static void setCurrPoc(int poc);
    static void insert(std::pair<int, int>);
    static void insertBlock(int x, int y, int size);

    static void insertUsage();
    static void report();
    static void reportSearchMap();
    
    static void setHeight(int h);
    static void setWidth(int w);
    static void setSearchRange(unsigned int range);
    static void setNumRefFrames(int num);
    static void setMvPredictor(int h, int v);
};

#endif