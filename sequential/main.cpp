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
#include <time.h>

using namespace std;
 

#define _DEBUG

uint64_t skip;

void printUsage(const char* name)
{
	const char* helptext = "a n m k infile\n\n"
	"a = natural number\n"
	"n = natural number representing #nodes of graph G, n>=5\n"
	"m = natural number representing #edges graph G, m>=n\n"
	"k = natural number representing average node degree of graph G, n>=k>=3\n"
	"infile = optional parametr -- path to file containing graph\n";

	printf("Usage: %s %s", name, helptext);
}

uint64_t gcd(uint64_t x, uint64_t y)
{
    while (y != 0){
        uint64_t t = x % y;
        x = y;
        y = t;
    }
    return x;
}

uint64_t comb(uint64_t n, uint64_t k)
{
    if (k > n)
        throw invalid_argument("k > n");

    uint64_t r = 1;
    for (uint64_t d = 1; d <= k; ++d, --n){
        uint64_t g = gcd(r, d);
        r /= g;
        uint64_t t = n / (d / g);
        if (r > numeric_limits<uint64_t>::max() / t)
           throw overflow_error("overflow in comb()");
        r *= t;
    }
    return r;
}

ifstream& operator>>(ifstream& infile, vector<vector<bool> >& mgraph)
{
	uint64_t nodes;
	infile >> dec >> nodes; //Read num of graph nodes
	while (infile.get() != '\n'); //Eat ws

	vector<bool> vec;

	for (uint64_t i = 0; i < nodes; ++i){	
		for (uint64_t i = 0; i < nodes; ++i){
			if (infile.get() == '0'){
				vec.push_back(0); //Add single bool value to the row
			}else{
				vec.push_back(1);
			}
		}
		while (infile.get() != '\n'); //Eat ws
		mgraph.push_back(vec); //Add the row to the main vector
		vec.clear(); //Clear vector vec
	}
	return infile;
}

ostream& operator<<(ostream& os, vector<vector<bool> >& mgraph)
{
	for (auto row : mgraph){
		for (auto elm : row){
			os << elm << "|";
		}
		os << "\n";
	}
	return os;
}

ostream& operator<<(ostream& os, const set<uint64_t>* mset)
{
	os << '{';
	for (auto a : *mset){
		os << a << (*(--mset->end()) != a ? ',' : '}');
	}
	return os;
}

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

pair<uint64_t, set<uint64_t>*> BBDFS(uint64_t k, uint64_t n, vector<vector<bool> >& mgraph)
{
	uint64_t* nodesx = new uint64_t[k];
	uint64_t maxVal, m, price, changes, lastVal, lastM = 0, prefixPrice; 
	set<uint64_t>* setx;
	//create first kombination
	for (uint64_t i = 0; i < k; ++i){
		nodesx[i] = i;
	}
	set<uint64_t>* minSetx = new set<uint64_t>(nodesx, nodesx+k);
	uint64_t minPrice = priceOfX(mgraph, *minSetx);

	#ifdef _DEBUG
	cout << minPrice << ":" << minSetx << "\n";
	#endif

	setx = minSetx;
	
	//go trough the rest of combinations 
	for (uint64_t i = 1; i < comb(n, k); ++i){		
		
		m = k - 1; 
		maxVal = n - 1;
		//search for first element from right which is not already maxed
		while (nodesx[m] == maxVal){
			m = m - 1; 
			maxVal = maxVal - 1;
		}
		nodesx[m] += 1;

		if (m > 0 && m < k) {
			setx = new set<uint64_t>(nodesx, nodesx+m);

			if (m != lastM) {
				prefixPrice = priceOfX(mgraph, *setx);
				cout << "prefix " << prefixPrice << ":" << setx << endl;
			}	
			
			delete setx;
			if (prefixPrice >= minPrice) {
				lastM = m;
				skip++;
				continue;
			}
		}
		lastM = m;

		changes = 1;
		//elements after m
		for (uint64_t j = m + 1; j < k; ++j){
			lastVal = nodesx[j];
			nodesx[j] = nodesx[j - 1] + 1;
			if (lastVal != nodesx[j]) {
				changes++;
			}
		}

		//calculate price for new combination
		setx = new set<uint64_t>(nodesx, nodesx+k);
		price = priceOfX(mgraph, *setx);

		#ifdef _DEBUG
		cout << price << ":" << setx << "\n";
		#endif

		//compare price and keep the smaller one
		if (price < minPrice){
			delete minSetx;
			minSetx = setx;
			minPrice = price;
		}else{
			delete setx;
		}
		if (nodesx[0] >= n - k) {
			break;
		}
	}
	//delete [] nodesx;
	return pair<uint64_t, set<uint64_t>*>(minPrice, minSetx);
}

int main(int argc, char const* argv[])
{
	const clock_t begin_time = clock();

	if (argc < 5){
		printUsage(argv[0]);
		return 1;
	}

	//read args
	uint64_t parA = strtoull(argv[1], NULL, 10);
	uint64_t parN = strtoull(argv[2], NULL, 10);
	uint64_t parM = strtoull(argv[3], NULL, 10);
	uint64_t parK = strtoull(argv[4], NULL, 10);

	if (parN < 5 || parM < parN || parK > parN || parK < 3){
		printUsage(argv[0]);
		return 1;
	}

	ifstream graphfile;

	if (argc < 6){ //generate graph when none is given
		string cmd;
		cmd = cmd + "generator" + " -t AD -n " + argv[2] + " -k " + argv[4] + " -o _graph";
		system(cmd.c_str());
		cmd.clear();
		cmd = cmd + "souvislost" + " -s -i _graph -o _graph";
		system(cmd.c_str());
		graphfile.open("_graph");
	}else{
		graphfile.open(argv[5]);
	}
	
	vector<vector<bool> > mgraph; //variable to store graph

	if (graphfile.is_open()){
		graphfile >> mgraph;
		graphfile.close();
	}else{
		throw ifstream::failure("unable to open file");
		return 1;
	}

	#ifdef _DEBUG
	cout << mgraph << "\n";
	#endif


	auto&& result = BBDFS(parA, parN, mgraph);
	cout << "\n" << "#edges: " << result.first << "\n" << result.second << "\n";

	#ifdef _DEBUG
	cout << "skipped: " << skip << "/" << comb(parN,parA) << "\n";
	#endif

	delete result.second;
	std::cout << "calculation time: " << float( clock () - begin_time ) /  CLOCKS_PER_SEC << endl;
	return 0;
}
