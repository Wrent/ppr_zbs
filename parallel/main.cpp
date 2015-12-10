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
#include <climits>

using namespace std;

#define _DEBUG
#define CHECK_MSG_AMOUNT  100

#define MSG_WORK_REQUEST 1000
#define MSG_WORK_SENT    1001
#define MSG_WORK_NOWORK  1002
#define MSG_TOKEN        1003
#define MSG_FINISH       1004
#define MSG_GRAPH		 1005
#define MSG_GRAPH_SIZE	 1006

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

vector<vector<char> >* prepareGraph(int argc, char * argv[]) {
	if (argc < 5){
		printUsage(argv[0]);
		return NULL;
	}

	//read args
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
		
	vector<vector<char> > *mgraph = new vector<vector<char> >; //variable to store graph

	if (graphfile.is_open()){
		graphfile >> *mgraph;
		graphfile.close();
	}else{
		throw ifstream::failure("unable to open file");
		return NULL;
	}

	#ifdef _DEBUG
	cout << *mgraph << "\n";
	#endif

	return mgraph;
}

bool localWorkExists() {
    return false;
}

void doLocalWorkStep() {

}

int main(int argc, char * argv[])
{
	int p, my_rank, i = 0, flag, askForWorkFrom;
	vector<vector<char> > *mgraph = NULL;

	MPI_Status status;
	MPI_Status recv_status;
	/* start up MPI */
  	MPI_Init( &argc, &argv );

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	askForWorkFrom = my_rank + 1;

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	cout << "processes " << p << endl;
	cout << "my process id " << my_rank << endl; 

	//nachazime se v ridicim procesu
	if (my_rank == 0) {
		uint64_t parA = strtoull(argv[1], NULL, 10);
		uint64_t parN = strtoull(argv[2], NULL, 10);

		mgraph = prepareGraph(argc, argv);

        //pokud nastala nejaka chyba
		if (mgraph == NULL) {
		    cout << "error while reading graph";
			return 1;
		}

		//tady by mel proces nagenerovat praci pro ostatni procesory a rozeslat je
		//rozeslat take samotny nacteny graf vsem procesorum

		//odeslat graf
		for (int i = 1; i < p; i++) {
			//posleme velikost grafu
			int graphSize = mgraph->size() * mgraph[0].size() * sizeof(char);
			MPI_Send(&graphSize, 1, MPI_INT, i, MSG_GRAPH_SIZE, MPI_COMM_WORLD);
			//posleme samotny graf
			MPI_Send(&mgraph->front(), mgraph->size(), MPI_CHAR, i, MSG_GRAPH, MPI_COMM_WORLD);
		}

        //vypocet se bude startovat jinak nez tadytim
		auto&& result = divideWork(parA, parN, *mgraph);
        cout << "\n" << "#edges: " << result.first << "\n" << result.second << "\n";

		delete result.second;
	} else {
	    //tady si ostatni procesory prijmou praci rozeslanou prvnim procesorem
	    //take by to slo tuhle cast vynechat, ze prvni procesor proste zacne pracovat, dostane zadosti o praci a ty vyridi
        int graphSize = 0;
        //prijmout velikost grafu
        MPI_Recv ( &graphSize, 1, MPI_INT, 0, MSG_GRAPH_SIZE, MPI_COMM_WORLD, &status);
        cout << p << " is receiving graph of size " << graphSize << endl;
	    //prijmout graf
	    MPI_Recv ( &mgraph, graphSize, MPI_CHAR, 0, MSG_GRAPH, MPI_COMM_WORLD, &status);
	    cout << p << " received graph." << endl;
	    cout << "graph " << *mgraph << endl;
	}
	MPI_Finalize();
	return 0;

    int recv;
    bool done = false;
	//hlavni pracovni smycka
    while (true) {
        i++;
        if ((i % CHECK_MSG_AMOUNT)==0 || done) {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
            if (flag) {
                //prisla zprava, je treba ji obslouzit
                //v promenne status je tag (status.MPI_TAG), cislo odesilatele (status.MPI_SOURCE)
                //a pripadne cislo chyby (status.MPI_ERROR)
                switch (status.MPI_TAG) {
                    case MSG_WORK_REQUEST :
                                            //prijmeme zpravu
                                            MPI_Recv(&recv, 1, MPI_INT, status.MPI_SOURCE, MSG_WORK_REQUEST, MPI_COMM_WORLD, &recv_status);
                                            if (localWorkExists()) {
                                                //rozdelime si svou praci a pulku posleme procesoru
                                                //rozdelPraci()
                                                //MPI_Send(prace, velikost prace, MPI_CHAR, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD);
                                            } else {
                                                //zadnou praci nemam
                                                MPI_Send(0, 1, MPI_INT, status.MPI_SOURCE, MSG_WORK_NOWORK, MPI_COMM_WORLD);
                                            }
                                            break;
                    case MSG_WORK_SENT :    // prisel rozdeleny zasobnik, prijmout
                                            // deserializovat a spustit vypocet
                                            //MPI_Recv(&prace, velikost prace, MPI_CHAR, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD, &recv_status);
                                            //zpracuj a pokracuj v pocitani
                                            done = false;
                                            break;
                    case MSG_WORK_NOWORK :  // odmitnuti zadosti o praci
                                            MPI_Recv(&recv, 1, MPI_INT, status.MPI_SOURCE, MSG_WORK_REQUEST, MPI_COMM_WORLD, &recv_status);
                                            // zkusit jiny proces
                                            askForWorkFrom = (askForWorkFrom) % p;
                                            if (askForWorkFrom == my_rank) {
                                                done = true;
                                            } else {
                                                MPI_Send(0, 1, MPI_INT, askForWorkFrom, MSG_WORK_REQUEST, MPI_COMM_WORLD);
                                            }
                                            break;
                    case MSG_TOKEN :        //ukoncovaci token, prijmout a nasledne preposlat
                                            MPI_Recv(&recv, 1, MPI_INT, status.MPI_SOURCE, MSG_TOKEN, MPI_COMM_WORLD, &recv_status);
                                            recv = recv && done;
                                            if (my_rank == 0) {
                                                if (recv == 1) {
                                                    //vsichni jsou hotovy
                                                    //odesli finish token
                                                    for (int i = 1; i < p; i++) {
                                                        MPI_Send(0, 1, MPI_INT, i, MSG_FINISH, MPI_COMM_WORLD);
                                                    }
                                                    //min = reseni nalezene procesem 0;

                                                    //a prijmi vysledek
                                                    for (int i = 1; i < p; i++) {
                                                        //MPI_Recv(recvMin, velikost reseni, MPI_CHAR, i, MSG_FINISH, MPI_COMM_WORLD, &recv_status);
                                                        // if (recvMin < min) {
                                                        //      min = recvMin;
                                                        // }

                                                    }
                                                    //vypis vysledek a ukonci se
                                                    // vypis()
                                                    MPI_Finalize();
                                                    exit (0);
                                                }
                                            } else {
                                                MPI_Send(&recv, 1, MPI_INT, (my_rank + 1) % p, MSG_TOKEN, MPI_COMM_WORLD);
                                            }
                                            break;
                    case MSG_FINISH :
                                            if (my_rank != 0) {
                                                MPI_Recv(&recv, 1, MPI_INT, 0, MSG_FINISH, MPI_COMM_WORLD, &recv_status);
                                                //MPI_Send(mojeReseni, velikost reseni, MPI_CHAR, 0, MSG_FINISH, MPI_COMM_WORLD);
                                                //jestlize se meri cas, nezapomen zavolat koncovou barieru MPI_Barrier (MPI_COMM_WORLD)
                                            }
                                            MPI_Finalize();
                                            exit (0);
                                            break;

                    default :               //error;
                                            break;
                }
            }

            if (!localWorkExists()) {
                MPI_Send(0, 1, MPI_INT, askForWorkFrom, MSG_WORK_REQUEST, MPI_COMM_WORLD);
                done = true;

                //nulovy proces take cas od casu rozesle token aby zjisil, jak na tom jsou ostatni procesory a pripadne necha ukoncit praci
                //rozesle to jen pokud sam nic nema
                if (my_rank == 0) {
                //rozesli procesu cislo 1 MSG_TOKEN a pak cekej na prijeti MSG_TOKEN od posledniho (to uz ve switchi)
                    int val = 1;
                    MPI_Send(&val, 1, MPI_INT, (my_rank + 1) % p, MSG_TOKEN, MPI_COMM_WORLD);
                }
            }
        }
        //zde se vola funkce, ktera provede jeden vypocetni krok procesu
        doLocalWorkStep();
    }

	/* shut down MPI */
  	MPI_Finalize();
	return 0;
}
