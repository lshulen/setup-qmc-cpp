#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstring>
#include "types.h"
#include "util.h"
#include "pwscfData.h"
#include "handleTilemats.h"
using namespace std;

threeThreeMat<int> getTilemat(const pwscfData& pd, int size) {

  threeThreeMat<int> tilemat;
  // first check if we can just look this up from the SYMMETRY file
  int found = 0;

  ifstream file;
  file.open("SYMMETRY");
  string line;
  stringstream ss;
  ss << size << ":";
  while (getline(file,line)) {
    stringstream ss3(line);
    string firstToken;
    ss3 >> firstToken;
    if (ss.str() == firstToken) {
      found = 1;
      stringstream ss2(line);
      string buf2;
      ss2 >> buf2;
      ss2 >> buf2;
      for (int i = 0; i < 9; i++) {
	ss2 >> buf2;
	tilemat[i] = atoi(buf2.c_str());
      }
    }
  }
  file.close();

  // if not in symmetry file, use tools to look for it (expensive...)
  if (found == 0) {
    physSys primcell(pd.ptv, pd.ionNames, pd.atpos);
    sortedTilemats stms(10000);
    getBestTiles(primcell, size, stms, 10000);
    getBestTileBySymAndScore(primcell, stms, tilemat, 20);

    physSys smartSuper(primcell, tilemat);
    FILE * ofile = fopen("SYMMETRY", "a");
    fprintf(ofile, "%4d:  [  %2d  %2d  %2d  %2d  %2d  %2d  %2d  %2d  %2d  ]  %8.4f  %4d\n",
	    size, tilemat[0], tilemat[1], tilemat[2], tilemat[3], tilemat[4],
	    tilemat[5], tilemat[6], tilemat[7], tilemat[8], 
	    SimCellRad(smartSuper.getPrim()), smartSuper.getSymmetry()/size);
    fclose(ofile);

  }
  return tilemat;
}

void getBestTilemat(int argc, char* argv[]) {
  string pwscfFileName;
  int ssize;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i],"--infile")) {
      pwscfFileName = string(argv[i+1]);
    } else if (!strcmp(argv[i],"--supercellsize")) {
      ssize = atoi(argv[i+1]);
    }
  }

  // use this to get information on the geometry of
  // the primitive cell
  pwscfString ps(pwscfFileName);
  pwscfData pd(ps);

  threeThreeMat<int> tilemat = getTilemat(pd, ssize);
  cout << "Including symmetry, best tile matrix is:" << endl;
  cout << "[";
  for (int i = 0; i < 9; i++) {
    cout << " " << tilemat[i];
  }
  cout << " ]" << endl;
  physSys primcell(pd.ptv, pd.ionNames, pd.atpos);
  physSys smartSuper(primcell, tilemat);
  cout << "This leads to a supercell with radius: " << SimCellRad(smartSuper.getPrim()) << endl;
  cout << "It has " << smartSuper.getSymmetry() / ssize << " symmetry operations." << endl;
}

void getBestSupercells(int argc, char* argv[]) {
  string pwscfFileName;
  int minsize;
  int maxsize;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i],"--infile")) {
      pwscfFileName= string(argv[i+1]);
    } else if (!strcmp(argv[i],"--mincellsize")) {
      minsize = atoi(argv[i+1]);
    } else if (!strcmp(argv[i],"--maxcellsize")) {
      maxsize = atoi(argv[i+1]);
    }
  }

  // use this to get information on the geometry of
  // the primitive cell
  pwscfString ps(pwscfFileName);
  pwscfData pd(ps);
  physSys primcell(pd.ptv, pd.ionNames, pd.atpos);  

  for (int size = minsize; size <= maxsize; size++) {
    threeThreeMat<int> tilemat = getTilemat(pd,size);
    physSys smartSuper(primcell, tilemat);
  
    printf("%4d:  [  %2d  %2d  %2d  %2d  %2d  %2d  %2d  %2d  %2d  ]  %8.4f  %4d\n",
	   size, tilemat[0], tilemat[1], tilemat[2], tilemat[3], tilemat[4],
	   tilemat[5], tilemat[6], tilemat[7], tilemat[8], 
	   SimCellRad(smartSuper.getPrim()), smartSuper.getSymmetry()/size);
  }
}
