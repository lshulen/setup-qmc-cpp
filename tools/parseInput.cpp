#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include "../xmlRep.h"
#include "ioTools.h"
#include "doStats.h"
using namespace std;


// if handed an xml file, will be looking at a single calculation
// so no .g???. in the output files

// eventually support handing a file that is the list of input files
// to it, and will handle the .g???. from there

// Final solution should be to look at the output file rather than 
// the input file

void getQMCInfo(const xmlNode& qmc, string& type, double& tstep) {
  string method;
  qmc.getAttribute("method", method);
  if (method == "linear" || method == "vmc") {
    type = "vmc";
  } else if (method == "dmc") {
    type = "dmc";
  } else if (method == "rmc") {
    type = "rmc";
  }
  
  tstep = -1;
  for (int i = 0; i < qmc.getNumChildren(); i++) {
    const xmlNode& child = qmc.children[i];
    if (child.name == "parameter") {
      string paramName;
      child.getAttribute("name", paramName);
      if (paramName == "timestep") {
	child.getValue<double>(tstep);
      }
    }
  }
}

string getBaseName(const string& base, const int series) {
  stringstream ss;
  ss << base << ".s";
  ss << setfill('0') << setw(3);
  ss << series;
  return ss.str();
}

void parseInputFile(const string& fname, string& id, vector<int>& seriesnum, vector<string>& calcType, vector<double>& tstep) {
	   
  ifstream f(fname.c_str());
  stringstream ss;
  string s;
  while (getline(f,s)) 
  {
    ss << s;
  }

  xmlNode simulationNode(ss);
  xmlNode projectNode = simulationNode.children[simulationNode.getChildIndex("project")];
	
  projectNode.getAttribute<string>("id", id);
  int initSeries;
  projectNode.getAttribute<int>("series", initSeries);
  
  int series = initSeries;
  for (int i = 0; i < simulationNode.getNumChildren(); i++) 
  {
    const xmlNode& child = simulationNode.children[i];
    if (child.name == "qmc") {
      string type;
      double ts;
      getQMCInfo(child, type, ts);
      seriesnum.push_back(series);
      series++;
      calcType.push_back(type);
      tstep.push_back(ts);
    } else if (child.name == "loop") {
      int times;
      child.getAttribute<int>("max", times);
      xmlNode qmcChild = child.children[0];
      string type;
      double ts;
      getQMCInfo(qmcChild, type, ts);
      for (int i = 0; i < times; i++) {
	seriesnum.push_back(series);
	series++;
	calcType.push_back(type);
	tstep.push_back(ts);
      }
    }
  }
}


int main (int argc, char* argv[])  
{
  string inputFileName;
 
  double factor = 1; // divide all energy like quantities by this factor
  int verbose = 0; // if set true, also give per quantity correlation times
  int rhok = 0; // if set true, print out rhok values produced by skall
  int sk = 0; // if set true, print out sk values from s(k)
  int terse = 0; // if set true, only give energy and variance
  int equil = -1; // if provided, throw out input number of bins rather than detecting equilibration

  vector<string> vecParams;
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
    } else {
      inputFileName = vecParams[i];
    }
  }

  
  if (vecParams.size() >= 1) 
  {
    inputFileName = vecParams[0];
  } else 
  {
    cout << "Must give as input a qmcpack input file name" << endl;
    exit(1);
  }

  vector<int> seriesnum;
  vector<string> calcType;
  vector<double> tstep;
  string id;

  parseInputFile(inputFileName, id, seriesnum, calcType, tstep);

  cout << "The input file specifies " << seriesnum.size() << " qmc calculations." << endl;
  cout << "Here are the details for them. " << endl;;
  cout << "filename     calculation_type      timestep" << endl;
  for (int i = 0; i < seriesnum.size(); i++) {
    cout << getBaseName(id, seriesnum[i]) << ".scalar.dat"  << "    " << calcType[i] << "     " << tstep[i] << endl;
  }


  vector<vector<double> > vals;
  vector<vector<double> > errs;
  vector<vector<int> > ctimes;
  vector<string> labels;

  vector<double> tmp;
  vector<int> inttmp;
  for (int i = 0; i < seriesnum.size(); i++) {
    vals.push_back(tmp);
    errs.push_back(tmp);
    ctimes.push_back(inttmp);
  }
  
  for (int i = 0; i < seriesnum.size(); i++) {
    stringstream ss;
    ss << getBaseName(id, seriesnum[i]) << ".scalar.dat";
    const string datFile(ss.str());
    vector<string> lab;
    doAllStats(datFile, vals[i], errs[i], ctimes[i], lab, factor, equil, verbose, rhok, sk, terse);
    labels = lab;
  }

  for (int i = 0; i < vals.size(); i++) {
    cout << calcType[i] << "   " << tstep[i] << "   ";
    doOutput(labels[0], vals[i][0], errs[i][0], ctimes[i][0], verbose);
  }



}

