#include <iostream>
#include <cmath>
#include <stdio.h>
#include <string>
#include <cstring>
#include <sstream>
#include <algorithm>
#include "util.h"
#include "types.h"
using namespace std;

double dot(const location& a, const location& b) {
  return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}

void cross(const location& a, const location& b, location& c) {
  c[0] = a[1]*b[2]-a[2]*b[1];
  c[1] = -a[0]*b[2]+a[2]*b[0];
  c[2] = a[0]*b[1]-a[1]*b[0];
}

void getBestTile(const physSys& sys, int target,
		 threeThreeMat<int>& tilemat, double& radius, int range) {
  static const double tol = 0.0000001;
  double largest = 0.0;
  double bestScore = 0.0;
  double detPrim = sys.getDet();
  
  if (detPrim < 0) {
    target *= -1;
  }

  #pragma omp parallel
  {
    double my_largest = 0.0;
    threeThreeMat<int> my_besttile;
    double localBestScore = 0.0;
    int flag = 0;
    
    const int span = range*2+1;
    const int maxindex = span*span*span*span*span;
    #pragma omp for
    for (int ijklm = 0; ijklm < maxindex; ijklm++) {
      threeThreeMat<int> d;
      threeThreeMat<double> super;
      const int i = ijklm / span / span / span / span - range; // i
      const int j = (ijklm / span / span / span) % span - range; // j
      const int k = (ijklm / span / span ) % span % span - range; // k
      const int l = (ijklm / span) % span % span % span - range; // l
      const int m = ijklm % span % span % span % span - range; // m
      d[0] = i;
      d[1] = j;
      d[2] = k;
      d[3] = l;
      d[4] = m;
      
      int denominator = j*l-i*m;
      for (int n = -range; n <= range; n++) {
	d[5] = n;
	int fpp = k*l-i*n;
	for (int o = -range; o <= range; o++) {
	  d[6] = o;
	  int sp = o*(n*j-k*m);
	  for (int p = -range; p <= range; p++) {
	    d[7] = p;
	    int numpart = p*fpp+sp;
	    if (denominator != 0) {
	      int rem = 5;
	      rem = (numpart - target) % denominator;
	      if (rem == 0) {
		d[8] = (numpart - target) / denominator;
		sys.calcSupercell(d, super);
		double score = getScore(super);
		double rad = SimCellRad(super);
		//double rad = WigSeitzRad(super);
		if (rad > my_largest+tol || (rad > my_largest-tol && score > localBestScore)) {
		  my_largest = rad;
		  localBestScore = score;
		  std::memcpy(my_besttile.storage, d.storage, 9*sizeof(int));
		}
	      }
	    }
	    else {
	      // Handle case where denominator is 0
	      if (numpart == target) {
		for (int q = -range; q <= range; q++) {
		  d[8] = q;
		  sys.calcSupercell(d, super);
		  double score = getScore(super);
		  double rad = SimCellRad(super);
		  //double rad = WigSeitzRad(super);
		  if (rad > my_largest+tol || (rad > my_largest-tol && score > localBestScore)) {
		    my_largest = rad;
		    localBestScore = score;
		    std::memcpy(my_besttile.storage, d.storage, 9*sizeof(int));
		  }
		}
	      }
	    }
	  }
	}
      }
    }
    if (my_largest > largest+tol || (my_largest > largest-tol && localBestScore > bestScore)) {
#pragma omp critical
      {
        if (my_largest > largest+tol || (my_largest > largest-tol && localBestScore > bestScore) ) {
          largest = my_largest;
          bestScore = localBestScore;
          std::memcpy(tilemat.storage, my_besttile.storage, 9*sizeof(int));
        }
      }
    }
  }
  radius = largest;
}

