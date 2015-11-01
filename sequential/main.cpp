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

using namespace std;
 

//#define _DEBUG

const char* helptext = "a n m k infile\n\n"
	"a = natural number\n"
	"n = natural number representing #nodes of graph G, n>=5\n"
	"m = natural number representing #edges graph G, m>=n\n"
	"k = natural number representing average node degree of graph G, n>=k>=3\n"
	"infile = optional parametr -- path to file containing graph\n";


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

void printUsage(const char* name)
{
	printf("Usage: %s %s", name, helptext);

}

void readGraph(ifstream& infile, vector<vector<bool> >& mgraph)
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
}

void printGraph(vector<vector<bool> >& mgraph, ostream& oss)
{
	for (auto row : mgraph){
		for (auto elm : row){
			oss << elm << "|";
		}
		printf("\n");
	}
}

void printArray(const uint64_t * array, uint64_t len)
{
	for (uint64_t i = 0; i < len; ++i){
		cout << array[i];
	}
	cout << endl;
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

uint64_t countEdgesIntoSet(uint64_t node,uint64_t node2, vector<vector<bool> >& mgraph, set<uint64_t>& xnodes)
{	
	uint64_t edges = 0;
	for (uint64_t i = 0; i < mgraph[node].size(); ++i)
	{
		if (node2 == i)
			continue;
		if (mgraph[node][i] && xnodes.count(i)) {
			edges++;
		}
	}
	#ifdef _DEBUG
	cout << "node " << node << " ";
	cout << "edgesIn " << edges << endl; 
	#endif
	return edges;
}

uint64_t countEdgesOutOfSet(uint64_t node,uint64_t node2, vector<vector<bool> >& mgraph, set<uint64_t>& xnodes)
{	
	uint64_t edges = 0;
	for (uint64_t i = 0; i < mgraph[node].size(); ++i)
	{
		if (node2 == i)
			continue;
		if (mgraph[node][i] && !xnodes.count(i)) {
			edges++;
		}
	}
	#ifdef _DEBUG
	cout << "node " << node << " ";
	cout << "edgesOut " << edges << endl; 
	#endif
	return edges;
}

uint64_t BBDFS(uint64_t k, uint64_t n, vector<vector<bool> >& mgraph)
{
	uint64_t* nodesx = new uint64_t[k];
	uint64_t maxVal, m, price, original, changed, lastPrice, changes, lastVal; 
	set<uint64_t>* setx;
	//create first kombination
	for (uint64_t i = 0; i < k; ++i){
		nodesx[i] = i;
	}
	set<uint64_t>* minSetx = new set<uint64_t>(nodesx, nodesx+k);
	uint64_t minPrice = priceOfX(mgraph, *minSetx);
	lastPrice = minPrice;
	#ifdef _DEBUG
	cout << minPrice << ":";
	printArray(nodesx, k);
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
		original = nodesx[m];
		nodesx[m] += 1;
		changes = 1;
		changed = nodesx[m];
		//elements after m
		for (uint64_t j = m + 1; j < k; ++j){
			lastVal = nodesx[j];
			nodesx[j] = nodesx[j - 1] + 1;
			if (lastVal != nodesx[j]) {
				changes++;
			}
		}

		//calculate price for new combination
		if (changes > 1) {
			setx = new set<uint64_t>(nodesx, nodesx+k);
			price = priceOfX(mgraph, *setx);
		} else {
			price = lastPrice + countEdgesIntoSet(original, changed, mgraph, *setx) - countEdgesOutOfSet(original, changed, mgraph, *setx)
		- countEdgesIntoSet(changed, original, mgraph, *setx) + countEdgesOutOfSet(changed, original, mgraph, *setx);
			setx = new set<uint64_t>(nodesx, nodesx+k);
		}


		#ifdef _DEBUG
		cout << price << ":";
		printArray(nodesx, k);
		#endif

		//compare price and keep the smaller one
		if (price < minPrice){
			delete minSetx;
			minSetx = setx;
			minPrice = price;
		}else{
			delete setx;
		}
		lastPrice = price;
	}
	delete [] nodesx;
	return minPrice;
}

int main(int argc, char const* argv[])
{
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
		readGraph(graphfile, mgraph);
		graphfile.close();
	}else{
		throw ifstream::failure("unable to open file");
		return 1;
	}

	#ifdef _DEBUG
	printGraph(mgraph, cout);
	#endif


	cout << "#edges: " << BBDFS(parA, parN, mgraph) << endl;


	return 0;
}
