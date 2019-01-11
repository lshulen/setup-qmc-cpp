#include "xmlRep.h"
#ifdef TIMER
#include <chrono>
#endif
#include <iostream>
using namespace std;

#ifdef TIMER
double xmlNode::getNextTagTimer = 0.0;
double xmlNode::getNextTagGroupTimer = 0.0;
#endif

// note, will return only the first child index that matches!
int xmlNode::getChildIndex(const string& childName, int strict) const {
  int index = -1;
  for (int i = 0; i < children.size(); i++) {
    if (children[i].name == childName) {
      index = i;
    }
  }
  if (strict != 0 && index < 0) {
    cout << "In xmlNode with name: " << name << ", could not find index for child with name: " << childName << endl;
    exit(1);
  }
  return index;
}

int xmlNode::getAttributeIndex(const string& attrName, int strict) const {
  int index = -1;
  for (int i = 0; i < attributes.size(); i++) {
    if (attributes[i].name == attrName) {
      index = i;
    }
  }
  if (strict != 0 && index < 0) {
    cout << "In xmlNode with name: " << name << ", could not find index for attribute with name: " << attrName << endl;
    exit(1);
  }
  return index;
}

xmlNode::xmlNode(stringstream& ss) {
  stripcomments(ss);
  string tag = getNextTag(ss, 0);
  string tagGroup = getNextTagGroup(ss, 0);

  handleTagString(tag, *this);
  valInline = 1;
  value = getTextElement(tagGroup);

  const int numElements = numSubGroups(tagGroup);
  if (numElements > 0) {
    valInline = 0;
    getNextTag(ss, 1);
    for (int i = 0; i < numElements; i++) {
      string subElement = getNextTagGroup(ss, 1);
      stringstream ss2(subElement);
      children.push_back(xmlNode(ss2));
    }
  }
}

xmlNode::xmlNode(const xmlNode& c) {
  isSelfClosing = c.isSelfClosing;
  name = c.name;
  for (int i = 0; i < c.attributes.size(); i++) {
    attributes.push_back(c.attributes[i]);
  }
  valInline = c.valInline;
  value = c.value;
  for (int i = 0; i < c.children.size(); i++) {
    children.push_back(c.children[i]);
  }
}

xmlNode& xmlNode::operator=(const xmlNode& c) {
  isSelfClosing = c.isSelfClosing;
  name = c.name;
  for (int i = 0; i < c.attributes.size(); i++) {
    attributes.push_back(c.attributes[i]);
  }
  valInline = c.valInline;
  value = c.value;
  for (int i = 0; i < c.children.size(); i++) {
    children.push_back(c.children[i]);
  }
  return *this;
}


void xmlNode::write(ostream& os, int indentLevel) const {
  string str = getString(indentLevel);
  os << str;
}

string xmlNode::getString(int indentLevel) const {
  stringstream ss;
  ss << getInStr(indentLevel);
  ss << "<" << name;
  
  for (int i = 0; i < attributes.size(); i++) {
    ss << " " << attributes[i].name << "=\"" << attributes[i].value << "\"";
  }
  if (isSelfClosing == 1) {
    ss << "/>" << endl;
    return ss.str();
  } else {
    ss << ">";
  } 
  if (valInline == 1) {
    ss << value << "</" << name << ">" << endl;
  } else {
    ss << endl;
    for (int i = 0; i < children.size(); i++) {
      ss << children[i].getString(indentLevel+2);
    }
    if(value.size() > 0) ss << getInStr(indentLevel+2) << value << endl;
    ss << getInStr(indentLevel) << "</" << name << ">" << endl;
  }
  return ss.str();
}

// this doesn't work at all
// what if there are no xml comments but there is an xml declaration (currently nothing happens)
// 
void xmlNode::stripcomments(stringstream& ss) const {
  stringstream ss2;
  const string contents = stripXmlDeclaration(ss);
  ss.str(contents);
  int inComment = 0;
  
  int commentStart = -1;
  int commentEnd = 0;
  
  while (contents.find("<!--",commentEnd) != string::npos) {
    commentStart = contents.find("<!--",commentEnd);
    ss2 << contents.substr(commentEnd, commentStart-commentEnd);
    commentEnd = contents.find("-->",commentStart);
  }
  if (commentStart == -1) {
    return;
  } else {
    ss2 << contents.substr(commentEnd+3, contents.size()-commentEnd-3);
  }

  ss.str(ss2.str());
}

string xmlNode::stripXmlDeclaration(stringstream& ss) const {
  if (ss.str().find("<?") == 0) {
    getNextTag(ss, 1);
  }
  return ss.str();
}

