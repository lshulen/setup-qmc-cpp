#include <sstream>
#include "xmlRep.h"
#include "observables.h"
using namespace std;

string observable::getObsString(int indentSize) const {
  xmlNode xn = createXmlNode();
  return xn.getString();
}

void ionInteractions::addPP(const string& eType, const string& fname) {
  elementType.push_back(eType);
  isAe.push_back(0);
  zval.push_back(0);
  ppName.push_back(fname);
}

void ionInteractions::addAE(const string& eType, int val) {
  elementType.push_back(eType);
  isAe.push_back(1);
  zval.push_back(val);
  string blank;
  ppName.push_back(blank);
}

ionInteractions::ionInteractions(const ionInteractions& ii) {
  name = ii.name;
  source = ii.source;
  wfn = ii.wfn;
  elementType = ii.elementType;
  isAe = ii.isAe;
  zval = ii.zval;
  ppName = ii.ppName;
}

xmlNode ionInteractions::createXmlNode() const {
  xmlNode pairPotNode;
  pairPotNode.name = "paripot";
  pairPotNode.addAttribute("type", "pseudo");
  pairPotNode.addAttribute("name", name);
  pairPotNode.addAttribute("source", source);
  pairPotNode.addAttribute("wavefunction", wfn);
  pairPotNode.addAttribute("format", "xml");

  // do all all electron elements first
  for (int i = 0; i < isAe.size(); i++) {
    if(isAe[i] == 1) {
      xmlNode& pseudoNode = pairPotNode.addChild();
      pseudoNode.name = "pseudo";
      pseudoNode.addAttribute("elementType", elementType[i]);

      xmlNode& headerNode = pseudoNode.addChild();
      headerNode.addAttribute("symbol", elementType[i]);
      headerNode.addAttribute("atomic-number", zval[i]);
      headerNode.addAttribute("zval", zval[i]);

      xmlNode& localNode = headerNode.addChild();
      localNode.name = "local";

      xmlNode& gridNode = localNode.addChild();
      gridNode.name = "grid";
      gridNode.addAttribute("type", "linear");
      gridNode.addAttribute("ri", "0.0");
      gridNode.addAttribute("rf", "4.0");
      gridNode.addAttribute("npts", "128");
      gridNode.isSelfClosing=1;
    }
  }
  // now add all pseudopotential nodes
  for (int i = 0; i < isAe.size(); i++) {
    if(isAe[i] == 0) {
      xmlNode& pseudoNode = pairPotNode.addChild();
      pseudoNode.name = "pseudo";
      pseudoNode.addAttribute("elementType", elementType[i]);
      pseudoNode.addAttribute("href", ppName[i]);
      pseudoNode.isSelfClosing=1;
    }
  }
  return pairPotNode;
}

chiesa::chiesa(const string& ss, const string& wfns) : source(ss), wfn(wfns) {
  source = ss;
  wfn = wfns;
  name = "KEcorr";
}

chiesa::chiesa(const chiesa& co) {
  source = co.source;
  wfn = co.wfn;
  name = co.name;
}

xmlNode chiesa::createXmlNode() const {
  xmlNode estimatorNode;
  estimatorNode.name = "estimator";
  estimatorNode.addAttribute("type", "chiesa");
  estimatorNode.addAttribute("name", name);
  estimatorNode.addAttribute("source", source);
  estimatorNode.addAttribute("psi", wfn);
  estimatorNode.isSelfClosing=1;
  return estimatorNode;
}

mpc::mpc(const string& ss, const string& ts, double ec, int ip) {
  source = ss;
  target = ts;
  ecut = ec;
  isPhysical = ip;
  name = "mpc";
}

mpc::mpc(const mpc& mo) {
  source = mo.source;
  target = mo.target;
  ecut = mo.ecut;
  isPhysical = mo.isPhysical;
  name = mo.name;
}

xmlNode mpc::createXmlNode() const {
  string physString;
  if (isPhysical == 1) {
    physString = "true";
  } else {
    physString = "false";
  }

  xmlNode pairpotNode;
  pairpotNode.name = "pairpot";
  pairpotNode.addAttribute("type", "MPC");
  pairpotNode.addAttribute("name", name);
  pairpotNode.addAttribute("source", source);
  pairpotNode.addAttribute("target", target);
  pairpotNode.addAttribute("ecut", ecut);
  pairpotNode.addAttribute("physical", physString);
  pairpotNode.isSelfClosing=1;
  return pairpotNode;
}

spindensity::spindensity(int nx, int ny, int nz) {
  name = "SpinDensity";
  isGridOrDr = 1;
  isCornerOrCenter = 0;
  hasCell = 0;

  grid.push_back(nx);
  grid.push_back(ny);
  grid.push_back(nz);

  for (int i = 0; i < 3; i++) {
    dr.push_back(0.0);
    origin.push_back(0.0);
  }
}

spindensity::spindensity(double drx, double dry, double drz) {
  name = "SpinDensity";
  isGridOrDr = 0;
  isCornerOrCenter = 0;
  hasCell = 0;

  dr.push_back(drx);
  dr.push_back(dry);
  dr.push_back(drz);

  for (int i = 0; i < 3; i++) {
    grid.push_back(0);
    origin.push_back(0.0);
  }
}

spindensity::spindensity(const spindensity& sd) {
  name = sd.name;
  isGridOrDr = sd.isGridOrDr;
  isCornerOrCenter = sd.isCornerOrCenter;
  hasCell = sd.hasCell;
  
  dr = sd.dr;
  grid = sd.grid;
  origin = sd.origin;
  cell = sd.cell;
}

