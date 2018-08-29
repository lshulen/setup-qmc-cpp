#include <sstream>
#include "jastrows.h"
#include "xmlRep.h"
using namespace std;

string jastrow::getOptString() const {
  string optString = "yes";
  if (isOptimizable == 0) {
    optString = "no";
  } 
  return optString;
} 

string jastrow::getWfnString(int indentSize) const {
  xmlNode xn = createXmlNode();
  return xn.getString();
}

xmlNode J1Spline::createXmlNode() const {
  xmlNode jasNode;
  jasNode.name = "jastrow";
  jasNode.addAttribute("type", type);
  jasNode.addAttribute("name", "J1Spline");
  jasNode.addAttribute("function", "bspline");
  jasNode.addAttribute("source", source);
  jasNode.addAttribute("print", "yes");

  for (int i = 0; i < elementType.size(); i++) {
    xmlNode& corrNode = jasNode.addChild();
    corrNode.name = "correlation";
    corrNode.addAttribute("elementType", elementType[i]);
    corrNode.addAttribute("size", coeffList[i].size());
    corrNode.addAttribute("cusp", cuspValue[i]);
    corrNode.addAttribute("rcut", rcut[i]);

    xmlNode& coeffNode = corrNode.addChild();
    coeffNode.addAttribute("id", iD[i]);
    coeffNode.addAttribute("type", "Array");
    coeffNode.addAttribute("optimize", getOptString());
    
    stringstream coeffss;
    for (int j = 0; j < coeffList[i].size(); j++) {
      coeffss << coeffList[i][j] << "  ";
    }
    coeffNode.value = coeffss.str();
  }
  return jasNode;
}


void J1Spline::addCenter(const string& eType, int numCoeffs, int cusp, const string& id, double rc) {
  elementType.push_back(eType);
  vector<double> tempVec;
  for (int i = 0; i< numCoeffs; i++) {
    tempVec.push_back(0.0);
  }
  coeffList.push_back(tempVec);
  cuspValue.push_back(cusp);
  iD.push_back(id);
  rcut.push_back(rc);
};
		
J1Spline::J1Spline(const J1Spline& js) {
  type = js.type;
  isIncluded = js.isIncluded;
  isOptimizable = js.isOptimizable;

  source = js.source;
  elementType = js.elementType;
  cuspValue = js.cuspValue;
  iD = js.iD;
  rcut = js.rcut;
  for (int i = 0; i < js.coeffList.size(); i++) {
    coeffList.push_back(js.coeffList[i]);
  }
}

xmlNode J1Pade::createXmlNode() const {
  vector<string> names;
  names.push_back("A");
  names.push_back("B");
  names.push_back("C");

  xmlNode jasNode;
  jasNode.name = "jastrow";
  jasNode.addAttribute("type", type);
  jasNode.addAttribute("name", "J1Pade");
  jasNode.addAttribute("function", "pade2");
  jasNode.addAttribute("source", source);
  jasNode.addAttribute("print", "yes");
  
  for (int i = 0; i < elementType.size(); i++) {
    xmlNode& correlationNode = jasNode.addChild();
    correlationNode.name = "correlation";
    correlationNode.addAttribute("elementType", elementType[i]);
    
    for (int j = 0; j < 3; j++) {
      xmlNode& paramNode = correlationNode.addChild();
      paramNode.name = "parameter";
      paramNode.addAttribute("id", iD[i] + names[j]);
      paramNode.addAttribute("name", names[j]);
      paramNode.addAttribute("optimize", getOptString());
      paramNode.value = coeffList[i][j];
    }
  }
  return jasNode;
}

void J1Pade::addCenter(const string& eType, const string& id, double A, double B, double C) {
  elementType.push_back(eType);
  vector<double> tempVec;
  tempVec.push_back(A);
  tempVec.push_back(B);
  tempVec.push_back(C);
  coeffList.push_back(tempVec);
  iD.push_back(id);
};

J1Pade::J1Pade(const J1Pade& jp) {
  type = jp.type;
  isIncluded = jp.isIncluded;
  isOptimizable = jp.isOptimizable;
  
  source = jp.source;
  elementType = jp.elementType;
  iD = jp.iD;
  for (int i = 0; i < jp.coeffList.size(); i++) {
    coeffList.push_back(jp.coeffList[i]);
  }
}
 
