#include "xmlRep.h"
#include "qmcActions.h"
using namespace std;

vmc::vmc(const xmlNode& xn) {
  stringstream ss;
  for (int i = 0; i < xn.getNumAttributes(); i++) {
    const string atName = xn.attributes[i].name;
    const string atValue = xn.attributes[i].value;

    if (atName == "checkpoint") {
      ss << atValue;
      ss >> checkpoint;
    } else if (atName == "gpu") {
      if (atValue == "yes") {
	useGPU = 1;
      } else {
	useGPU = 0;
      }
    }
  }

  for (int i = 0; i < xn.getNumChildren(); i++) {
    const xmlNode& cn = xn.children[i];
    for (int j = 0; j < cn.getNumAttributes(); i++) {
      if (cn.attributes[j].name == "name") {
	const string value = cn.attributes[j].value;
	ss << cn.value;
	if (value == "walkers") {
	  ss >> vmcWalkers;
	} else if (value == "samplesperthread") {
	  ss >> samplesPerThread;
	} else if (value == "samples") {
	  ss >> targetPop;
	} else if (value == "substeps") {
	  ss >> numSubSteps;
	} else if (value == "minimumsamples") {
	  ss >> minimumTargetSamples;
	} else if (value == "warmupSteps") {
	  ss >> warmupSteps;
	} else if (value == "blocks") {
	  ss >> vmcBlocks;
	} else if (value == "timestep") {
	  ss >> timestep;
	} else if (value == "useDrift") {
	  if (cn.value == "yes") {
	    useDrift = 1;
	  } else {
	    useDrift = 0;
	  }
	}
      }
    }
  }
}
	
xmlNode vmc::createXmlNode() const {
  xmlNode vmcNode;
  vmcNode.name = "qmc";
  vmcNode.addAttribute("method", "vmc");
  vmcNode.addAttribute("move", "pbyp");
  vmcNode.addAttribute("checkpoint", checkpoint);
  vmcNode.addYesNoAttribute("gpu", useGPU);
  
  xmlNode estNode = vmcNode.addChild();
  estNode.name="estimator";
  estNode.addAttribute("name", "LocalEnergy");
  estNode.addAttribute("hdf", "no");
  
  vmcNode.addParameterChild("walkers", vmcWalkers);
  if (samplesPerThread > 0) {
    vmcNode.addParameterChild("samplesperthread", samplesPerThread);
  } else {
    vmcNode.addParameterChild("samples", targetPop);
  }
  vmcNode.addParameterChild("stepsbetweensamples", 1);
  vmcNode.addParameterChild("substeps", numSubSteps);
  if (minimumTargetSamples > 0) {
    vmcNode.addParameterChild("minimumsamples", minimumTargetSamples);
  }
  vmcNode.addParameterChild("warmupSteps", warmupSteps);
  vmcNode.addParameterChild("blocks", vmcBlocks);
  vmcNode.addParameterChild("timestep", timestep);
  vmcNode.addYesNoParameterChild("usedrift", useDrift);

  return vmcNode;
}

dmc::dmc(const xmlNode& xn) {
  stringstream ss;
  for (int i = 0; i < xn.getNumAttributes(); i++) {
    const string atName = xn.attributes[i].name;
    const string atValue = xn.attributes[i].value;
    
    if (atName == "checkpoint") {
      ss << atValue;
      ss >> checkpoint;
    } else if (atName == "gpu") {
      if (atValue == "yes") {
	useGPU = 1;
      } else {
	useGPU = 0;
      }
    } else if (atName == "branching_cutoff_scheme") {
      branchCutoffScheme = 1;
      if (atValue == "YL") {
	branchCutoffScheme = 2;
      } else if (atValue == "ZSGMA") {
	branchCutoffScheme = 3;
      } else if (atValue == "UNR") {
	branchCutoffScheme = 4;
      }
    }
  }

  // handle children here
    for (int i = 0; i < xn.getNumChildren(); i++) {
    const xmlNode& cn = xn.children[i];
    for (int j = 0; j < cn.getNumAttributes(); i++) {
      if (cn.attributes[j].name == "name") {
	const string value = cn.attributes[j].value;
	ss << cn.value;
	if (value == "targetwalkers") {
	  ss >> targetPop;
	} else if (value == "minimumtargetwalkers") {
	  ss >> minTargetPop;
	} else if (value == "reconfiguration") {
	  if (cn.value == "yes") {
	    useReconfiguration = 1;
	  } else {
	    useReconfiguration = 0;
	  }
	} else if (value == "warmupSubsteps") {
	  ss >> warmupSteps;
	} else if (value == "timestep") {
	  ss >> dmcTstep;
	} else if (value == "steps") {
	  ss >> dmcStepsPerBlock;
	} else if (value == "blocks") {
	  ss >> dmcBlocks;
	} else if (value == "nonlocalmoves") {
	  if (cn.value == "yes") {
	    useTmoves = 1;
	  } else {
	    useTmoves = 0;
	  }
	}
      }
    }
  }
}

