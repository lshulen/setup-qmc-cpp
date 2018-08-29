#ifndef HANDLE_PWSCF_H
#define HANDLE_PWSCF_H
#include "handleTilemats.h"
#include "types.h"
#include "util.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <fstream>
#include <cmath>
#include <iomanip>
using namespace std;

void genWfn(int argc, char* argv[]) {

  string inFileName = "undef";
  int ssize;
  int sskgrid[3];
  int sskshift[3] = {0, 0, 0};

  // read necessary info from the command line
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i],"--infile")) {
      inFileName= string(argv[i+1]);
    } else if (!strcmp(argv[i],"--supercellsize")) {
      ssize = atoi(argv[i+1]);
    } else if (!strcmp(argv[i],"--sskgrid")) {
      for (int j = 0; j < 3; j++) {
	stringstream ss1(argv[i+1+j]);
	ss1 >> sskgrid[j];
      }
      i+=3;
    } else if (!strcmp(argv[i],"--sskshift")) {
      for (int j = 0; j < 3; j++) {
	stringstream ss1(argv[i+1+j]);
	ss1 >> sskshift[j];
      }
      i+=3;
    }
  }
  
  if (inFileName == "undef") {
    cout << "No pwscf input filename was given" << endl;
  }

  pwscfString ps(inFileName);
  pwscfData pd(ps);

  threeThreeMat<int> tilemat = getTilemat(pd, ssize);  
  physSys primcell(pd.ptv, pd.ionNames, pd.atpos);
  physSys supercell(primcell, tilemat);

  // These are the weighted supercell k-points;
  vector<weightedKpt> outks;
  supercell.getMesh(sskgrid[0], sskgrid[1], sskgrid[2], sskshift[0], sskshift[1], sskshift[2], outks);

  // Now need to find the primitive cell k-points necessary
  int numCopies = getDet(tilemat);
  vector<weightedKpt> primks;
  
  // loop over supercell kpoints
  for (int i = 0; i < outks.size(); i++) {
    double weight = outks[i].first;
    vector<location> locPrimKs;
    getPrimKpts(primcell, supercell, outks[i].second, locPrimKs, numCopies);
    // for each found primitive cell kpt, see if it is in primks
    // if it is add it's weight to the existing one
    // if it is not, add it and its weight
    for (int j = 0; j < locPrimKs.size(); j++) {
      int found = 0;
      for (int k = 0; k < primks.size(); k++) {
	if (abs(locPrimKs[j][0] - primks[k].second[0]) < 1e-8 &&
	    abs(locPrimKs[j][1] - primks[k].second[1]) < 1e-8 &&
	    abs(locPrimKs[j][2] - primks[k].second[2]) < 1e-8) {
	  primks[k].first+=weight;
	  found = 1;
	  k+= primks.size();
	}
      }
      if (!found) {
	weightedKpt kp;
	kp.first = weight;
	for (int k = 0; k < 3; k++) {
	  kp.second[k] = locPrimKs[j][k];
	}
	primks.push_back(kp);
      }
    }
  }

  // Now outks has the symmetry independent supercell kpoints
  // primks has the necessary primitive cell kpts to generate outks

  stringstream ss;
  ss << "K_POINTS {CRYSTAL}" << endl;
  ss.setf(ios_base::fixed, ios_base::floatfield);
  ss.precision(8);
  ss << setw(8) << primks.size() << endl;
  //  ss << setw(16) << "kx" << setw(16) << "ky" << setw(16) << "kz" << setw(16) << "wt" << endl;
  for (int i = 0; i < primks.size(); i++) {
    ss << setw(16) << primks[i].second[0] << setw(16) << primks[i].second[1] << setw(16) << primks[i].second[2] << setw(16) << primks[i].first;
    if (i+1 < primks.size()) {
      ss << endl;
    }
  }
  string kpointstring = ss.str();

  // now need to replace the pwscfString (ps) object's K_POINTS card with a new
  // one using kpointstring  (need a pwscfReplaceWholeCard)
  // Also need to change the calculation from scf to nscf  (pwscfString.changePwscfToken)
  // Also need to make sure the output has nosym=.true. and noinv=.true.  (need an addToPwscfSection)
  ps.changePwscfToken("calculation", "'nscf'");
  ps.removePwscfToken("nosym");
  ps.removePwscfToken("noinv");
  ps.addToPwscfSection("system", "nosym", ".true.");
  ps.addToPwscfSection("system", "noinv", ".true.");
  ps.replaceWholePwscfCard("K_POINTS", kpointstring);

  string nscfstring = ps.getOutput();

  stringstream p2xstringss;
  p2xstringss << "&inputpp" << endl;
  p2xstringss << "   outdir = '" << pd.outputDir << "'" << endl;
  p2xstringss << "   prefix = '" << pd.prefix << "'" << endl;
  p2xstringss << "   write_psir=.false." << endl;
  p2xstringss << "/" << endl;


  string nscfFname = inFileName;
  // get rid of -scf if file has it
  size_t index = nscfFname.find("-scf");
  if (index != string::npos) {
    nscfFname.erase(index, 4);
  }
  // get rid of .in if file has it
  size_t index2 = nscfFname.find(".in");
  if (index2 != string::npos) {
    nscfFname.erase(index2,3);
  }
  stringstream fnss;
  fnss << nscfFname;
  fnss << "-supertwist" << sskgrid[0] << sskgrid[1] << sskgrid[2];
  fnss << "-supershift" << sskshift[0] << sskshift[1] << sskshift[2];
  fnss << "-nscf.in";
  stringstream p2xss;
  p2xss << nscfFname;
  p2xss << "-supertwist" << sskgrid[0] << sskgrid[1] << sskgrid[2];
  p2xss << "-supershift" << sskshift[0] << sskshift[1] << sskshift[2];
  p2xss << "-pw2x.in";

  //cout << "nscf filename: " << fnss.str() << endl;
  //cout << "pw2x filename: " << p2xss.str() << endl;

  // Now write the files themselves
  ofstream nscffile(fnss.str().c_str());
  nscffile << nscfstring;
  nscffile.close();

  ofstream pw2xfile(p2xss.str().c_str());
  pw2xfile << p2xstringss.str();
  pw2xfile.close();
  

}

#endif
