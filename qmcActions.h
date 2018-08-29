#ifndef QMC_ACTIONS_H
#define QMC_ACTIONS_H
#include <vector>
#include <string>

class xmlNode;

class qmcActions {
 public:
  virtual xmlNode createXmlNode() const = 0;
  std::string getMethodString() const;
};


class vmc : public qmcActions {
 private:
  int useGPU;
  int vmcWalkers;
  int samplesPerThread;
  int targetPop;
  int numSubSteps;
  int warmupSteps;
  int minimumTargetSamples;
  int vmcBlocks;
  int timestep;
  int useDrift;
  int checkpoint;
 public:
  vmc(int _useGPU, double _vmcTstep, double _vmcEquilTime, double _vmcDecorrTime,
      int _vmcBlocks, int _vmcWalkers, int _targetPop, int _useDrift,
      int _samplesPerThread = -1, int _minimumTargetSamples = -1, int _checkpoint = -1) :
  useGPU(_useGPU), vmcWalkers(_vmcWalkers), samplesPerThread(_samplesPerThread),
    targetPop(_targetPop), numSubSteps(static_cast<int>(_vmcEquilTime / _vmcTstep + 0.5)),
    warmupSteps(static_cast<int>(_vmcDecorrTime / _vmcTstep + 0.5)), 
    minimumTargetSamples(_minimumTargetSamples), vmcBlocks(_vmcBlocks), timestep(_vmcTstep),
    useDrift(_useDrift), checkpoint(_checkpoint) { ; }
  // vmc(const vmc& vc)  // Not needed because default copy constructor should work
  vmc(const xmlNode& vmcNode);
  xmlNode createXmlNode() const;
};


//branching_cutoff_scheme (classic, YL, ZSGMA, UNR)
//   1 = classic
//   2 = YL
//   3 = ZSGMA
//   4 = UNR
class dmc: public qmcActions {
 private:
  int checkpoint;
  int useGPU;
  int branchCutoffScheme;
  int minTargetPop;
  int targetPop;
  int useReconfiguration;
  int warmupSteps;
  double dmcTstep;
  int dmcStepsPerBlock;
  int dmcBlocks;
  int useTmoves;
 public:
  dmc(int _useGPU, double _dmcTstep, double _dmcEquilTime, double _dmcRunTime, 
      double _dmcBlockTime, int _useTmoves, int _targetPop, int _minTargetPop = 0, 
      int _useReconfiguration = 0, int _branchCutoffScheme = 1, int _checkpoint = -1) :
  checkpoint(_checkpoint), useGPU(_useGPU), branchCutoffScheme(_branchCutoffScheme),
    minTargetPop(_minTargetPop), targetPop(_targetPop), useReconfiguration(_useReconfiguration),
    warmupSteps(static_cast<int>(_dmcEquilTime / _dmcTstep + 0.5)), dmcTstep(_dmcTstep),
    dmcStepsPerBlock(static_cast<int>(_dmcBlockTime / _dmcTstep + 0.5)),
    dmcBlocks( static_cast<int> ( (static_cast<int>((_dmcRunTime + _dmcEquilTime) / _dmcTstep + 0.5))
				  / (static_cast<int>(_dmcBlockTime / _dmcTstep + 0.5)) + 0.5) ),
    useTmoves(_useTmoves) { ; }
  //dmc(const dmc& dmc) // Not needed because default copy constructor should work
  dmc(const xmlNode& xn);
  xmlNode createXmlNode() const;
};

// minMethod (OneShiftOnly, Quartic, adaptive, rescale)
// 1 = OneShiftOnly
// 2 = Quartic
// 3 = adaptive (Not sure what parameters this needs...)
// 4 = rescale (Not sure if this works)
class optimize : public qmcActions {
 private:
   int numOptLoops;
   int checkpoint;
   int useGPU;
   int walkers;
   int numOptSamples;
   int numSubsteps;
   int vmcBlocks;
   int warmupSteps;
   double vmcTstep;
  
   double enWeight;
   int useReweightedVariance;
   int useNonlocal;
   int useDrift;
   int minMethod;
   double minWalkers;
  
  double beta;
  double exp0;
  double bigChange;
  double allowedDifference;
  double stepsize;
  int stabilizerScale;
  int nstabilizers;
  int maxIts;
 public:
  optimize(int _useGPU, int _walkers, double _vmcTstep, double _vmcEquilTime,
	   double _vmcDecorrTime, int _vmcBlocks, int _numOptSamples,
	   int _numOptLoops, int _minMethod, double _minWalkers, double _enWeight, 
	   int _useReweightedVariance = 1, int _useNonlocal = 0, int _useDrift = 0, 
	   int _checkpoint = -1) :
  numOptLoops(_numOptLoops), checkpoint(_checkpoint), useGPU(_useGPU), walkers(_walkers),
    numOptSamples(_numOptSamples), numSubsteps(static_cast<int>(_vmcDecorrTime / _vmcTstep + 0.5)),
    vmcBlocks(_vmcBlocks), warmupSteps(static_cast<int>(_vmcEquilTime / _vmcTstep + 0.5)),
    vmcTstep(_vmcTstep), useDrift(_useDrift), enWeight(_enWeight), 
    useReweightedVariance(_useReweightedVariance), useNonlocal(_useNonlocal),
    minMethod(_minMethod), minWalkers(_minWalkers) {
    
    beta = 0.0;    
    exp0 = -8.0;
    bigChange = 8.0;
    allowedDifference = 0.0001;
    stepsize = 0.1;
    stabilizerScale = 4;
    nstabilizers = 1;
    maxIts = 1;
  };
  //optimize(const optimize&); // not needed because copy constructor should be sufficient
  optimize(const xmlNode& xn);
  xmlNode createXmlNode() const;
};

#endif
