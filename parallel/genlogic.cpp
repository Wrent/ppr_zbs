#include "genlogic.h"

int prefixLessEqual(uint64_t *a, uint64_t *b, uint64_t size){
	for (uint64_t i = 0; i < size; ++i){
		cout << "checking " << a[i] << " < " << b[i] << endl;
		if (a[i] > b[i]) return 0;
	}
	return 1;
}


uint64_t priceOfX(Array2D<char>& mgraph, std::set<uint64_t>& xnodes)
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

CLocalWorker::CLocalWorker(uint64_t k, uint64_t n, Array2D<char>& mgraph, uint64_t processRank)
						: k(k), n(n), processRank(processRank), mgraph(mgraph)
{
	minSetx = setx = NULL;
	startPrefix = endPrefix = NULL;
}

void CLocalWorker::setPrefixes(uint64_t *start, uint64_t startSize,
			 					uint64_t *end, uint64_t endSize)
{
	if (start != NULL){
		if (startPrefix != NULL) delete[] startPrefix;	
		startPrefix = start;
		startPrefixSize = startSize;
	}
	
	if (end != NULL){
		if (endPrefix != NULL) delete[] endPrefix;
		endPrefix = end;
		endPrefixSize = endSize;
	}

	if (start == NULL || end == NULL){
		#ifdef _DEBUG
		std::cout << processRank << " setPrefixes: prefix(-y) nezmenen(-y)!" << '\n';
		#endif
		return;
	}


	//expand startPrefix to combination of length k
	for (uint64_t i = startPrefixSize; i < k; ++i){
		startPrefix[i] = startPrefix[i-1] + 1;
	}

	//init first set
	if (setx != NULL) delete setx;
	minSetx = new std::set<uint64_t>(startPrefix, startPrefix+k);
	setx = minSetx;
	//init minimal price of set
	minPriceSet = priceOfX(mgraph, *minSetx);

	#ifdef _DEBUG
	std::cout << processRank << " " << minPriceSet << ":" << minSetx << "\n";
	#endif

	prepareForLocalWorkStep();
}

bool CLocalWorker::localWorkExists()
{
	if (startPrefix == NULL || endPrefix == NULL)
	{
		#ifdef _DEBUG
		std::cout << processRank << " error: prefixy nejsou nastaveny!" << '\n';
		#endif
		return false;
	}

	//break if done everything to endPrefix
	if (!prefixLessEqual(startPrefix, endPrefix, endPrefixSize)) return false;

	//yes it does
	return true;
}

void CLocalWorker::prepareForLocalWorkStep()
{
	//index m
	m = k - 1; 
	maxValAtPos = n - 1;
	
	//search for first element from right which is not already maxed
	while (startPrefix[m] == maxValAtPos){
		//check prefix bound
		if (m <= 0) return;
		if (startPrefix[m] > maxValAtPos) {
			std::cout << processRank << " error prefix overflow" << '\n';
			return;
		}
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
		std::cout << processRank << " error: prefixy nejsou nastaveny!" << '\n';
		#endif
		return;
	}

	//check if prefix price is not more or equal then current minimum
	//and if prefix is
	if (m > 0 && m < k) {

		setx = new std::set<uint64_t>(startPrefix, startPrefix+m+1);

		if (m != lastM) {
			prefixPrice = priceOfX(mgraph, *setx);
		}
		std::cout << processRank << " prefix " << prefixPrice << ":" << setx << '\n';

		//delete tmp set
		delete setx;
		//save m
		lastM = (m < lastM ? m : lastM);
		
		if (prefixPrice >= minPriceSet) {
			lastM = (m < lastM ? m : lastM);
			startPrefixSize = lastM;
			prepareForLocalWorkStep();
			return;
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
	std::cout << processRank << " " << priceSet << ":" << setx << "\n";
	#endif

	//compare price and keep the smaller one
	if (priceSet < minPriceSet){
		delete minSetx;
		minSetx = setx;
		minPriceSet = priceSet;
	}else{
		delete setx;
	}

	prepareForLocalWorkStep();
}

std::pair<uint64_t, std::set<uint64_t>*> CLocalWorker::getResults()
{
	return std::pair<uint64_t, std::set<uint64_t>*>(minPriceSet, minSetx);
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
    std::cout << "prefix ";
    for (uint64_t i = 0; i < startPrefixSize; i ++) {
        std::cout << startPrefix[i] << " ";
    }
    std::cout << "of size " << startPrefixSize << std::endl;
    std::cout << "prefixEnd ";
    for (uint64_t i = 0; i < endPrefixSize; i ++) {
            std::cout << endPrefix[i] << " ";
        }
        std::cout << "of size " << endPrefixSize << std::endl;
}

std::pair<uint64_t, uint64_t*> CLocalWorker::getMiddlePrefix() {
    	uint64_t diffPos = 0;
    	uint64_t val;
    	while (startPrefix[diffPos] == endPrefix[diffPos]) {
    		diffPos++;
    	}

    	if (diffPos >= startPrefixSize || endPrefix[diffPos] - startPrefix[diffPos] == 1) {
    		//prodlouzime endVektor a dame polovicni hodnotu
    		diffPos = endPrefixSize;
    		val = (n + diffPos + 1 + endPrefix[diffPos - 1] - k) / 2;
    	} else {
    		//vratime polovicni hodnotu na diff pozici
    		val = (startPrefix[diffPos] + endPrefix[diffPos]) / 2;
    	}

    	uint64_t * middle = new uint64_t[k];
    	for (uint64_t i = 0; i < diffPos; i++) {
    		middle[i] = endPrefix[i];
    	}
    	middle[diffPos] = val;


    	return std::pair<uint64_t, uint64_t*>(diffPos + 1, middle);
}