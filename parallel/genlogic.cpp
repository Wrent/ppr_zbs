#include "genlogic.h"

int prefixLessEqual(uint64_t *a, uint64_t *b, uint64_t size){
	for (uint64_t i = 0; i < size; ++i){
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

CLocalWorker::CLocalWorker(uint64_t k, uint64_t n, Array2D<char>& mgraph) 
						: k(k), n(n), mgraph(mgraph)
{
	setPrefixes(NULL, 0, NULL, 0);	
}

void CLocalWorker::setPrefixes(uint64_t *startPrefix, uint64_t&& startPrefixSize,
			 					uint64_t *endPrefix, uint64_t&& endPrefixSize)
{
	startPrefix = startPrefix;
	startPrefixSize = startPrefixSize;
	endPrefix = endPrefix;
	endPrefixSize = endPrefixSize;

	if (startPrefix == NULL || endPrefix == NULL)
	{
		#ifdef _DEBUG
		std::cout << "setPrefixes: prefix/-y je NULL" << '\n';
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
	std::cout << minPriceSet << ":" << minSetx << "\n";
	#endif

	prepareForLocalWorkStep();
}

bool CLocalWorker::localWorkExists()
{
	if (startPrefix == NULL || endPrefix == NULL)
	{
		#ifdef _DEBUG
		std::cout << "error: prefixy nejsou nastaveny!" << '\n';
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
			std::cout << "error prefix overflow" << '\n';
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
		std::cout << "error: prefixy nejsou nastaveny!" << '\n';
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
		std::cout << "prefix " << prefixPrice << ":" << setx << '\n';

		//delete tmp set
		delete setx;
		//save m
		lastM = (m < lastM ? m : lastM);
		
		if (prefixPrice >= minPriceSet) {
			lastM = (m < lastM ? m : lastM);
			startPrefixSize = lastM;
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

	prepareForLocalWorkStep();
}

std::pair<uint64_t, std::set<uint64_t>*> CLocalWorker::getResults()
{
	return std::pair<uint64_t, std::set<uint64_t>*>(minPriceSet, minSetx);
}