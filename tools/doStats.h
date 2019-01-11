#ifndef DO_STATS_H
#define DO_STATS_H
#include <vector>
#include <cmath>
#include <string>
#include <sys/stat.h>
#include "scalarDatFile.h"

void getNaiveAvgVar(const std::vector<double>&, int, double&, double&);
double corr(int, int, const std::vector<double>&, double, double);
int getEquil(const std::vector<double>&);
void getVariance(const scalarDatFile& data, double&, double&, int&);
void doStats(const std::vector<double>& data, int equilIn, int& foundCorr, double& avg, double& err);


void doAllStats(const std::string& fname, std::vector<double>& values, std::vector<double>& errs, std::vector<int>& ctime,
		std::vector<std::string>& labels, double factor, int equil, int verbose, int rhok,
		int sk, int terse) {

  struct stat buffer;   
  bool fileExists = (stat (fname.c_str(), &buffer) == 0); 
  if (!fileExists) {
    std::cout << "The file " << fname << " does not exist!" << std::endl;
    exit(1);
  }
  
  scalarDatFile data(fname);

  if (rhok == 0 && sk == 0) {
    double var;
    double varErr;
    int varCorrTime;
    
    getVariance(data, var, varErr, varCorrTime);
    double en;
    double err;
    int corrtime;
    if (equil < 0) {
      doStats(data.getValues(1), getEquil(data.getValues(1)), corrtime, en, err);
    } else {
      doStats(data.getValues(1), equil, corrtime, en, err);
    }
    //std::cout << "energy = " << en/factor << " +/- " << err/factor/factor << std::endl;
    values.push_back(en/factor);
    values.push_back(var/factor/factor);
    errs.push_back(err/factor);
    errs.push_back(varErr/factor/factor);
    ctime.push_back(corrtime);
    ctime.push_back(varCorrTime);
    labels.push_back("LocalEnergy");
    labels.push_back("Variance");
  }
  
  double blockWeight;
  double blockCPU;
  if (!terse) {
    for (int i = 3; i < data.getNumCols(); i++) {
      std::string colName = data.getColName(i);
      size_t rhokidx = colName.find("rhok");
      size_t skidx = colName.find("sk");

      if (rhok == 0 && sk == 0) {
	if (colName == "LocalPotential" || colName == "Kinetic" || colName == "LocalECP" || 
	    colName == "NonLocalECP" || colName == "IonIon" || colName == "ElecElec" || colName == "KEcorr") {
	  double val, valerr;
	  int corrtime;
	  int locEquilTime = equil;
	  if (equil < 0) {
	    locEquilTime = getEquil(data.getValues(i));
	  }
	  doStats(data.getValues(i), locEquilTime, corrtime, val, valerr);
	  if (colName == "KEcorr" && std::abs(val) < 1e-20) {
	    ;
	  } else {
	    values.push_back(val/factor);
	    errs.push_back(valerr/factor);
	    ctime.push_back(corrtime);
	    labels.push_back(colName);
	  }
	} else if (colName == "BlockWeight" || colName == "BlockCPU" || colName == "AcceptRatio") {
	  double val, valerr;
	  int corrtime;
	  int locEquilTime = equil;
	  if (equil < 0) {
	    locEquilTime = getEquil(data.getValues(i));
	  }
	  doStats(data.getValues(i), locEquilTime, corrtime, val, valerr);
	  if (colName == "BlockWeight") {
	    blockWeight = val;
	  } else if (colName == "BlockCPU") {
	    blockCPU = val;
	  }
	  values.push_back(val);
	  errs.push_back(valerr);
	  ctime.push_back(corrtime);
	  labels.push_back(colName);
	}
      }
      if (rhok == 1 && rhokidx != std::string::npos) {
	double val, valerr;
	int corrtime;

	int locEquilTime = equil;
	if (equil < 0) {
	  locEquilTime = getEquil(data.getValues(i));
	}
	doStats(data.getValues(i), locEquilTime, corrtime, val, valerr);
	values.push_back(val);
	errs.push_back(valerr);
	ctime.push_back(corrtime);
	labels.push_back(colName);
      }
      if (sk == 1 && skidx != std::string::npos) {
	double val, valerr;
	int corrtime;

	int locEquilTime = equil;
	if (equil < 0) {
	  locEquilTime = getEquil(data.getValues(i));
	}
	doStats(data.getValues(i), locEquilTime, corrtime, val, valerr);
	values.push_back(val);
	errs.push_back(valerr);
	ctime.push_back(corrtime);
	labels.push_back(colName);
      }	
    }
    if (rhok == 0 && sk == 0) {
      values.push_back(blockWeight/blockCPU);
      errs.push_back(0.0);
      ctime.push_back(0);
      labels.push_back("Efficiency");
    }
  }
}
  
  

