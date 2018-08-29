#ifndef UTIL_H
#define UTIL_H
#include <vector>
#include "types.h"
class physSys;
class rankedTilemat;
class sortedTilemats;
class location;


template<typename T>
T getDet(const threeThreeMat<T>& mat) {
  return mat[0]*(mat[4]*mat[8] - mat[7]*mat[5]) 
    - mat[1]*(mat[3]*mat[8] - mat[5]*mat[6]) 
    + mat[2]*(mat[3]*mat[7]-mat[4]*mat[6]);
}

void getBestTile(const physSys& sys, int target, 
		 threeThreeMat<int>& tilemat, double& radius, int range = 7);

void getBestTiles(const physSys& sys, int target, 
		  sortedTilemats& tms, int numMats, int range = 7);

double getScore(const threeThreeMat<double>& mat);
double WigSeitzRad(const threeThreeMat<double>& mat);
double SimCellRad(const threeThreeMat<double>& mat);

void getBestTileByScore(const physSys& sys, sortedTilemats& tms, threeThreeMat<int>& tilemat);
void getBestTileBySymAndScore(const physSys& sys, sortedTilemats& tms, threeThreeMat<int>& tilemat, double tolerance);

double dot(const location& a, const location& b);
void cross(const location& a, const location& b, location& c);
template<typename T>
void matvec3(const threeThreeMat<T>& mat, const location& vec, location& out) {
  for (int i = 0; i < 3; i++) {      
    out[i] = 0.0;		     
    for (int j = 0; j < 3; j++) {  
      out[i] += mat[i*3+j]*vec[j]; 
    }				     
  }				     
}                                  

void getPrimKpts(const physSys& prim, const physSys& ss, const location& ssKpt, std::vector<location>& primKpts, int numCopies);

#endif
