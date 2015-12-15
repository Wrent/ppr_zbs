#pragma once

#include "assistfunc.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <utility>

//#define _DEBUG

#define std::pair<const uint64_t *, uint64_t> pair_set

class CLocalWorker
{
public:
	CLocalWorker(uint64_t k, uint64_t n, Array2D<char>& mgraph, uint64_t my_rank);
	//~CLocalWorker()
	void setPrefixes(uint64_t *startPrefix, uint64_t startPrefixSize,
			 		uint64_t *endPrefix, uint64_t endPrefixSize);
	uint64_t *getStartPrefix();
	uint64_t *getEndPrefix();
	uint64_t getStartPrefixSize();
	uint64_t getEndPrefixSize();
	bool localWorkExists();
	void doLocalWorkStep();
	void printPrefixes();
	uint64_t getMiddlePrefix(uint64_t *);
	std::pair<uint64_t, uint64_t*> getResults();
	uint64_t priceOfX();
private:
	void prepareForLocalWorkStep();

	uint64_t k;
	uint64_t n;
	uint64_t *startPrefix;
	uint64_t startPrefixSize;
	uint64_t *endPrefix;
	uint64_t endPrefixSize;
	uint64_t processRank;
	Array2D<char>& mgraph;

	//variables 
	uint64_t maxValAtPos, m, priceSet, minPriceSet, lastM = 0, prefixPrice;

	//set of nodes belonging to current best combination
	uint64_t *minSetArray;
};


