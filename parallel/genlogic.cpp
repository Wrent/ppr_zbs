#include "genlogic.h"

bool prefixMoreOrEqualThan(uint64_t *a, uint64_t *b, uint64_t size){
	bool same = true;
	for (uint64_t i = 0; i < size; ++i){
		//std::cout << "checking " << a[i] << " < " << b[i] << std::endl;
		if (a[i] > b[i]) return true;
		if (a[i] != b[i]) same = false;
	}
	if (same) return true;
	return false;
}


uint64_t CLocalWorker::priceOfX(uint64_t size)
{
	uint64_t price = 0;
	uint64_t *xbegin = startPrefix;
	uint64_t *xend = xbegin + size;
	
	//Go trough nodes above diagonal
	for (uint64_t mrow = 0; mrow < mgraph.rowSize(); ++mrow){
		for (uint64_t i = mrow + 1; i < mgraph.rowSize(); ++i){
			if (mgraph[mrow][i] && 
				((std::find(xbegin,xend,mrow) != xend) != (std::find(xbegin,xend,i) != xend))){
				//When node mrow neighbours with node i and both nodes 
				//are not in the same set increase the price
				price++;
			}
		}
	}
	return price;
}

CLocalWorker::CLocalWorker(uint64_t k, uint64_t n, Array2D<char>& mgraph, uint64_t processRank)
						: k(k), n(n), processRank(processRank), mgraph(mgraph)
{
	startPrefix = endPrefix = NULL;
	minPriceSet = -1;
	errorFlag = false;
	minSetArray = new uint64_t[k];
	for (uint64_t i = 0; i < k; ++i)
	{
		minSetArray[i] = i;
	}
	
}

void CLocalWorker::setPrefixes(uint64_t *start, uint64_t startSize,
			 					uint64_t *end, uint64_t endSize)
{
	if (start != NULL){
		startPrefix = start;
		startPrefixSize = startSize;
	}
	
	if (end != NULL){
		endPrefix = end;
		endPrefixSize = endSize;
	}

	if (start == NULL || end == NULL){
		#ifdef _DEBUG
		std::cout << processRank << " Info: setPrefixes: prefix(-y) nezmenen(-y)" << '\n';
		#endif
	}

	//expand startPrefix to combination of length k
	for (uint64_t i = startPrefixSize; i < k; ++i){
		startPrefix[i] = startPrefix[i-1] + 1;
	}

	//expand endPrefix
	for (uint64_t i = endPrefixSize; i < k; ++i){
		endPrefix[i] = endPrefix[i-1] + 1;
	}

	priceSet = priceOfX(k);
	//init minimal price of set if it is not
	if (minPriceSet == (uint64_t)-1){
		minPriceSet = priceSet;
	//compare first combination and its price
	}else if (priceSet < minPriceSet){
		minPriceSet = priceSet;
		memcpy(minSetArray, startPrefix, k*sizeof(uint64_t));
	}
	
	//reset lastM
	lastM = -1;

	//#ifdef _DEBUG
	std::cout << "[" << processRank << "]setPrefixes " << "(" << minPriceSet << "):" << pair_set(startPrefix,k) << "\n";
	//#endif

	prepareForLocalWorkStep();
}

bool CLocalWorker::localWorkExists()
{
	if (startPrefix == NULL || endPrefix == NULL)
	{
		#ifdef _DEBUG
		std::cout << "[" << processRank << "]error: prefixy nejsou nastaveny!" << '\n';
		#endif
		return false;
	}

	//true if done everything to endPrefix
	if (prefixMoreOrEqualThan(startPrefix, endPrefix, k)) return false;
	//check for error occurence
	if (errorFlag) return false;

	//yes it does and everything is in check
	return true;
}

void CLocalWorker::prepareForLocalWorkStep()
{
	//index m
	m = k - 1; 
	maxValAtPos = n - 1;
	
	//search for first element from right which is not already maxed
	while (startPrefix[m] >= maxValAtPos){
		//check prefix bounds
		if (startPrefix[m] > maxValAtPos) {
			std::cout << "[" << processRank << "]error prefix overflow:" << pair_set(startPrefix,m) << '\n';
			errorFlag = true;
			return;
		}
		//dont do anything if prefix size is 0
		if (m <= 0) return;

		m = m - 1; 
		maxValAtPos = maxValAtPos - 1;
	}
	//increment found element
	startPrefix[m] += 1;
}

