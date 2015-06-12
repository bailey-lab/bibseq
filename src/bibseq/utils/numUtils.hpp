#pragma once
/*

 * numUtils.hpp
 *
 *  Created on: Jun 22, 2014
 *      Author: nickhathaway
 */

#include "bibseq/utils/utils.hpp"

namespace bibseq {
template<typename T>
class scale {
public:

	scale(const std::pair<T,T> & in,
			const std::pair<T,T> & out):
				min_(in.first), max_(in.second),
				start_(out.first), stop_(out.second){

	}

	T min_;
	T max_;


	T start_;
	T stop_;

	T get(T num){
		auto ret = start_ + (stop_ - start_) * ((num - min_)/(std::abs(max_ - min_)));
		return ret;
	}
};

template<typename T>
std::vector<T> getIntRange(T start, T stop){
	std::vector<T> ret;
	if(start == stop){
		return std::vector<T> {start};
	}else if (start > stop){
		ret = std::vector<T>(start - stop + 1);
		std::iota(ret.begin(), ret.end(), stop);
		std::reverse(ret.begin(), ret.end());
	}else if (start < stop){
		ret = std::vector<T>(stop - start + 1);
		std::iota(ret.begin(), ret.end(), start);
	}
	return ret;
}


inline bool isPrime (uint64_t num) {
	if(num % 2 == 0){
		if(num == 2){
			return true;
		}else{
			return false;
		}
	} else {
		uint64_t stop = std::sqrt(num);
		for(uint64_t i = 3; i <= stop; i+=2){
			if(num % i == 0){
				return false;
			}
		}
	}
	return true;
}
inline double roundDecPlaces(double num, int decPlaces) {
  double rounder = pow(10, decPlaces);
  return (floor(num * rounder + 0.5) / rounder);
}

template <typename T>
T Factorial(T x) {
  return (x == 1 ? x : x * Factorial(x - 1));
}

std::vector<double> getRange(double start, double stop, uint32_t num);

inline unsigned uAbsdiff( unsigned a, unsigned b ){
      unsigned n= (unsigned)(
         (long long)((unsigned long long)a - (unsigned long long)b)>>32
                      ); // same n as 2nd example
      unsigned result = a-b;
      return (result^n)-n; // 'result' if n = 0; '-result' if n = 0xFFFFFFFF
}

} /* namespace bib */

#ifndef NOT_HEADER_ONLY
#include "numUtils.cpp"
#endif


