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
#define MSG_PAR_A        1007
#define MSG_PAR_N        1008

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

bool localWorkExists(uint64_t rank) {
    if (rank == 0) {
    return true;}
    return false;
}

void doLocalWorkStep() {

}


pair<uint64_t, uint64_t*> getMiddlePrefix(uint64_t* start, uint64_t startSize, uint64_t* end, uint64_t endSize, uint64_t k, uint64_t n) {
	uint64_t diffPos = 0;
	uint64_t val;
	while (start[diffPos] == end[diffPos]) {
		diffPos++;
	}

	if (diffPos >= startSize || end[diffPos] - start[diffPos] == 1) {
		//prodlouzime endVektor a dame polovicni hodnotu
		diffPos = endSize;
		val = (n + diffPos + 1 + end[diffPos - 1] - k) / 2;
	} else {
		//vratime polovicni hodnotu na diff pozici
		val = (start[diffPos] + end[diffPos]) / 2;
	}

	uint64_t * middle = new uint64_t[k];
	for (uint64_t i = 0; i < diffPos; i++) {
		middle[i] = end[i];
	}
	middle[diffPos] = val;


	return pair<uint64_t, uint64_t*>(diffPos + 1, middle);
}


void printPrefixes(uint64_t p, uint64_t *prefix, uint64_t prefixSize, uint64_t *prefixEnd, uint64_t prefixEndSize) {
    cout << "process " << p << " has:" << endl;
    cout << "prefix ";
    for (uint64_t i = 0; i < prefixSize; i ++) {
        cout << prefix[i] << " ";
    }
    cout << "of size " << prefixSize << endl;
    cout << "prefixEnd ";
    for (uint64_t i = 0; i < prefixEndSize; i ++) {
            cout << prefixEnd[i] << " ";
        }
        cout << "of size " << prefixEndSize << endl;
}


