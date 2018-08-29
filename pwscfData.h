#ifndef PWSCF_DATA_H
#define PWSCF_DATA_H
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include "types.h"

class pwscfString;
 
void testPwscfParser(int argc, char* argv[]);

// Copy functionality of parsePwscfInput from setup-qmc.pl here
class pwscfData {
 private:
  void setupPtv(const pwscfString& ps);
  void handleAtomicSpecies(const pwscfString& ps);
  void handleAtomicPositions(const pwscfString& ps);
 public:
  std::string prefix;
  std::string ppDir;
  std::string outputDir;
  int ntyp;
  int numAt;
  int numSpin;
  threeThreeMat<double> ptv;
  std::vector<location> atpos;
  std::vector<std::string> atNames; // list of the atom types (will be ntyp of these)
  std::map<std::string, std::string> atNameToPPFile;
  std::vector<std::string> ionNames; // list of each atom's name (will be numAt of these)
 public:
  pwscfData(const pwscfString& ps);
};


class pwscfOutString {
 private:
  std::string outputFileText;
 public:
  std::map<std::string, int> atNamesToValence;
  std::map<std::string, double> atNamesToMass;
  double magnetization;
 public:
  pwscfOutString(const std::string& outFileName);
};


class pwscfString {
 private:
  std::string inputFileText;
  std::string doctoredText;

 public:
  pwscfString(const std::string& pwscfFileName); 
  pwscfString(const pwscfString& is) : inputFileText(is.inputFileText), doctoredText(is.doctoredText) { ; }

  std::string getPwscfToken (const std::string& tokenName, const std::string& searchText) const;
  std::string getPwscfToken (const std::string& tokenName) const {
    return getPwscfToken(tokenName, inputFileText);
  }
  std::string getRawPwscfToken (const std::string& tokenName, const std::string& searchText) const;
  std::string getRawPwscfToken (const std::string& tokenName) const {
    return getRawPwscfToken(tokenName, inputFileText);
  }
  void changePwscfToken(const std::string& tokenName, const std::string& newValue);
  void removePwscfToken(const std::string& tokenName);
  void addToPwscfSection(const std::string& secName, const std::string& tokenName, const std::string& valName);
  

  std::string getWholePwscfCard (const std::string& cardName, const std::string& searchText) const;
  std::string getWholePwscfCard (const std::string& cardName) const {
    return getWholePwscfCard(cardName, inputFileText);
  }
  std::string getPwscfCard(const std::string& cardName, const std::string& searchText) const;
  std::string getPwscfCard (const std::string& cardName) const {
    return getPwscfCard(cardName, inputFileText);
  }
  void replacePwscfCard(const std::string& cardName, const std::string& newText);
  void replaceWholePwscfCard(const std::string& cardName, const std::string& newText);

  void printOutput() const { std::cout << doctoredText; }
  std::string getOutput() const { return doctoredText; }
  void commitChanges() { inputFileText = doctoredText; }
};
    
  


#endif
