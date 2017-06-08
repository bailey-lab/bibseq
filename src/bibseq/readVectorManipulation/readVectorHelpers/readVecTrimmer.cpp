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
#include "readVecTrimmer.hpp"




namespace bibseq {


void readVecTrimmer::trimRstripQualScore(seqInfo &seq, const uint32_t qualCutOff){
	if(!seq.qual_.empty()){
		if(qualCutOff == seq.qual_.back()){
			uint32_t pos = seq.qual_.size() - 1;
			while(pos != 0 && qualCutOff == seq.qual_[pos - 1]){
				--pos;
			}
			seq.trimBack(pos);
		}
	}
}

void readVecTrimmer::trimAtRstripBase(seqInfo &seq, const char base){
	if(!seq.qual_.empty()){
		if(base == seq.seq_.back()){
			uint32_t pos = seq.seq_.size() - 1;
			while(pos != 0 && base == seq.seq_[pos - 1]){
				--pos;
			}
			seq.trimBack(pos);
		}
	}
}



void readVecTrimmer::trimLstripQualScore(seqInfo &seq, const uint32_t qualCutOff){
	if(!seq.qual_.empty()){
		if(qualCutOff == seq.qual_.front()){
			uint32_t pos = 0;
			while(pos + 1 != seq.qual_.size() && qualCutOff == seq.qual_[pos + 1]){
				++pos;
			}
			seq.trimFront(pos + 1);
		}
	}
}

void readVecTrimmer::trimAtLstripBase(seqInfo &seq, const char base){
	if(!seq.seq_.empty()){
		if(base == seq.seq_.front()){
			uint32_t pos = 0;
			while(pos + 1 != seq.seq_.size() && base == seq.seq_[pos + 1]){
				++pos;
			}
			seq.trimFront(pos + 1);
		}
	}
}




void readVecTrimmer::trimAtFirstBase(seqInfo &seq, const char base){
	auto pos = seq.seq_.find(base);
	if(std::string::npos != pos){
		seq.trimBack(pos);
	}
}


void readVecTrimmer::trimToMaxLength(seqInfo &seq, size_t maxLength) {
	if (maxLength != 0 && len(seq) > maxLength - 1) {
		seq.trimBack(maxLength);
	} else if (len(seq) < maxLength) {
		seq.on_ = false;
	}
}

void readVecTrimmer::trimAtFirstQualScore(seqInfo &seq,
		const uint32_t qualCutOff) {
	auto iter = std::find_if(seq.qual_.begin(), seq.qual_.end(),
			[&qualCutOff](uint32_t qual) {return qual <= qualCutOff;});
	if (seq.qual_.end() != iter) {
		seq.trimBack(iter - seq.qual_.begin());
	}
}

void readVecTrimmer::trimToLastQualScore(seqInfo &seq,
		const uint32_t qualCutOff) {
	if(0 == len(seq)){
		return;
	}
	auto iter = std::find_if(seq.qual_.rbegin(), seq.qual_.rend(),
			[&qualCutOff](uint32_t qual) {return qual <= qualCutOff;});

	if (seq.qual_.rend() != iter) {
		//position is 1 plus the actual found quality because seq.trimFront position is not inclusive
		uint32_t position = seq.qual_.size() - (iter - seq.qual_.rbegin());
		seq.trimFront(position);
	}
}



void readVecTrimmer::trimAtLastBase(seqInfo &seq, const char base){
	auto pos = seq.seq_.rfind(base);
	if(std::string::npos != pos){
		seq.trimBack(pos);
	}
}

void readVecTrimmer::trimAtFirstSeq(seqInfo &seq, const std::string & str){
	auto pos = seq.seq_.find(str);
	if(std::string::npos != pos){
		seq.trimBack(pos);
	}
}

void readVecTrimmer::trimAtLastSeq(seqInfo &seq, const std::string & str){
	auto pos = seq.seq_.rfind(str);
	if(std::string::npos != pos){
		seq.trimBack(pos);
	}
}

void readVecTrimmer::trimOffEndBases(seqInfo &seq, size_t endBases){
	if(len(seq) < endBases){
		std::stringstream ss;
		ss << __PRETTY_FUNCTION__ << " : error, length of seq is smaller than the number of bases to trim" << "\n";
		ss << "len of seq: " << len(seq) << ", bases to trim: " << endBases << "\n";
		throw std::runtime_error{ss.str()};
	}
	seq.trimBack(seq.seq_.size() - endBases);
}

void readVecTrimmer::trimOffForwardBases(seqInfo &seq, size_t forwardBases){
	if(len(seq) < forwardBases){
		std::stringstream ss;
		ss << __PRETTY_FUNCTION__ << " : error, length of seq is smaller than the number of bases to trim" << "\n";
		ss << "len of seq: " << len(seq) << ", bases to trim: " << forwardBases << "\n";
		throw std::runtime_error{ss.str()};
	}
	seq.trimFront(forwardBases);
}

void readVecTrimmer::trimEnds(seqInfo &seq, size_t forwardBases, size_t endBases){
	trimOffForwardBases(seq, forwardBases);
	trimOffEndBases(seq, endBases);
}

void readVecTrimmer::trimAtSequence(seqInfo &seq, seqInfo &reversePrimer,
		aligner &alignObj, comparison allowableErrors, FullTrimReadsPars::trimSeqPars tSeqPars) {
	//first is position of the first base, second is the position of the last base of the match
	std::pair<int, int> rPos;
	if (tSeqPars.within_ >= len(seq)) {
		if (tSeqPars.local_) {
			rPos = alignObj.findReversePrimer(seq, reversePrimer);
			alignObj.rearrangeObjs(seq, reversePrimer, tSeqPars.local_);
			alignObj.profilePrimerAlignment(seq, reversePrimer);
		} else {
			alignObj.alignCacheGlobal(seq, reversePrimer);
			alignObj.rearrangeObjs(seq, reversePrimer, tSeqPars.local_);
			alignObj.profilePrimerAlignment(seq, reversePrimer);
			if('-' == alignObj.alignObjectA_.seqBase_.seq_.front() ||
					'-' != alignObj.alignObjectB_.seqBase_.seq_.front()){
				rPos.first = 0;
			}else{
				//not if if reversePrimer is all - to begin with
				rPos.first = alignObj.getSeqPosForAlnAPos(alignObj.alignObjectB_.seqBase_.seq_.find_first_not_of('-'));
			}
			if ('-' == alignObj.alignObjectA_.seqBase_.seq_.back()
					|| '-' != alignObj.alignObjectB_.seqBase_.seq_.back()) {
				rPos.second = len(seq) - 1;
			}else{
				rPos.second = alignObj.getSeqPosForAlnAPos(alignObj.alignObjectB_.seqBase_.seq_.find_last_not_of('-'));
			}
		}
	}else{
		auto fromPos = len(seq) - tSeqPars.within_;
		auto backSeq = seq.getSubRead(fromPos);
		if(tSeqPars.local_){
			rPos = alignObj.findReversePrimer(backSeq, reversePrimer);
			alignObj.rearrangeObjs(backSeq, reversePrimer, tSeqPars.local_);
			alignObj.profilePrimerAlignment(backSeq, reversePrimer);
		}else{
			alignObj.alignCacheGlobal(backSeq, reversePrimer);
			alignObj.rearrangeObjs(backSeq, reversePrimer, tSeqPars.local_);
			alignObj.profilePrimerAlignment(backSeq, reversePrimer);
			if('-' == alignObj.alignObjectA_.seqBase_.seq_.front() ||
					'-' != alignObj.alignObjectB_.seqBase_.seq_.front()){
				rPos.first = 0;
			}else{
				//not if if reversePrimer is all - to begin with
				rPos.first = alignObj.getSeqPosForAlnAPos(alignObj.alignObjectB_.seqBase_.seq_.find_first_not_of('-'));
			}
			if ('-' == alignObj.alignObjectA_.seqBase_.seq_.back()
					|| '-' != alignObj.alignObjectB_.seqBase_.seq_.back()) {
				rPos.second = len(backSeq) - 1;
			}else{
				rPos.second = alignObj.getSeqPosForAlnAPos(alignObj.alignObjectB_.seqBase_.seq_.find_last_not_of('-'));
			}
		}
		rPos.first += fromPos;
		rPos.second += fromPos;
		/*debugging
		alignObj.alignObjectA_.seqBase_.outPutSeqAnsi(std::cout);
		alignObj.alignObjectB_.seqBase_.outPutSeqAnsi(std::cout);
		std::cout << rPos.first << " : " << rPos.second << std::endl;*/
	}



	bool passInspection = allowableErrors.passErrorProfile(alignObj.comp_);
	if (alignObj.comp_.distances_.query_.coverage_
			< allowableErrors.distances_.query_.coverage_) {
		passInspection = false;
	}
	/*debugging
	if(seq.name_ == "contig1[length=273;region=ama1_subRegion;sample=QG0183-C]_t210"){
		std::cout << seq.toJsonJustInfo() << std::endl;
		std::cout << alignObj.comp_.toJson() << std::endl;
		std::cout << "passInspection: " << bib::colorBool(passInspection) << std::endl;

	}*/

	seq.on_ = passInspection;
	if(seq.on_){
		if (tSeqPars.includeSequence_) {
			seq.trimBack(rPos.second + 1);
			seq.setClip(0, rPos.second);
			if (tSeqPars.sequenceToLowerCase_) {
				if (tSeqPars.removePreviousSameBases_) {
					while (seq.seq_[rPos.first] == seq.seq_[rPos.first - 1]) {
						--rPos.first;
					}
				}
				changeSubStrToLowerToEnd(seq.seq_, rPos.first);
			}
		} else {
			if (tSeqPars.removePreviousSameBases_) {
				bool foundPrevious = false;
				while (seq.seq_[rPos.first] == seq.seq_[rPos.first - 1]) {
					foundPrevious = true;
					--rPos.first;
				}

				//set clip will keep the second position you give it so you have to minus one more if you found any
				if (foundPrevious) {
					--rPos.first;
				}
			}
			seq.setClip(0, rPos.first - 1);
		}
	}
}

void readVecTrimmer::trimBeforeSequence(seqInfo &seq, seqInfo &forwardSeq,
		aligner &alignObj, comparison allowableErrors, FullTrimReadsPars::trimSeqPars tSeqPars) {
	std::pair<int, int> fPos;
	if (tSeqPars.within_ >= len(seq)) {
		if (tSeqPars.local_) {
			fPos = alignObj.findReversePrimer(seq, forwardSeq);
			alignObj.rearrangeObjs(seq, forwardSeq, tSeqPars.local_);
			alignObj.profilePrimerAlignment(seq, forwardSeq);
		} else {
			alignObj.alignCacheGlobal(seq, forwardSeq);
			alignObj.rearrangeObjs(seq, forwardSeq, tSeqPars.local_);
			alignObj.profilePrimerAlignment(seq, forwardSeq);
			if('-' == alignObj.alignObjectA_.seqBase_.seq_.front() ||
					'-' != alignObj.alignObjectB_.seqBase_.seq_.front()){
				fPos.first = 0;
			}else{
				//not if if forwardSeq is all - to begin with
				fPos.first = alignObj.getSeqPosForAlnAPos(alignObj.alignObjectB_.seqBase_.seq_.find_first_not_of('-'));
			}
			if ('-' == alignObj.alignObjectA_.seqBase_.seq_.back()
					|| '-' != alignObj.alignObjectB_.seqBase_.seq_.back()) {
				fPos.second = len(seq) - 1;
			}else{
				fPos.second = alignObj.getSeqPosForAlnAPos(alignObj.alignObjectB_.seqBase_.seq_.find_last_not_of('-'));
			}
		}
	}else{
		auto frontSeq = seq.getSubRead(0, tSeqPars.within_);
		if(tSeqPars.local_){
			fPos = alignObj.findReversePrimer(frontSeq, forwardSeq);
			alignObj.rearrangeObjs(frontSeq, forwardSeq, tSeqPars.local_);
			alignObj.profilePrimerAlignment(frontSeq, forwardSeq);
		}else{
			alignObj.alignCacheGlobal(frontSeq, forwardSeq);
			alignObj.rearrangeObjs(frontSeq, forwardSeq, tSeqPars.local_);
			alignObj.profilePrimerAlignment(frontSeq, forwardSeq);
			if('-' == alignObj.alignObjectA_.seqBase_.seq_.front() ||
					'-' != alignObj.alignObjectB_.seqBase_.seq_.front()){
				fPos.first = 0;
			}else{
				//not if if forwardSeq is all - to begin with
				fPos.first = alignObj.getSeqPosForAlnAPos(alignObj.alignObjectB_.seqBase_.seq_.find_first_not_of('-'));
			}
			if ('-' == alignObj.alignObjectA_.seqBase_.seq_.back()
					|| '-' != alignObj.alignObjectB_.seqBase_.seq_.back()) {
				fPos.second = len(frontSeq) - 1;
			}else{
				fPos.second = alignObj.getSeqPosForAlnAPos(alignObj.alignObjectB_.seqBase_.seq_.find_last_not_of('-'));
			}
		}
		//debugging
		/*
		alignObj.alignObjectA_.seqBase_.outPutSeqAnsi(std::cout);
		alignObj.alignObjectB_.seqBase_.outPutSeqAnsi(std::cout);
		std::cout << fPos.first << " : " << fPos.second << std::endl;
		*/
	}
	/*
	std::pair<int, int> fPos = alignObj.findReversePrimer(seq, forwardSeq);
	alignObj.rearrangeObjs(seq, forwardSeq, true);
	alignObj.profilePrimerAlignment(seq, forwardSeq);
*/


	bool passInspection = allowableErrors.passErrorProfile(alignObj.comp_);
	if (alignObj.comp_.distances_.query_.coverage_
			< allowableErrors.distances_.query_.coverage_) {
		passInspection = false;
	}
	//debugging
	/*
	if(!passInspection){
		std::cout << seq.toJsonJustInfo() << std::endl;
		std::cout << alignObj.comp_.toJson() << std::endl;
		std::cout << "passInspection: " << bib::colorBool(passInspection) << std::endl;
	}*/
	seq.on_ = passInspection;
	if(seq.on_){
		if (tSeqPars.includeSequence_) {
			seq.trimFront(fPos.first);
			if (tSeqPars.sequenceToLowerCase_) {
				if (tSeqPars.removePreviousSameBases_) {
					while (seq.seq_[fPos.second] == seq.seq_[fPos.second + 1]) {
						++fPos.second;
					}
				}
				changeSubStrToLowerFromBegining(seq.seq_, fPos.second);
			}
		} else {
			if (tSeqPars.removePreviousSameBases_) {
				while (seq.seq_[fPos.second] == seq.seq_[fPos.second + 1]) {
					++fPos.second;
				}
			}
			//debugging
			/*
			std::cout << "Trimming to " << fPos.second + 1 << std::endl;
			*/
			seq.trimFront(fPos.second + 1);
		}
	}

}


void readVecTrimmer::trimBetweenSequences(seqInfo &seq, seqInfo &forwardSeq,
		seqInfo &backSeq, aligner &alignObj, comparison allowableErrors,
		FullTrimReadsPars::trimSeqPars tSeqPars) {
	trimAtSequence(seq, backSeq, alignObj, allowableErrors, tSeqPars);
	if(seq.on_){
		trimBeforeSequence(seq, forwardSeq, alignObj, allowableErrors, tSeqPars);
	}
}
FullTrimReadsPars::FullTrimReadsPars(){
	tSeqPars_.includeSequence_ = false;
	tSeqPars_.sequenceToLowerCase_ = false;
	tSeqPars_.removePreviousSameBases_ = false;
	allowableErrors.distances_.query_.coverage_ = .75;
}

void FullTrimReadsPars::initForKSharedTrim(){
  allowableErrors.distances_.query_.coverage_ = .50;
  allowableErrors.hqMismatches_ = 2;
  allowableErrors.lqMismatches_ = 2;
  allowableErrors.oneBaseIndel_ = 2;
  allowableErrors.twoBaseIndel_ = 2;
  allowableErrors.largeBaseIndel_ = 2;
}



}  // namespace bibseq