// note, use 1 for corner and 0 for center
void spindensity::setCell(int centerOrCorner, double ox, double oy, double oz, const threeThreeMat<double>& ci) {
  hasCell = 1;
  isCornerOrCenter = centerOrCorner;
  cell = ci;

  origin[0] = ox;
  origin[1] = oy;
  origin[2] = oz;
}

xmlNode spindensity::createXmlNode() const {
  xmlNode estimatorNode;
  estimatorNode.name = "estimator";
  estimatorNode.addAttribute("type", "spindensity");
  estimatorNode.addAttribute("name", name);
  estimatorNode.addAttribute("report", "yes");

  xmlNode& paramNode = estimatorNode.addChild();
  paramNode.name = "parameter";
  if (isGridOrDr == 1) {
    paramNode.addAttribute("name", "grid");
    stringstream ss;
    ss << grid[0] << "  " << grid[1] << "  " << grid[2];
    paramNode.value = ss.str();
  } else {
    paramNode.addAttribute("name", "dr");
    stringstream ss;
    ss << dr[0] << "  " << dr[1] << "  " << dr[2];
    paramNode.value = ss.str();
  }

  if (hasCell == 1) {
    xmlNode& locNode = estimatorNode.addChild();
    locNode.name = "parameter";
    if (isCornerOrCenter == 1) {
      locNode.addAttribute("name", "corner");
    } else {
      locNode.addAttribute("name", "center");
    }
    stringstream ss2;
    ss2 << origin[0] << "   " << origin[1] << "   " << origin[2];
    locNode.value = ss2.str();

    xmlNode& cellNode = estimatorNode.addChild();
    cellNode.name = "parameter";
    cellNode.addAttribute("name", "cell");
    stringstream ss3;
    ss3 << cell[0] << "   " << cell[1] << "   " << cell[2] << endl;
    ss3 << cell[3] << "   " << cell[4] << "   " << cell[5] << endl;
    ss3 << cell[6] << "   " << cell[7] << "   " << cell[8] << endl;
    cellNode.value = ss3.str();
  }
  return estimatorNode;
}

skall::skall(const string& ss, const string& ts, int aii) {
  name = "Skall";
  source = ss;
  target = ts;
  addIonIon = aii;
}

skall::skall(const skall& ska) {
  name = ska.name;
  source = ska.source;
  target = ska.target;
  addIonIon = ska.addIonIon;
}

xmlNode skall::createXmlNode() const {
  xmlNode estimatorNode;
  estimatorNode.name = "estimator";
  estimatorNode.addAttribute("type", "skall");
  estimatorNode.addAttribute("name", name);
  estimatorNode.addAttribute("source", source);
  estimatorNode.addAttribute("target", target);
  if (addIonIon == 1) {
    estimatorNode.addAttribute("addionion", "yes");
  } else {
    estimatorNode.addAttribute("addionion", "no");
  }
  estimatorNode.isSelfClosing=1;
  
  return estimatorNode;
}

force::force(double rc, int nb, int we, int aii) {
  name = "force";
  rcut = rc;
  nbasis = nb;
  weightexp = we;
  addIonIon = aii;
}

force::force(const force& fo) {
  name = fo.name;
  rcut = fo.rcut;
  nbasis = fo.nbasis;
  weightexp = fo.weightexp;
  addIonIon = fo.addIonIon;
}

xmlNode force::createXmlNode() const {
  xmlNode estimatorNode;
  estimatorNode.name="estimator";
  estimatorNode.addAttribute("type", "Force");
  estimatorNode.addAttribute("name", name);
  estimatorNode.addAttribute("mode", "cep");
  if (addIonIon == 1) {
    estimatorNode.addAttribute("addionion", "yes");
  } else {
    estimatorNode.addAttribute("addionion", "no");
  }

  xmlNode& paramNode1 = estimatorNode.addChild();
  paramNode1.name = "parameter";
  paramNode1.addAttribute("name", "rcut");
  paramNode1.value = rcut;

  xmlNode& paramNode2 = estimatorNode.addChild();
  paramNode2.name = "parameter";
  paramNode2.addAttribute("name", "nbasis");
  paramNode2.value = nbasis;

  xmlNode& paramNode3 = estimatorNode.addChild();
  paramNode3.name = "parameter";
  paramNode3.addAttribute("name", "weightexp");
  paramNode3.value = weightexp;
  
  return estimatorNode;
}

ionIon::ionIon(const string& ss, const string& ts) {
  name="IonIon";
  source = ss;
  target = ts;
}

ionIon::ionIon(const ionIon& ii) {
  name = ii.name;
  source = ii.source;
  target = ii.target;
}

xmlNode ionIon::createXmlNode() const {
  xmlNode pairpotNode;
  pairpotNode.name="pairpot";
  pairpotNode.addAttribute("type", "coulomb");
  pairpotNode.addAttribute("name", name);
  pairpotNode.addAttribute("source", source);
  pairpotNode.addAttribute("target", target);
  pairpotNode.isSelfClosing=1;
  return pairpotNode;
}

elecElec::elecElec(const string& ss, const string& ts) {
  name="ElecElec";
  source = ss;
  target = ts;
}

elecElec::elecElec(const elecElec& ii) {
  name = ii.name;
  source = ii.source;
  target = ii.target;
}

xmlNode elecElec::createXmlNode() const {
  xmlNode pairpotNode;
  pairpotNode.name="pairpot";
  pairpotNode.addAttribute("type", "coulomb");
  pairpotNode.addAttribute("name", name);
  pairpotNode.addAttribute("source", source);
  pairpotNode.addAttribute("target", target);
  pairpotNode.isSelfClosing=1;
  return pairpotNode;
}
