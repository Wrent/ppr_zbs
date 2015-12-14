#include "mpi.h"
#include "genlogic.h"
#include "assistfunc.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <set>
#include <cstdint>
#include <string>
#include <limits>
#include <stdexcept>
#include <iterator>
#include <climits>

using namespace std;

//#define _DEBUG
#define CHECK_MSG_AMOUNT  5

#define MSG_WORK_REQUEST 1000
#define MSG_WORK_SENT    1001
#define MSG_WORK_NOWORK  1002
#define MSG_TOKEN        1003
#define MSG_FINISH       1004
#define MSG_GRAPH		 1005
#define MSG_GRAPH_SIZE	 1006
#define MSG_PAR_A        1007
#define MSG_PAR_N        1008

Array2D<char> mgraph; //variable to store graph

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

uint64_t prepareGraph(int argc, char * argv[]) {
	if (argc < 5){
		printUsage(argv[0]);
		return 1;
	}

	//read args
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

	return 0;
}

int main(int argc, char * argv[])
{
	int p, my_rank, i = 0, flag, askForWorkFrom;

	CLocalWorker *localWorker;

    uint64_t *prefix, *prefixEnd, *middlePrefix;
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
        middlePrefix = new uint64_t[parA];
        prefix = new uint64_t[parA];
        prefixEnd = new uint64_t[parA];
        prefix[0] = 0;
        prefixEnd[0] = parN - parA;
        prefixSize = 1;
        prefixEndSize = 1;
        done = false;

		auto retcode = prepareGraph(argc, argv);

        //pokud nastala nejaka chyba
		if (retcode) {
		    cout << "error while reading graph";
			return 1;
		}

		//tady by mel proces nagenerovat praci pro ostatni procesory a rozeslat je

		//odeslat graf vsem procesum
		for (int i = 1; i < p; i++) {
		    //posleme parA
		    MPI_Send(&parA, 1, MPI_UNSIGNED_LONG_LONG, i, MSG_PAR_A, MPI_COMM_WORLD);
            //posleme parN
    		MPI_Send(&parN, 1, MPI_UNSIGNED_LONG_LONG, i, MSG_PAR_N, MPI_COMM_WORLD);

			//posleme velikost grafu
			uint64_t graphSize = mgraph.size();
			MPI_Send(&graphSize, 1, MPI_UNSIGNED_LONG_LONG, i, MSG_GRAPH_SIZE, MPI_COMM_WORLD);
			//posleme samotny graf
			MPI_Send(mgraph.getData(), graphSize, MPI_CHAR, i, MSG_GRAPH, MPI_COMM_WORLD);
		}

        //TODO:rozdelit prefixy a poslat nebo si je ostatni vytvori na zacatku sami?

        //vytvoreni instance lokalniho pracovnika pro hlavni process, teda pokud bude pocitat taky
	    localWorker = new CLocalWorker(parA, parN, mgraph, my_rank);
        localWorker->setPrefixes(prefix, prefixSize, prefixEnd, prefixEndSize);
	} else {
	    //tady si ostatni procesory prijmou praci rozeslanou prvnim procesorem
	    //take by to slo tuhle cast vynechat, ze prvni procesor proste zacne pracovat, dostane zadosti o praci a ty vyridi
        //prijmout parA
        MPI_Recv ( &parA, 1, MPI_UNSIGNED_LONG_LONG, 0, MSG_PAR_A, MPI_COMM_WORLD, &status);
        //prijmout parN
        MPI_Recv ( &parN, 1, MPI_UNSIGNED_LONG_LONG, 0, MSG_PAR_N, MPI_COMM_WORLD, &status);

        prefix = new uint64_t[parA];
        prefixEnd = new uint64_t[parA];
        prefix[0] = 1;
        prefixEnd[0] = 0;
        prefixSize = 1;
        prefixEndSize = 1;

        cout << my_rank << " received parA " << parA << endl;
        cout << my_rank << " received parN " << parN << endl;
        uint64_t graphSize = 0;
        //prijmout velikost grafu
        MPI_Recv ( &graphSize, 1, MPI_UNSIGNED_LONG_LONG, 0, MSG_GRAPH_SIZE, MPI_COMM_WORLD, &status);
        cout << my_rank << " is receiving graph of size " << graphSize << endl;
	    //prijmout graf
	    char *buffer = new char[graphSize];
	    MPI_Recv ( buffer, graphSize, MPI_CHAR, 0, MSG_GRAPH, MPI_COMM_WORLD, &status);
	    cout << my_rank << " received graph." << endl;
	    mgraph.setData(buffer);
	    mgraph.setSize(parN);

	    //cout << "graph " << mgraph << endl;

	    //vytvoreni instance lokalniho pracovnika
	    localWorker = new CLocalWorker(parA, parN, mgraph, my_rank);

	    //TODO:vytvorit nebo prijmout pocatectni prefixy

	    //nastavit pocatectni prefixy
	    cout << my_rank << " setting prefix" << endl;
	    localWorker->setPrefixes(prefix, prefixSize, prefixEnd, prefixEndSize);
	}

	
    int recv;
    bool requestSent = false;
    cout << my_rank << " entering loop" << endl;
	//hlavni pracovni smycka
    while (true) {
        i++;
        if ((i % CHECK_MSG_AMOUNT) == 0 || done) {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
            if (flag) {
                //prisla zprava, je treba ji obslouzit
                //v promenne status je tag (status.MPI_TAG), cislo odesilatele (status.MPI_SOURCE)
                //a pripadne cislo chyby (status.MPI_ERROR)
                switch (status.MPI_TAG) {
                            case MSG_WORK_REQUEST :
                                                    cout << my_rank << " received request from " << status.MPI_SOURCE << endl;
                                                    //prijmeme zpravu
                                                    MPI_Recv(&recv, 1, MPI_INT, status.MPI_SOURCE, MSG_WORK_REQUEST, MPI_COMM_WORLD, &recv_status);
                                                    if (localWorker->localWorkExists()) {
                                                        cout << my_rank << " dividing prefixes " << endl;

                                                        localWorker->printPrefixes();
                                                        //rozdelime si svou praci a pulku posleme procesoru

                                                        uint64_t *newPrefix, *newPrefixEnd;
                                                        uint64_t newPrefixSize, newPrefixEndSize;

                                                        newPrefixSize = localWorker->getMiddlePrefix(middlePrefix);

                                                        newPrefixEnd = localWorker->getEndPrefix();
                                                        newPrefixEndSize = localWorker->getEndPrefixSize();
                                                        newPrefix = middlePrefix;

                                                        std::cout << my_rank << " sending:" << std::endl;
                                                            std::cout << "prefix ";
                                                            for (uint64_t i = 0; i < newPrefixSize; i ++) {
                                                                std::cout << newPrefix[i] << " ";
                                                            }
                                                            std::cout << "of size " << newPrefixSize << std::endl;
                                                            std::cout << "prefixEnd ";
                                                            for (uint64_t i = 0; i < newPrefixEndSize; i ++) {
                                                                    std::cout << newPrefixEnd[i] << " ";
                                                                }
                                                                std::cout << "of size " << newPrefixEndSize << std::endl;

                                                        MPI_Send(&newPrefixSize, 1, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD);
                                                        MPI_Send(&newPrefixEndSize, 1, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD);
                                                        MPI_Send(newPrefix, parA, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD);
                                                        MPI_Send(newPrefixEnd, parA, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD);

                                                      	localWorker->setPrefixes(NULL, 0, newPrefix, newPrefixSize);
                                                        localWorker->printPrefixes();
                                                    } else {
                                                        //zadnou praci nemam
                                                        cout << my_rank << " has no work, sending notice to " << status.MPI_SOURCE << endl;
                                                        recv = 0;
                                                        MPI_Send(&recv, 1, MPI_INT, status.MPI_SOURCE, MSG_WORK_NOWORK, MPI_COMM_WORLD);
                                                    }
                                                    break;
                            case MSG_WORK_SENT :    // prisel rozdeleny zasobnik, prijmout
                                                    // deserializovat a spustit vypocet
                                                    localWorker->printPrefixes();
                                                    MPI_Recv(&prefixSize, 1, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD, &recv_status);
                                                    //cout << my_rank << " received prefixSize " << prefixSize << endl;
                                                    MPI_Recv(&prefixEndSize, 1, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD, &recv_status);
                                                   // cout << my_rank << " received prefixEndSize " << prefixEndSize << endl;
                                                    MPI_Recv(prefix, parA, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD, &recv_status);
                                                    //cout << my_rank << " received prefix" << endl;
                                                    MPI_Recv(prefixEnd, parA, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD, &recv_status);
                                                    //cout << my_rank << " received prefixEnd" << endl;

                                                    std::cout << my_rank << " received:" << std::endl;
                                                    std::cout << "prefix ";
                                                    for (uint64_t i = 0; i < prefixSize; i ++) {
                                                        std::cout << prefix[i] << " ";
                                                    }
                                                    std::cout << "of size " << prefixSize << std::endl;
                                                    std::cout << "prefixEnd ";
                                                    for (uint64_t i = 0; i < prefixSize; i ++) {
                                                            std::cout << prefixEnd[i] << " ";
                                                        }
                                                        std::cout << "of size " << prefixEndSize << std::endl;

                                                    localWorker->setPrefixes(prefix, prefixSize, prefixEnd, prefixEndSize);
                                                    localWorker->printPrefixes();
                                                    done = false;
                                                    requestSent = false;
                                                    break;
                            case MSG_WORK_NOWORK :  // odmitnuti zadosti o praci
                                                    cout << my_rank << " received refusal for work from " << status.MPI_SOURCE << endl;
                                                    MPI_Recv(&recv, 1, MPI_INT, status.MPI_SOURCE, MSG_WORK_NOWORK, MPI_COMM_WORLD, &recv_status);
                                                    // zkusit jiny proces


                                                    askForWorkFrom = (askForWorkFrom + 1) % p;
                                                    if (askForWorkFrom == my_rank) {
                                                        done = true;
                                                        cout << my_rank << " didnt find any work" << endl;
                                                        requestSent = true;
                                                    } else {
                                                        recv = 0;
                                                        cout << my_rank << " requests work from " << askForWorkFrom << endl;
                                                        MPI_Send(&recv, 1, MPI_INT, askForWorkFrom, MSG_WORK_REQUEST, MPI_COMM_WORLD);
                                                        requestSent = false;
                                                    }

                                                    break;
                            case MSG_TOKEN :        //ukoncovaci token, prijmout a nasledne preposlat
                                                    MPI_Recv(&recv, 1, MPI_INT, status.MPI_SOURCE, MSG_TOKEN, MPI_COMM_WORLD, &recv_status);
                                                    cout << my_rank << " received end token" << endl;

                                                    recv = recv && done;
                                                    if (my_rank == 0) {
                                                        if (recv == 1) {
                                                            //vsichni jsou hotovy
                                                            //odesli finish token
                                                            cout << my_rank << " sending finish tokens" << endl;
                                                            for (int i = 1; i < p; i++) {
                                                                int val = 0;
                                                                MPI_Send(&val, 1, MPI_INT, i, MSG_FINISH, MPI_COMM_WORLD);
                                                            }
                                                            //min = reseni nalezene procesem 0;

                                                            //a prijmi vysledek
                                                            uint64_t recvMin, min;
                                                            uint64_t *minSet, *setRcv = new uint64_t[parA];
                                                            min = localWorker->getResults().first;
                                                            cout << "0 my result is "<< min << endl;
                                                            minSet = localWorker->getResults().second;
                                                            for (int i = 1; i < p; i++) {
                                                            cout << "aaa";
                                                                //MPI_Recv(&recvMin, 1, MPI_UNSIGNED_LONG_LONG, i, MSG_FINISH, MPI_COMM_WORLD, &recv_status);
                                                                cout << "0 received " << recvMin << " from " << i << endl;
                                                                MPI_Recv(setRcv, parA, MPI_UNSIGNED_LONG_LONG, i, MSG_FINISH, MPI_COMM_WORLD, &recv_status);
                                                                cout << "0 received set from " << i << endl;
                                                                if (recvMin < min) {
                                                                      min = recvMin;
                                                                      memcpy(minSet, setRcv, parA*sizeof(uint64_t));
                                                                }
                                                            }
                                                            std::cout << "Final Result\n#edges: " << min << "\n{";
                                                            for (uint64_t k = 0; k < parA; ++k){
                                                            	std::cout << minSet[k] << (k==(parA-1)? ",":"");
                                                            }
                                                            std::cout << "}\n";
                                                            delete[] minSet;
                                                            delete[] setRcv;
                                                            goto END;
                                                            MPI_Finalize();
                                                            exit (0);
                                                        }
                                                    } else {
                                                        cout << my_rank << " sends end token to " << (my_rank + 1) % p << endl;
                                                        MPI_Send(&recv, 1, MPI_INT, (my_rank + 1) % p, MSG_TOKEN, MPI_COMM_WORLD);
                                                    }
                                                    break;
                            case MSG_FINISH :
                                                    if (my_rank != 0) {

                                                        MPI_Recv(&recv, 1, MPI_INT, 0, MSG_FINISH, MPI_COMM_WORLD, &recv_status);
                                                        std::pair<uint64_t, uint64_t*> result = localWorker->getResults();
                                                        cout << my_rank << " is sending result " << result.first << endl;
                                                        uint64_t min = result.first;
                                                        uint64_t *minSet = result.second;
                                                        MPI_Send(&min, 1, MPI_UNSIGNED_LONG_LONG, 0, MSG_FINISH, MPI_COMM_WORLD);
                                                         cout << my_rank << " result sent " << endl;
                                                        MPI_Send(minSet, parA, MPI_UNSIGNED_LONG_LONG, 0, MSG_FINISH, MPI_COMM_WORLD);
                                                        cout << my_rank << " result set sent " << endl;
                                                        //jestlize se meri cas, nezapomen zavolat koncovou barieru MPI_Barrier (MPI_COMM_WORLD)
                                                    }
                                                    goto END;
                                                    MPI_Finalize();
                                                    exit (0);
                                                    break;

                            default :               //error;
                                                    break;
                }
            }
        }
        if ((!localWorker->localWorkExists() || done)) {
                done = true;
                if (!requestSent) {
                        recv = 0;
                        cout << my_rank << " sending work request to " << askForWorkFrom << endl;
                        MPI_Send(&recv, 1, MPI_INT, askForWorkFrom, MSG_WORK_REQUEST, MPI_COMM_WORLD);
                        requestSent = true;


                        //nulovy proces take cas od casu rozesle token aby zjisil, jak na tom jsou ostatni procesory a pripadne necha ukoncit praci
                        //rozesle to jen pokud sam nic nema
                        if (my_rank == 0) {
                        //rozesli procesu cislo 1 MSG_TOKEN a pak cekej na prijeti MSG_TOKEN od posledniho (to uz ve switchi)
                            int val = 1;
                            MPI_Send(&val, 1, MPI_INT, (my_rank + 1) % p, MSG_TOKEN, MPI_COMM_WORLD);
                        }
                   }
            } else {
            //cout << my_rank << " doing work step" << endl;
            //zde se vola funkce, ktera provede jeden vypocetni krok procesu
            localWorker->doLocalWorkStep();
       	}
    }

END: delete[] prefix;
	delete[] prefixEnd;
	/* shut down MPI */
  	MPI_Finalize();
	return 0;
}

