#include "assistfunc.h"

using namespace std;

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

ifstream& operator>>(ifstream& infile, vector<vector<char> >& mgraph)
{
	uint64_t nodes;
	infile >> dec >> nodes; //Read num of graph nodes
	while (infile.get() != '\n'); //Eat ws

	vector<char> vec;

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

ostream& operator<<(ostream& os, vector<vector<char> >& mgraph)
{
	for (auto row : mgraph){
		for (auto elm : row){
			os << elm << "|";
		}
		os << "\n";
	}
	return os;
}

ostream& operator<<(ostream& os, const set<uint64_t> *mset)
{
	os << '{';
	for (auto a : *mset){
		os << a << (*(--mset->end()) != a ? ',' : '}');
	}
	return os;
}