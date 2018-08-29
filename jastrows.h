#ifndef JASTROWS_H
#define JASTROWS_H
#include <vector>
#include <string>

class xmlNode;

// will need to add a constructor that takes a string which comes from an .opt.xml file
// which they will then parse and populate themselves
class jastrow {
 public: 
  std::string type;
  std::string getWfnString(int indentSize) const;
  virtual xmlNode createXmlNode() const = 0;
  int isIncluded;
  int isOptimizable;
  std::string getOptString() const;
};

class J1Spline : public jastrow {
 public:
  std::string source;
  std::vector<std::string> elementType;
  std::vector<std::vector<double> > coeffList;
  std::vector<int> cuspValue;
  std::vector<std::string> iD;
  std::vector<double> rcut;
  void addCenter(const std::string& eType, int numCoeffs, int cusp, const std::string& id, double rc);
  xmlNode createXmlNode() const;
  J1Spline(const std::string& src) {
    type = "One-Body";
    isIncluded = 1;
    isOptimizable = 1;
    source = src;
  }
  J1Spline(const J1Spline& js);
};

class J1Pade : public jastrow {
 public:
  std::string source;
  std::vector<std::string> elementType;
  std::vector<std::vector<double> > coeffList; // (A, B and C)
  std::vector<std::string> iD;
  xmlNode createXmlNode() const;
  void addCenter(const std::string& eType, const std::string& id, double A, double B, double C);
  J1Pade(const std::string& src) {
    type = "One-Body";
    isIncluded = 1;
    isOptimizable = 1;
    source = src;
  }
  J1Pade(const J1Pade& jp);			
};

class J2Spline : public jastrow {
 public:
  int isSpinDep;
  std::vector<std::vector<double> > coeffList;
  std::vector<double> rcut;
  std::vector<std::string> iD;
  xmlNode createXmlNode() const;
  J2Spline(int spinDep, int numCoeff, double rc);
  J2Spline(const J2Spline& js);    
};

class J2Pade : public jastrow {
 public:
  int isSpinDep;
  std::vector<std::vector<double> > coeffList; // (just B)
  std::vector<std::string> iD;
  xmlNode createXmlNode() const;
  J2Pade(int spinDep);
  J2Pade(const J2Pade& jp);
};

class J2kSpace : public jastrow {
  double kc;
  std::vector<double> coeffs;
  std::string iD;
  xmlNode createXmlNode() const;
  J2kSpace(const std::string& id, double inKc) {
    type = "kSpace";
    isIncluded = 1;
    isOptimizable = 1;
    kc = inKc;
    iD = id;
  }
  J2kSpace(const J2kSpace& j2k) {
    type = j2k.type;
    isIncluded = j2k.isIncluded;
    isOptimizable = j2k.isOptimizable;
    kc = j2k.kc;
    iD = j2k.iD;
    coeffs = j2k.coeffs;
  }
};

class J3 : public jastrow {
  int isSpinDep;
  std::string source;
  std::vector<std::string> elemType;
  std::vector<std::string> iD;
  std::vector<double> rcut;
  std::vector<int> esize;
  std::vector<int> isize;
  std::vector<std::vector<double> > coeffList;
  void addCenter(const std::string& eType, double rc, int es = 3, int is = 3);  
  J3(const std::string& source, int spinDep = 1) {
    type = "eeI";
    isIncluded = 1;
    isOptimizable = 1;
    isSpinDep = spinDep;
  };
  J3(const J3& j3);
  xmlNode createXmlNode() const;
};

#endif
