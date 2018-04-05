//
// bibseq - A library for analyzing sequence data
// Copyright (C) 2012-2018 Nicholas Hathaway <nicholas.hathaway@umassmed.edu>,
//
// This file is part of bibseq.
//
// bibseq is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// bibseq is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with bibseq.  If not, see <http://www.gnu.org/licenses/>.
//
#include "gaps.hpp"
#include <iostream>
#include <sstream>
#include "bibseq/utils.h"

namespace bibseq {

gap::gap(uint32_t startPos,
		uint32_t refPos,
		uint32_t seqPos,
		const std::string& gapedSequence,
		uint32_t firstQual,
		bool ref)
    : startPos_(startPos),
			refPos_(refPos),
			seqPos_(seqPos),
			size_(gapedSequence.size()),
			gapedSequence_(gapedSequence),
			qualities_{firstQual},
			ref_(ref)
      {}

std::string gap::strInfoHeader(const std::string & delim) {
	return vectorToString(
			toVecStr("indelType", "refPos", "seqPos", "gapedSeq", "qualities"), delim);
}

std::string gap::strInfo(const std::string & delim) const {
	return vectorToString(
			toVecStr(ref_ ? "insertion" : "deletion", refPos_, seqPos_,
					gapedSequence_, vectorToString(qualities_, ",")), delim);
}

void gap::switchSeqAndRef(){
	ref_ = !ref_;
	auto oldRefPos = refPos_;
	refPos_ = seqPos_;
	seqPos_ = oldRefPos;
}

bool gap::compare(const gap & otherGap) const {
	return seqPos_ == otherGap.seqPos_ && refPos_ == otherGap.refPos_
			&& gapedSequence_ == otherGap.gapedSequence_ && ref_ == otherGap.ref_;
}

Json::Value gap::toJson() const {
	Json::Value ret;
	ret["class"] = bib::json::toJson("bibseq::gap");
	ret["startPos_"] = bib::json::toJson(startPos_);
	ret["size_"] = bib::json::toJson(size_);
	ret["gapedSequence_"] = bib::json::toJson(gapedSequence_);
	ret["qualities_"] = bib::json::toJson(qualities_);

	ret["refPos_"] = bib::json::toJson(refPos_);
	ret["seqPos_"] = bib::json::toJson(seqPos_);
	ret["ref_"] = bib::json::toJson(ref_);
	return ret;
}

}  // namespace bibseq
