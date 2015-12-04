#include "assistfunc.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <set>
#include <cstdint>
#include <string>
#include <limits>
#include <stdexcept>
#include <iterator>

using namespace std;

uint64_t priceOfX(vector<vector<bool> >& mgraph, set<uint64_t>& xnodes);
pair<uint64_t, set<uint64_t>*> BBDFS(uint64_t k, uint64_t n, vector<vector<bool> > &mgraph);