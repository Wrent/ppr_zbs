#include "genlogic.h"

int prefixLessEqual(uint64_t *a, uint64_t *b, uint64_t size){
	for (uint64_t i = 0; i < size; ++i){
		if (a[i] > b[i]) return 0;
	}
	return 1;
}

uint64_t priceOfX(Array2D<char> & mgraph, std::set<uint64_t> & xnodes)
{
	uint64_t price = 0;
	
	//Go trough nodes above diagonal
	for (uint64_t mrow = 0; mrow < mgraph.rowSize(); ++mrow){
		for (uint64_t i = mrow + 1; i < mgraph.rowSize(); ++i){
			if (mgraph[mrow][i] && 
				(xnodes.count(mrow) != xnodes.count(i))){
				//When node mrow neighbours with node i and both nodes 
				//are not in the same set increase the price
				price++;
			}
		}
	}
	return price;
}

std::pair<uint64_t, std::set<uint64_t>*> workUnit(uint64_t k, uint64_t n, uint64_t *startPrefix, uint64_t startPrefixSize,
										uint64_t *endPrefix, uint64_t endPrefixSize, Array2D<char> &mgraph)
{
	//variables 
	uint64_t maxValAtPos, m, priceSet, minPriceSet, lastM = 0, prefixPrice; 

	//set of nodes belonging to combination
	std::set<uint64_t> *setx, *minSetx;

	//expand startPrefix to combination of length k
	for (uint64_t i = startPrefixSize; i < k; ++i){
		startPrefix[i] = startPrefix[i-1] + 1;
	}

	//init first set
	minSetx = new std::set<uint64_t>(startPrefix, startPrefix+k);
	setx = minSetx;
	//init minimal price of set
	minPriceSet = priceOfX(mgraph, *minSetx);

	#ifdef _DEBUG
	std::cout << minPriceSet << ":" << minSetx << "\n";
	#endif
	
	//go trough the combinations from startPrefix to endPrefix
	while (1){		
		//index m
		m = k - 1; 
		maxValAtPos = n - 1;
		
		//search for first element from right which is not already maxed
		while (startPrefix[m] == maxValAtPos){
			m = m - 1; 
			maxValAtPos = maxValAtPos - 1;
		}
		//increment found element
		startPrefix[m] += 1;
		
		//break if done everything to endPrefix
		if (!prefixLessEqual(startPrefix, endPrefix, endPrefixSize)) break;

		//check if prefix price is not more or equal then current minimum
		//and if prefix is
		if (m > 0 && m < k) {
			setx = new std::set<uint64_t>(startPrefix, startPrefix+m+1);

			if (m != lastM) {
				prefixPrice = priceOfX(mgraph, *setx);
			}	
			std::cout << "prefix " << prefixPrice << ":" << setx << '\n';

			//delete tmp set
			delete setx;
			//save m
			lastM = (m < lastM ? m : lastM);
			
			if (prefixPrice >= minPriceSet) {
				lastM = (m < lastM ? m : lastM);
				startPrefixSize = lastM;
				//continue;
			}
		}

		//expand combination from m
		for (uint64_t j = m + 1; j < k; ++j){
			startPrefix[j] = startPrefix[j - 1] + 1;
		}

		//calculate price for new combination
		setx = new std::set<uint64_t>(startPrefix, startPrefix+k);
		priceSet = priceOfX(mgraph, *setx);

		#ifdef _DEBUG
		std::cout << priceSet << ":" << setx << "\n";
		#endif

		//compare price and keep the smaller one
		if (priceSet < minPriceSet){
			delete minSetx;
			minSetx = setx;
			minPriceSet = priceSet;
		}else{
			delete setx;
		}
		if (startPrefix[0] >= n - k) {
			break;
		}
	}
	delete [] startPrefix;
	return std::pair<uint64_t, std::set<uint64_t>*>(minPriceSet, minSetx);
}

std::pair<uint64_t, std::set<uint64_t>*> divideWork(uint64_t k, uint64_t n, Array2D<char> &mgraph)
{
	//array containing prefix
	uint64_t *prefix = new uint64_t[k];
	uint64_t *prefixEnd = new uint64_t[k];

	for (int i = 0; i < 1; ++i){
		prefix[i] = i;
	}	
	for (int i = 0; i < 3; ++i){
		prefixEnd[i] = i;
	}
	return workUnit(k, n, prefix, 1, prefixEnd, 3, mgraph);
}
