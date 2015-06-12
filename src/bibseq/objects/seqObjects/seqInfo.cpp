
#include "seqInfo.hpp"
#include "bibseq/helpers/seqUtil.hpp"
#include <bibcpp/bashUtils.h>

namespace bibseq {

seqInfo::seqInfo() :
		name_(""), seq_(""), cnt_(1), frac_(0) {
}
seqInfo::seqInfo(const std::string & name) :
		name_(name), seq_(""), cnt_(1), frac_(0) {
}
seqInfo::seqInfo(const std::string& name, const std::string& seq,
		const std::vector<uint32_t>& qual) :
		name_(name), seq_(seq), qual_(qual), cnt_(1), frac_(0) {
}
seqInfo::seqInfo(const std::string& name, const std::string& seq,
		const std::vector<uint32_t>& qual, double cnt) :
		name_(name), seq_(seq), qual_(qual), cnt_(cnt), frac_(0) {
}
seqInfo::seqInfo(const std::string& name, const std::string& seq) :
		name_(name), seq_(seq), qual_(std::vector<uint32_t>(seq.size(), 40)), cnt_(
				1), frac_(0) {
}

seqInfo::seqInfo(const std::string& name, const std::string& seq,
		const std::string& stringQual) :
		name_(name), seq_(seq), qual_(stringToVector<uint32_t>(stringQual)), cnt_(
				1), frac_(0) {
}

seqInfo::seqInfo(const std::string& name, const std::string& seq,
		const std::string& stringQual, uint32_t off_set) :
		name_(name), seq_(seq), qual_(std::vector<uint32_t>(0)), cnt_(1), frac_(0) {
	for (const auto & c : stringQual) {
		qual_.emplace_back(c - off_set);
	}
}
seqInfo::seqInfo(const std::string& name, const std::string& seq,
		const std::vector<uint32_t>& qual, double cnt, double frac) :
		name_(name), seq_(seq), qual_(qual), cnt_(cnt), frac_(frac) {
}


seqInfo seqInfo::getSubRead(uint32_t pos, uint32_t size) const{
  return seqInfo(name_, seq_.substr(pos, size), getSubVector(qual_, pos, size),
                 cnt_, frac_);
}
seqInfo seqInfo::getSubRead(uint32_t pos) const{
  return seqInfo(name_, seq_.substr(pos, seq_.size() - pos),
                 getSubVector(qual_, pos, seq_.size() - pos), cnt_, frac_);
}

// description
void seqInfo::printDescription(std::ostream& out, bool deep) const {
  out << "seqInfo{" << "\n" << "name_:" << name_ << "\n"
      << "seq_:" << seq_ << "\n" << "qual_:" << qual_ << "\n"
      << "cnt_:" << cnt_ << "\n" << "frac_:" << frac_ << "\n" << "}"
      << "\n";
}

void seqInfo::updateName(){
  size_t totalPos = name_.rfind("_t");
  if (totalPos != std::string::npos) {
    name_ = name_.substr(0, totalPos);
  }
  name_ += "_t" + to_string(cnt_);
}

Json::Value seqInfo::toJson()const{
	Json::Value ret;
	ret["seq"] = bib::json::toJson(seq_);
	ret["qual"] = bib::json::toJson(qual_);
	ret["cnt"] = bib::json::toJson(cnt_);
	ret["frac"] = bib::json::toJson(frac_);
	ret["name"] = bib::json::toJson(name_);
	ret["on"] = bib::json::toJson(on_);
	return ret;
}

void seqInfo::convertToProteinFromcDNA(bool transcribeToRNAFirst, size_t start,
		bool forceStartM) {
	if (transcribeToRNAFirst) {
		seqUtil::convertToProteinFromcDNA(seq_, start, forceStartM);
	} else {
		seq_ = seqUtil::convertToProtein(seq_, start, forceStartM);
	}
}

std::string seqInfo::getProteinFromcDNA(bool transcribeToRNAFirst, size_t start,
		bool forceStartM) const {
	if (transcribeToRNAFirst) {
		return seqUtil::convertToProteinFromcDNAReturn(seq_, start, forceStartM);
	} else {
		return seqUtil::convertToProtein(seq_, start, forceStartM);
	}
}

void seqInfo::processRead(bool processed) {
  bool setFraction = false;
  if (processed) {
    VecStr toks;
    bool containsAllNumbers = true;
    if (name_.find("_t") == std::string::npos &&
        name_.find("_f") == std::string::npos) {
      if (name_.rfind("_") == std::string::npos) {
      	std::stringstream ss;
        ss << "Improper name format for processed read, should have a "
                     "_# or _t# where # is the number of reads the sequence "
                     "represents" << "\n";
        ss << "failed due to name not have a _ or _t, " << name_
                  << "\n";
        throw std::runtime_error{bib::bashCT::boldRed(ss.str())};
      } else {
        toks = tokenizeString(name_, "_");
      }
    } else if (name_.find("_t") != std::string::npos) {
      toks = tokenizeString(name_, "_t");
    } else {
      toks = tokenizeString(name_, "_f");
      setFraction = true;
    }
    containsAllNumbers = isDoubleStr(toks[toks.size() - 1]);
    if (containsAllNumbers) {
      cnt_ = std::stod(toks[toks.size() - 1]);
      if (setFraction) {
        frac_ = cnt_;
        cnt_ = frac_ * 1000;
      }
      name_ = name_.substr(0, name_.rfind("_")) +
                       "_t" + estd::to_string(cnt_);
    } else {
    	std::stringstream ss;
      ss << "Improper name format for processed read, should have a _# "
                   "or _t# where # is the number of reads the sequence "
                   "represents" << "\n";
      ss << "failed due to # containing a non-digit character, "
                << toks[toks.size() - 1] << "\n";
      throw std::runtime_error{bib::bashCT::boldRed(ss.str())};
    }
  } else {
  	//leave the count that it was originally when constructed
    //cnt_ = 1;
  }
}

const std::unordered_map<char, uint32_t> seqInfo::ansiBaseColor = std::unordered_map<char, uint32_t>{
		{'A',203},
		{'a',203},
		{'C',83},
		{'c',83},
		{'G',227},
		{'g',227},
		{'T',69},
		{'t',69},
		{'n',145},
		{'N',145},
		{'-',102}
};

void seqInfo::outPutSeqAnsi(std::ostream& fastaFile) const{
	fastaFile << bib::bashCT::addBGColor(145) << ">" << name_ << bib::bashCT::reset << "\n";
	for(const auto & c : seq_){
		fastaFile << bib::bashCT::addBGColor(ansiBaseColor.at(c)) << c << bib::bashCT::reset;
	}
	fastaFile << bib::bashCT::reset << "\n";
}
void seqInfo::prepend(const std::string& seq, uint32_t defaultQuality){
	prepend(seq, std::vector<uint32_t> (1,defaultQuality));
}
void seqInfo::append(const std::string& seq, uint32_t defaultQuality){
	append(seq, std::vector<uint32_t> (1,defaultQuality));
}
void seqInfo::append(const std::string& seq,
                     const std::vector<uint32_t>& qual) {
  if (qual.size() == 1) {
    seq_.append(seq);
    addOtherVec(qual_, std::vector<uint32_t>(seq.size(), qual.front()));
  } else if (qual.size() == seq.size()) {
    seq_.append(seq);
    addOtherVec(qual_, qual);
  } else {
  	std::stringstream ss;
    ss << "Need to supply either single a quality or same amount of "
                 "quality score for seq length" << "\n";
    ss << "trying to add " << qual.size()
              << " qualities and a seq of length " << seq.length() << "\n";
    throw std::runtime_error{bib::bashCT::boldRed(ss.str())};
  }
}

void seqInfo::prepend(const char & base, uint32_t quality){
	seq_.insert(seq_.begin(), base);
	qual_.insert(qual_.begin(), quality);
}

void seqInfo::append(const char & base, uint32_t quality){
	seq_.push_back(base);
	qual_.emplace_back(quality);
}

void seqInfo::reverseComplementRead(bool mark, bool regQualReverse) {
	if(regQualReverse){
		bib::reverse(qual_);
	}else{
	  std::vector<std::vector<uint32_t>> quals;
	  std::vector<uint32_t> currentQuals;
	  currentQuals.push_back(qual_[0]);
	  for (uint32_t i = 1; i < seq_.length(); ++i) {
	    if (seq_[i] == seq_[i - 1]) {
	      currentQuals.push_back(qual_[i]);
	    } else {
	      quals.push_back(currentQuals);
	      currentQuals.clear();
	      currentQuals.push_back(qual_[i]);
	    }
	  }
	  quals.push_back(currentQuals);
	  qual_.clear();
	  for (auto iter = quals.rbegin(); iter != quals.rend(); ++iter) {
	    addOtherVec(qual_, *iter);
	  }
	}
  seq_ = seqUtil::reverseComplement(seq_, "DNA");


  if(mark){
  	//if name already contains _Comp and then remove _Comp
  	if(name_.find("_Comp") != std::string::npos){
  		name_ = replaceString(name_, "_Comp", "");
  	}else{
  		name_.append("_Comp");
  	}
  }
}
void seqInfo::prepend(const std::string& seq,
                      const std::vector<uint32_t>& qual) {
  if (qual.size() == 1) {
    seq_.insert(seq_.begin(), seq.begin(), seq.end());
    prependVec(qual_, std::vector<uint32_t>(seq.size(), qual.front()));
  } else if (qual.size() == seq.size()) {
    seq_.insert(seq_.begin(), seq.begin(), seq.end());
    prependVec(qual_, qual);
  } else {
  	std::stringstream ss;
    ss << "Need to supply either single a quality or same amount of "
                 "quality score for seq length" << "\n";
    ss << "trying to add " << qual.size()
              << " qualities and a seq of length " << seq.length() << "\n";
    throw std::runtime_error{bib::bashCT::boldRed(ss.str())};
  }
}
const std::vector<uint32_t> seqInfo::getLeadQual(uint32_t posA, uint32_t out) const {
  std::vector<uint32_t> ans;
  uint32_t lowerBound = 0;
  if (posA - out > lowerBound) {
    lowerBound = posA - out;
  }
  for (auto i : iter::range(lowerBound, posA)) {
    ans.emplace_back(qual_[i]);
  }
  return ans;
}
const std::vector<uint32_t> seqInfo::getTrailQual(uint32_t posA, uint32_t out) const {
  std::vector<uint32_t> ret;
  uint32_t higherBound = qual_.size() - 1;
  if (posA + out + 1 < higherBound) {
    higherBound = posA + out + 1;
  }
  for (auto i : iter::range(posA + 1, higherBound)) {
    ret.emplace_back(qual_[i]);
  }
  return ret;
}
bool seqInfo::checkLeadQual(uint32_t pos, uint32_t secondayQual, uint32_t out) const {
	uint32_t lowerBound = 0;
  if (static_cast<int32_t>(pos) - out > lowerBound) {
    lowerBound = pos - out;
  }
  for (auto i : iter::range(lowerBound, pos)) {
    if (qual_[i] <= secondayQual) {
      return false;
    }
  }
  return true;
}
bool seqInfo::checkTrailQual(uint32_t pos, uint32_t secondayQual, uint32_t out) const {
	uint32_t higherBound = qual_.size() - 1;
  if (pos + out + 1 < higherBound) {
    higherBound = pos + out + 1;
  }
  for (auto i : iter::range(pos + 1, higherBound)) {
    if (qual_[i] <= secondayQual) {
      return false;
    }
  }
  return true;
}
bool seqInfo::checkPrimaryQual(uint32_t pos, uint32_t primaryQual) const {
  if (qual_[pos] <= primaryQual) {
    return false;
  }
  return true;
}
bool seqInfo::checkQual(uint32_t pos, uint32_t primaryQual, uint32_t secondayQual,
		uint32_t out) const {
  if (!checkPrimaryQual(pos, primaryQual)) {
    return false;
  }
  if (!checkLeadQual(pos, secondayQual, out) ||
      !checkTrailQual(pos, secondayQual, out)) {
    return false;
  }
  return true;
}
uint32_t seqInfo::findLowestNeighborhoodQual(uint32_t posA, uint32_t out) const {
	uint32_t lowerBound = 0;
	uint32_t higherBound = qual_.size() - 1;
  if (static_cast<int32_t>(posA) - out > lowerBound) {
    lowerBound = posA - out;
  }
  if (posA + out + 1 < higherBound) {
    higherBound = posA + out + 1;
  }
  uint32_t lowestQual = UINT32_MAX;
  for (auto i : iter::range(lowerBound, higherBound)) {
    if (posA != i) {
      if (qual_[i] < lowestQual) {
        lowestQual = qual_[i];
      }
    }
  }
  return lowestQual;
}
void seqInfo::removeBase(size_t pos) {
  if (pos >= seq_.size()) {
  	std::stringstream ss;
    ss << "pos: " << pos << " out of bounds of seq " << seq_.size()
              << "\n";
    throw std::runtime_error{bib::bashCT::boldRed(ss.str())};
  } else if(pos >= qual_.size()){
  	std::stringstream ss;
    ss << "pos: " << pos << " out of bounds of qual " << qual_.size()
              << "\n";
    throw std::runtime_error{bib::bashCT::boldRed(ss.str())};
  }else{
    seq_.erase(seq_.begin() + pos);
    qual_.erase(qual_.begin() + pos);
  }
}

void seqInfo::removeLowQualityBases(uint32_t qualCutOff) {
  std::vector<size_t> positions;
  size_t pos = 0;
  for (auto qIter : qual_) {
    if (qIter < qualCutOff) {
      positions.push_back(pos);
    }
    ++pos;
  }
  std::reverse(positions.begin(), positions.end());
  for (const auto& pIter : positions) {
    removeBase(pIter);
  }
}
void seqInfo::removeGaps() {
  for (auto pos : iter::range<int32_t>(seq_.size(), -1, -1)) {
    if (seq_[pos] == '-') {
      removeBase(pos);
    }
  }
  return;
}

// quality strings for printing
std::string seqInfo::getQualString() const { return vectorToString(qual_); }
std::string seqInfo::getFastqString(const std::vector<uint32_t>& quals,
                                    uint32_t offset) {
  std::string convertedQuals = "";
  for (const auto& q : quals) {
    if (q <= 93) {
      convertedQuals.push_back(static_cast<char>(q + offset));
    } else {
      convertedQuals.push_back(static_cast<char>(93 + offset));
    }
  }
  return convertedQuals;
}
std::string seqInfo::getFastqQualString(uint32_t offset) const {
  return getFastqString(qual_, offset);
}

//
void seqInfo::markAsChimeric() {
  if (name_.find("CHI") == std::string::npos) {
    name_ = "CHI_" + name_;
  }
}

void seqInfo::unmarkAsChimeric() {
  if (name_.find("CHI") != std::string::npos) {
    name_ = replaceString(name_, "CHI_", "");
  }
}

// outputs
void seqInfo::outPutFastq(std::ostream& fastqFile) const {
  fastqFile << "@" << name_ << "\n";
  fastqFile << seq_ << "\n";
  fastqFile << "+" << "\n";
  fastqFile << getFastqQualString(33) << "\n";
}

void seqInfo::outPutSeq(std::ostream& fastaFile) const {
  fastaFile << ">" << name_ << "\n";
  fastaFile << seq_ << "\n";
}

void seqInfo::outPutQual(std::ostream& qualFile) const {
  qualFile << ">" << name_ << "\n";
  qualFile << getQualString() << "\n";
}

void seqInfo::outPut(std::ostream& outFile, const readObjectIOOptions & options) const {
	if (options.outFormat_ == "fastq") {
		outPutFastq(outFile);
	} else if (options.outFormat_ == "fasta") {
		outPutSeq(outFile);
	} else {
		throw std::runtime_error { bib::bashCT::boldRed(
				"in seqInfo::outPut(): unrecognized option: " + options.outFormat_) };
	}
}

void seqInfo::outPut(std::ostream& outFile, std::ostream& outFile2, const readObjectIOOptions & options) const {
	if (options.outFormat_ == "fastqQual") {
		outPutSeq(outFile);
		outPutQual(outFile2);
	} else {
		throw std::runtime_error { bib::bashCT::boldRed(
				"in seqInfo::outPut(): unrecognized option: " + options.outFormat_) };
	}
}

bool seqInfo::degenCompare(const seqInfo & otherInfo, const substituteMatrix & compareScores)const{
	if(seq_.length() != otherInfo.seq_.length()){
		return false;
	}
	auto comp = [&](const char & a, const char & b){
		return compareScores.mat_[a][b] > 0;
	};
	return std::equal(seq_.begin(), seq_.end(), otherInfo.seq_.begin(), comp);
}

std::string seqInfo::getStubName(bool removeChiFlag) const {
  size_t tPos = name_.rfind("_t");
  size_t fPos = name_.rfind("_f");
  std::string outString = name_;
  if (tPos == std::string::npos && fPos == std::string::npos) {
    outString = name_;
  } else if (tPos != std::string::npos && fPos != std::string::npos) {
    if (tPos < fPos) {
      outString = name_.substr(0, tPos);
    } else {
      outString = name_.substr(0, fPos);
    }
  } else if (tPos == std::string::npos) {
    outString = name_.substr(0, fPos);
  } else {
    outString = name_.substr(0, tPos);
  }

  if (removeChiFlag) {
    outString = replaceString(outString, "CHI_", "");
  }
  return outString;
}

void seqInfo::setName(const std::string& newName) {
  name_ = newName + "_t" + estd::to_string(cnt_);
}

void seqInfo::addQual(const std::string & qualString) {
	addQual(stringToVector<uint32_t>(qualString));
}

void seqInfo::addQual(const std::string & qualString, uint32_t offSet) {
  qual_.clear();
  for (const auto & c : qualString) {
    qual_.emplace_back(c - offSet);
  }
  if(qual_.size() != seq_.size()){
  	std::stringstream ss;
  	ss << "adding qual size does not equal seq size, qualSize: " << qual_.size()
  			<< ", seqSize: " << seq_.size() << ", for " << name_;
  	throw std::runtime_error{ss.str()};
  }
}

void seqInfo::addQual(const std::vector<uint32_t> & quals){
  if(quals.size() != seq_.size()){
  	std::stringstream ss;
  	ss << "adding qual size does not equal seq size, qualSize: " << quals.size()
  			<< ", seqSize: " << seq_.size() << ", for " << name_;
  	printVector(quals,"\n", ss);
  	ss << seq_ << "\n";
  	throw std::runtime_error{bib::bashCT::boldRed(ss.str())};
  }
  qual_ = quals;
}

double seqInfo::getQualCheck(uint32_t qualCutOff)const{
	uint32_t count = bib::count_if(qual_, [&qualCutOff](uint32_t qual){ return qual >=qualCutOff;});
	return static_cast<double>(count)/qual_.size();
}

void seqInfo::setClip(size_t leftPos, size_t rightPos) {
	seq_ = seq_.substr(leftPos, rightPos - leftPos + 1);
	qual_.erase(qual_.begin() + rightPos + 1,
			qual_.end());
	qual_.erase(qual_.begin(),
			qual_.begin() + leftPos);
}
void seqInfo::setClip(size_t rightPos) {
	setClip(0, rightPos);
}
void seqInfo::setClip(const std::pair<int, int>& positions) {
	setClip(positions.first, positions.second);
}

void seqInfo::trimFront(size_t upToPosNotIncluding) {
  setClip(upToPosNotIncluding, seq_.size() - 1);
}
void seqInfo::trimBack(size_t fromPositionIncluding) {
  setClip(0, fromPositionIncluding - 1);

}

double seqInfo::getAverageQual() const {
  return static_cast<double>(getSumQual()) / qual_.size();
}

double seqInfo::getAverageErrorRate() const {
  double sum = 0;
  for (const auto& q : qual_) {
    sum += pow(10.0, -(q / 10.0));
  }
  return sum / qual_.size();
}

uint32_t seqInfo::getSumQual() const {
  uint32_t sum = 0;
  for (const auto& q : qual_) {
    sum += q;
  }
  return sum;
}

}  // namespace bib