J2Spline::J2Spline(int spinDep, int numCoeff, double rc) {
  type = "Two-Body";
  isIncluded = 1;
  isOptimizable = 1;
  isSpinDep = spinDep;
  
  std::vector<double> tempVec;
  for (int i = 0; i < numCoeff; i++) {
    tempVec.push_back(0.0);
  }
  iD.push_back("uu");
  rcut.push_back(rc);
  coeffList.push_back(tempVec);
  if (isSpinDep == 1) {
    iD.push_back("ud");
    rcut.push_back(rc);
    coeffList.push_back(tempVec);
  }
}

J2Spline::J2Spline(const J2Spline& js) {
  type = js.type;
  isIncluded = js.isIncluded;
  isOptimizable = js.isOptimizable;
  isSpinDep = js.isSpinDep;
    
  iD = js.iD;
  rcut = js.rcut;
  for (int i = 0; i < js.coeffList.size(); i++) {
    coeffList.push_back(js.coeffList[i]);
  }
}

xmlNode J2Spline::createXmlNode() const {
  xmlNode jasNode;
  jasNode.name = "jastrow";
  jasNode.addAttribute("name", "J2Spline");
  jasNode.addAttribute("type", type);
  jasNode.addAttribute("function", "bspline");
  jasNode.addAttribute("print", "yes");

  vector<string> secondSpecies;
  secondSpecies.push_back("u");
  secondSpecies.push_back("d");
  int numSpecies = 1;
  if (isSpinDep == 1) {
    numSpecies = 2;
  }

  for (int i = 0; i < numSpecies; i++) {
    xmlNode& corrNode = jasNode.addChild();
    corrNode.name = "correlation";
    corrNode.addAttribute("speciesA", "u");	      
    corrNode.addAttribute("speciesB", secondSpecies[i]); 
    corrNode.addAttribute("size", coeffList[i].size());  
  
    xmlNode& coeffNode = corrNode.addChild();
    coeffNode.name = "coefficients";
    coeffNode.addAttribute("id", iD[i]);		    
    coeffNode.addAttribute("type", "Array");	    
    coeffNode.addAttribute("optimize", getOptString());

    stringstream coeffStr;
    for (int j = 0; j < coeffList[i].size(); j++) {
      coeffStr << coeffList[i][j] << "   ";
    }
    coeffNode.value = coeffStr.str();
  }
  return jasNode;
}


J2Pade::J2Pade(int spinDep) {
  type = "Two-Body";
  isIncluded = 1;
  isOptimizable = 1;
  isSpinDep = spinDep;
  
  std::vector<double> tempVec;
  for (int i = 0; i < 1; i++) {
    tempVec.push_back(0.0);
  }
  iD.push_back("uu");
  coeffList.push_back(tempVec);
  if (isSpinDep == 1) {
    iD.push_back("ud");
    coeffList.push_back(tempVec);
  }
}

J2Pade::J2Pade(const J2Pade& jp) {
  type = jp.type;
  isIncluded = jp.isIncluded;
  isOptimizable = jp.isOptimizable;
  isSpinDep = jp.isSpinDep;
    
  iD = jp.iD;
  for (int i = 0; i < jp.coeffList.size(); i++) {
    coeffList.push_back(jp.coeffList[i]);
  }
}

xmlNode J2Pade::createXmlNode() const {
  xmlNode jasNode;
  jasNode.name = "jastrow";
  jasNode.addAttribute("name", "J2Spline");
  jasNode.addAttribute("type", type);
  jasNode.addAttribute("function", "pade");
  jasNode.addAttribute("print", "yes");

  vector<string> speciesBNames;
  speciesBNames.push_back("u");
  if (isSpinDep == 1) {
    speciesBNames.push_back("d");
  }
  
  for (int i = 0; i < speciesBNames.size(); i++) {
    xmlNode& corrNode = jasNode.addChild();
    corrNode.name = "correlation";
    corrNode.addAttribute("speciesA", "u");
    corrNode.addAttribute("speciesB", speciesBNames[i]);

    xmlNode& varNode = corrNode.addChild();
    varNode.name = "var";
    varNode.addAttribute("id", iD[i] + "_b");
    varNode.addAttribute("name", "B");
    varNode.addAttribute("optimize", getOptString());
    varNode.value = coeffList[i][0];
  }
  return jasNode;
}

