#include "qmcSysDescription.h"
#include "qmcInputGenerator.h"
#include "observables.h"
#include "jastrows.h"
using namespace std;

xmlNode qmcSysDescription::getHamiltonianNode() const {
  xmlNode nd;
  nd.name = "hamiltonian";
  nd.addAttribute("name", hamname);
  nd.addAttribute("type", "generic");
  nd.addAttribute("target", epsetname);
  
  for (int i = 0; i < observables.size(); i++) {
    xmlNode on = observables[i]->createXmlNode();
    nd.addChild(on);
  }
  return nd;
}

xmlNode qmcSysDescription::getIonPtclSetNode() const {
  xmlNode nd;
  nd.name = "particleset";
  nd.addAttribute("name", ipsetname);
  nd.addAttribute("size", numAt);

  for (int i = 0; i < getNumTypes(); i++) {
    xmlNode& groupNode = nd.addChild();
    groupNode.name = "group";
    const string atName = getAtName(i);
    groupNode.addAttribute("name", atName);

    groupNode.addParameterChild("charge", getCharge(atName));
    groupNode.addParameterChild("valence", getValence(atName));
    groupNode.addParameterChild("atomicnumber", getAtomicNumber(atName));
    groupNode.addParameterChild("mass", getMass(atName));
    if (isHybrid == 1) {
      groupNode.addParameterChild("cutoff_radius", getCutoffRadius(atName));
      groupNode.addParameterChild("spline_radius", getSplineRadius(atName));
      groupNode.addParameterChild("spline_npoints", getSplinePts(atName));
      groupNode.addParameterChild("lmax", getLmax(atName));
				
    }
    
    xmlNode& attribNode = groupNode.addChild();
    attribNode.name = "attrib";
    attribNode.addAttribute("name", "position");
    attribNode.addAttribute("dataType", "posArray");
    attribNode.addAttribute("condition", 1);

    stringstream ss;
    for (int j = 0; j < getPosVec(atName).size(); j++) {
      double locx = getPosVec(atName)[j][0];
      double locy = getPosVec(atName)[j][1];
      double locz = getPosVec(atName)[j][2];
      ss << locx << "   " << locy << "   " << locz << endl;
    }
    attribNode.value = ss.str();
  }
  return nd;
}

xmlNode qmcSysDescription::getElectronPtclSetNode() const {
  xmlNode nd;
  nd.name = "particleset";
  nd.addAttribute("name", epsetname);
  nd.addAttribute("random", "yes");
  nd.addAttribute("randomsrc", ipsetname);

  if (nelup > 0) {
    xmlNode upNode = nd.addChild();
    upNode.name = "group";
    upNode.addAttribute("name", "u");
    upNode.addAttribute("size", nelup);
    upNode.addParameterChild("charge", -1);
    upNode.addParameterChild("mass", 1);
  }

  if (neldn > 0) {
    xmlNode& dnNode = nd.addChild();
    nd.addChild(dnNode);
    dnNode.name = "group";
    dnNode.addAttribute("name", "d");
    dnNode.addAttribute("size", neldn);
    dnNode.addParameterChild("charge", -1);
    dnNode.addParameterChild("mass", 1);
  }

  return nd;
}

xmlNode qmcSysDescription::getWfnNode(int twistnum, const location tw) const {
  xmlNode nd;
  nd.name = "wavefunction";
  nd.addAttribute("name", wfnname);
  nd.addAttribute("target", epsetname);

  xmlNode& detsetNode = nd.addChild();
  detsetNode.name = "determinantset";
  detsetNode.addAttribute("type", "einspline");
  detsetNode.addAttribute("href", h5name);
  detsetNode.addAttribute("meshfactor", meshfactor);
  detsetNode.addAttribute("source", ipsetname);
  if (useDpCoeffs == 1) {
    detsetNode.addAttribute("precision", "double");
  } else {
    detsetNode.addAttribute("precision", "single");
  }
  if (useGPU == 1) {
    detsetNode.addAttribute("gpu", useGPU);
  }
  if (useTruncate == 1) {
    detsetNode.addAttribute("truncate", useTruncate);
    detsetNode.addAttribute("buffer", bufferSize);
  }
  detsetNode.addAttribute("twistnum", twistnum);
  if (twistnum == -1) {
    stringstream ss;
    ss << tw[0] << "  " << tw[1] << "  " << tw[2];
    detsetNode.addAttribute("twist", ss.str());
  }

  xmlNode& slaterNode = detsetNode.addChild();
  slaterNode.name = "slaterdeterminant";
  
  if (nelup > 0) {
    xmlNode& detNode = slaterNode.addChild();
    detNode.name = "determinant";
    detNode.addAttribute("id", "updet");
    detNode.addAttribute("size", nelup);
    detNode.addAttribute("ref", "updet");

    xmlNode& occNode = detNode.addChild();
    occNode.name = "occupation";
    occNode.addAttribute("spindataset", "0");
    if (upExcit.size() == 0) {
      occNode.addAttribute("mode", "ground");
    } else {
      occNode.addAttribute("mode", "excited");
      stringstream ss2;
      for (std::map<int,int>::const_iterator it = upExcit.begin(); it != upExcit.end(); ++it) {
	ss2 << "-" << it->first << "  ";
      }
      for (std::map<int,int>::const_iterator it = upExcit.begin(); it != upExcit.end(); ++it) {
	ss2 << it->second << "  ";
      }
      occNode.value = ss2.str();
    }
  }

  if (neldn > 0) {
    xmlNode& detNode = slaterNode.addChild();
    detNode.name = "determinant";
    detNode.addAttribute("id", "downdet");
    detNode.addAttribute("size", neldn);
    detNode.addAttribute("ref", "downdet");

    xmlNode& occNode = detNode.addChild();
    occNode.name = "occupation";
    int sDataSetNum = 0;
    if (isSpinDependent) {
      sDataSetNum = 1;
    }
    occNode.addAttribute("spindataset", sDataSetNum);
    if (downExcit.size() == 0) {
      occNode.addAttribute("mode", "ground");
    } else {
      occNode.addAttribute("mode", "excited");
      stringstream ss2;
      for (std::map<int,int>::const_iterator it = downExcit.begin(); it != downExcit.end(); ++it) {
	ss2 << "-" << it->first << "  ";
      }
      for (std::map<int,int>::const_iterator it = downExcit.begin(); it != downExcit.end(); ++it) {
	ss2 << it->second << "  ";
      }
      occNode.value = ss2.str();
    }
  }

  // add jastrow factors
  for (int i = 0; i < jastrows.size(); i++) {
    xmlNode jasNode = jastrows[i]->createXmlNode();
    nd.addChild(jasNode);
  }
 
  return nd;
}

xmlNode qmcSysDescription::getSimulationCellNode() const {
  xmlNode nd;
  nd.name = "simulationcell";
  
  stringstream ss;
  ss << ptv[0] << "  " << ptv[1] << "  " << ptv[2] << endl;
  ss << ptv[3] << "  " << ptv[4] << "  " << ptv[5] << endl;
  ss << ptv[6] << "  " << ptv[7] << "  " << ptv[8] << endl;
  nd.addParameterChild("lattice", ss.str());

  stringstream ss2;
  for (int i = 0; i < 3; i++) {
    if (bconds[i] == 1) {
      ss2 << "p ";
    } else {
      ss2 << "n ";
    }
  } 
  nd.addParameterChild("bconds", ss2.str());  
  nd.addParameterChild("LR_dim_cutoff", lr_dim_cutoff);
  
  return nd;
}
