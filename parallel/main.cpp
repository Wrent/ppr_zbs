#include "mpi.h"
#include "genlogic.h"
#include "assistfunc.h"
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

vector<vector<bool> >* prepareGraph(int argc, char * argv[]) {
	if (argc < 5){
		printUsage(argv[0]);
		return NULL;
	}

	//read args
	uint64_t parA = strtoull(argv[1], NULL, 10);
	uint64_t parN = strtoull(argv[2], NULL, 10);
	uint64_t parM = strtoull(argv[3], NULL, 10);
	uint64_t parK = strtoull(argv[4], NULL, 10);

	if (parN < 5 || parM < parN || parK > parN || parK < 3){
		printUsage(argv[0]);
		return NULL;
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
		
	vector<vector<bool> > *mgraph = new vector<vector<bool> >; //variable to store graph

	if (graphfile.is_open()){
		graphfile >> mgraph;
		graphfile.close();
	}else{
		throw ifstream::failure("unable to open file");
		return NULL;
	}

	#ifdef _DEBUG
	cout << mgraph << "\n";
	#endif

	return mgraph;
}

int main(int argc, char * argv[])
{
	int p; int my_rank;
	/* start up MPI */
  	MPI_Init( &argc, &argv );

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	cout << "processes " << p << endl;
	cout << "my process id " << my_rank << endl; 

	//nachazime se v ridicim procesu
	if (my_rank == 0) {
		uint64_t parA = strtoull(argv[1], NULL, 10);
		uint64_t parN = strtoull(argv[2], NULL, 10);

		vector<vector<bool> > *mgraph = prepareGraph(argc, argv);

		if (mgraph == NULL) {
			return 1;
		}

		auto&& result = BBDFS(parA, parN, mgraph);
		cout << "\n" << "#edges: " << result.first << "\n" << result.second << "\n";

		delete result.second;
	} else {
		//nachazime se v ostatnich procesech
	}

	/* shut down MPI */
  	MPI_Finalize();
	return 0;
}