int main(int argc, char * argv[])
{
	int p, my_rank, i = 0, flag, askForWorkFrom;
	vector<vector<char> > *mgraph = NULL;

    uint64_t *prefix, *prefixEnd;
    uint64_t parA, parN;
    uint64_t prefixSize, prefixEndSize;
    bool done = true;

	MPI_Status status;
	MPI_Status recv_status;
	/* start up MPI */
  	MPI_Init( &argc, &argv );

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);


	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	askForWorkFrom = (my_rank + 1) % p;

	cout << "processes " << p << endl;
	cout << "my process id " << my_rank << endl; 

	//nachazime se v ridicim procesu
	if (my_rank == 0) {
		parA = strtoull(argv[1], NULL, 10);
		parN = strtoull(argv[2], NULL, 10);

        //pripravime prefix
        prefix = new uint64_t[parA];
        prefixEnd = new uint64_t[parA];
        prefix[0] = 0;
        prefixEnd[0] = parN - parA;
        prefixSize = 1;
        prefixEndSize = 1;
        done = false;

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
		    //posleme parA
		    MPI_Send(&parA, 1, MPI_UNSIGNED_LONG_LONG, i, MSG_PAR_A, MPI_COMM_WORLD);
            //posleme parN
    		MPI_Send(&parN, 1, MPI_UNSIGNED_LONG_LONG, i, MSG_PAR_N, MPI_COMM_WORLD);

			//posleme velikost grafu
			uint64_t graphSize = mgraph->size() * mgraph[0].size() * sizeof(char);
			MPI_Send(&graphSize, 1, MPI_UNSIGNED_LONG_LONG, i, MSG_GRAPH_SIZE, MPI_COMM_WORLD);
			//posleme samotny graf
			MPI_Send(&mgraph->front(), graphSize, MPI_CHAR, i, MSG_GRAPH, MPI_COMM_WORLD);
		}

        //vypocet se bude startovat jinak nez tadytim
		auto&& result = divideWork(parA, parN, *mgraph);
        cout << "\n" << "#edges: " << result.first << "\n" << result.second << "\n";

		delete result.second;
	} else {
	    //tady si ostatni procesory prijmou praci rozeslanou prvnim procesorem
	    //take by to slo tuhle cast vynechat, ze prvni procesor proste zacne pracovat, dostane zadosti o praci a ty vyridi
        //prijmout parA
        MPI_Recv ( &parA, 1, MPI_UNSIGNED_LONG_LONG, 0, MSG_PAR_A, MPI_COMM_WORLD, &status);
        //prijmout parN
        MPI_Recv ( &parN, 1, MPI_UNSIGNED_LONG_LONG, 0, MSG_PAR_N, MPI_COMM_WORLD, &status);

        prefix = new uint64_t[parA];
        prefixEnd = new uint64_t[parA];
        prefixSize = 0;
        prefixEndSize = 0;

        cout << my_rank << " received parA " << parA << endl;
        cout << my_rank << " received parN " << parN << endl;
        uint64_t graphSize = 0;
        //prijmout velikost grafu
        MPI_Recv ( &graphSize, 1, MPI_UNSIGNED_LONG_LONG, 0, MSG_GRAPH_SIZE, MPI_COMM_WORLD, &status);
        cout << my_rank << " is receiving graph of size " << graphSize << endl;
	    //prijmout graf
	    MPI_Recv ( &mgraph, graphSize, MPI_CHAR, 0, MSG_GRAPH, MPI_COMM_WORLD, &status);
	    cout << my_rank << " received graph." << endl;
	    //cout << "graph " << *mgraph << endl;
	}

    int recv;
    bool requestSent = false;
    cout << my_rank << " entering loop" << endl;
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
                                                    if (localWorkExists(my_rank)) {
                                                        //rozdelime si svou praci a pulku posleme procesoru
                                                        //TODO sem se musi posilat jako start Prefix current Prefix
                                                        pair<uint64_t, uint64_t*> divided = getMiddlePrefix(prefix, prefixSize, prefixEnd, prefixEndSize, parA, parN);

                                                        uint64_t *newPrefix, *newPrefixEnd;
                                                        uint64_t newPrefixSize, newPrefixEndSize;
                                                        newPrefixEnd = prefixEnd;
                                                        newPrefixEndSize = prefixEndSize;
                                                        newPrefix = divided.second;
                                                        newPrefixSize = divided.first;
                                                        prefixEnd = newPrefix;
                                                        prefixEndSize = newPrefixSize;

                                                        printPrefixes(my_rank, prefix, prefixSize, prefixEnd, prefixEndSize);

                                                        MPI_Send(&newPrefixSize, 1, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD);
                                                        MPI_Send(&newPrefixEndSize, 1, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD);
                                                        MPI_Send(&newPrefix, parA, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD);
                                                        MPI_Send(&newPrefixEnd, parA, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD);
                                                    } else {
                                                        //zadnou praci nemam
                                                        recv = 0;
                                                        MPI_Send(&recv, 1, MPI_INT, status.MPI_SOURCE, MSG_WORK_NOWORK, MPI_COMM_WORLD);
                                                    }
                                                    break;
                            case MSG_WORK_SENT :    // prisel rozdeleny zasobnik, prijmout
                                                    // deserializovat a spustit vypocet
                                                    MPI_Recv(&prefixSize, 1, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD, &recv_status);
                                                    cout << my_rank << " received prefixSize " << prefixSize << endl;
                                                    MPI_Recv(&prefixEndSize, 1, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD, &recv_status);
                                                    cout << my_rank << " received prefixEndSize " << prefixEndSize << endl;
                                                    MPI_Recv(prefix, parA, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD, &recv_status);
                                                    cout << my_rank << " received prefix" << endl;
                                                    MPI_Recv(prefixEnd, parA, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD, &recv_status);
                                                    cout << my_rank << " received prefixEnd" << endl;

                                                    printPrefixes(my_rank, prefix, prefixSize, prefixEnd, prefixEndSize);
                                                    done = false;
                                                    requestSent = false;
                                                    break;
                            case MSG_WORK_NOWORK :  // odmitnuti zadosti o praci
                                                    MPI_Recv(&recv, 1, MPI_INT, status.MPI_SOURCE, MSG_WORK_REQUEST, MPI_COMM_WORLD, &recv_status);
                                                    // zkusit jiny proces
                                                    askForWorkFrom = (askForWorkFrom) % p;
                                                    if (askForWorkFrom == my_rank) {
                                                        done = true;
                                                    } else {
                                                        recv = 0;
                                                        MPI_Send(&recv, 1, MPI_INT, askForWorkFrom, MSG_WORK_REQUEST, MPI_COMM_WORLD);
                                                    }
                                                    requestSent = false;
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

            if ((!localWorkExists(my_rank) || done) && !requestSent) {
                recv = 0;
                cout << my_rank << " sending work request to " << askForWorkFrom << endl;
                MPI_Send(&recv, 1, MPI_INT, askForWorkFrom, MSG_WORK_REQUEST, MPI_COMM_WORLD);
                requestSent = true;
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
