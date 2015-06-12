#include "identicalCluster.hpp"

namespace bibseq {

identicalCluster::identicalCluster(const readObject& firstRead) : baseCluster(firstRead) {
	firstReadName = firstRead.seqBase_.name_;
	firstReadCount = firstRead.seqBase_.cnt_;
	basesAboveQualCheck_ = firstRead.basesAboveQualCheck_;
  //std::cout << "identicalCluster constructor: " << std::endl;
  //std::cout << seqBase_.name_ << std::endl;
  //std::cout << seqBase_.cnt_ << std::endl;
  //std::cout << seqBase_.frac_ << std::endl;
}


void identicalCluster::addRead(const readObject& identicalRead) {
  reads_.emplace_back(identicalRead);
  seqBase_.cnt_ += identicalRead.seqBase_.cnt_;
}
////////setting of the representive quality and seq
void identicalCluster::setSeq() {
  readVecSorter::sort(reads_);
  seqBase_.seq_ = reads_.front().seqBase_.seq_;
  seqBase_.name_ = reads_.front().seqBase_.name_;
  //seqBase_.cnt_ = (int)reads_.size();
  updateName();
}
void identicalCluster::setRep(const std::string& repQual){
	if (repQual == "worst") {
		setWorstQualRep();
	} else if (repQual == "median") {
		setMedianQualRep();
	} else if (repQual == "average") {
		setAverageQualRep();
	} else if (repQual == "bestSeq") {
		setBestSeqRep();
	} else if (repQual == "bestQual") {
		setBestQualRep();
	} else {
		std::stringstream ss;
		ss << "Unrecognized qualRep: " << repQual << std::endl;
		ss << "Needs to be median, average, bestSeq, bestQual, or worst"
							<< std::endl;
		throw std::runtime_error{ss.str()};
	}
}
void identicalCluster::setBestQualRep() {
  setSeq();
  seqBase_.qual_.clear();
  for (auto i : iter::range(seqBase_.seq_.length())) {
    uint32_t currentQual = 0;
    for (const auto& read : reads_) {
      if (read.seqBase_.qual_[i] > currentQual) {
        currentQual = read.seqBase_.qual_[i];
      }
    }
    seqBase_.qual_.push_back(currentQual);
  }
}
void identicalCluster::setWorstQualRep() {
  setSeq();
  seqBase_.qual_.clear();
  for (auto i : iter::range(seqBase_.seq_.length())) {
    uint32_t currentQual = UINT32_MAX;
    for (const auto& read : reads_) {
      if (read.seqBase_.qual_[i] < currentQual) {
        currentQual = read.seqBase_.qual_[i];
      }
    }
    seqBase_.qual_.push_back(currentQual);
  }
}

void identicalCluster::setBestSeqRep() {
  setSeq();
  seqBase_.qual_.clear();
  seqBase_.qual_ = reads_.front().seqBase_.qual_;
}

void identicalCluster::setAverageQualRep() {
  setSeq();
  seqBase_.qual_.clear();
  for (auto i : iter::range(seqBase_.seq_.length())) {
    int qualSum = 0;
    for (const auto& read : reads_) {
      qualSum += read.seqBase_.qual_[i];
    }
    seqBase_.qual_.push_back((int)(qualSum / seqBase_.cnt_));
  }
}
void identicalCluster::setMedianQualRep() {
  setSeq();
  seqBase_.qual_.clear();
  for (auto i : iter::range(seqBase_.seq_.length())) {
    std::vector<uint32_t> qualities;
    for (auto read : reads_) {
      qualities.push_back(read.seqBase_.qual_[i]);
    }
    uint32_t qualToInsert = vectorMedian(qualities);
    seqBase_.qual_.push_back(qualToInsert);
  }
}
}  // namespace bib
