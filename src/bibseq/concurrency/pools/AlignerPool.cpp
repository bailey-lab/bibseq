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
 * AlignerPool.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: nick
 */



#include "AlignerPool.hpp"

namespace bibseq {
namespace concurrent {


AlignerPool::AlignerPool(uint64_t startingMaxLen, const gapScoringParameters & gapInfo,
		const substituteMatrix & scoring, const size_t size) :
		startingMaxLen_(startingMaxLen), gapInfo_(gapInfo), scoring_(scoring), size_(size) {
	for(uint32_t i = 0; i < size_; ++i){
		aligners_.emplace_back(aligner(startingMaxLen_, gapInfo_, scoring_));
	}
}

AlignerPool::AlignerPool(const aligner & alignerObj, const size_t size) :
		startingMaxLen_(alignerObj.parts_.maxSize_), gapInfo_(alignerObj.parts_.gapScores_), scoring_(
				alignerObj.parts_.scoring_), size_(size) {
	for (uint32_t i = 0; i < size_; ++i) {
		aligners_.emplace_back(alignerObj);
	}
}

AlignerPool::AlignerPool(const size_t size) :
		size_(size) {
	startingMaxLen_ = 400;
	gapInfo_ = gapScoringParameters(5, 1);
	scoring_.setWithSimple(2, -2);
	for(uint32_t i = 0; i < size_; ++i){
		aligners_.emplace_back(aligner(startingMaxLen_, gapInfo_, scoring_));
	}
}

AlignerPool::AlignerPool(const AlignerPool& that) :
		startingMaxLen_(that.startingMaxLen_), gapInfo_(that.gapInfo_), scoring_(that.scoring_), size_(
				that.size_) {
	for(uint32_t i = 0; i < size_; ++i){
		aligners_.emplace_back(aligner(startingMaxLen_, gapInfo_, scoring_));
	}
}
AlignerPool::~AlignerPool() {
	std::lock_guard<std::mutex> lock(poolmtx_); // GUARD
	destoryAlignersNoLock();
}

void AlignerPool::initAligners(){
	std::lock_guard<std::mutex> lock(poolmtx_); // GUARD
	// now push them onto the queue
	for (aligner& alignerObj : aligners_) {
		alignerObj.processAlnInfoInput(inAlnDir_);
		pushAligner(alignerObj);
	}
}

void AlignerPool::destoryAligners(){
	std::lock_guard<std::mutex> lock(poolmtx_); // GUARD
	destoryAlignersNoLock();
}

void AlignerPool::destoryAlignersNoLock(){
	closing_ = true;
	for (size_t i = 0; i < size_; ++i) {
		PooledAligner aligner_ptr;
		queue_.waitPop(aligner_ptr);
		aligner_ptr->processAlnInfoOutput(outAlnDir_, false);
	}
}

void AlignerPool::pushAligner(aligner& alignerObj){
	if(!closing_){
		AlignerPool* p = this;
		queue_.push(
				std::shared_ptr<aligner>(&alignerObj,
						[p](aligner* alignerObj)
						{
							p->pushAligner(*alignerObj);
						}));
	}
}


PooledAligner AlignerPool::popAligner() {
	PooledAligner aligner_ptr;
	queue_.waitPop(aligner_ptr);
	return aligner_ptr;
}

} // namespace concurrent
} // namespace bibseq