int xmlNode::checkSelfClosing(const string& tagstr) const {
  int retval = 0;
  if (tagstr[tagstr.size()-2] == '/') {
    retval = 1;
  }
  return retval;
}

int xmlNode::isEndingTag(const string& tagstr) const {
  int retval = 0;
  if (tagstr.size() > 1 && tagstr[1] == '/') {
    retval = 1;
  }
  return retval;
}

string xmlNode::trimWhitespace(const string& str) const {
  size_t first = str.find_first_not_of(' ');
  if (string::npos == first) {
    return string("");
  }
  size_t last = str.find_last_not_of(' ');
  return str.substr(first, (last - first + 1));
}

size_t xmlNode::getPosNextLiveChar(const std::string& str, char c) const {
  size_t index = string::npos;
  if (str[0] == c) {
    index = 0;
  } else {
    for(int i = 1; i < str.size(); i++) {
      if (str[i] == c && str[i-1] != '\\') {
	index = i;
	break;
      }
    }
  }
  return index;
}

int numQuotes(const string& str, int start, int stop) {
  int counter = 0;
  int quoteloc = str.find('\"', start);
  while(quoteloc < stop && quoteloc != string::npos) {
    if (quoteloc < 1 || str[quoteloc-1] != '\\') {
      counter++;
    }
    quoteloc = str.find('\"', quoteloc+1);
      
  }
  return counter;
}


string xmlNode::getNextTag(stringstream& ss, int removeFromSS) const {
#ifdef TIMER
  auto begin = std::chrono::high_resolution_clock::now();  
#endif   

  char prev;
  
  int start = 0;
  int stop = 0;
  int inquote = 0;
  stringstream result;

  int length = ss.str().size();
  int closeCaretFound = 0;
  int closeCaretLoc = -1;
  int openCaretFound = 0;
  int lastCaretLoc = 0;
  int openCaretLoc = ss.str().find("<");
  int quotesFound = 0;
  while (openCaretFound == 0 && openCaretLoc != string::npos) {
    quotesFound += numQuotes(ss.str(), lastCaretLoc, openCaretLoc);
    // need number of quotes found before this caret to be even, 
    // otherwise we are in a quote and should skip this one
    // also check that the previous character is not an \, 
    // otherwise the caret is escaped and should be skipped
    if (quotesFound % 2 == 0 && (openCaretLoc == 0 || ss.str()[openCaretLoc-1] != '\\')) {
      openCaretFound = 1;
      lastCaretLoc = openCaretLoc;
      closeCaretLoc = ss.str().find(">", lastCaretLoc+1);
      // need total number of quotes found before this caret to be even, 
      // otherwise we are in a quote and should skip this one
      // also check that the previous character is not an \, 
      // otherwise the caret is escaped and should be skipped
      while (closeCaretFound == 0 && closeCaretLoc != string::npos) {
	quotesFound += numQuotes(ss.str(), lastCaretLoc, closeCaretLoc);
	if (quotesFound % 2 == 0 && ss.str()[closeCaretLoc-1] != '\\') {
	  closeCaretFound = 1;
	} else {
	  lastCaretLoc = closeCaretLoc;
	  closeCaretLoc = ss.str().find(">", lastCaretLoc+1);
	}
      }
    } else {
      lastCaretLoc = openCaretLoc;
      openCaretLoc = ss.str().find("<", lastCaretLoc+1);
    }
  }

  string nextTag;
  if (closeCaretFound && openCaretFound) {
    nextTag = ss.str().substr(openCaretLoc, closeCaretLoc-openCaretLoc+1);
  }
  if (removeFromSS == 1) {
    string temp = ss.str();
    temp = temp.substr(closeCaretLoc+1);
    ss.str(temp);    
  }
#ifdef TIMER
  auto end = std::chrono::high_resolution_clock::now();  
  getNextTagTimer += std::chrono::duration_cast<std::chrono::milliseconds>(end-begin).count();
#endif
  return nextTag;
}

