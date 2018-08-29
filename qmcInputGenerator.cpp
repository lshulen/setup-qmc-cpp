#include <string>
#include <sstream>
#include <iomanip>
#include "xmlRep.h"
#include "qmcInputGenerator.h"
using namespace std;

// Note the qmcpack input file should start with the following comment:
// <?xml version="1.0"?>
// And then everything is contained in a tag:
// <simulation>
// Under simulation, first you have a project tag, than a qmcsystem tag and then one or more qmc tags that say what is to be done

xmlNode getProjectNode(const string& baseName, int seriesnum, const string& jobdescription) {
  xmlNode pn;
  pn.name = "project";
  pn.addAttribute("id", baseName);
  pn.addAttribute("series", seriesnum);

  xmlNode apNode = pn.addChild();
  apNode.name="application";
  apNode.addAttribute("name", "qmcpack");
  apNode.addAttribute("role", "molecu");
  apNode.addAttribute("class", "serial");
  apNode.addAttribute("version", "3.1.0");
  apNode.value = jobdescription;
  return pn;
}

xmlNode getRandomNode(int seed) {
  xmlNode rn;
  rn.name = "random";
  rn.addAttribute("seed", seed);
  rn.isSelfClosing = 1;
  return rn;
}
