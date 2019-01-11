#ifndef IO_TOOLS_H
#define IO_TOOLS_H
#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <algorithm>
#include <vector>
#include <dirent.h>

void convertToVecStrings(int argc, char* argv[], std::vector<std::string>& vec) {
  for (int i = 1; i < argc; i++) {
    vec.push_back(argv[i]);
  }
}

class fnameComparer {
 private:
  const std::string prefix;
  const std::string suffix;
  int getNumber(const std::string& value) const {
    size_t prefixIdx = value.find(prefix);
    size_t suffixIdx = value.find(suffix); 
    int number = std::stoi(value.substr(prefixIdx+prefix.size(),suffixIdx-prefixIdx+prefix.size()));
    //std::cout << "Looking at string: " << value << " and found the number: " << number << std::endl;
    return number;
  }
 public:
 fnameComparer(const std::string& pre, const std::string& suff) : prefix(pre), suffix(suff) { ; }
  bool operator()(const std::string& a, const std::string& b) {
    return (getNumber(a) < getNumber(b));
  }
};

void listFileNames(std::vector<std::string>& fnames, std::string nameTemplate, std::string prefix, int numfiles) {

  size_t prefixIdx = nameTemplate.find(prefix);
  size_t suffixIdx = nameTemplate.find(".", prefixIdx);
  std::string prefixSubstr = nameTemplate.substr(0,prefixIdx+prefix.size());
  std::string suffixSubstr = nameTemplate.substr(suffixIdx, -1);
  //std::cout << "Prefix part of template filename is " << prefixSubstr << std::endl;
  //std::cout << "Suffix part of template filename is " << suffixSubstr << std::endl;

  fnameComparer fc(prefixSubstr, suffixSubstr);

  DIR * dpdf;
  struct dirent *epdf;
  
  dpdf = opendir("./");
  if (dpdf != NULL) {
    while (epdf = readdir(dpdf)) {
      std::string fname = epdf->d_name;

      size_t locPrefixIdx = fname.find(prefixSubstr);
      size_t locSuffixIdx = fname.find(suffixSubstr);
      
      if (locPrefixIdx != std::string::npos && locSuffixIdx != std::string::npos) {
	fnames.push_back(fname);
      }
    }
  }
  closedir(dpdf);
  std::sort(fnames.begin(), fnames.end(), fc);
  /*
  std::cout << "There are " << fnames.size() << " files that matched the template" << std::endl;
  if (fnames.size() > 4) {
    for (int i = 0; i < 4; i++) {
      std::cout << "fname " << i << " is " << fnames[i] << std::endl;
    }
  }
  */
}



int getDecimals(double value) {
  int rightDigits = 15;
  if (value > 1e-15) {
    rightDigits = -std::floor(log10(value)/log10(10))+1;
  } else {
    rightDigits = 9;
  }
  return rightDigits;
}

std::string toSigf(double num, int prec) {
  std::ostringstream oss;
  oss.setf(std::ios::fixed);
  oss << std::setprecision(prec) << num;
  return oss.str();
}

std::string toPrecision(double num,int n) {
  if (n > 0) {
    return toSigf(num, n);
  } 
  long val = ::round(num/pow(10,-n));
  val *= pow(10,-n);
  std::ostringstream oss;
  oss << val;
  return oss.str();
}

void doOutput(const std::string& label, double value, double err, int corrtime, int verbose) {
  const char separator = ' ';
  const int nameWidth = 20;
  const int valWidth = 17;
  const int prec = getDecimals(err);
  
  std::cout << std::left << std::setw(nameWidth) << std::setfill(separator) << label << "=";
  std::cout << std::right << std::setw(valWidth) << std::setfill(separator) << toPrecision(value,prec) << " +/- ";
  std::cout << std::setw(valWidth) << std::setfill(separator) << toPrecision(err,prec);
  if (verbose == 1) {
    std::cout << std::setw(valWidth) << std::setfill(separator) << corrtime;
  }
  std::cout << std::endl;
}

#endif
