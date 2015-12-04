#include "assistfunc.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include <set>
#include <cstdint>

using namespace std;

#define _DEBUG

uint64_t priceOfX(vector<vector<bool> >& mgraph, set<uint64_t>& xnodes);
pair<uint64_t, set<uint64_t>*> workUnit(uint64_t k, uint64_t n,
										uint64_t *startPrefix, uint64_t *endPrefix,
										vector<vector<bool> > &mgraph);
pair<uint64_t, set<uint64_t>*> divideWork(uint64_t k, uint64_t n, vector<vector<bool> > &mgraph);