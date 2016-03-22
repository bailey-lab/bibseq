#pragma once
//
// bibseq - A library for analyzing sequence data
// Copyright (C) 2012-2016 Nicholas Hathaway <nicholas.hathaway@umassmed.edu>,
// Jeffrey Bailey <Jeffrey.Bailey@umassmed.edu>
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
/*
 * GFFCore.hpp
 *
 *  Created on: Dec 28, 2015
 *      Author: nick
 */

#include "bibseq/utils.h"


namespace bibseq {

class GFFCore {
public:
	GFFCore();
	GFFCore(const std::string & line);

	std::string seqName_;
	std::string source_;
	std::string feature_;
	size_t start_; //counting from 1 as per the GFF file specifications (ugh)
	size_t end_; //counting from 1 as per the GFF file specifications (ugh)
	double score_;
	char strand_; //either + or - (. for NA)
	uint16_t frame_; //ether 0, 1, or 2 (uint16_t max for NA/.)
	std::unordered_map<std::string, std::string> attribute_;

	bool hasAttr(const std::string & attr) const;
	template<typename T>
	T getAttrAs(const std::string & attr) const {
		if(hasAttr(attr)){
			return bib::lexical_cast<T>(attribute_.at(attr));
		}
		return T{};
	}
	std::string getAttr(const std::string & attr) const;

	Json::Value toJson() const;
};



}  // namespace bibseq


