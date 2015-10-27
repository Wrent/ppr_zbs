#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <set>
#include <cstdint>
#include <string>

#define DEBUG

using namespace std;


const char* helptext = "a n m k infile\n"
	"a = přirozené číslo\n"
	"n = přirozené číslo představující počet uzlů grafu G, n≥5\n"
	"m = přirozené číslo představující počet hran grafu G, m≥n\n"
	"k = přirozené číslo řádu jednotek představující průměrný stupeň uzlu grafu G, n≥k≥3\n"
	"infile = volitelný parametr pro cestu k souboru s grafem\n";


void printUsage(const char* name)
{
	printf("Usage: %s %s", name, helptext);

}

void readGraph(ifstream &infile, vector<vector<bool> > &mgraph)
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

void printGraph(vector<vector<bool> > &mgraph, ostream &oss)
{
	for (auto row : mgraph){
		for (auto elm : row){
			oss << elm << " ";
		}
		printf("\n");
	}
}

uint64_t priceOfX(vector<vector<bool> > &mgraph, set<uint64_t> &xnodes)
{
	uint64_t price = 0;
	
	//Go trough nodes above diagonal
	for (uint64_t mrow = 0; mrow < mgraph.size(); ++mrow){
		for (uint64_t i = mrow + 1; i < mgraph[mrow].size(); ++i){
			if (mgraph[mrow][i] && xnodes.count(mrow) && !xnodes.count(i)){
				//When node mrow neighbours with node i and node i 
				//is not in same set (X) as node mrow increase the price
				price++;
			}
		}
	}
	return price;
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
		printf("error opening file\n");
		return 1;
	}

	#ifdef DEBUG
	printGraph(mgraph, cout);
	#endif


	//todo

	

	return 0;
}
