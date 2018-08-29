#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <map>
#include "types.h"
#include "util.h"
extern "C" {
#include "spglib.h"
}
using namespace std;

void physSys::showCell() {
  cout << "The input primitive translation vectors are: \n";
  cout << prim[0] << "   " << prim[1] << "   " << prim[2] << endl;
  cout << prim[3] << "   " << prim[4] << "   " << prim[5] << endl;
  cout << prim[6] << "   " << prim[7] << "   " << prim[8] << endl;
  cout << endl;

  cout << "There are " << attypes.size() << " atoms in the cell\n";
  cout << "The atoms are located at: \n";
  cout << "label,  type,  posx,  posy,  posz" << endl;
  for (int i = 0; i < atlabels.size(); i++) {
    printf("%4s,  %2d,  %10.7f,  %10.7f,  %10.7f\n", atlabels[i].c_str(), 
	   attypes[i], atpos[i][0], atpos[i][1], atpos[i][2]);
  }
}

void physSys::setTypes() {
  vector<string> uniqnames;
  for (int i = 0; i < atlabels.size(); i++) {
    std::vector<string>::iterator it;
    it = find(uniqnames.begin(), uniqnames.end(), atlabels[i]);
    int index;
    if (it == uniqnames.end()) {
      uniqnames.push_back(atlabels[i]);
      it = find(uniqnames.begin(), uniqnames.end(), atlabels[i]);
    }
    index = distance(uniqnames.begin(), it);
    attypes.push_back(index);
  }
}

void physSys::calcSupercell(const threeThreeMat<int>& tilemat, threeThreeMat<double>& ss) const {
  ss[0] = tilemat[0]*prim[0]+tilemat[1]*prim[3]+tilemat[2]*prim[6];
  ss[1] = tilemat[0]*prim[1]+tilemat[1]*prim[4]+tilemat[2]*prim[7];
  ss[2] = tilemat[0]*prim[2]+tilemat[1]*prim[5]+tilemat[2]*prim[8];
  ss[3] = tilemat[3]*prim[0]+tilemat[4]*prim[3]+tilemat[5]*prim[6];
  ss[4] = tilemat[3]*prim[1]+tilemat[4]*prim[4]+tilemat[5]*prim[7];
  ss[5] = tilemat[3]*prim[2]+tilemat[4]*prim[5]+tilemat[5]*prim[8];
  ss[6] = tilemat[6]*prim[0]+tilemat[7]*prim[3]+tilemat[8]*prim[6];
  ss[7] = tilemat[6]*prim[1]+tilemat[7]*prim[4]+tilemat[8]*prim[7];
  ss[8] = tilemat[6]*prim[2]+tilemat[7]*prim[5]+tilemat[8]*prim[8];
}

void physSys::genRlv(const threeThreeMat<double>& p, threeThreeMat<double>& outrlv) const {
  double invdet = 1.0/::getDet(p);
  
  outrlv[0] = invdet * (p[4]*p[8] - p[5]*p[7]);
  outrlv[1] = invdet * (p[5]*p[6] - p[3]*p[8]);
  outrlv[2] = invdet * (p[3]*p[7] - p[4]*p[6]);
  outrlv[3] = invdet * (p[2]*p[7] - p[1]*p[8]);
  outrlv[4] = invdet * (p[0]*p[8] - p[2]*p[6]);
  outrlv[5] = invdet * (p[1]*p[6] - p[0]*p[7]);
  outrlv[6] = invdet * (p[1]*p[5] - p[2]*p[4]);
  outrlv[7] = invdet * (p[2]*p[3] - p[0]*p[5]);
  outrlv[8] = invdet * (p[0]*p[4] - p[1]*p[3]);
} 

void physSys::setRlv() {
  genRlv(prim, rlv);
}

double physSys::getDet() const {
  return ::getDet(prim);
}

// constructor to make a supercell from a primitive cell and a tilematrix
physSys::physSys(const physSys& primsys, const threeThreeMat<int>& tilemat) {
  primsys.calcSupercell(tilemat, prim);
  setRlv();

  int numCopies = ::getDet(tilemat);
  //  cout << "Looking for " << numCopies << " copies of each atom" << endl;

  // do work here (basically search atom positions 
  // plus multiples of the ptvs and see if they 
  // end up in the supercell until we have N copies of every atom)
  int multiples[3];

  for (int i = 0; i < primsys.getNumAts(); i++) {
    int added = 0;
    int intmults[3];
    location curposredprim;
    location curposrs;
    location curposredss;

    int size = 14;
    // note, rather than starting at -size, -size, -size and working
    // until we get to size,size,size it would be more efficient to use some
    // sort of space filling curve type construct starting at the 
    // origin and spiraling outward.  
    for(intmults[0] = -size; intmults[0] <= size; intmults[0] += 1) {
      curposredprim[0] = primsys.getAtPos(i)[0] + intmults[0];
      for(intmults[1] = -size; intmults[1] <= size; intmults[1] += 1) {
	curposredprim[1] = primsys.getAtPos(i)[1] + intmults[1];
	for(intmults[2] = -size; intmults[2] <= size; intmults[2] += 1) {
	  curposredprim[2] = primsys.getAtPos(i)[2] + intmults[2];
	  
	  // get the real space coordinates of the trial location
	  for (int j = 0; j < 3; j++) { 
	    curposrs[j] = 0.0;
	    for (int k = 0; k < 3; k++) {
	      curposrs[j] += primsys.getPrim()[k*3+j] * curposredprim[k];
	    }
	  } 
	  
	  // Now figure out its supercell reduced coordinates.
	  // If these are all in the range [0,1) than it is in the
	  // new supercell and should be added.
	  for (int j = 0; j < 3; j++) {
	    curposredss[j] = 0.0;
	    for (int k = 0; k < 3; k++) {
	      curposredss[j] += rlv[j*3+k] * curposrs[k];
	    }
	  }
	  
	  static const double eps = 1e-8;
	  int incell = 1;
	  for (int j = 0; j < 3; j++) {
	    if (curposredss[j] < 0-eps) {
	      incell = 0;
	    } else if (curposredss[j] > 1-eps) {
	      incell = 0;
	    }
	  }
	  
	  if (incell) {
	    added++;
	    atlabels.push_back(primsys.getAtLabel(i));
	    atpos.push_back(curposredss);
	    attypes.push_back(primsys.getAtType(i));
	  }
	}
      }
    }
  }
}

