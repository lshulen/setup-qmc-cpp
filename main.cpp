#include <iostream>
#include <cstring>
#include <sstream>
#include "pwscfData.h"
#include "handlePwscf.h"
#include "handleTilemats.h"
using namespace std;

int main(int argc, char* argv[]) {
  // Parse command line to find out the action being taken
  // Note that only one action should be taken at a time and if multiple
  // are given, only the first will be acted upon
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i],"--gettilemat")) {
      //      cout << "Get the best tiling matrix for a given supercell size" << endl;
      getBestTilemat(argc, argv);
      break;
    } else if (!strcmp(argv[i],"--getbestsupercell")) {
      //cout << "Ask for a range of supercell sizes to find the best simulation" << endl;
      //cout << "cell radii and potential for symmetry.  To be used to find best" << endl;
      //cout << "supercells to use for future calculations." << endl;
      getBestSupercells(argc,argv);
      break;
    } else if (!strcmp(argv[i],"--testPwscfParser")) {
      //cout << "Give this the names of pwscf input and output files and it will" << endl;
      //cout << "dump what the parser tells it about the system." << endl;
      testPwscfParser(argc,argv);
      break;
    } else if (!strcmp(argv[i],"--genwfn")) {
      //cout << "Create necessary pwscf (and pw2qmcpack) input files to make a" << endl;
      //cout << "hdf wavefunction file for qmcpack calculations." << endl;
      genWfn(argc,argv);
      break;
    } else if (!strcmp(argv[i],"--genfsdft")) {
      cout << "Generate necessary pwscf input files to estimate the finite size" << endl;
      cout << "error using the KZK method." << endl;
      break;
    } else if (!strcmp(argv[i],"--splconv")) {
      cout << "Generate necessary qmcpack input files to test for convergence of the" << endl;
      cout << "energy and variance with respect to the spline spacing within VMC." << endl;
      break;
    } else if (!strcmp(argv[i],"--optwvfcn")) {
      cout << "Generate necessary qmcpack input files to optimize wavefunction parameters." << endl;
      break;
    } else if (!strcmp(argv[i],"--convdmctstep")) {
      cout << "Generate necessary qmcpack input files to test the convergence of DMC" << endl;
      cout << "calculations with respect to the time step." << endl;
      break;
    } else if (!strcmp(argv[i],"--dmc")) {
      cout << "Generate necessary qmcpack input files for production DMC calculations." << endl;
      break;
    }
  }
}
      
