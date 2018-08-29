#include <sstream>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <string>
#include <fstream>
#include <cmath>
#include "types.h"
#include "pwscfData.h"
using namespace std;


void testPwscfParser(int argc, char* argv[]) {
  string inFileName = "undef";
  string outFileName = "undef";
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i],"--infile")) {
      inFileName = string(argv[i+1]);
    } else if (!strcmp(argv[i],"--outfile")) {
      outFileName = string(argv[i+1]);
    }
  }
  
  if (inFileName == "undef") {
    cout << "No pwscf input filename was given" << endl;
  } else {
    pwscfString ps(inFileName);
    pwscfData pd(ps);
    cout << "Scraped Data in input file: " << inFileName << endl;
    cout << "Prefix = " << pd.prefix << endl;
    cout << "Pseudopotential Directory = " << pd.ppDir << endl;
    cout << "Output Directory = " << pd.outputDir << endl;
    cout << "There are " << pd.ntyp << " types of ion" << endl;
    cout << "The ion types are:" << endl;
    for(int i = 0; i < pd.atNames.size(); i++) {
      cout << "   " << pd.atNames[i] << ", with pseudopotential at: " << pd.atNameToPPFile[pd.atNames[i]] << endl;
    }
    cout << "There are " << pd.numAt << " ions in total" << endl;
    cout << "The ions are at: " << endl;
    for (int i = 0; i < pd.ionNames.size(); i++) {
      cout << "   " << pd.ionNames[i] << "  " << pd.atpos[i][0] << "  " << pd.atpos[i][1] << "  " << pd.atpos[i][2] << endl;
    }
    cout << "The primitive translation vectors for the cell are:" << endl;
    cout << "   " << pd.ptv[0] << "   " << pd.ptv[1] << "   " << pd.ptv[2] << endl;
    cout << "   " << pd.ptv[3] << "   " << pd.ptv[4] << "   " << pd.ptv[5] << endl;
    cout << "   " << pd.ptv[6] << "   " << pd.ptv[7] << "   " << pd.ptv[8] << endl;
  }

  if (outFileName == "undef") {
    cout << "No pwscf output filename was given" << endl;
  } else {
    pwscfOutString pos(outFileName);
    cout << endl;
    cout << "Scraped Data in output file: " << outFileName << endl;
    cout << "The final magnetization of the calculation was: " << pos.magnetization << endl;
    cout << "Details on the pseudopotentials in the output: " << endl;
    for (map<string, int>::iterator it = pos.atNamesToValence.begin(); it != pos.atNamesToValence.end(); it++) {
      cout << "   " << it->first << ", valence = " << it->second << ", mass = " << pos.atNamesToMass[it->first] << endl;
    }
  }
}

// for the moment, always asking values to be set in bohr and 
// atom positions to be given in crystal units
pwscfData::pwscfData(const pwscfString& ps) {
  prefix = ps.getPwscfToken("prefix");
  ppDir = ps.getPwscfToken("pseudo_dir");
  outputDir = ps.getPwscfToken("outdir");
  ntyp = atoi((ps.getPwscfToken("ntyp")).c_str());
  numAt = atoi((ps.getPwscfToken("nat")).c_str());
  numSpin = atoi((ps.getPwscfToken("nspin")).c_str());
  setupPtv(ps);
  handleAtomicSpecies(ps);
  handleAtomicPositions(ps);
}  

  
void pwscfData::setupPtv(const pwscfString& ps) {
  // set up primitive translation vectors
  const double scale = atof((ps.getPwscfToken("celldm(1)")).c_str());
  const int ibrav = atoi((ps.getPwscfToken("ibrav")).c_str());
  for (int i = 0; i < 9; i++) {
    ptv[i] = 0.0;
  }
  if (ibrav == 0) {
    string ptvstr = ps.getPwscfCard("CELL_PARAMETERS");
    stringstream ss(ptvstr);
    string value;
    for (int i = 0; i < 9; i++) {
      ss >> value;
      ptv[i] = atof(value.c_str());
    }
  } else if (ibrav == 1) {
    ptv[0] = scale;
    ptv[4] = scale;
    ptv[8] = scale;
  } else if (ibrav == 2) {
    ptv[0] = -scale*0.5;
    ptv[2] = scale*0.5;
    ptv[4] = scale*0.5;
    ptv[5] = scale*0.5;
    ptv[6] = -scale*0.5;
    ptv[7] = scale*0.5;
  } else if (ibrav == 3) {
    for (int i = 0; i < 9; i++) {
      ptv[i] = 0.5*scale;
    }
    ptv[3] *= -1;
    ptv[6] *= -1;
    ptv[7] *= -1;
  } else if (ibrav == 4) {
    const double covera = atof((ps.getPwscfToken("celldm(1)")).c_str());
    ptv[0] = scale;
    ptv[3] = -0.5*scale;
    ptv[4] = sqrt(3.0)*0.5*scale;
    ptv[8] = scale*covera;
  }
}
 
