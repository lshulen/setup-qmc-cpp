#ifndef HANDLE_TILEMATS_H
#define HANDLE_TILEMATS_H
#include "types.h"

class pwscfData;

threeThreeMat<int> getTilemat(const pwscfData& pd, int size); 
void getBestTilemat(int argc, char* argv[]);
void getBestSupercells(int argc, char* argv[]);

#endif