void getBestTiles(const physSys& sys, int target,
		  sortedTilemats& tms, int numtiles, int range) {
  static const double tol = 0.0000001;
  double largest = 0.0;
  double bestScore = 0.0;
  double detPrim = sys.getDet();
  
  if (detPrim < 0) {
    target *= -1;
  }

  #pragma omp parallel
  {
    sortedTilemats loctms(numtiles);
    int flag = 0;
    
    const int span = range*2+1;
    const int maxindex = span*span*span*span*span;
    #pragma omp for
    for (int ijklm = 0; ijklm < maxindex; ijklm++) {
      //int d[9];
      //double super[9];
      threeThreeMat<int> d;
      threeThreeMat<double> super;
      const int i = ijklm / span / span / span / span - range; // i
      const int j = (ijklm / span / span / span) % span - range; // j
      const int k = (ijklm / span / span ) % span % span - range; // k
      const int l = (ijklm / span) % span % span % span - range; // l
      const int m = ijklm % span % span % span % span - range; // m
      d[0] = i;
      d[1] = j;
      d[2] = k;
      d[3] = l;
      d[4] = m;
      
      int denominator = j*l-i*m;
      for (int n = -range; n <= range; n++) {
	d[5] = n;
	int fpp = k*l-i*n;
	for (int o = -range; o <= range; o++) {
	  d[6] = o;
	  int sp = o*(n*j-k*m);
	  for (int p = -range; p <= range; p++) {
	    d[7] = p;
	    int numpart = p*fpp+sp;
	    if (denominator != 0) {
	      int rem = 5;
	      rem = (numpart - target) % denominator;
	      if (rem == 0) {
		d[8] = (numpart - target) / denominator;
		sys.calcSupercell(d, super);
		double score = getScore(super);
		double rad = SimCellRad(super);
		//double rad = WigSeitzRad(super);
		// for now go to strict ordering
		// if (rad > loctms.curMin+tol || (rad > loctms.curMin-tol && score > localBestScore)) {
		if (rad > loctms.curMin || loctms.size < numtiles) {
		  rankedTilemat rtm;
		  rtm.first = rad;
		  for (int index = 0; index < 9; index++) {
		    (rtm.second)[index] = d[index];
		  }
		  loctms.insert(rtm);
		}
	      }
	    }
	    else {
	      // Handle case where denominator is 0
	      if (numpart == target) {
		for (int q = -range; q <= range; q++) {
		  d[8] = q;
		  sys.calcSupercell(d, super);
		  double score = getScore(super);
		  double rad = SimCellRad(super);
		  //double rad = WigSeitzRad(super);
		  // for now go to strict ordering
		  // if (rad > loctms.curMin+tol || (rad > loctms.curMin-tol && score > localBestScore)) {
		  if (rad > loctms.curMin || loctms.size < numtiles) {
		    rankedTilemat rtm;
		    rtm.first = rad;
		    for (int index = 0; index < 9; index++) {
		      (rtm.second)[index] = d[index];
		    }
		    //cout << "curMin = " << loctms.curMin << ", inserting tilematrix with radius: " << rtm.first << endl;;
		    loctms.insert(rtm);
		  }
		}
	      }
	    }
	  }
	}
      }
    }

    for (int i = 0; i < loctms.size; i++) {
      if (loctms.vec[i].first > tms.curMin || tms.size < numtiles) {
#pragma omp critical
	{
	  if (loctms.vec[i].first > tms.curMin || tms.size < numtiles) {
	    //cout << "Found new element to put into tms" << endl;
	    tms.insert(loctms.vec[i]);
	    //cout << "   tms.curMin = " << tms.curMin << ", tms.size = " << tms.size << endl;

	  }
	}
      }
    }
  }
}




double getScore(const threeThreeMat<double>& mat) {
  double score = 0;
  static const double tol = 0.001;
  // Highest preference for diagonal matrices
  const double abssumoffidag = std::abs(mat[1]) + std::abs(mat[2]) + std::abs(mat[3]) + std::abs(mat[5]) + std::abs(mat[6]) + std::abs(mat[7]);
  if (abssumoffidag < tol) {
    //    std::cout << "Found a diagonal supercell!" << std::endl;
    score += 50000;
  }
  // instead of preferring positive elements, prefer larger Wigner Seitz radius
  double wig = WigSeitzRad(mat);
  double simcell = SimCellRad(mat);
  score += wig/simcell * 1000;

  // Now prefer positive elements that come as early as possible
  /*
  for (int i = 0; i < 9; i++)
  {
    if (mat[i] > 0.0)
    {
      score += 10;
      double v = (9.0-static_cast<double>(i))*0.1;
      score += v*v;
    }
  }
  */
  return score;
}

void getBestTileByScore(const physSys& sys, sortedTilemats& tms, threeThreeMat<int>& tilemat) {
  static const double tol = 0.0000001;
  int bestIndex = 0;
  threeThreeMat<double> super;
  double bestRadius = tms.vec[0].first;
  threeThreeMat<int> tm;
  for (int i = 0; i < 9; i++) {
    tm[i] = tms.vec[0].second[i];
  }
  sys.calcSupercell(tm, super);
  double bestScore = getScore(super);
  //cout << "About to start, best score is: " << bestScore << endl;

  for (int i = 0; i < tms.size; i++) {
    for (int j = 0; j < 9; j++) {
      tm[j] = tms.vec[i].second[j];
    }
    double curRadius = tms.vec[i].first;
    if (curRadius > bestRadius - tol) {
      sys.calcSupercell(tm, super);
      double curScore = getScore(super);
      if (curScore > bestScore) {
	bestScore = curScore;
	bestRadius = curRadius;
	bestIndex = i;
      }
    }
  }

  for (int i = 0; i < 9; i++) {
    tilemat[i] = tms.vec[bestIndex].second[i];
  }
}

