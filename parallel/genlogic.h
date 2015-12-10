#pragma once

#include "assistfunc.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include <set>
#include <cstdint>

#define _DEBUG

uint64_t priceOfX(Array2D<char>& mgraph, std::set<uint64_t>& xnodes);
std::pair<uint64_t, std::set<uint64_t>*> workUnit(uint64_t k, uint64_t n,
										uint64_t *startPrefix, uint64_t *endPrefix,
										Array2D<char> &mgraph);
std::pair<uint64_t, std::set<uint64_t>*> divideWork(uint64_t k, uint64_t n, Array2D<char> &mgraph);