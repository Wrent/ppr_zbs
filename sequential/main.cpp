#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <cstdint>

using namespace std;


const char* helptext = "a n m k file\n"
	"a = přirozené číslo\n"
	"n = přirozené číslo představující počet uzlů grafu G, n≥5\n"
	"m = přirozené číslo představující počet hran grafu G, m≥n\n"
	"k = přirozené číslo řádu jednotek představující průměrný stupeň uzlu grafu G, n≥k≥3\n";


void printUsage(const char * name)
{
	printf("Usage: %s %s", name, helptext);

}

void readGraph(ifstream &infile, vector<vector<bool> > &graph)
{
	uint64_t nodes;
	infile >> dec >> nodes; //read num of graph nodes
	while (infile.get() != '\n'); //eat ws

	vector<bool> vec;

	for (uint64_t i = 0; i < nodes; ++i){	
		for (uint64_t i = 0; i < nodes; ++i){
			if (infile.get() == '0'){
				vec.push_back(0); //add single bool value to the row
			}else{
				vec.push_back(1);
			}
		}
		while (infile.get() != '\n'); //eat ws
		graph.push_back(vec); //add the row to the main vector
		vec.clear(); //clear vec
	}
}

void printGraph(vector<vector<bool> > &graph)
{
	for (auto row : graph){
		for (auto elm : row){
			cout << elm;
		}
		printf("\n");
	}
}

uint64_t priceOfXY()
{
	uint64_t price;
	//todo
	return price;
}


int main(int argc, char const *argv[])
{
	if (argc != 6){
		printUsage(argv[0]);
		return 1;
	}

	uint64_t parA = strtoull(argv[1], NULL, 10);
	uint64_t parN = strtoull(argv[2], NULL, 10);
	uint64_t parM = strtoull(argv[3], NULL, 10);
	uint64_t parK = strtoull(argv[4], NULL, 10);

	if (parN < 5 || parM < parN || parK > parN || parK < 3){
		printUsage(argv[0]);
		return 1;
	}

	ifstream graphfile;
	graphfile.open(argv[5]);
	
	vector<vector<bool> > graph; //variable to store graph

	if (graphfile.is_open()){
		readGraph(graphfile, graph);
		graphfile.close();
	}else{
		printf("error opening file\n");
		return 1;
	}

	//todo

	//printGraph(graph);
	

	return 0;
}
