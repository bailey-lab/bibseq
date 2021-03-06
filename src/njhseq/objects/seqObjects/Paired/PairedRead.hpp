#pragma once
/*
 * PairedRead.hpp
 *
 *  Created on: Jul 26, 2015
 *      Author: nick
 */
//
// njhseq - A library for analyzing sequence data
// Copyright (C) 2012-2018 Nicholas Hathaway <nicholas.hathaway@umassmed.edu>,
//
// This file is part of njhseq.
//
// njhseq is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// njhseq is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with njhseq.  If not, see <http://www.gnu.org/licenses/>.
//
#include "njhseq/objects/seqObjects/readObject.hpp"

namespace njhseq {

class PairedRead: public readObject {
public:
	PairedRead(const seqInfo & seqFirst, const seqInfo & seqSecond,
			bool processed = false);

	PairedRead();

	// just for holding seq and qual of paired read, cnt_, frac_, name_,
	// and on_ should be ignored and seqBase_ should be used instead
	seqInfo mateSeqBase_;

	bool mateRComplemented_ = false;

	void toggleReverseComplement();

	virtual double getQualCheck(uint32_t qualCutOff) const;

	virtual void setBaseCountOnQualCheck(uint32_t qualCheck);
  virtual void setLetterCount();
  virtual void setLetterCount(const std::vector<char> & alph);

  virtual double getAverageErrorRate() const;

  virtual void createCondensedSeq();


  void outFastq(std::ostream & firstOut, std::ostream & secondOut )const;

  using size_type = readObject::size_type;
};

template<>
inline PairedRead::size_type len(const PairedRead & read){
	return len(read.seqBase_) + len(read.mateSeqBase_);
}

} /* namespace njhseq */


