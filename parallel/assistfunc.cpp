#include "assistfunc.h"

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
        throw std::invalid_argument("k > n");

    uint64_t r = 1;
    for (uint64_t d = 1; d <= k; ++d, --n){
        uint64_t g = gcd(r, d);
        r /= g;
        uint64_t t = n / (d / g);
        if (r > std::numeric_limits<uint64_t>::max() / t)
           throw std::overflow_error("overflow in comb()");
        r *= t;
    }
    return r;
}


template <class Type>
Type * Array2D<Type>::mdata()
{
	if (_mdata == NULL)
	{
		_mdata = new Type[msize];
	}
	return _mdata;
}

template <class Type>
Array2D<Type>::~Array2D()
{
	delete [] mdata;
}

template <class Type>
uint64_t Array2D<Type>::rowSize()
{
	return mrowSize;
}

template <class Type>
uint64_t Array2D<Type>::size()
{
	return msize;
}

template <class Type>
void Array2D<Type>::setData(Type * data, uint64_t rowSize)
{
	delete [] mdata;
	mdata = data;
}

template <class Type>
void Array2D<Type>::setSize(uint64_t rowSize)
{
	mrowSize = rowSize;
	msize = rowSize * rowSize;
}

template <class Type>
Type * Array2D<Type>::getData()
{
	return mdata;
}

template <class Type>
Type * Array2D<Type>::operator[](uint64_t x)
{
	return (mdata + (x * msize));
}

template <class Type>
std::ostream& operator<<(std::ostream& os, Array2D<Type> & obj)
{
	for (uint64_t i = 0; i < obj.msize; ++i){
		os << obj.mdata[i] << "|";
		if (i == obj.mrowSize) os << "\n";
	}
	return os;
}

template <class Type>
std::ifstream& operator>>(std::ifstream& infile, Array2D<Type> & obj)
{
	uint64_t nodes;
	infile >> std::dec >> nodes; //Read num of graph nodes
	while (infile.get() != '\n'); //Eat Whitespaces

	for (uint64_t i = 0; i < nodes; ++i){	
		for (uint64_t j = 0; j < nodes; ++j){
			if (infile.get() == '0'){
				obj[i][j] = 0;
			}else{
				obj[i][j] = 1; 
			}
		}
		while (infile.get() != '\n'); //Eat ws
	}
	return infile;
}


std::ifstream& operator>>(std::ifstream& infile, std::vector<std::vector<bool> >& mgraph)
{
	uint64_t nodes;
	infile >> std::dec >> nodes; //Read num of graph nodes
	while (infile.get() != '\n'); //Eat ws

	std::vector<bool> vec;

	for (uint64_t i = 0; i < nodes; ++i){	
		for (uint64_t j = 0; j < nodes; ++j){
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



std::ostream& operator<<(std::ostream& os, std::vector<std::vector<bool> >& mgraph)
{
	for (auto row : mgraph){
		for (auto elm : row){
			os << elm << "|";
		}
		os << "\n";
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const std::set<uint64_t> *mset)
{
	os << '{';
	for (auto a : *mset){
		os << a << (*(--mset->end()) != a ? ',' : '}');
	}
	return os;
}