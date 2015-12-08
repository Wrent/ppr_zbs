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
	int p; int my_rank, i = 0, flag;
	vector<vector<bool> > *mgraph = NULL;

	MPI_Status status;
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

		mgraph = prepareGraph(argc, argv);

        //pokud nastala nejaka chyba
		if (mgraph == NULL) {
			return 1;
		}

		//tady by mel proces nagenerovat praci pro ostatni procesory a rozeslat je
		//rozeslat take samotny nacteny graf vsem procesorum

		//odeslat graf
		for (int i = 1; i < p; i++) {
			MPI_Send(&mgraph->front(), mgraph->size(), MPI_CHAR, i, MSG_GRAPH, MPI_COMM_WORLD);
		}

        //vypocet se bude startovat jinak nez tadytim
		auto&& result = BBDFS(parA, parN, *mgraph);
		cout << "\n" << "#edges: " << result.first << "\n" << result.second << "\n";

		delete result.second;
	} else {
	    //tady si ostatni procesory prijmou praci rozeslanou prvnim procesorem
	    //take by to slo tuhle cast vynechat, ze prvni procesor proste zacne pracovat, dostane zadosti o praci a ty vyridi

	    //prijmout graf
	    MPI_Recv ( mgraph, ULONG_MAX, MPI_CHAR, 0, MSG_GRAPH, MPI_COMM_WORLD, status);
	    cout << p << mgraph << endl;
	}
	return 0;

	//hlavni pracovni smycka
    while (true) {
        i++;
        if ((i % CHECK_MSG_AMOUNT)==0) {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
            if (flag) {
                //prisla zprava, je treba ji obslouzit
                //v promenne status je tag (status.MPI_TAG), cislo odesilatele (status.MPI_SOURCE)
                //a pripadne cislo chyby (status.MPI_ERROR)
                switch (status.MPI_TAG) {
                    case MSG_WORK_REQUEST : // zadost o praci, prijmout a dopovedet
                                            // zaslat rozdeleny zasobnik a nebo odmitnuti MSG_WORK_NOWORK
                                            break;
                    case MSG_WORK_SENT :    // prisel rozdeleny zasobnik, prijmout
                                            // deserializovat a spustit vypocet
                                            break;
                    case MSG_WORK_NOWORK :  // odmitnuti zadosti o praci
                                            // zkusit jiny proces
                                            // a nebo se prepnout do pasivniho stavu a cekat na token
                                            break;
                    case MSG_TOKEN :        //ukoncovaci token, prijmout a nasledne preposlat
                                            // - bily nebo cerny v zavislosti na stavu procesu
                                            break;
                    case MSG_FINISH :       //konec vypoctu - proces 0 pomoci tokenu zjistil, ze jiz nikdo nema praci
                                            //a rozeslal zpravu ukoncujici vypocet
                                            //mam-li reseni, odeslu procesu 0
                                            //nasledne ukoncim spoji cinnost
                                            //jestlize se meri cas, nezapomen zavolat koncovou barieru MPI_Barrier (MPI_COMM_WORLD)
                                            MPI_Finalize();
                                            exit (0);
                                            break;
                    default :               //error;
                                            break;
                }
            }

            if (!localWorkExists()) {
                //kontaktuj jiny proces s zadosti o praci
            }

            //nulovy proces take cas od casu rozesle token aby zjisil, jak na tom jsou ostatni procesory a pripadne necha ukoncit praci
            if (my_rank == 0) {
                //rozesli procesu cislo 1 MSG_TOKEN a pak cekej na prijeti MSG_TOKEN od posledniho (to uz ve switchi)
            }
        }
        //zde se vola funkce, ktera provede jeden vypocetni krok procesu
        doLocalWorkStep();
    }

	/* shut down MPI */
  	MPI_Finalize();
	return 0;
}