// Gets the whole tag from starting to ending, including everything in between
string xmlNode::getNextTagGroup(stringstream& ss, int removeFromSS) const {
#ifdef TIMER
  auto begin = std::chrono::high_resolution_clock::now();  
#endif
  string retstr;
  string openingTag = getNextTag(ss, 0);
  int startPos = ss.str().find(openingTag);
  int charactersRemoved = 0;
  stringstream remainder;

  // if it's self closing, we're done
  if (checkSelfClosing(openingTag)) {
    retstr = openingTag;
    charactersRemoved = retstr.size();
  } else {
    // search for the closing tag
    string name = getTagName(openingTag);
    int level = 1;

    remainder.str(ss.str());
    getNextTag(remainder, 1);
    charactersRemoved += openingTag.size();

    while(level > 0) {

      string nextTag = getNextTag(remainder, 0);
      charactersRemoved += remainder.str().find(nextTag) + nextTag.size();
      getNextTag(remainder, 1);

      if (checkSelfClosing(nextTag) == 0) {// && getTagName(nextTag) != name) {
	if (isEndingTag(nextTag) == 1) {
	  level--;
	} else {
	  level++;
	}
      }
    } 
    retstr = ss.str().substr(startPos, charactersRemoved);
  }
  if (removeFromSS == 1) {   
    string temp = ss.str();
    temp.erase(0, charactersRemoved+startPos);
    ss.str(temp);
  }
#ifdef TIMER
  auto end = std::chrono::high_resolution_clock::now();  
  getNextTagGroupTimer += std::chrono::duration_cast<std::chrono::milliseconds>(end-begin).count();
#endif
  return retstr;
}

string xmlNode::getTagName(const string& tagstr) const {
  string tagName;
  
  size_t spaceLoc = tagstr.find(" ");
  size_t closeLoc = tagstr.find('>');
  size_t slashLoc = string::npos;
  if (checkSelfClosing(tagstr) == 1) {
    slashLoc = tagstr.find("/");
  }
  
  int endChar = tagstr.size();
  if (spaceLoc != string::npos) {
    endChar = spaceLoc;
  }
  if (closeLoc < endChar && closeLoc != string::npos) {
    endChar = closeLoc;
  }
  if (slashLoc < endChar && slashLoc != string::npos) {
    endChar = slashLoc;
  }

  if (isEndingTag(tagstr) == 1) {
    endChar -= 2;
    tagName = tagstr.substr(2, endChar);  
  } else {
    endChar -= 1;
    tagName = tagstr.substr(1, endChar);  
  }
  return tagName;
}

void xmlNode::getNextKeyVal(string& contentstr, string& key, string& val) const {
  size_t breakone = getPosNextLiveChar(contentstr, '=');

  key = contentstr.substr(0,breakone);
  key = trimWhitespace(key);
  contentstr = contentstr.substr(breakone+1);
 
  size_t firstquote = getPosNextLiveChar(contentstr, '\"');
  contentstr = contentstr.substr(firstquote+1);
  size_t secondquote = getPosNextLiveChar(contentstr, '\"');
  val = contentstr.substr(0,secondquote);
  val = trimWhitespace(val);

  contentstr = contentstr.substr(secondquote+1);
}

// creates all of the attributes
void xmlNode::handleTagString(const string& tagstr, xmlNode& node) const {
  node.isSelfClosing = checkSelfClosing(tagstr);
  node.name = getTagName(tagstr);

  string decliningstring = tagstr.substr(tagstr.find(node.name)+node.name.size()+1);

  int numToRemove = 1;
  if (node.isSelfClosing == 1) {
    numToRemove++;
  }
  decliningstring = decliningstring.substr(0,decliningstring.size()-numToRemove);
  decliningstring = trimWhitespace(decliningstring);

  while(decliningstring.size() > 1) {
    attrpair att;
    getNextKeyVal(decliningstring, att.name, att.value);
    node.attributes.push_back(att);
  } 
}

int xmlNode::numSubGroups(const string& tagGroupStr) const {
  stringstream ss(tagGroupStr);
  getNextTag(ss, 1);
  int numSubGroups = 0;
  string nextTag = getNextTag(ss,0);
  while(nextTag.size() != 0 && isEndingTag(nextTag) == 0) {
    numSubGroups++;
    getNextTagGroup(ss, 1);
    nextTag = getNextTag(ss, 0);
  }
  return numSubGroups;
}

string xmlNode::getTextElement(const string& tagGroupStr) const {
  string text("");
  int nsg = numSubGroups(tagGroupStr);
  
  stringstream ss(tagGroupStr);
  getNextTag(ss,1);
  string nextTag = getNextTag(ss,0);

  string candidateText = ss.str().substr(0,ss.str().find(nextTag));
  candidateText = trimWhitespace(candidateText);
  if (candidateText.size() != 0) {
    text = candidateText;
  }
  for (int i = 0; i < nsg; i++) {
    getNextTagGroup(ss,1);
    nextTag = getNextTag(ss,0);
    candidateText = ss.str().substr(0,ss.str().find(nextTag));
    candidateText = trimWhitespace(candidateText);
    if (candidateText.size() != 0) {
      text = candidateText;
    }
  }
  return text;
}
