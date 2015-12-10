#include <cstdlib>
#include <fstream>
#include <vector>
#include <set>
#include <cstdint>
#include <limits>
#include <stdexcept>

using namespace std;

uint64_t gcd(uint64_t x, uint64_t y);
uint64_t comb(uint64_t n, uint64_t k);
ifstream& operator>>(ifstream& infile, vector<vector<char> >& mgraph);
ostream& operator<<(ostream& os, vector<vector<char> >& mgraph);
ostream& operator<<(ostream& os, const set<uint64_t> *mset);