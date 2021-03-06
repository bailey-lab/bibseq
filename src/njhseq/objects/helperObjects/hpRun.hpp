#pragma once
//
//  hpRun.hpp
//
//  Created by Nick Hathaway on 1/6/13.
//
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
#include <vector>
#include "njhseq/utils.h"
namespace njhseq {

class hpRun {

 public:
  hpRun() {}
  hpRun(char hpRunBase, int position, int hpSize)
      : base(hpRunBase), pos(position), runSize(hpSize), count(1) {}
  hpRun(char hpRunBase, int position, int hpSize, int readCount)
      : base(hpRunBase), pos(position), runSize(hpSize), count(readCount) {}
  hpRun(char hpRunBase, int position, int hpSize, int readCount,
        const std::vector<int>& firstQual)
      : base(hpRunBase),
        pos(position),
        runSize(hpSize),
        count(readCount),
        quals(firstQual) {}
  hpRun(char hpRunBase, int position, int homopolyerPosition, int hpSize,
        int readCount, const std::vector<int>& firstQual)
      : base(hpRunBase),
        pos(position),
        hpPosition(homopolyerPosition),
        runSize(hpSize),
        count(readCount),
        quals(firstQual) {}
  char base;
  int pos;
  int hpPosition;
  int runSize;
  int count;
  std::vector<int> quals;
  void increaseCount(int runAdd, int qualAdd) {
    count += runAdd;
    quals.push_back(qualAdd);
  }
  hpRun& operator++() {
    ++count;
    return *this;
  }
  hpRun& operator+=(int add) {
    count += add;
    return *this;
  }
  std::string getStringInfo();
};
}  // namespace njhseq


