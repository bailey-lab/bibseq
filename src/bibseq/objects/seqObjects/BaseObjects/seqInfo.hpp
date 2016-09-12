#pragma once
//
//  seqInfo.hpp
//  sequenceTools
//
//  Created by Nicholas Hathaway on 03/03/14.
//  Copyright (c) 2014 Nicholas Hathaway. All rights reserved.
//
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

#include <bibcpp/jsonUtils.h>
#include "bibseq/common/stdAddition.hpp"
#include "bibseq/utils.h"
#include "bibseq/alignment/alignerUtils/substituteMatrix.hpp"
#include "bibseq/IO/SeqIO/SeqIOOptions.hpp"
#include "bibseq/alignment/alignerUtils/QualScorePars.hpp"

namespace bibseq {


struct seqInfo {
  // constructors
  /**@brief Empty constructor
   *
   */
	seqInfo();
  /**@brief Construct with just a name, no sequence data
   *
   * @param name
   */
  seqInfo(const std::string & name);
  seqInfo(const std::string& name, const std::string& seq,
          const std::vector<uint32_t>& qual);
  seqInfo(const std::string& name, const std::string& seq,
          const std::vector<uint32_t>& qual, double cnt);
  seqInfo(const std::string& name, const std::string& seq);
  seqInfo(const std::string& name, const std::string& seq,
          const std::string& stringQual);
  seqInfo(const std::string& name, const std::string& seq,
          const std::string& stringQual, uint32_t off_set);
  seqInfo(const std::string& name, const std::string& seq,
          const std::vector<uint32_t>& qual, double cnt, double frac);
  // Members
  std::string name_;
  std::string seq_;
  std::vector<uint32_t> qual_;
  double cnt_;
  double frac_;
  bool on_ = true;

  // functions
	// get a sub-portion of the read
	seqInfo getSubRead(uint32_t pos, uint32_t size) const;
	seqInfo getSubRead(uint32_t pos) const;
	// changing the seq and qual of the read
	void prepend(const std::string& seq, const std::vector<uint32_t>& qual);
	void append(const std::string& seq, const std::vector<uint32_t>& qual);
	void prepend(const std::string& seq, uint32_t defaultQuality = 40);
	void append(const std::string& seq, uint32_t defaultQuality = 40);
	void prepend(const char & base, uint32_t quality = 40);
	void append(const char & base, uint32_t quality = 40);
	bool checkLeadQual(uint32_t pos, uint32_t secondayQual, uint32_t out) const;
	bool checkTrailQual(uint32_t pos, uint32_t secondayQual, uint32_t out) const;
	bool checkPrimaryQual(uint32_t pos, uint32_t primaryQual) const;
	bool checkQual(uint32_t pos, const QualScorePars & qScorePars) const;
	uint32_t findLowestNeighborhoodQual(uint32_t posA, uint32_t out) const;
	const std::vector<uint32_t> getLeadQual(uint32_t posA, uint32_t out) const;
	const std::vector<uint32_t> getTrailQual(uint32_t posA, uint32_t out) const;
  // get a quality string from vector
  std::string getQualString() const;
  std::string getFastqQualString(uint32_t offset) const;
  static std::string getFastqString(const std::vector<uint32_t>& quals,
                                    uint32_t offset);
  // remove base at position
  void removeBase(size_t pos);
  void removeLowQualityBases(uint32_t qualCutOff);
  // handle gaps
  void removeGaps();
  // change name
  void updateName();
  void markAsChimeric();
  void unmarkAsChimeric();
  //check for chiemric flag
  bool isChimeric() const;

  //reverse complement read
  void reverseComplementRead(bool mark = false, bool regQualReverse = false);
  void reverseHRunsQuals();
  //comparison
  bool degenCompare(const seqInfo & otherInfo,
  		const substituteMatrix & compareScores)const;
  // output
  void outPutFastq(std::ostream& fastqFile) const;
  void outPutSeq(std::ostream& fastaFile) const;
  void outPutSeqAnsi(std::ostream& fastaFile) const;
  void outPutQual(std::ostream& qualFile) const;
  // description
  Json::Value toJson()const;
  const static std::unordered_map<char, uint32_t> ansiBaseColor;

  std::string getStubName(bool removeChiFlag) const;
  void setName(const std::string& newName);

  std::string getReadId() const;

  // Protein conversion
  void translate( bool complement, bool reverse, size_t start = 0);
  seqInfo translateRet(bool complement, bool reverse,  size_t start = 0) const;

  void processRead(bool processed);

  void addQual(const std::string & qualString);
  void addQual(const std::string & qualString, uint32_t offSet);
  void addQual(const std::vector<uint32_t> & quals);

  void setClip(size_t leftPos, size_t rightPos);
  void setClip(size_t rightPos);
  void setClip(const std::pair<int, int>& positions);
  void trimFront(size_t upToPosNotIncluding);
  void trimBack(size_t fromPositionIncluding);

  // get various information about the qualities with the sequence
  double getAverageQual() const;
  double getAverageErrorRate() const;
  uint32_t getSumQual() const;
  double getQualCheck(uint32_t qualCutOff)const;
  bool operator ==(const seqInfo & other) const;

  //
	std::string getOwnSampName() const;

	void processNameForMeta(std::unordered_map<std::string, std::string> & meta)const;

  using size_type = std::string::size_type;

};

template<>
inline seqInfo::size_type len(const seqInfo & info) {
	return info.seq_.size();
}

template<typename T>
const seqInfo & getSeqBase(const T & read) {
	return read.seqBase_;
}

template<>
inline const seqInfo & getSeqBase(const seqInfo & read) {
	return read;
}

template<typename T>
const seqInfo & getSeqBase(const std::shared_ptr<const T> & read) {
	return getSeqBase(*read);
}

template<typename T>
const seqInfo & getSeqBase(const std::unique_ptr<const T> & read) {
	return getSeqBase(*read);
}

template<typename T>
seqInfo & getSeqBase(T & read) {
	return read.seqBase_;
}

template<>
inline seqInfo & getSeqBase(seqInfo & read) {
	return read;
}

template<typename T>
seqInfo & getSeqBase(const std::shared_ptr<T> & read) {
	return getSeqBase(*read);
}

template<typename T>
seqInfo & getSeqBase(const std::unique_ptr<T> & read) {
	return getSeqBase(*read);
}

template<typename T>
seqInfo & getSeqBase(std::shared_ptr<T> & read) {
	return getSeqBase(*read);
}

template<typename T>
seqInfo & getSeqBase(std::unique_ptr<T> & read) {
	return getSeqBase(*read);
}

template<typename T>
const seqInfo & getSeqBase(std::shared_ptr<const T> & read) {
	return getSeqBase(*read);
}

template<typename T>
const seqInfo & getSeqBase(std::unique_ptr<const T> & read) {
	return getSeqBase(*read);
}

}  // namespace bibseq