void doStats(const std::vector<double>& data, int equilIn, int& foundCorr, double& avg, double& err) {
  double var;
  // ignoring autocorrelation, get average and variance
  getNaiveAvgVar(data, equilIn, avg, var);
  if (var < 0.0) {
    var = 0;
  }

  // estimate autocorrelation time
  int lag=0;
  int n = data.size() - equilIn + 1;
  double tempC = 0.5;
  double kappa = 0.0;
  while (tempC > 0.0 and lag < (n-1)) {
    kappa += 2*tempC;
    lag += 1;
    tempC = corr(equilIn, lag, data, avg, var);
  }
  if (std::abs(kappa) < 1e-6) {
    kappa = 1.0;
  }

  foundCorr = kappa;
  // deal with effects of autocorrelation time
  double neff = n / kappa;
  if (neff < 1e-6) {
    neff = 1.0;
  }
  err = std::sqrt(var/neff);
}

void getNaiveAvgVar(const std::vector<double>& data, int equilIn, double& avg, double& var) {
  double estavg = 0.0;
  int numpts = 0;
  for (int i = equilIn; i < data.size(); i++) {
    estavg += data[i];
    numpts++;
  }
  estavg /= static_cast<double>(numpts);
  double residual = 0.0;
  for (int i = equilIn; i < data.size(); i++) {
    residual += (data[i]-estavg);
  }
  avg = estavg + residual / static_cast<double>(numpts);
  
  var = 0.0;
  for (int i = equilIn; i < data.size(); i++) {
    var += (data[i]-avg)*(data[i]-avg);
  }
  var /= static_cast<double>(numpts-1);
}


double corr(int equilIn, int lag, const std::vector<double>& data, double avg, double var) {
  if (std::abs(var) < 1e-10) {
    return 1;
  }
  double runSum = 0.0;
  int pts = 0;
  for (int i = equilIn; i < data.size()-lag; i++) {
    pts++;
    const double v1 = data[i]-avg;
    const double v2 = data[i+lag]-avg;
    runSum += v1*v2;
  }
  return runSum / var / static_cast<double>(pts);
}

int getEquil(const std::vector<double>& data) {
  double avg;
  int cross1 = data.size()-1;
  int cross2 = data.size()-1;
  int cross3 = data.size()-1;
  int counter = 0;
  for (int j = data.size()-1; j > 0; j--) {
    counter++;
    avg *= static_cast<double>(counter)-1;
    avg += data[j];
    avg /= static_cast<double>(counter);
    if ( (avg-data[j]) * (avg-data[j-1]) < 0) {
      cross3 = cross2;
      cross2 = cross1;
      cross1 = j;
    }
  }
  if (cross3 > (data.size()-15)) {
    cross3 = data.size()-15;
  }

  return cross3;
}

void getVariance(const scalarDatFile& data, double& var, double& varErr, int& corrTime) {
  int encol;
  int ensqcol;
  for (int i = 0; i < data.getNumCols(); i++) {
    if (data.getColName(i) == "LocalEnergy") {
      encol = i;
    } else if (data.getColName(i) == "LocalEnergy_sq") {
      ensqcol = i;
    }
  }
  std::vector<double> temp;
  const std::vector<double>& enRef = data.getValues(encol);
  const std::vector<double>& enSqRef = data.getValues(ensqcol);
  for (int i = 0; i < enRef.size(); i++) {
    temp.push_back(enSqRef[i]-enRef[i]*enRef[i]);
  }
  doStats(temp, getEquil(temp), corrTime, var, varErr);
}


#endif
