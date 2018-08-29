#ifndef TYPES_H
#define TYPES_H
#include <vector>
#include <string>
#include <functional>
#include <algorithm>

struct location {
  double loc[3];
  location() {
    for (int i = 0; i < 3; i++) {
      loc[i] = 0.0;
    }
  }
  location(const location& rhsloc) {
    for (int i = 0; i < 3; i++) {
      loc[i] = rhsloc[i];
    }
  }
  double& operator[](int i) {
    return loc[i];
  }
  double operator[](int i) const {
    return loc[i];
  }
};

class weightedKpt : public std::pair<double, location> { };

template<typename T>
struct threeThreeMat {
  T storage[9];
  threeThreeMat() {
    for (int i = 0; i < 9; i++) {
      storage[i] = 0;
    }
  }
  threeThreeMat(const threeThreeMat<T>& rhs) {
    for (int i = 0; i < 9; i++) {
      storage[i] = rhs[i];
    }
  }
  threeThreeMat<T>& operator=(const threeThreeMat<T>& rhs) {
    for (int i = 0; i < 9; i++) {
      storage[i] = rhs[i];
    }
    return *this;
  }
  T& operator[](int i) {
    return storage[i];
  }
  T operator[](int i) const {
    return storage[i];
  }
};


class rankedTilemat : public std::pair<double, threeThreeMat<int> > { };

inline bool compareTilemats(const rankedTilemat& lhs, const rankedTilemat& rhs) {
  return lhs.first > rhs.first;
}


// Will store a vector of rankedTileMatrices of size at most max_size
// where the largest element is at position 0 and the smallest at position size-1
struct sortedTilemats {
  int maxSize;
  int size;
  double curMin;
  typedef std::vector<rankedTilemat> vectype;
  vectype vec;
  typedef vectype::iterator iterator;
  typedef vectype::const_iterator const_iterator;
  
  sortedTilemats(int inMaxSize) : maxSize(inMaxSize) {
    size = 0;
    curMin = 0.0;
  }

  iterator insert(const rankedTilemat& t) {
    iterator i = std::lower_bound(vec.begin(), vec.end(), t, compareTilemats);
    vec.insert(i,t); 
    if (size >= maxSize) {
      //      std::cout << "----- popping and element off!" << std::endl;
      vec.pop_back();
    } else {
      size++;
    }
    curMin = vec.back().first;
    return i;
  }
};
    

class physSys {
 private:
  std::vector<std::string> atlabels;
  std::vector<location> atpos;
  std::vector<int> attypes;
  
  threeThreeMat<double> prim;
  threeThreeMat<double> rlv;

  void setTypes();
  void setRlv();
 public:
  physSys(const threeThreeMat<double>& inPrim, const std::vector<std::string>& inAtLabels,
	 const std::vector<location>& inAtpos) {
    for (int i = 0; i < 9; i++) {
      prim[i] = inPrim[i];
    }
    for (int i = 0; i < inAtLabels.size(); i++) {
      atlabels.push_back(inAtLabels[i]);
      location loc(inAtpos[i]);
      atpos.push_back(loc);
    }
    setTypes();
    setRlv();
  }
  // constructor to make a supercell
  physSys(const physSys& primsys, const threeThreeMat<int>& tilemat);

  void showCell();
  double getDet() const; // get the determinant of the primitive cell
  void calcSupercell(const threeThreeMat<int>& tilemat, threeThreeMat<double>& ss) const;
  void genRlv(const threeThreeMat<double>& p, threeThreeMat<double>& outrlv) const;

  int getNumAts() const { return atlabels.size(); }
  const threeThreeMat<double>& getPrim() const { return prim; }
  const threeThreeMat<double>& getRlv() const { return rlv; }

  // These are unsafe, but keeping them that way for performance
  const std::string& getAtLabel(int i) const { return atlabels[i]; }
  const location& getAtPos(int i) const { return atpos[i]; }
  int getAtType(int i) const { return attypes[i]; }

  // requires spglib
  // returns the number of symmetry operations
  int getSymmetry() const;
  int getMeshSize(int inputsize) const;
  int getMesh(int nx, int ny, int nz, int shiftx, int shifty, int shiftz, std::vector<weightedKpt>&) const;
};



#endif
