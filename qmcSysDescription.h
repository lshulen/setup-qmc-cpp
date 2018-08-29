#ifndef QMC_SYS_DESCRIPTION
#define QMC_SYS_DESCRIPTION
#include <map>
#include <vector>
#include <string>
#include "types.h"
#include "xmlRep.h"

class jastrow;
class observable;

class atomTypeDescriptions {
 private:
  std::vector<std::string> atNames; // list of the atom types (will be ntyp of these)
  std::map<std::string, std::vector<location> > atNameToPosVec; // from each atom type a vector of that atom's positions

  std::map<std::string, std::string> atNameToPPFile; // list from type to pp file name (will say AE if treated as AE)
  std::map<std::string, int> atNameToCharge;
  std::map<std::string, int> atNameToValence;
  std::map<std::string, int> atNameToAtomicNumber;
  std::map<std::string, double> atNameToMass;

  // need these only if using hybrid representation
  std::map<std::string, double> atNameToCutoffRadius;
  std::map<std::string, double> atNameToSplineRadius;
  std::map<std::string, int> atNameToSplinePts;
  std::map<std::string, int> atNameToLmax;

 public:
  atomTypeDescriptions();
  
  void addAtomType(const std::string& name, const std::string& ppFile, int charge,
		   int valence, int atomicNumber, double mass, const std::vector<location>& pos, 
		   double cutoffRadius = 0, double splineRadius = 0, int splinePts = 0, int lmax = 0) {
    atNames.push_back(name);
    atNameToPosVec[name] = pos;
    atNameToPPFile[name] = ppFile;
    atNameToCharge[name] = charge;
    atNameToValence[name] = valence;
    atNameToAtomicNumber[name] = atomicNumber;
    atNameToMass[name] = mass;
    atNameToCutoffRadius[name] = cutoffRadius;
    atNameToSplineRadius[name] = splineRadius;
    atNameToSplinePts[name] = splinePts;
    atNameToLmax[name] = lmax;
  } 

  int getNumTypes() const { return atNames.size(); }
  std::string getAtName(int i) const { return atNames[i]; }
 
  std::string getPPFile(const std::string& atName) const {
    std::map<std::string, std::string>::const_iterator it = atNameToPPFile.find(atName);
    return it->second;
  }
  int getCharge(const std::string& atName) const {
    std::map<std::string, int>::const_iterator it = atNameToCharge.find(atName);
    return it->second;
  }
  int getValence(const std::string& atName) const {
    std::map<std::string, int>::const_iterator it = atNameToValence.find(atName);
    return it->second;
  }
  int getAtomicNumber(const std::string& atName) const {
    std::map<std::string, int>::const_iterator it = atNameToAtomicNumber.find(atName);
    return it->second;
  }
  double getMass(const std::string& atName) const {
    std::map<std::string, double>::const_iterator it = atNameToMass.find(atName);
    return it->second;
  }
  double getCutoffRadius(const std::string& atName) const {
    std::map<std::string, double>::const_iterator it = atNameToCutoffRadius.find(atName);
    return it->second;
  }
  double getSplineRadius(const std::string& atName) const {
    std::map<std::string, double>::const_iterator it = atNameToSplineRadius.find(atName);
    return it->second;
  }
  int getSplinePts(const std::string& atName) const {
    std::map<std::string, int>::const_iterator it = atNameToSplinePts.find(atName);
    return it->second;
  }
  int getLmax(const std::string& atName) const {
    std::map<std::string, int>::const_iterator it = atNameToLmax.find(atName);
    return it->second;
  }
  const std::vector<location>& getPosVec(const std::string& atName) const {
    std::map<std::string, std::vector<location> >::const_iterator it = atNameToPosVec.find(atName);
    return it->second;
  }
};
  

// still need to add a --jastrowperion option (will affect both jastrows and particleset)


// Main task now is to write the constructor!!!
class qmcSysDescription {
 private:

  double lr_dim_cutoff;
  
  // variables to store ptv, atom types and positions and ppnames
  int numAt;
  int nelup;
  int neldn;

  atomTypeDescriptions atd;
  int getNumTypes() const { return atd.getNumTypes(); }
  std::string getAtName(int i) const { return atd.getAtName(i); }
  std::string getPPFile(const std::string& atName) const { return atd.getPPFile(atName); }
  int getCharge(const std::string& atName) const { return atd.getCharge(atName); }
  int getValence(const std::string& atName) const { return atd.getValence(atName); }
  int getAtomicNumber(const std::string& atName) const { return atd.getAtomicNumber(atName); }
  double getMass(const std::string& atName) const { return atd.getMass(atName); }
  
  // need these only if using hybrid representation
  double getCutoffRadius(const std::string& atName) const { return atd.getCutoffRadius(atName); }
  double getSplineRadius(const std::string& atName) const { return atd.getSplineRadius(atName); }
  int getSplinePts(const std::string& atName) const { return atd.getSplinePts(atName); }
  int getLmax(const std::string& atName) const { return atd.getLmax(atName); }
  const std::vector<location>& getPosVec(const std::string& atName) const { return atd.getPosVec(atName); }

  threeThreeMat<double> ptv;
  std::vector<int> bconds;
  
  // variables to store info about detset type
  int isHybrid;
  int isTiled;
  std::string h5name;
  threeThreeMat<int> tilemat;
  double meshfactor;
  int useTruncate;
  double bufferSize;
  int useDpCoeffs;
  int useGPU;
  int isSpinDependent;

  // variables to store names of sections
  std::string hamname;
  std::string wfnname;
  std::string epsetname;
  std::string ipsetname;

  // variables to store info about wfntype
  std::vector<jastrow*> jastrows;
  
  // variables to store info about hamiltonianType
  std::vector<observable*> observables;

  // map to store excitations (from band to be depopulated to one to be populated)
  std::map<int, int> upExcit;
  std::map<int, int> downExcit;
  
 public:
  void createJastrows(int argc, char* argv[]);
  void createJastrows(const std::string& optDirName);
  void createObservables(int argc, char* argv[]);

  void addExcit(int spin, int depopulate, int populate) {
    if (spin == 0) {
      upExcit[depopulate] = populate;
    } else {
      downExcit[depopulate] = populate;
    }
  }

  xmlNode getHamiltonianNode() const;
  xmlNode getIonPtclSetNode() const;
  xmlNode getElectronPtclSetNode() const;
  xmlNode getWfnNode(int twistnum = 0, const location tw = location()) const;
  xmlNode getSimulationCellNode() const;
};

#endif