void pwscfData::handleAtomicSpecies(const pwscfString& ps) {
  string asstr = ps.getPwscfCard("ATOMIC_SPECIES");
  stringstream ss(asstr);
  string line;
  while(getline(ss, line)) {
    string atSymbol;
    string atWeight;
    string ppName;

    stringstream liness(line);
    liness >> atSymbol;
    liness >> atWeight;
    liness >> ppName;
    
    atNames.push_back(atSymbol);
    string fullPPName = ppDir;
    if (fullPPName.at(fullPPName.length()-1) != '/') {
      fullPPName.append("/");
    }
    fullPPName.append(ppName);
    atNameToPPFile[atSymbol] = fullPPName;
  }
}

void pwscfData::handleAtomicPositions(const pwscfString& ps) {
  string apstr = ps.getPwscfCard("ATOMIC_POSITIONS");
  stringstream ss(apstr);
  string line;
  while(getline(ss, line)) {
    string atSymbol;
    location temploc;

    stringstream liness(line);
    liness >> atSymbol;
    liness >> temploc[0];
    liness >> temploc[1];
    liness >> temploc[2];

    ionNames.push_back(atSymbol);
    atpos.push_back(temploc);
  }
}


pwscfOutString::pwscfOutString(const string& outFileName) {
  ifstream t(outFileName.c_str());
  stringstream buffer;
  
  if (t.is_open()) {
    buffer << t.rdbuf();
    outputFileText = buffer.str();
  } else {
    cout << "Unable to open pwscf input file: " << outFileName << endl;
  }

  magnetization = 0.0;
  string line;
  int grabPPdata = 0;

  const string ppHeader = "atomic species   valence";
  const string magString = "total magnetization";

  while(getline(buffer, line)) {
    size_t magindex = line.find(magString);
    if (magindex != string::npos) {
      stringstream ss(line);
      string dummy;
      ss >> dummy;
      ss >> dummy;
      ss >> dummy;
      ss >> magnetization;
    }
    
    if (grabPPdata == 1) {
      if(line.find_first_not_of(' ') == string::npos) {
	// if we grab a blank line then we have left the pp definitions
	grabPPdata = 0;
      } else {
	stringstream ss(line);
	string name;
	double valence;
	int intValence;
	double mass;
	ss >> name;
	ss >> valence;
	valence += 0.5;
	intValence = static_cast<int>(valence);
	ss >> mass;
	atNamesToValence[name] = intValence;
	atNamesToMass[name] = mass;
      }
    }

    size_t index = line.find(ppHeader);
    if (index != string::npos) {
      grabPPdata = 1;
    }
  }
}

pwscfString::pwscfString(const string& pwscfFileName) {
  ifstream t(pwscfFileName.c_str());
  if (t.is_open()) {
    stringstream buffer;
    buffer << t.rdbuf();
    inputFileText = buffer.str();
    doctoredText = inputFileText;
  } else {
    cout << "Unable to open pwscf input file: " << pwscfFileName << endl;
  }
}

string pwscfString::getPwscfToken (const string& tokenName, const string& searchText) const {
  stringstream buffer(searchText);
  string line;
  string outval;
  string outval2;
  while(getline(buffer, line)) {
    size_t index = line.find(tokenName);
    if (index != string::npos) {
      outval = line.substr(line.find(tokenName)+tokenName.size()); // get everything after token
      outval = outval.substr(outval.find('=')+1); // get everything after equals sign
      // next two lines are a hacky way to get only the first set of characters in outval
      stringstream temp(outval);
      temp >> outval2;
      
      outval2.erase(remove(outval2.begin(), outval2.end(), ','),outval2.end()); //get rid of commas
      outval2.erase(remove(outval2.begin(), outval2.end(), '\''),outval2.end()); //get rid of quotes
    }
  }
  return outval2;
}

