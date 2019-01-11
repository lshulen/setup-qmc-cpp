#include <iostream>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include "scalarDatFile.h"
#include "doStats.h"
#include "ioTools.h"
using namespace std;

int main(int argc, char* argv[]) {
  vector<string> vecParams;

  string datFileName;
  double factor = 1; // divide all energy like quantities by this factor
  int verbose = 0; // if set true, also give per quantity correlation times
  int rhok = 0; // if set true, print out rhok values produced by skall
  int sk = 0; // if set true, print out sk values from s(k)
  int terse = 0; // if set true, only give energy and variance
  int equil = -1; // if provided, throw out input number of bins rather than detecting equilibration
  int numFiles = -1;

  vector<string> fileNames;

  convertToVecStrings(argc, argv, vecParams);
  for (int i = 0; i < vecParams.size(); i++) {
    if (vecParams[i] == "--factor" || vecParams[i] == "-f") {
      factor *= stod(vecParams[i+1]);
      i++;
    } else if (vecParams[i] == "--equil") {
      equil = stoi(vecParams[i+1]);
      i++;
    } else if (vecParams[i] == "--eV" || vecParams[i] == "--ev") {
      factor /= 27.2113966413;
    } else if (vecParams[i] == "-v" || vecParams[i] == "--verbose") {
      verbose = 1;
    } else if (vecParams[i] == "--rhok") {
      rhok = 1;
    } else if (vecParams[i] == "--sk") {
      sk = 1;
    } else if (vecParams[i] == "--terse" || vecParams[i] == "-t") {
      terse = 1;
    } else if (vecParams[i] == "--twistAverage" || vecParams[i] == "-ta") {
      string fnameTemplate = vecParams[i+1];
      string incrPrefix = vecParams[i+2];
      numFiles = stoi(vecParams[i+3]);
      i+=3;
      //cout << "template filename is " << fnameTemplate << endl;
      //cout << "characters that prefix increment are: " << incrPrefix << endl;
      //cout << "looking for " << numFiles << " files" << endl;
      listFileNames(fileNames, fnameTemplate, incrPrefix, numFiles);
    } else {
      datFileName = vecParams[i];
    }
  }

  if (numFiles == -1) {
    numFiles = 1;
    fileNames.push_back(datFileName);
  }

  if (numFiles > fileNames.size()) {
    cout << "Asked for " << numFiles << " data files, but only found" << fileNames.size() << endl;
    cout << "For reference, the first file found was " << fileNames[0] << " and the last was" << fileNames[fileNames.size()-1] << endl;
    exit(1);
  }

  // now want to loop over all files and do what is necessary
  vector<vector<double> > vals;
  vector<vector<double> > errs;
  vector<vector<int> > ctimes;
  vector<string> labels;

  vector<double> tmp;
  vector<int> inttmp;
  for (int i = 0; i < numFiles; i++) {
    vals.push_back(tmp);
    errs.push_back(tmp);
    ctimes.push_back(inttmp);
  }
  
  for (int i = 0; i < numFiles; i++) {
    vector<string> lab;
    //cout << "About to doAllStats for data file " << i << " named " << fileNames[i] << endl;
    doAllStats(fileNames[i], vals[i], errs[i], ctimes[i], lab, factor, equil, verbose, rhok, sk, terse);
    //cout << "Finished doAllStats" << endl;
    labels = lab;
  }

  vector<double> avgVals;
  vector<double> avgErrs;
  vector<double> avgCtimes;
  for (int i = 0; i < labels.size(); i++) {
    avgVals.push_back(0.0);
    avgErrs.push_back(0.0);
    avgCtimes.push_back(0);
  }

  for (int i = 0; i < numFiles; i++) {
    for (int j = 0; j < labels.size(); j++) {
      avgVals[j] += vals[i][j];
      avgErrs[j] += errs[i][j]*errs[i][j];
      avgCtimes[j] += ctimes[i][j];
    }
  }
  for (int i = 0; i < labels.size(); i++) {
    avgVals[i] /= static_cast<double>(numFiles);
    avgErrs[i] = sqrt(avgErrs[i]) / static_cast<double>(numFiles);
    avgCtimes[i] /= static_cast<double>(numFiles);
  }

  for (int i = 0; i < labels.size(); i++) {
    doOutput(labels[i], avgVals[i], avgErrs[i], avgCtimes[i], verbose);
  }
}