xmlNode J2kSpace::createXmlNode() const {
  xmlNode jasNode;
  jasNode.name = "jastrow";
  jasNode.addAttribute("name", "myk");
  jasNode.addAttribute("type", type);
  jasNode.addAttribute("print", "yes");

  xmlNode& corrNode = jasNode.addChild();
  corrNode.name = "correlation";
  corrNode.addAttribute("type", "Two-Body");
  corrNode.addAttribute("kc", kc);
  corrNode.addAttribute("symmetry", "isotropic");

  xmlNode& coeffNode = corrNode.addChild();
  corrNode.addChild(coeffNode);
  coeffNode.name = "coefficients";
  coeffNode.addAttribute("id", iD);
  coeffNode.addAttribute("type", "Array");
  coeffNode.addAttribute("optimize", getOptString());

  if (coeffs.size() > 0) {
    stringstream ss;
    for (int i = 0; i < coeffs.size(); i++) {
      ss << coeffs[i] << "  ";
    }
    coeffNode.value = ss.str();
  }
  return jasNode;
}
    

void J3::addCenter(const std::string& eType, double rc, int es, int is) {
  elemType.push_back(eType);
  stringstream ss;
  ss << "uu" << eType;
  iD.push_back(ss.str());
  rcut.push_back(rc);
  esize.push_back(es);
  isize.push_back(is);
  std::vector<double> vecTmp;
  coeffList.push_back(vecTmp);

  if(isSpinDep == 1) {
    elemType.push_back(eType);
    stringstream ss;
    ss << "ud" << eType;
    iD.push_back(ss.str());
    rcut.push_back(rc);
    esize.push_back(es);
    isize.push_back(is);
    std::vector<double> vecTmp;
    coeffList.push_back(vecTmp);
  }
}

J3::J3(const J3& j3) {
  type = j3.type;
  isIncluded = j3.isIncluded;
  isOptimizable = j3.isOptimizable;
  isSpinDep = j3.isSpinDep;
  source = j3.source;

  elemType = j3.elemType;
  iD = j3.elemType;
  rcut = j3.rcut;
  esize = j3.esize;
  isize = j3.isize;
  for (int i = 0; i < j3.coeffList.size(); i++) {
    coeffList.push_back(j3.coeffList[i]);
  }
}


xmlNode J3::createXmlNode() const {
  xmlNode jasNode;
  jasNode.name = "jastrow";
  jasNode.addAttribute("name", "J3");
  jasNode.addAttribute("type", type);
  jasNode.addAttribute("function", "polynomial");
  jasNode.addAttribute("source", source);
  jasNode.addAttribute("print", "yes");

  for (int elemNum = 0; elemNum < elemType.size(); elemNum++) {
    xmlNode& corrNode = jasNode.addChild();
    corrNode.name = "correlation";
    corrNode.addAttribute("ispecies", elemType[elemNum]);
    corrNode.addAttribute("especies", "u");
    corrNode.addAttribute("isize", isize[elemNum]);
    corrNode.addAttribute("esize", esize[elemNum]);
    corrNode.addAttribute("rcut", rcut[elemNum]);

    xmlNode& coeffNode = corrNode.addChild();
    coeffNode.name = "coefficients";
    coeffNode.addAttribute("id", iD[elemNum]);
    coeffNode.addAttribute("type", "Array");
    coeffNode.addAttribute("optimize", getOptString());
    
    if (coeffList[elemNum].size() > 0) {
      stringstream ss;
      for (int i = 0; i < coeffList[elemNum].size(); i++) {
	ss << coeffList[elemNum][i] << "  ";
      }
      coeffNode.value = ss.str();
    }

    if (isSpinDep == 1) {
      elemNum++;
      xmlNode& corrNode2 = jasNode.addChild();
      corrNode2.name = "correlation";
      corrNode2.addAttribute("ispecies", elemType[elemNum]);
      corrNode2.addAttribute("especies1", "u");
      corrNode2.addAttribute("especies1", "d");
      corrNode2.addAttribute("isize", isize[elemNum]);
      corrNode2.addAttribute("esize", esize[elemNum]);
      corrNode2.addAttribute("rcut", rcut[elemNum]);

      xmlNode& coeffNode2 = corrNode2.addChild();
      coeffNode2.name = "coefficients";
      coeffNode2.addAttribute("id", iD[elemNum]);
      coeffNode2.addAttribute("type", "Array");
      coeffNode2.addAttribute("optimize", getOptString());
    
      if (coeffList[elemNum].size() > 0) {
	stringstream ss;
	for (int i = 0; i < coeffList[elemNum].size(); i++) {
	  ss << coeffList[elemNum][i] << "  ";
	}
	coeffNode2.value = ss.str();
      }
    }
  }
  return jasNode;
}
