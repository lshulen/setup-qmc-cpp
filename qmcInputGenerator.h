#ifndef QMC_INPUT_GENERATOR_H
#define QMC_INPUT_GENERATOR_H
#include <string>
#include <sstream>

xmlNode getProjectNode(const std::string& baseName, int seriesnum, const std::string& jobdescription);
xmlNode getRandomNode(int seed);

// Need to be given an optimization directory
// make sure we can choose a new twist
// also make sure we can turn on and off wavefunction elements
// LNS LNS LNS probably don't do this and instead just add a qmcSysDescription constructor that takes a directory and then alter it
xmlNode getWvfcnNodeFromOptDir();

#endif