xmlNode dmc::createXmlNode() const {
  xmlNode dmcNode;
  dmcNode.name = "qmc";
  dmcNode.addAttribute("method", "dmc");
  dmcNode.addAttribute("move", "pbyp");
  dmcNode.addAttribute("checkpoint", checkpoint);
  dmcNode.addYesNoAttribute("gpu", useGPU);
  if (branchCutoffScheme == 1) {
    dmcNode.addAttribute("branching_cutoff_scheme", "classic");
  } else if (branchCutoffScheme == 2) {
    dmcNode.addAttribute("branching_cutoff_scheme", "YL");
  } else if (branchCutoffScheme == 3) {
    dmcNode.addAttribute("branching_cutoff_scheme", "ZSGMA");
  } else if (branchCutoffScheme == 4) {
    dmcNode.addAttribute("branching_cutoff_scheme", "UNR");
  }
  
  xmlNode& estNode = dmcNode.addChild();
  estNode.name="estimator";
  estNode.addAttribute("name", "LocalEnergy");
  estNode.addAttribute("hdf", "no");

  if (minTargetPop > 0) {
    dmcNode.addAttribute("minimumtargetwalkers", minTargetPop);
  } else {
    dmcNode.addAttribute("targetwalkers", targetPop);
  }
  dmcNode.addYesNoAttribute("reconfiguration", useReconfiguration);
  dmcNode.addAttribute("warmupSteps", warmupSteps);
  dmcNode.addAttribute("timestep", dmcTstep);
  dmcNode.addAttribute("steps", dmcStepsPerBlock);
  dmcNode.addAttribute("blocks", dmcBlocks);
  dmcNode.addYesNoAttribute("nonlocalmoves", useTmoves);
  
  return dmcNode;
}

optimize::optimize(const xmlNode& xn) {
  stringstream ss;
  const xmlNode* optNode = &xn;
  numOptLoops = 0;
  if (xn.name == "loop") {
    optNode = &(xn.children[0]);
    for (int i = 0; i < xn.getNumAttributes(); i++) {
      const string atName = xn.attributes[i].name;
      const string atValue = xn.attributes[i].value;

      if (atName == "max") {
	ss << atValue;
	ss >> numOptLoops;
      }
    }
  }

  // handling the optimization section here
  for (int i = 0; i < optNode->getNumAttributes(); i++) {
    const string atName = optNode->attributes[i].name;
    const string atValue = optNode->attributes[i].value;
 
    if (atName == "checkpoint") {
      ss << atValue;
      ss >> checkpoint;   
    } else if (atName == "gpu") {
      if (atValue == "yes") {
	useGPU = 1;
      } else {
	useGPU = 0;
      }
    } 
  }

  ////////////////////////////////////

  // handle children here
  for (int i = 0; i < optNode->getNumChildren(); i++) {
    const xmlNode* cn = &(optNode->children[i]);
    for (int j = 0; j < cn->getNumAttributes(); i++) {
      if (cn->attributes[j].name == "name") {
	const string value = cn->attributes[j].value;
	ss << cn->value;
	if (value == "walkers") {
	  ss >> walkers;
	} else if (value == "samples") {
	  ss >> numOptSamples;
	} else if (value == "substeps") {
	  ss >> numSubsteps;
	} else if (value == "blocks") {
	  ss >> vmcBlocks;
	} else if (value == "warmupSteps") {
	  ss >> warmupSteps;
	} else if (value == "timestep") {
	  ss >> vmcTstep;
	} else if (value == "usedrift") {
	  if (cn->value == "yes") {
	    useDrift = 1;
	  } else {
	    useDrift = 0;
	  }
	} else if (value == "energy") {
	  ss >> enWeight;
	} else if (value == "reweightedvariance") {
	  useReweightedVariance = 1;
	} else if (value == "unreweightedvariance") {
	  useReweightedVariance = 0;
	} else if (value == "nonlocalpp") {
	  if (cn->value == "yes") {
	    useNonlocal = 1;
	  } else {
	    useNonlocal = 0;
	  }
	} else if (value == "minwalkers") {
	  ss >> minWalkers;
	} else if (value == "MinMethod") {
	  if (cn->value == "OneShiftOnly") {
	    minMethod = 1;
	  } else if (cn->value == "Quartic") {
	    minMethod = 2;
	  } else if (cn->value == "adaptive") {
	    minMethod = 3;
	  } else if (cn->value == "rescale") {
	    minMethod = 4;
	  }
	} else if (value == "beta") {
	  ss >> beta;
	} else if (value == "exp0") {
	  ss >> exp0;
	} else if (value == "bigChange") {
	  ss >> bigChange;
	} else if (value == "alloweddifference") {
	  ss >> allowedDifference;
	} else if (value == "stepsize") {
	  ss >> stepsize;
	} else if (value == "stabilizerscale") {
	  ss >> stabilizerScale;
	} else if (value == "nstabilizers") {
	  ss >> nstabilizers;
	} else if (value == "max_its") {
	  ss >> maxIts;
	}
      }
    }
  }
}

