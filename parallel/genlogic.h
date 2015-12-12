#pragma once

#include "assistfunc.h"
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <set>

#define _DEBUG


uint64_t priceOfX(Array2D<char>& mgraph, std::set<uint64_t>& xnodes);


class CLocalWorker
{
public:
	CLocalWorker(uint64_t k, uint64_t n, uint64_t *startPrefix, uint64_t startPrefixSize,
			 uint64_t *endPrefix, uint64_t endPrefixSize, Array2D<char>& mgraph);
	bool localWorkExists();
	void doLocalWorkStep();
	std::pair<uint64_t, std::set<uint64_t>*> getResults();
private:
	void prepareForLocalWorkStep();

	uint64_t k;
	uint64_t n;
	uint64_t *startPrefix;
	uint64_t startPrefixSize;
	uint64_t *endPrefix;
	uint64_t endPrefixSize;
	Array2D<char>& mgraph;

	//variables 
	uint64_t maxValAtPos, m, priceSet, minPriceSet, lastM = 0, prefixPrice;

	//set of nodes belonging to combination
	std::set<uint64_t> *setx, *minSetx;
};


