#pragma once

#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <vector>
#include <limits>
#include <stdexcept>
#include <utility>

//declare Array2D
template <class Type> class Array2D;

//declare tempates
template <class Type>
std::ostream& operator<< (std::ostream& os, Array2D<Type>& obj);
template <class Type>
std::ifstream& operator>> (std::ifstream& infile, Array2D<Type>& obj);

//define class Array2D
template <class Type>
class Array2D
{
public:
	Array2D();
	~Array2D();
	uint64_t rowSize();
	uint64_t size();
	void setData(Type *data);
	void setSize(uint64_t rowSize);
	Type *getData();
	Type *operator[](uint64_t x);
	friend std::ostream& operator<< <Type> (std::ostream& os, Array2D<Type>& obj);
	friend std::ifstream& operator>> <Type> (std::ifstream& infile, Array2D<Type>& obj);
private:
	Type *mdata;
	uint64_t mrowSize;
	uint64_t msize;
};


//functions
uint64_t gcd(uint64_t x, uint64_t y);
uint64_t comb(uint64_t n, uint64_t k);

//vector bool
std::ifstream& operator>>(std::ifstream& infile, std::vector<std::vector<bool> >& mgraph);
std::ostream& operator<<(std::ostream& os, std::vector<std::vector<bool> >& mgraph);
//set
std::ostream& operator<<(std::ostream& os, std::pair<const uint64_t *, uint64_t> mset);
