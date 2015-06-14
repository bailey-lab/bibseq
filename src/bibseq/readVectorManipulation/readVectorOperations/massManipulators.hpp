#pragma once
//
// bibseq - A library for analyzing sequence data
// Copyright (C) 2012, 2015 Nicholas Hathaway <nicholas.hathaway@umassmed.edu>,
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
//  massManipulators.hpp
//  sequenceTools
//
//  Created by Nicholas Hathaway on 11/18/13.
//  Copyright (c) 2013 Nicholas Hathaway. All rights reserved.
//

#include "bibseq/utils.h"
#include "bibseq/helpers/seqUtil.hpp"

namespace bibseq {
namespace readVec {
template <typename T>
void changeFrontEndToLowerCase(std::vector<T>& reads, int numberOfBases) {
  for_each(reads, [&](T& read) {
    changeSubStrToLowerFromBegining(read.seqBase_.seq_, numberOfBases);
  });
}

template <typename T>
void changeBackEndToLowerCase(std::vector<T>& reads, int numberOfBases) {
  for_each(reads, [&](T& read) {
  	changeSubStrToLowerToEnd(read.seqBase_.seq_, read.seqBase_.seq_.size() - numberOfBases);
  });
}

template <typename T>
void allRemoveLowQualityBases(std::vector<T>& reads, int qualCutOff) {
  for_each(reads,
           [&](T& read) { read.seqBase_.removeLowQualityBases(qualCutOff); });
}

template <typename T>
void allAdjustHomopolymerRunsQualities(std::vector<T>& reads) {
  for_each(reads, [](T& read) { read.adjustHomopolyerRunQualities(); });
}
template <typename T>
void allReverseComplement(std::vector<T>& reads, bool mark = false) {
	for_each(reads, [&](T & read) {read.seqBase_.reverseComplementRead(mark);});
}

template <typename T>
void removeGapsFromReads(std::vector<T>& reads) {
  for_each(reads, [](T& read) { read.seqBase_.removeGaps(); });
}
// options to handle lower case letters
template <typename T>
void removeLowerCaseBases(std::vector<T>& reads) {
  for_each(reads, [](T& read) {
    seqUtil::removeLowerCase(read.seqBase_.seq_, read.seqBase_.qual_);
  });
}

template <typename T>
void lowerCaseBasesToUpperCase(std::vector<T>& reads) {
  for_each(reads, [](T& read) { stringToUpper(read.seqBase_.seq_); });
}
template <typename T>
void handelLowerCaseBases(std::vector<T>& reads, const std::string& lower) {
  if (lower == "remove") {
    removeLowerCaseBases(reads);
  } else if (lower == "upper") {
    lowerCaseBasesToUpperCase(reads);
  } else {
    // any other option will be considered do nothing to them
  }
}

template <typename T>
void handelLowerCaseBases(T & read, const std::string& lower) {
  if (lower == "remove") {
  	seqUtil::removeLowerCase(read.seqBase_.seq_, read.seqBase_.qual_);
  } else if (lower == "upper") {
  	stringToUpper(read.seqBase_.seq_);
  } else {
    // any other option will be considered do nothing to them
  }
}

template <typename T>
void convertReadsToProteinFromcDNA(std::vector<T>& reads,
                                   bool transcribeToRNAFirst, size_t start = 0,
                                   bool forceStartM = false) {
  for_each(reads, [&](T& read) {
    read.convertToProteinFromcDNA(transcribeToRNAFirst, start, forceStartM);
  });
}
template <typename T1, typename T2>
std::vector<T2> convertVec(const std::vector<T1>& vec) {
  std::vector<T2> ans;
  for (const auto& read : vec) {
    ans.emplace_back(T2(read.seqBase_));
  }
  return ans;
}
template <typename T>
void prependAll(std::vector<T>& reads, const std::string& seq,
                const std::vector<uint32_t>& qual) {
  for_each(reads, [&](T& read) { read.seqBase_.prepend(seq, qual); });
}

template <typename T>
void appendAll(std::vector<T>& reads, const std::string& seq,
               const std::vector<uint32_t>& qual) {
  for_each(reads, [&](T& read) { read.seqBase_.append(seq, qual); });
}

}  // namespace readVec
}  // namespace bib