xmlNode optimize::createXmlNode() const {
  xmlNode optNode;
  optNode.name="qmc";
  
  optNode.addAttribute("method", "linear");
  optNode.addAttribute("move", "pbyp");
  optNode.addAttribute("checkpoint", checkpoint);
  optNode.addYesNoAttribute("gpu", useGPU);
  
  xmlNode& estNode = optNode.addChild();
  estNode.name="estimator";
  estNode.addAttribute("name", "LocalEnergy");
  estNode.addAttribute("hdf", "no");

  optNode.addParameterChild("walkers", walkers);
  optNode.addParameterChild("samples", numOptSamples);
  optNode.addParameterChild("stepsbetweensamples", 1);
  optNode.addParameterChild("substeps", numSubsteps);
  optNode.addParameterChild("blocks", vmcBlocks);
  optNode.addParameterChild("warmupSteps", warmupSteps);
  optNode.addParameterChild("timestep", vmcTstep);
  optNode.addYesNoParameterChild("usedrift", useDrift);

  double varWeight = 1 - enWeight;
  xmlNode& enCostNode = optNode.addChild();
  enCostNode.name = "cost";
  enCostNode.addAttribute("name", "energy");
  enCostNode.setValue(enWeight);
  
  xmlNode& varCostNode = optNode.addChild();
  varCostNode.name = "cost";
  if (useReweightedVariance == 1) {
    varCostNode.addAttribute("name", "reweightedvariance");
  } else {
    varCostNode.addAttribute("name", "unreweightedvariance");
  }
  varCostNode.setValue(varWeight);

  optNode.addYesNoParameterChild("nonlocalpp", useNonlocal);
  optNode.addParameterChild("minwalkers", minWalkers);
  
  if (minMethod == 1) {
    optNode.addParameterChild("MinMethod", "OneShiftOnly");
  } else if (minMethod == 2 || minMethod == 4) {
    if (minMethod == 2) {
      optNode.addParameterChild("MinMethod", "Quartic");
    } else if (minMethod == 4) {
      optNode.addParameterChild("MinMethod", "rescale");
    }
    optNode.addParameterChild("beta", beta);
    optNode.addParameterChild("exp0", exp0);
    optNode.addParameterChild("bigChange", bigChange);
    optNode.addParameterChild("alloweddifference", allowedDifference);
    optNode.addParameterChild("stepsize", stepsize);
    optNode.addParameterChild("stabilizerscale", stabilizerScale);
    optNode.addParameterChild("nstabilizers", nstabilizers);
    optNode.addParameterChild("max_its", maxIts);
  } else if (minMethod == 3) {
    optNode.addParameterChild("MinMethod", "adaptive");
  }


  // If we are doing a loop encapsulate the optimization in a loop node and return
  if (numOptLoops > 0) {
    xmlNode loopNode; 
    loopNode.name = "loop";
    loopNode.addAttribute("max", numOptLoops);
    loopNode.addChild(optNode);
    return loopNode;
  }
  // If no loop, directly return the qmc action
  return optNode;
}
