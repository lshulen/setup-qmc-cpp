#ifndef XML_REP_H
#define XML_REP_H
#include <string>
#include <sstream>
#include <vector>

struct attrpair {
  std::string name;
  std::string value;

  attrpair() { ; }
  attrpair(const std::string& n, const std::string& v) {
    name = n;
    value = v;
  }
  attrpair(const attrpair& ap) {
    name = ap.name;
    value = ap.value;
  }
};

// Should probably hide the children data structure behind operator[] or operator() and
// return only references to an xmlNode
struct xmlNode {
public:  
  int isSelfClosing;
  std::string name;
  std::vector<attrpair> attributes;
  int valInline; // one for being inline, zero for putting the value on the next line
  std::string value;
  std::vector<xmlNode> children;

  int getChildIndex(const std::string& name, int strict = 0) const;
  int getAttributeIndex(const std::string& name, int strict = 0) const;
  // note this is somewhat dangerous.  Will die if an attribute with that name is not found
  // but, if the conversion to the specified type using a stringstream is not good, you will get garbage
  template<typename T>
  T getAttribute(const std::string& name) const {
    std::string value = attributes[getAttributeIndex(name, 1)].value;
    std::stringstream ss(value);
    T retval;
    ss >> retval;
    return retval;
  }
    
  int getNumAttributes() const { return attributes.size(); }
  int getNumChildren() const { return children.size(); }

  template<typename T, typename TT> void addAttribute(const T& n, const TT& v) {
    std::stringstream ss1;
    ss1 << n;
    std::stringstream ss2;
    ss2 << v;
    attrpair ap(ss1.str(), ss2.str());
    attributes.push_back(ap);
  }
  template<typename T> void addYesNoAttribute(const T& n, int i) {
    std::stringstream ss;
    ss << n;
    attrpair ap;
    ap.name = ss.str();
    if (i == 0) {
      ap.value = "no";
    } else {
      ap.value = "yes";
    }
    attributes.push_back(ap);
  }
  template<typename T> void setValue(const T& v) {
    std::stringstream ss;
    ss << v;
    value = ss.str();
  }
  xmlNode& addChild(const xmlNode& nd) {
    children.push_back(nd);
    return children.back();
  }
  xmlNode& addChild() {
    xmlNode nd;
    children.push_back(nd);
    return children.back();
  }

  template<typename T, typename TT> void addParameterChild(const T& n, const TT& v) {
    xmlNode nd;
    nd.name = "parameter";
    nd.addAttribute("name", n);
    std::stringstream ss;
    ss << v;
    nd.value = ss.str();
    children.push_back(nd);
  }
  template<typename T> void addYesNoParameterChild(const T& n, int v) {
    xmlNode nd;
    nd.name = "parameter";
    nd.addAttribute("name", n);
    if (v == 0) {
      nd.value = "no";
    } else {
      nd.value = "yes";
    }
    addChild(nd);
  }
  xmlNode() { ; }
  xmlNode(std::stringstream& ss);
  xmlNode(const xmlNode& c);
  xmlNode& operator=(const xmlNode& c);
  void write(std::ostream& os, int indentLevel = 0) const; 
  std::string getString(int indentLevel = 0) const;

#ifdef TIMER
  static double getNextTagTimer;
  static double getNextTagGroupTimer;
#endif
private:
  std::string getInStr(int is) const {
    std::stringstream ss;
    for (int i = 0; i < is; i++) {
      ss << " ";
    }
    return ss.str(); 
  }
  void stripcomments(std::stringstream& ss) const;
  std::string stripXmlDeclaration(std::stringstream& ss) const;
  int checkSelfClosing(const std::string& tagstr) const;
  int isEndingTag(const std::string& tagstr) const;
  std::string trimWhitespace(const std::string& str) const;
  size_t getPosNextLiveChar(const std::string& str, char c) const;
  std::string getNextTag(std::stringstream& ss, int removeFromSS = 0) const;
  std::string getNextTagGroup(std::stringstream& ss, int removeFromSS = 0) const;
  std::string getTagName(const std::string& tagstr) const;
  void getNextKeyVal(std::string& contentstr, std::string& key, std::string& val) const;
  void handleTagString(const std::string& tagstr, xmlNode& node) const;
  int numSubGroups(const std::string& tagGroupStr) const;
  std::string getTextElement(const std::string& tagGroupStr) const;
};

#endif
