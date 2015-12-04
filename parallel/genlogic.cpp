#include "genlogic.h"

using namespace std;

uint64_t priceOfX(vector<vector<bool> >& mgraph, set<uint64_t>& xnodes)
{
	uint64_t price = 0;
	
	//Go trough nodes above diagonal
	for (uint64_t mrow = 0; mrow < mgraph.size(); ++mrow){
		for (uint64_t i = mrow + 1; i < mgraph[mrow].size(); ++i){
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

pair<uint64_t, set<uint64_t>*> BBDFS(uint64_t k, uint64_t n, vector<vector<bool> > &mgraph)
{
	//array containing combination
	uint64_t *nodesx = new uint64_t[k];
	//variables 
	uint64_t maxValAtPos, m, priceSet, minPriceSet, lastM = 0, prefixPrice; 

	//set of nodes in combination
	set<uint64_t> *setx, *minSetx;

	//create first kombination
	for (uint64_t i = 0; i < k; ++i){
		nodesx[i] = i;
	}

	//init first set 
	minSetx = new set<uint64_t>(nodesx, nodesx+k);
	setx = minSetx;
	//init minimal price of set
	minPriceSet = priceOfX(mgraph, *minSetx);

	#ifdef _DEBUG
	cout << minPriceSet << ":" << minSetx << "\n";
	#endif
	
	//go trough the rest of combinations 
	for (uint64_t i = 1; i < comb(n, k); ++i){		
		
		m = k - 1; 
		maxValAtPos = n - 1;
		//search for first element from right which is not already maxed
		while (nodesx[m] == maxValAtPos){
			m = m - 1; 
			maxValAtPos = maxValAtPos - 1;
		}
		nodesx[m] += 1;

		if (m > 0 && m < k) {
			setx = new set<uint64_t>(nodesx, nodesx+m);

			if (m != lastM) {
				prefixPrice = priceOfX(mgraph, *setx);
				cout << "prefix " << prefixPrice << ":" << setx << endl;
			}	
			
			delete setx;
			if (prefixPrice >= minPriceSet) {
				lastM = m;
				continue;
			}
		}
		lastM = m;

		//elements after m
		for (uint64_t j = m + 1; j < k; ++j){
			nodesx[j] = nodesx[j - 1] + 1;
		}

		//calculate price for new combination
		setx = new set<uint64_t>(nodesx, nodesx+k);
		priceSet = priceOfX(mgraph, *setx);

		#ifdef _DEBUG
		cout << priceSet << ":" << setx << "\n";
		#endif

		//compare price and keep the smaller one
		if (priceSet < minPriceSet){
			delete minSetx;
			minSetx = setx;
			minPriceSet = priceSet;
		}else{
			delete setx;
		}
		if (nodesx[0] >= n - k) {
			break;
		}
	}
	delete [] nodesx;
	return pair<uint64_t, set<uint64_t>*>(minPriceSet, minSetx);
}
