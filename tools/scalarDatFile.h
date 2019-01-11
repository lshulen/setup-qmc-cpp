#ifndef SCALAR_DAT_FILE_H
#define SCALAR_DAT_FILE_H
#include <string>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <stdlib.h>
#include <stdio.h>



class scalarDatFile {
 private:
  std::vector<std::string> colnames;
  std::vector<std::vector<double> > values;
 public:
  scalarDatFile(const std::string& fname) {
    struct stat buffer; 
    bool exists = (stat (fname.c_str(), &buffer) == 0);
    if (!exists) {
      std::cout << "The file " << fname << " does not exist!" << std::endl;
      exit(1);
    }
    
    std::ifstream fs(fname.c_str());
    int linenum = 0;
    std::string line;
    std::getline(fs,line);
    // go through first line, figure out titles and set up data structures
    std::stringstream ss(line);
    std::string dummy;
    ss >> dummy; // throw away first token (it is #)
    // now put the rest into colnames
    while (ss >> dummy) {
      colnames.push_back(dummy);
    }
    for (int i = 0; i < colnames.size(); i++) {
      std::vector<double> dummyvec;
      values.push_back(dummyvec);
    }

    /*
    // now read all of the rest of the file and put it into the values vectors
    while(std::getline(fs,line)) {
      std::stringstream ss2(line);
      for (int i = 0; i < colnames.size(); i++) {
        double ddummy;
	ss2 >> ddummy;
	values[i].push_back(ddummy);
      }
    }
    */
    // alternative version that should be a bit faster by avoiding stringstream
    int counter = 0;
    double ddummy;
    while (fs >> ddummy) {
      if (counter >= colnames.size()) {
	counter = 0;
      }
      values[counter].push_back(ddummy);
      counter++;

    }

  }
  int getNumCols() const { return colnames.size(); }
  const std::string& getColName(int i) const { 
    if (i < 0 || i >= getNumCols()) {
      std::cout << "Asking for column out of bounds!" << std::endl;
      exit(1);
    }
    return colnames[i];
  }
  const std::vector<double>& getValues(int i) const {
    if (i < 0 || i >= getNumCols()) {
      std::cout << "Asking for column out of bounds!" << std::endl;
      exit(1);
    }
    return values[i];
  }
  int getNumDataPoints() const { return values[0].size(); }
};

#endif
