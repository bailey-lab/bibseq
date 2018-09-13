#pragma once
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
//
//  IOUtils.hpp
//
//  Created by Nicholas Hathaway on 9/14/13.
//

#include "bibseq/IO/IOOptions.h"
#include "bibseq/IO/InputStream.hpp"
#include "bibcpp/IO/OutputStream.hpp"

#include <bibcpp/IO/IOUtils.hpp>
#include "bibseq/utils.h"





namespace bibseq {

std::vector<std::string> getInputValues(const std::string & valuesStr, const std::string & delim);




}  // namespace bibseq
