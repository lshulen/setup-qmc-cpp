#ifndef OBSERVABLES_H
#define OBSERVABLES_H
#include <vector>
#include <string>
#include "types.h"

class xmlNode;

class observable {
 public:
  std::string name;
  virtual xmlNode createXmlNode() const = 0;
  std::string getObsString(int indentsize) const;
};

class ionInteractions : public observable {
 public:
  std::string source;
  std::string wfn;
  std::vector<std::string> elementType;
  std::vector<int> isAe;
  std::vector<int> zval;
  std::vector<std::string> ppName;
  ionInteractions(const std::string& source, const std::string& wfn, const std::string& name);
  ionInteractions(const ionInteractions& ii);
  void addPP(const std::string& eType, const std::string& fname);
  void addAE(const std::string& eType, int val);	      
  xmlNode createXmlNode() const;
};

class chiesa : public observable {
 public:
  std::string source;
  std::string wfn;
  chiesa(const std::string& ss, const std::string& wfns);
  chiesa(const chiesa& co);
  xmlNode createXmlNode() const;
};

class mpc : public observable {
 public:
  std::string source;
  std::string target;
  int isPhysical;
  double ecut;
  mpc(const std::string& ss, const std::string& ts, double ec, int ip = 0);
  mpc(const mpc& mo);
  xmlNode createXmlNode() const;
};

class spindensity : public observable {
 public:
  int isGridOrDr; // 1 for grid, 0 for dr
  int isCornerOrCenter; // 1 for corner 0 for center
  int hasCell; // 1 if has, 0 if not  (default is to use whole cell for periodic calculation)
  std::vector<int> grid;
  std::vector<double> dr;
  std::vector<double> origin;
  threeThreeMat<double> cell;
  
  spindensity(int nx, int ny, int nz); // constructor for defined grid
  spindensity(double drx, double dry, double drz); // constructor for defined grid spacing
  spindensity(const spindensity& sd);
  void setCell(int centerOrCorner, double ox, double oy, double oz, const threeThreeMat<double>& ci);
  xmlNode createXmlNode() const;
};

class skall : public observable {
 public:
  std::string source;
  std::string target;
  int addIonIon;
  skall(const std::string& ss, const std::string& ts, int aii = 1);
  skall(const skall& ska);
  xmlNode createXmlNode() const;
};

class force : public observable {
 public:
  int addIonIon;
  double rcut;
  int nbasis;
  int weightexp;
  force(double rc, int nb, int we, int aii = 1);
  force(const force& fo);
  xmlNode createXmlNode() const;
};

class ionIon : public observable {
 public:
  std::string source;
  std::string target;
  ionIon(const std::string& ss, const std::string& ts);
  ionIon(const ionIon& ii);
  xmlNode createXmlNode() const;
};

class elecElec : public observable {
  std::string source;
  std::string target;
  elecElec(const std::string& ss, const std::string& ts);
  elecElec(const elecElec& ii);
  xmlNode createXmlNode() const;
}; 


#endif