string pwscfString::getRawPwscfToken (const string& tokenName, const string& searchText) const {
  stringstream buffer(searchText);
  string line;
  string outval;
  string outval2;
  while(getline(buffer, line)) {
    size_t index = line.find(tokenName);
    if (index != string::npos) {
      outval = line.substr(line.find(tokenName)+tokenName.size()); // get everything after token
      outval = outval.substr(outval.find('=')+1); // get everything after equals sign
      // next two lines are a hacky way to get only the first set of characters in outval
      stringstream temp(outval);
      temp >> outval2;
      
      outval2.erase(remove(outval2.begin(), outval2.end(), ','),outval2.end()); //get rid of quotes
    }
  }
  return outval2;
}


void pwscfString::changePwscfToken(const string& tokenName, const string& newValue) {
  string valueToChange = getRawPwscfToken(tokenName, doctoredText);
  size_t index = doctoredText.find(valueToChange);
  if (index == string::npos) {
  } else {
    doctoredText.replace(index, valueToChange.size(), newValue);
  }
}

void pwscfString::addToPwscfSection(const string& secName, const string& tokenName, const string& valName) {
  string secNameUpper;
  string secNameLower;
  secNameUpper.resize(secName.size());
  secNameLower.resize(secName.size());
  transform(secName.begin(), secName.end(), secNameUpper.begin(), ::toupper); 
  transform(secName.begin(), secName.end(), secNameLower.begin(), ::tolower); 

  stringstream buffer(doctoredText);
  stringstream outbuffer;
  string line;
  while(getline(buffer, line)) {
    outbuffer << line << endl;
    size_t lowerindex = line.find(secNameUpper);
    size_t upperindex = line.find(secNameLower);
    if (lowerindex != string::npos || upperindex != string::npos) {
      outbuffer << "    " << tokenName << " = " << valName << endl;
    }
  }
  doctoredText = outbuffer.str();
}

void pwscfString::removePwscfToken(const string& tokenName) {
  stringstream buffer(doctoredText);
  stringstream outbuffer;
  string line;
  while(getline(buffer, line)) {
    size_t index = line.find(tokenName);
    if (index == string::npos) {
      outbuffer << line << endl;
    } else {
      // if there is no comma, just remove the line
      // if there is a comma and there is something after it
      // just remove the section of the string that is the token
      size_t endindex = line.find(',');
      if (endindex != string::npos) {
	if (endindex != line.size()) {
	  string newline = line.erase(index, endindex-index);
	  outbuffer << newline;
	}
      }
    }
  }
  doctoredText = outbuffer.str();
}

string pwscfString::getWholePwscfCard(const string& cardName, const string& searchText) const {
  stringstream buffer(searchText);
  stringstream outbuffer;
  string line;
  int start = 0;
  int stop = 0;
  while(getline(buffer, line)) {
    size_t index = line.find(cardName);
    if (start == 1) {
      size_t asindex = line.find("ATOMIC_POSITIONS");
      size_t kpindex = line.find("K_POINTS");
      size_t cpindex = line.find("CELL_PARAMETERS");
      size_t consindex = line.find("CONSTRAINTS");
      size_t occindex = line.find("OCCUPATIONS");
      size_t afindex = line.find("ATOMIC_FORCES");
      if (asindex != string::npos || kpindex != string::npos || cpindex != string::npos
	  || consindex != string::npos || occindex != string::npos || afindex != string::npos) {
	stop = 1;
      }
    }
    if (index != string::npos) { start = 1; }
    if (start == 1 && stop == 0) {
      if(line.find_first_not_of(' ') != string::npos) {
	outbuffer << line << endl;
      }
    }
  }
  return outbuffer.str();
}

string pwscfString::getPwscfCard(const string& cardName, const string& searchText) const {
  string str = getWholePwscfCard(cardName, searchText);
  stringstream buffer(str);
  stringstream outbuffer;
  string line;
  getline(buffer,line);
  while(getline(buffer, line)) {
    outbuffer << line << endl;
  }
  return outbuffer.str();
}

void pwscfString::replacePwscfCard(const string& cardName, const string& newText) {
  string valueToChange = getPwscfCard(cardName);
  size_t index = doctoredText.find(valueToChange);
  if (index == string::npos) {
  } else {
    doctoredText.replace(index, valueToChange.size(), newText);
  }
}

void pwscfString::replaceWholePwscfCard(const string& cardName, const string& newText) {
  string valueToChange = getWholePwscfCard(cardName);
  size_t index = doctoredText.find(valueToChange);
  if (index == string::npos) {
  } else {
    doctoredText.replace(index, valueToChange.size(), newText);
  }
}