void CLocalWorker::doLocalWorkStep()
{
	if (startPrefix == NULL || endPrefix == NULL)
	{
		#ifdef _DEBUG
		std::cout << "[" << processRank << "]error: prefixy nejsou nastaveny!" << '\n';
		#endif
		return;
	}

	if (m > 0){
		//check if prefix price is not more or equal then current minimum
		//and if prefix is
		if (m != lastM) {
			//save last m
			lastM = m;
			prefixPrice = priceOfX(m);

			//skip prefix with worse solution then current
			if (prefixPrice >= minPriceSet) {
				//#ifdef _DEBUG
				std::cout << "[" << processRank << "]skip-prefix " << "(" << prefixPrice << "):" << pair_set(startPrefix,m) << "\n";
				//#endif
				startPrefix[m] = n - k + m;
				prepareForLocalWorkStep();
				return;
			}
		}		
	}	
	
	//expand combination from m
	for (uint64_t j = m + 1; j < k; ++j){
		startPrefix[j] = startPrefix[j - 1] + 1;
	}

	//calculate price for new combination
	priceSet = priceOfX(k);

	#ifdef _DEBUG
	std::cout << "[" << processRank << "]doLocalWorkStep " << "(" << priceSet << "):" << pair_set(startPrefix,k) << "\n";
	#endif

	//compare price and keep the smaller one
	if (priceSet < minPriceSet){
		minPriceSet = priceSet;
		memcpy(minSetArray, startPrefix, k*sizeof(uint64_t));
	}

	prepareForLocalWorkStep();
}

std::pair<uint64_t, uint64_t*> CLocalWorker::getResults()
{
	return std::pair<uint64_t, uint64_t*>(minPriceSet, minSetArray);
}

uint64_t *CLocalWorker::getStartPrefix() {
	return startPrefix;
}

uint64_t *CLocalWorker::getEndPrefix() {
	return endPrefix;
}

uint64_t CLocalWorker::getStartPrefixSize() {
	return startPrefixSize;
}

uint64_t CLocalWorker::getEndPrefixSize() {
	return endPrefixSize;
}

void CLocalWorker::printPrefixes() {
    std::cout << "process " << processRank << " has:" << std::endl;
    std::cout << processRank<< "prefix ";
    for (uint64_t i = 0; i < startPrefixSize; i ++) {
        std::cout << startPrefix[i] << " ";
    }
    std::cout << "of size " << startPrefixSize << std::endl;
    std::cout << processRank << "prefixEnd ";
    for (uint64_t i = 0; i < endPrefixSize; i ++) {
            std::cout << endPrefix[i] << " ";
        }
        std::cout << "of size " << endPrefixSize << std::endl;
}

uint64_t CLocalWorker::getMiddlePrefix(uint64_t *middle) {
    	uint64_t diffPos = 0;
    	uint64_t val, prev;
    	while (startPrefix[diffPos] == endPrefix[diffPos]) {
    		diffPos++;
    	}
		//std::cout << processRank << " aa " << std::endl;
    	if (diffPos >= startPrefixSize || (endPrefix[diffPos] - startPrefix[diffPos]) == 1) {
    		//prodlouzime endVektor a dame polovicni hodnotu
    		if (diffPos == 0) {
				prev = endPrefix[0];
    		} else {
    			prev = endPrefix[diffPos - 1];
    		}
    		diffPos = endPrefixSize;
    		val = (n + diffPos + 1 + prev - k) / 2;
    		//std::cout << processRank << " aa" << std::endl;
    	} else {
    		//vratime polovicni hodnotu na diff pozici
    		val = (startPrefix[diffPos] + endPrefix[diffPos]) / 2;

    		//std::cout << processRank << " bb" << std::endl;
    	}
    	//std::cout << processRank << " bb " << std::endl;
    	for (uint64_t i = 0; i < diffPos; i++) {
    		middle[i] = endPrefix[i];
    	}
    	middle[diffPos] = val;

		//std::cout << processRank << " cc " << std::endl;
    	return diffPos + 1;
}