int physSys::getSymmetry() const {
  double lattice[3][3] = {{prim[0], prim[1], prim[2]}, {prim[3], prim[4], prim[5]}, {prim[6], prim[7], prim[8]}};
  const int num_atom = atlabels.size();
  static const int max_size = 5000;
  int size;
  int rotation[max_size][3][3];
  double translation[max_size][3];

  double position[num_atom][3];
  int types[num_atom];
  for (int i = 0; i < num_atom; i++) {
    types[i] = attypes[i];
    for (int j = 0; j < 3; j++) {
      position[i][j] = atpos[i][j];
    }
  }

  size = spg_get_symmetry(rotation, translation, max_size, lattice, position, types, num_atom, 1e-5);
  //size = spg_get_multiplicity(lattice, position, types, num_atom, 1e-5);
  return size;
}

int physSys::getMeshSize(int inputsize) const {
  double lattice[3][3] = {{prim[0], prim[1], prim[2]}, {prim[3], prim[4], prim[5]}, {prim[6], prim[7], prim[8]}};
  const int num_atom = atlabels.size();
  double position[num_atom][3];
  int types[num_atom];
  for (int i = 0; i < num_atom; i++) {
    types[i] = attypes[i];
    for (int j = 0; j < 3; j++) {
      position[i][j] = atpos[i][j];
    }
  }

  int mesh[] = {inputsize, inputsize, inputsize};
  int is_shift[] = {0,0,0};
  int grid_address[inputsize*inputsize*inputsize][3];
  int grid_mapping_table[inputsize*inputsize*inputsize];

  int num_ir = spg_get_ir_reciprocal_mesh(grid_address, grid_mapping_table, mesh,
					  is_shift, 1, lattice, position, types,
					  num_atom, 1e-5);
  return num_ir;
}


int physSys::getMesh(int nx, int ny, int nz, int shiftx, int shifty, int shiftz, std::vector<weightedKpt>& outks) const {
  double lattice[3][3] = {{prim[0], prim[1], prim[2]}, {prim[3], prim[4], prim[5]}, {prim[6], prim[7], prim[8]}};
  const int num_atom = atlabels.size();
  double position[num_atom][3];
  int types[num_atom];
  for (int i = 0; i < num_atom; i++) {
    types[i] = attypes[i];
    for (int j = 0; j < 3; j++) {
      position[i][j] = atpos[i][j];
    }
  }

  const int nks = nx*ny*nz;
  int mesh[] = {nx, ny, nz};
  int is_shift[] = {shiftx,shifty,shiftz};
  int grid_address[nks][3];
  int grid_mapping_table[nks];

  int num_ir = spg_get_ir_reciprocal_mesh(grid_address, grid_mapping_table, mesh,
					  is_shift, 1, lattice, position, types,
					  num_atom, 1e-5);

  // make a map that has index --> number of copies of index
  map<int,int> dedupe;
  for (int i = 0; i < nks; i++) {
    int index = grid_mapping_table[i];
    if (dedupe.find(index) == dedupe.end()) {
      dedupe[index] = 0;
    }
    dedupe[index]++;
  }

  for (std::map<int,int>::iterator it = dedupe.begin(); it != dedupe.end(); ++it) {
    int index = it->first;
    int multiplicity = it->second;
    double weight = multiplicity / static_cast<double>(nks);
    weightedKpt kp;
    kp.first = weight;
    for (int i = 0; i < 3; i++) {
      kp.second[i] = (grid_address[index][i] + 0.5 * is_shift[i]) / static_cast<double>(mesh[i]);
    }
    outks.push_back(kp);
  }
  return num_ir;

  /*
  for (int i = 0; i < nks; i++) {
    weightedKpt kp;
    //    kp.first = 1.0;
    kp.first = grid_mapping_table[i];
    for (int j = 0; j < 3; j++) {
      kp.second[j] = grid_address[i][j] / static_cast<double>(mesh[j]);
    }
    outks.push_back(kp);
  }
  return num_ir;
  */
}
