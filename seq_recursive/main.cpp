#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <set>
#include <cstdint>
#include <string>
#include <limits>
#include <stdexcept>
#include <cstring>

using namespace std;
 

#define _DEBUG


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

ostream& operator<<(ostream& os, const set<uint64_t>&& mset)
{
	os << '{';
	for (auto a : mset){
		os << a << (*(--mset.end()) != a ? ',' : '}');
	}
	return os;
}

class BisectionWidth
{
public:
	BisectionWidth(vector<vector<bool> >& g, uint64_t n, uint64_t m) 
	: mgraph(g), nodes(n), minW(m) {};
	
	~BisectionWidth()
	{
		delete [] CA;
		delete minSet;
	}

	pair<uint64_t, set<uint64_t>*> getMinWidth(uint64_t k)
	{
		CA = new uint64_t[k+2];
		memset(CA, 0, k+2);
		CAsize = k + 1;

		CA[k+1] = nodes + 1;
		recursionC(k);

		return pair<uint64_t, set<uint64_t>*>(minW, minSet);
	}
private:
	uint64_t* CA;
	uint64_t CAsize;
	set<uint64_t>* minSet;
	set<uint64_t>* mset;
	vector<vector<bool> >& mgraph;
	uint64_t nodes;
	uint64_t minW;

	uint64_t priceOfSet(set<uint64_t>* xnodes)
	{
		uint64_t price = 0;
		//Go through nodes above diagonal
		for (uint64_t mrow = 0; mrow < mgraph.size(); ++mrow){
			for (uint64_t i = mrow + 1; i < mgraph[mrow].size(); ++i){
				if (mgraph[mrow][i] && 
					(xnodes->count(mrow+1) != xnodes->count(i+1))){
					//When node mrow neighbours with node i and both nodes 
					//are not in the same set increase the price
					price++;
				}
			}
		}
		return price;
	}

	void recursionC(uint64_t i)
	{
		//if i is odd
		if (i % 2) {
			for (uint64_t j = CA[i+1] - 1; j >= i; --j){
				CA[i] = j;
				//Create a set from array.
				mset = new set<uint64_t>(CA+1, CA+CAsize);
				uint64_t price = priceOfSet(mset);
				#ifdef _DEBUG
				cout << price << ":" <<  mset << "\n";
				#endif
				//Got a complete combination (set), compute its price
				//and compare the price/width with current minimum.
				if (i == 1){
					if (price < minW){
						minW = price;
						minSet = mset;
					}else 
						delete mset;
				//Not a complete set, check if this branch is viable and continue
				//otherwise return.
				}else{
					if (price <= minW) recursionC(i-1);
				}
			}
		}else{
			for (uint64_t j = i; j <= CA[i+1] - 1; ++j){
				CA[i] = j;
				//Create set from array, get its price and delete it.
				mset = new set<uint64_t>(CA+1, CA+CAsize);
				uint64_t price = priceOfSet(mset);
				#ifdef _DEBUG
				cout << price << ":" <<  mset << "\n";
				#endif
				delete mset;
				//Check if this branch is viable and continue otherwise return.
				if (price <= minW) recursionC(i-1);
			}
		}	
	}
};

void countEdges(vector<vector<bool> >& mgraph, uint64_t& var)
{
	var = 0;
	for (uint64_t mrow = 0; mrow < mgraph.size(); ++mrow){
			for (uint64_t i = mrow + 1; i < mgraph[mrow].size(); ++i){
				if (mgraph[mrow][i]) var++;
			}
	}
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
		graphfile >> mgraph;
		graphfile.close();
	}else{
		throw ifstream::failure("unable to open file");
		return 1;
	}

	//count edges if graph was generated
	if (argc < 6) countEdges(mgraph, parM);

	#ifdef _DEBUG
	cout << mgraph << "\n";
	#endif


	BisectionWidth zbs(mgraph, parN, parM);
	auto&& result = zbs.getMinWidth(parA);
	cout << "\n" << "#edges: " << result.first << "\n"
		 << result.second << "\n";


	return 0;
}