// Will require radius within tolerance percent of best and will find maximum symmetry
//void getBestTileBySymAndScore(const physSys& sys, sortedTilemats& tms, int* tilemat, double tolerance) {
void getBestTileBySymAndScore(const physSys& sys, sortedTilemats& tms, threeThreeMat<int>& tilemat, double tolerance) {
  static const double tol = 0.0000001;
  
  int bestIndex = 0;
  double bestRadius = tms.vec[0].first;
  double bestScore = 0.0;
  int bestSym = 0;

  //  cout << "Looking for something within " << tolerance << " percent of the optimal simulation cell radius" << endl;
  const double fractol = 1 - tolerance / 100.0;

  // first do a quick loop and find the best simulation cell radius
  threeThreeMat<int> starttm;
  for (int i = 0; i < tms.size; i++) {
    double curRadius = tms.vec[i].first;
    if (curRadius > bestRadius - tol) {
      bestRadius = curRadius;
      bestIndex = i;
    }
  }
  for (int i = 0; i < 9; i++) {
    starttm[i] = tms.vec[bestIndex].second[i];
  }
  physSys startCandidate(sys, starttm);
  const threeThreeMat<double>& startPtv = startCandidate.getPrim();
  bestScore = getScore(startPtv);
  bestSym = startCandidate.getSymmetry();
						 
#pragma omp parallel
  {
    int threadBestIndex = -1;
    double threadBestRadius = 0.0;
    double threadBestScore = 0.0;
    int threadBestSym = 0;
    
    #pragma omp for
    for (int i = 0; i < tms.size; i++) {
      if (tms.vec[i].first > fractol*threadBestRadius) {
	//int tm[9];
	threeThreeMat<int> tm;
	for (int j = 0; j < 9; j++) {
	  tm[j] = tms.vec[i].second[j];
	}
	
	physSys supercell(sys, tm);
	int numSym = supercell.getSymmetry();
       
	if (numSym >= threadBestSym) {
	  const threeThreeMat<double>& super = supercell.getPrim();
	  double locScore = getScore(super);   
	  
	  if (numSym > threadBestSym || locScore > threadBestScore) {
	    threadBestIndex = i;
	    threadBestRadius = tms.vec[i].first;
	    threadBestScore = locScore;
	    threadBestSym = numSym;
	  }
	}
      }
    }
    
    if (threadBestRadius > fractol*bestRadius && (threadBestSym > bestSym || (threadBestSym == bestSym && threadBestScore > bestScore))) {
#pragma omp critical
      {
	if (threadBestRadius > fractol*bestRadius && (threadBestSym > bestSym || (threadBestSym == bestSym && threadBestScore > bestScore))) {
	  bestIndex = threadBestIndex;
	  //bestRadius = threadBestRadius;
	  bestSym = threadBestSym;
	  bestScore = threadBestScore;
	}
      }
    }
  }

  for (int i = 0; i < 9; i++) {
    tilemat[i] = tms.vec[bestIndex].second[i];
  }
}

   
double WigSeitzRad(const threeThreeMat<double>& mat)
{
  double rmin = 1000000000000000;
  for (int i = -1; i <= 1; i++)
  {
    for (int j = -1; j <= 1; j++)
    {
      for (int k = -1; k <= 1; k++)
      {
        if ((i != 0) || (j != 0) || (k != 0))
        {
          double d[3];
          d[0] = i*mat[0]+j*mat[3]+k*mat[6];
          d[1] = i*mat[1]+j*mat[4]+k*mat[7];
          d[2] = i*mat[2]+j*mat[5]+k*mat[8];
          double dist = 0.5 * sqrt(d[0]*d[0]+d[1]*d[1]+d[2]*d[2]);
          if (dist < rmin)
            rmin = dist;
        }
      }
    }
  }
  return rmin;
}

double SimCellRad(const threeThreeMat<double>& mat) {
  location A;
  location B;
  location C;
  location BxC;
  double radius = 5000000000000000.0;
  for (int i = 0; i < 3; i++)
  {
    const int astart = i*3;
    const int bstart = ((i+1)%3)*3;
    const int cstart = ((i+2)%3)*3;
    for (int j = 0; j < 3; j++)
    {
      A[j] = mat[astart+j];
      B[j] = mat[bstart+j];
      C[j] = mat[cstart+j];
    }
    cross(B,C,BxC);
    double val = std::abs(0.5*dot(A,BxC) / sqrt(dot(BxC,BxC)));
    if (val < radius)
      radius = val;
  }
  return radius;
}

