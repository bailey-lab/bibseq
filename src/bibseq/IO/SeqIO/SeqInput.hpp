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
 * SeqInput.hpp
 *
 *  Created on: Jan 17, 2016
 *      Author: nick
 */

#include <api/BamReader.h>

#include <bibcpp/files/gzTextFileCpp.hpp>

#include "bibseq/IO/cachedReader.hpp"
#include "bibseq/objects/seqObjects/readObject.hpp"
#include "bibseq/objects/seqObjects/Paired/PairedRead.hpp"
#include "bibseq/objects/seqObjects/sffObject.hpp"
#include "bibseq/IO/SeqIO/SeqIOOptions.hpp"
#include "bibseq/readVectorManipulation/readVectorOperations.h"


namespace bibseq {

class SeqInput{
public:

	SeqInput(const SeqIOOptions & options);
	SeqInput(const SeqInput& that);

	SeqIOOptions ioOptions_;
	bool includeSpaceInNames = true;

	bool readNextRead(seqInfo & read);
	bool readNextRead(PairedRead & read);



	template<typename T>
	bool readNextRead(T & read) {
		return readNextRead(read.seqBase_);
	}

	template<typename T>
	bool readNextRead(std::shared_ptr<T> & read) {
		return readNextRead(*read);
	}
	template<typename T>
	bool readNextRead(std::unique_ptr<T> & read) {
		return readNextRead(*read);
	}

	template<typename T>
	std::shared_ptr<T> readNextRead() {
		auto ret = std::make_shared<T>();
		auto status = readNextRead(ret);
		if (status) {
			return ret;
		} else {
			return nullptr;
		}
	}

	template<typename T>
	std::vector<T> readAllReads(){
		if(!inOpen_){
			openIn();
		}
		T read;
		std::vector<T> ret;
		while(readNextRead(read)){
			ret.emplace_back(read);
		}
	  readVec::handelLowerCaseBases(ret, ioOptions_.lowerCaseBases_);
	  if (ioOptions_.removeGaps_) {
	    readVec::removeGapsFromReads(ret);
	  }
		return ret;
	}

	template<typename T>
	std::vector<std::shared_ptr<T>> readAllReadsPtrs(){
		if(!inOpen_){
			openIn();
		}
		std::vector<std::shared_ptr<T>> ret;
		auto read = readNextRead<T>();
		while(read){
			ret.emplace_back(read);
			read = readNextRead<T>();
		}
	  readVec::handelLowerCaseBases(ret, ioOptions_.lowerCaseBases_);
	  if (ioOptions_.removeGaps_) {
	    readVec::removeGapsFromReads(ret);
	  }
		return ret;
	}

	size_t tellgPri();
	void seekgPri(size_t pos);

	size_t tellgSec();
	void seekgSec(size_t pos);

	bool inOpen() const;
	void openIn();
	void closeIn();

	std::mutex mut_;
	std::unique_ptr<sffObject> lastSffRead_;


	static std::vector<readObject> getReferenceSeq(
			const SeqIOOptions & refOptions, uint64_t& maxLength);

private:

	std::unique_ptr<BamTools::BamReader> bReader_;
	std::unique_ptr<BamTools::BamAlignment> aln_;

	std::unique_ptr<bib::files::gzTextFileCpp<>> priGzReader_;
	std::unique_ptr<bib::files::gzTextFileCpp<>> secGzReader_;
	std::unique_ptr<std::ifstream> priReader_;
	std::unique_ptr<std::ifstream> secReader_;
public:
	std::unique_ptr<VecStr> sffTxtHeader_;
	std::unique_ptr<sffBinaryHeader> sffBinHeader_;
	bool readNextFastaStream(std::istream& fastaFile, seqInfo& read,
			bool processed);
	bool readNextFastqStream(std::istream& fastqFile, uint32_t offSet, seqInfo& read,
			bool processed);
private:
	bool readNextQualStream(std::ifstream& qualFile, std::vector<uint32_t>& quals,
			std::string & name);
	bool readNextFastaQualStream(std::ifstream& fastaFile,
			std::ifstream& qualFile, seqInfo & read, bool processed);

	bool readNextFastaStream(bib::files::gzTextFileCpp<>& fastaGzFile, seqInfo& read,
			bool processed);
	bool readNextQualStream(bib::files::gzTextFileCpp<>& qualGzFile, std::vector<uint32_t>& quals,
			std::string & name);
	bool readNextFastaQualStream(bib::files::gzTextFileCpp<>& fastaGzFile,
			bib::files::gzTextFileCpp<>& qualGzFile, seqInfo & read, bool processed);


	bool readNextFastqStream(bib::files::gzTextFileCpp<>& fastqGzFile, uint32_t offSet, seqInfo& read,
			bool processed);
	bool readNextFastqStream(const VecStr & data,const uint32_t lCount, uint32_t offSet, seqInfo& read,
			bool processed);

	bool readNextBam(BamTools::BamReader & bReader, seqInfo& read,
			BamTools::BamAlignment & aln, bool processed);

	VecStr readSffTxtHeader(std::ifstream & inFile);
	bool readNextSff(std::ifstream & inFile, sffObject & read);
	bool readNextSff(std::ifstream & inFile, seqInfo & read);

	bool readNextSffBin(std::ifstream& in, sffObject& read, int32_t numFlowReads);
	void readHeader(std::ifstream& in, sffBinaryHeader& header);

	bool inOpen_ = false;

	friend class SeqIO;
};


}  // namespace bibseq

