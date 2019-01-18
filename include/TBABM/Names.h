#pragma once

#include <RNG.h>

using std::vector;
using std::string;

class Names {
public:
	Names(RNG& rng, vector<string> names_ = {}) : rng(rng) {
	  	if (names_.size() != 0)
	  		names = names_;
	  };

	string getName(void) {
		size_t length = names.size();
		auto num = rng.mt_();

		return names.at(static_cast<size_t>(num) % length);
	}

private:
	RNG& rng;
	vector<string> names = 
	#include "Names.inc"
};