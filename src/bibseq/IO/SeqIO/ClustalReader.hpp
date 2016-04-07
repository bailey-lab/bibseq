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
//
//  ClustalReader.hpp
//  sequenceTools
//
//  Created by Nick Hathaway on 11/19/12.
//  Copyright (c) 2012 Nick Hathaway. All rights reserved.
//

#include "bibseq/objects/seqObjects/sffObject.hpp"
#include "bibseq/readVectorManipulation.h"
#include <api/BamReader.h>
#include <bibcpp/jsonUtils.h>
#include "bibseq/IO/cachedReader.hpp"
#include "bibseq/IO/SeqIO/SeqIOOptions.hpp"

namespace bibseq {

class ClustalReader {
public:
	static std::vector<readObject> readClustal(std::string filename, bool processed);
	static std::vector<readObject> readClustalNg(std::string filename, bool processed);

};

}  // namespace bib

#ifndef NOT_HEADER_ONLY
#include "ClustalReader.cpp"
#endif