void getPrimKpts(const physSys& prim, const physSys& ss, const location& ssKpt, vector<location>& primKpts, int numCopies) {
  const threeThreeMat<double>& primRlv = prim.getRlv();
  const threeThreeMat<double>& ssRlv = ss.getRlv();
  const threeThreeMat<double>& primPtv = prim.getPrim();

  /*
  cout << "The primitive translation vectors are: \n";
  cout << primPtv[0] << "   " << primPtv[1] << "   " << primPtv[2] << endl;
  cout << primPtv[3] << "   " << primPtv[4] << "   " << primPtv[5] << endl;
  cout << primPtv[6] << "   " << primPtv[7] << "   " << primPtv[8] << endl;
  cout << endl;

  cout << "The primitive cell reciprocal lattice vectors are: \n";
  cout << primRlv[0] << "   " << primRlv[1] << "   " << primRlv[2] << endl;
  cout << primRlv[3] << "   " << primRlv[4] << "   " << primRlv[5] << endl;
  cout << primRlv[6] << "   " << primRlv[7] << "   " << primRlv[8] << endl;
  cout << endl;

  cout << "The supercell reciprocal lattice vectors are: \n";
  cout << ssRlv[0] << "   " << ssRlv[1] << "   " << ssRlv[2] << endl;
  cout << ssRlv[3] << "   " << ssRlv[4] << "   " << ssRlv[5] << endl;
  cout << ssRlv[6] << "   " << ssRlv[7] << "   " << ssRlv[8] << endl;
  cout << endl;
  */

  // search through multiples of the supercell RLV's (with shifts)
  // and find if they belong to the FBZ of the primitive cell
  static const double eps = 1e-8;
  static const int nmax = 14;
  int ns[3];
  for (ns[0] = -nmax; ns[0] <= nmax; ns[0]++) {
    for (ns[1] = -nmax; ns[1] <= nmax; ns[1]++) {
      for (ns[2] = -nmax; ns[2] <= nmax; ns[2]++) {
	
	// get location of G vector
	location g;
	for (int i = 0; i < 3; i++) {
	  g[i] = 0.0;
	}
	for (int i = 0; i < 3; i++) {
	  for (int j = 0; j < 3; j++) {
	    g[j] += (ns[i] + ssKpt[i]) * ssRlv[i*3+j];
	  }
	}
	//	cout << "trying g vector: " << g[0] << "  " << g[1] << "  " << g[2] << endl;

	// Check if it is in FBZ of the primitive lattice
	int inFBZ = 1;
	for (int i = 0; i < 3; i++) {
	  double dotval = 0.0;
	  for (int j = 0; j < 3; j++) {
	    dotval += primPtv[i*3+j]*g[j];
	  }
	  //	  cout << "dotval = " << dotval << endl;
	  if (dotval < -1-eps || dotval > eps) {
	    inFBZ = 0;
	  }
	}

	if (inFBZ) {
	  //cout << "Did we get here?" << endl;
	  location twist;
	  matvec3(primPtv, g, twist);
	  // get only fractional part
	  for (int i = 0; i < 3; i++) {
	    twist[i] -= floor(twist[i]+0.5);
	    // turn all -0.5 components into 0.5
	    // also turn small roundoff errors around 0 to 0
	    if (twist[i] < -0.5+eps) twist[i] += 1.0;
	    if (abs(twist[i]) < eps) twist[i] = 0.0;
	  }
	  location tempg;

	  // Check to make sure we are not adding a duplicate entry
	  int duplicate = 0;
	  for (int i = 0; i < primKpts.size(); i++) {
	    if (abs(twist[0] - primKpts[i][0]) < eps &&
		abs(twist[1] - primKpts[i][1]) < eps &&
		abs(twist[2] - primKpts[i][2]) < eps) {
	      duplicate = 1;
	      i += primKpts.size();
	    }
	  }
	  if (!duplicate) {
	    for (int i = 0; i < 3; i++) {
	      tempg[i] = twist[i];
	    }
	    primKpts.push_back(tempg);
	  }
	}
      }
    }
  }
  if (primKpts.size() != numCopies) {
    cout << "Warning, found " << primKpts.size() << " primitive cell k-points, but expecting " << numCopies << endl;
    for (int i = 0; i < primKpts.size(); i++) {
      cout << i << "  (" << primKpts[i][0] << ", " << primKpts[i][1] << ", " << primKpts[i][2] << ")" << endl;
    }
    exit(1);
  }
}
  
