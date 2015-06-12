#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <iterator>
#include <cppitertools/range.hpp>
#include <cppitertools/reverse.hpp>

#include "stringUtils.hpp"
#include "bibseq/utils/numUtils.hpp"
#include <bibcpp/files.h>

namespace bibseq {
void addToStr(std::string & str, char c){
	str.push_back(c);
}
void addToStr(std::string & str, const std::string & otherStr){
	str.append(otherStr);
}

std::string get_cwd() {
  // from http://stackoverflow.com/a/2869667
  const size_t chunkSize = 255;
  const int maxChunks =
      10240;  // 2550 KiBs of current path are more than enough

  char stackBuffer[chunkSize];  // Stack buffer for the "normal" case
  if (getcwd(stackBuffer, sizeof(stackBuffer)) != NULL){
  	std::string ret = std::string(stackBuffer);
  	bib::files::appendAsNeeded(ret, "/");
  	return ret;
  }

  if (errno != ERANGE) {
    // It's not ERANGE, so we don't know how to handle it
    throw std::runtime_error("Cannot determine the current path.");
    // Of course you may choose a different error reporting method
  }
  // Ok, the stack buffer isn't long enough; fallback to heap allocation
  for (int chunks = 2; chunks < maxChunks; chunks++) {
    // With boost use scoped_ptr; in C++0x, use unique_ptr
    // If you want to be less C++ but more efficient you may want to use realloc
    std::unique_ptr<char> cwd (new char[chunkSize * chunks]);
    if (getcwd(cwd.get(), chunkSize * chunks) != NULL) {
    	std::string ret = std::string(cwd.get());
    	bib::files::appendAsNeeded(ret, "/");
    	return ret;
    }
    if (errno != ERANGE) {
      // It's not ERANGE, so we don't know how to handle it
      throw std::runtime_error("Cannot determine the current path.");
      // Of course you may choose a different error reporting method
    }
  }
  throw std::runtime_error(
      "Cannot determine the current path; the path is apparently unreasonably "
      "long");
  // return "";
}



VecStr streamToVecStr(std::istream& stream) {
  VecStr lines;
  for (std::string line; getline(stream, line);) {
    lines.emplace_back(line);
  }
  return lines;
}
std::vector<char> getUpperCaseLetters() {
  std::vector<char> letters(26);
  std::iota(letters.begin(), letters.end(), 'A');
  return letters;
}
std::vector<char> getLowerCaseLetters() {
  std::vector<char> letters(26);
  std::iota(letters.begin(), letters.end(), 'a');
  return letters;
}
std::vector<char> getLowerUpperCaseLetters() {
  std::vector<char> up = getUpperCaseLetters();
  std::vector<char> low = getLowerCaseLetters();
  up.insert(up.end(), low.begin(), low.end());
  return up;
}
std::vector<char> determineAlph(const std::string & str){
	std::unordered_map<char, uint32_t> counts;
	for(const auto & c : str){
		++counts[c];
	}
	return getVectorOfMapKeys(counts);
}

bool allWhiteSpaceStr(const std::string & str){
	return str.find_first_not_of(' ') == std::string::npos;
}
std::string getTimeFormat(double timeInSecondsOriginal, bool wordy,
                          int secondsDecimalPlaces) {
  std::stringstream duration;
  double timeInSeconds = timeInSecondsOriginal;
  if (timeInSeconds > 31536000) {
    int years = (int)timeInSeconds / 31536000;
    if (wordy) {
      duration << "yrs:" << years << ",";
    } else {
      duration << years << ":";
    }
    timeInSeconds = timeInSeconds - years * 31536000.0;
  }
  if (timeInSeconds > 86400) {
    int days = (int)timeInSeconds / 86400;
    if (wordy) {
      duration << "days:" << leftPadNumStr(days, 365) << ",";
    } else {
      duration << days << ":";
    }
    timeInSeconds = timeInSeconds - days * 86400.0;
  } else if (timeInSecondsOriginal > 86400) {
    if (wordy) {
      duration << "days:000,";
    } else {
      duration << "000:";
    }
  }
  if (timeInSeconds > 3600) {
    int hrs = (int)timeInSeconds / 3600;
    if (wordy) {
      duration << "hrs:" << leftPadNumStr(hrs, 24) << ",";
    } else {
      duration << leftPadNumStr(hrs, 24) << ":";
    }
    timeInSeconds = timeInSeconds - hrs * 3600.0;
  } else if (timeInSecondsOriginal > 3600.0) {
    if (wordy) {
      duration << "hrs:00,";
    } else {
      duration << "00:";
    }
  }
  if (timeInSeconds > 60) {
    int minutes = (int)timeInSeconds / 60;
    if (wordy) {
      duration << "mins:" << leftPadNumStr(minutes, 60) << ",";
    } else {
      duration << leftPadNumStr(minutes, 60) << ":";
    }

    timeInSeconds = timeInSeconds - minutes * 60.0;
  } else if (timeInSecondsOriginal > 60) {
    if (wordy) {
      duration << "mins:00,";
    } else {
      duration << "00:";
    }
  }
  if (timeInSeconds > 0) {
    if (timeInSecondsOriginal < 1) {
      if (wordy) {
        duration << "secs:" << roundDecPlaces(timeInSeconds,
                                              secondsDecimalPlaces);
      } else {
        duration << roundDecPlaces(timeInSeconds, secondsDecimalPlaces);
      }
    } else {
      if (wordy) {
        duration << "secs:"
                 << leftPadNumStr(
                        roundDecPlaces(timeInSeconds, secondsDecimalPlaces),
                        60.0);
      } else {
        duration << leftPadNumStr(
                        roundDecPlaces(timeInSeconds, secondsDecimalPlaces),
                        60.0);
      }
    }
  } else {
    if (wordy) {
      duration << "secs:00";
    } else {
      duration << "00";
    }
  }
  return duration.str();
}

std::string getStringFromSubstrings(const std::string& seq,
                                    const std::vector<size_t>& positons,
                                    int subStringSize) {
  std::string ans;
  for (const auto& pos : positons) {
    ans.append(seq.substr(pos, subStringSize));
  }
  return ans;
}



std::string stripQuotes( const std::string& str ){
  if (str.size() > 2){
    if ( ( str.front() == '"'  && str.back() == '"' ) ||
         ( str.front() == '\'' && str.back() == '\'') ){
    	return std::string( str.begin() + 1, str.end() - 1 );
    }
  }
  return str;
}

void trimStringAtFirstOccurence(std::string& str,
                                const std::string& occurence) {
  size_t pos = str.find(occurence);
  if (pos != std::string::npos) {
    str = str.substr(0, pos);
  }
}

void trimStringsAtFirstOccurence(VecStr& strings,
                                 const std::string& occurence) {
  for (auto& currentString : strings) {
    trimStringAtFirstOccurence(currentString, occurence);
  }
}

bool containsSubString(const std::string& str, const std::string& subString) {
  return (str.find(subString) != std::string::npos);
}

bool endsWith(const std::string& a, const std::string& b) {
  // http://stackoverflow.com/a/874160
  if (a.size() >= b.size()) {
    return (0 == a.compare(a.size() - b.size(), b.size(), b));
  }
  return false;
}

bool beginsWith(const std::string& str, const std::string& target) {
  if (target.size() <= str.size()){
  	return (0 == str.compare(0, target.size(), target));
  }
  return false;
}

std::string replaceString(std::string theString,
                          const std::string& replaceSpace,
                          const std::string& newSpace) {
  size_t spaceSize = replaceSpace.size();
  size_t currPos = theString.find(replaceSpace);
  while (currPos != std::string::npos) {
    theString.replace(currPos, spaceSize, newSpace);
    currPos = theString.find(replaceSpace, currPos + newSpace.size());
  }
  return theString;
}

std::string removeCharReturn(std::string inputStr, const char& theChar) {
	removeChar(inputStr, theChar);
  return inputStr;
}
void removeChar(std::string& inputStr, const char& theChar) {
  inputStr.erase(std::remove(inputStr.begin(), inputStr.end(), theChar),
                 inputStr.end());
  return;
}

// remove lower case letters and their corresponding qualities from the
// sequence.
void rstrip(std::string & str, char c){
	uint32_t pos = str.size();
	while (pos != 0 && str[pos - 1] == c){
		--pos;
	}
	if(pos != str.size()){
		str.erase(str.begin() + pos, str.end());
	}
}

std::string rstripReturn(std::string str, char c){
	rstrip(str,c);
	return str;
}
VecStr tokenizeString(const std::string& str, const std::string& delim,
                      bool addEmptyToEnd) {
  VecStr output;
  if("whitespace" == delim){
    std::stringstream tempStream(str);
    while (!tempStream.eof()) {
      std::string tempName;
      tempStream >> tempName;
      output.emplace_back(tempName);
    }
  }else{
    if (str.find(delim.c_str()) == std::string::npos) {
      output.emplace_back(str);
    } else {
      std::size_t pos = str.find(delim, 0);
      std::size_t oldPos = -delim.size();
      while (pos != std::string::npos) {
        output.emplace_back(
            str.substr(oldPos + delim.size(), pos - oldPos - delim.size()));
        oldPos = pos;
        pos = str.find(delim, pos + 1);
      }
      if (oldPos + delim.size() == str.size()) {
        if (addEmptyToEnd) {
          output.emplace_back("");
        }
      } else {
        output.emplace_back(str.substr(oldPos + delim.size(), str.size() - 1));
      }
    }
  }
  return output;
}

std::vector<size_t> findOccurences(const std::string& target,
                                   const std::string& subSeq) {
  std::vector<size_t> indexs;
  size_t pos = target.find(subSeq, 0);
  while (pos != std::string::npos) {
    indexs.push_back(pos);
    pos = target.find(subSeq, pos + 1);
  }
  return indexs;
}

int countOccurences(const std::string& target, const std::string& subSeq) {
  return findOccurences(target, subSeq).size();
}

void translateStringWithKey(std::string& str, MapStrStr& key) {

  for (size_t i = 0; i < str.size(); ++i) {
    str.replace(i, 1, key[str.substr(i, 1)]);
  }
}

std::string condenseSeqSimple(const std::string& seq) {
  std::string condensedSeq = "";
  int currentCount = 1;
  uint32_t i = 1;
  for (; i < seq.size(); i++) {
    if (seq[i] == seq[i - 1]) {
      currentCount++;
    } else {
      condensedSeq.push_back(seq[i - 1]);
      currentCount = 1;
    }
  }
  condensedSeq.push_back(seq[i - 1]);
  return condensedSeq;
}

/////lower case handling functions

// this function changes each element of a string to upper case:
void stringToUpper(std::string& str) {
  for (auto& c : str) {
    c = toupper(c);
  }
}
void stringToLower(std::string& str) {
  for (auto& c : str) {
    c = tolower(c);
  }
}
std::string stringToUpperReturn(std::string str) {
  stringToUpper(str);
  return str;
}
std::string stringToLowerReturn(std::string str) {
  stringToLower(str);
  return str;
}

void changeSubStrToLower(std::string& str, size_t pos, size_t length) {
  for (size_t i = pos; i < pos + length; ++i) {
    if (pos + i > str.size()) {
      break;
    }
    str[i] = tolower(str[i]);
  }
}
void changeCertainSubStrToLower(std::string& str,
                                const std::string& substring) {
  size_t pos = str.find(substring);
  if (pos != std::string::npos) {
    changeSubStrToLower(str, pos, substring.size());
  }
}

void changeSubStrToLowerToEnd(std::string& str, size_t pos) {
  for (size_t i = pos; i < str.size(); ++i) {
    str[i] = tolower(str[i]);
  }
}
void changeSubStrToLowerFromBegining(std::string& str, size_t pos) {
  for (size_t i = 0; i <= pos; ++i) {
    str[i] = tolower(str[i]);
  }
}
void changeSubStrToUpperFromBegining(std::string& str, size_t pos) {
  for (size_t i = 0; i <= pos; ++i) {
    str[i] = toupper(str[i]);
  }
}
void subStrToUpper(std::string & str, uint32_t pos, uint32_t len){
	for(auto p : iter::range(pos, pos + len)){
		str[p] = toupper(str[p]);
	}
}
void changeStringVectorToLowerCase(VecStr& vec) {
  for (auto& v : vec) {
    stringToLower(v);
  }
}

bool isIntStr(const std::string& str) {
  for (const auto& c : str) {
  	if(c == str.front() && c == '-'){
  		continue;
  	}
    if (!isdigit(c)) {
      return false;
    }
  }
  return true;
}
//std::regex intPat { R"([-+]?[0-9]*)"};
const std::regex doublePat { R"([-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?)"};
bool isDoubleStr(const std::string& str) {
  return std::regex_match(str, doublePat);
}

bool isVecOfIntStr(const VecStr& vec) {
  for (const auto& s : vec) {
    if (!isIntStr(s)) {
      return false;
    }
  }
  return true;
}

//vectorOfNumberStringsDouble
bool isVecOfDoubleStr(const VecStr& vec) {
  for (const auto& s : vec) {
    if (!isDoubleStr(s)) {
      return false;
    }
  }
  return true;
}



// combiners
std::string combineStrings(const VecStr& strings) {
  std::stringstream ans;
  for (const auto& sIter : strings) {
    ans << sIter;
  }
  return ans.str();
}

std::string repeatString(const std::string& stringToRepeat, uint32_t n) {
  std::ostringstream os;
  for (uint32_t i = 0; i < n; ++i) {
    os << stringToRepeat;
  }
  return os.str();
}

void trimEndWhiteSpace(std::string& str) {
  size_t lastPlacePos = str.size() - 1;
  char lastPlace = str[str.size() - 1];
  while (isspace(lastPlace)) {
    lastPlacePos--;
    lastPlace = str[lastPlacePos];
  }
  size_t firstPlacePos = 0;
  char firstPlace = str[0];
  while (isspace(firstPlace)) {
    firstPlacePos++;
    firstPlace = str[firstPlacePos];
  }
  str = str.substr(firstPlacePos, lastPlacePos - firstPlacePos + 1);
}

std::string trimEndWhiteSpaceReturn(std::string str) {
  size_t lastPlacePos = str.size() - 1;
  char lastPlace = str[str.size() - 1];
  while (isspace(lastPlace)) {
    lastPlacePos--;
    lastPlace = str[lastPlacePos];
  }
  size_t firstPlacePos = 0;
  char firstPlace = str[0];
  while (isspace(firstPlace)) {
    firstPlacePos++;
    firstPlace = str[firstPlacePos];
  }
  return str.substr(firstPlacePos, lastPlacePos - firstPlacePos + 1);
}
VecStr checkTwoRotatingStrings(const std::string& str1, const std::string& str2,
                               int allowableMismatches) {
  if (str1.length() != str2.length()) {
    std::cout << "Strings should be the same length" << std::endl;
    std::cout << "Str1: " << str1 << " str2: " << str2 << std::endl;
    return VecStr{};
  }
  int minError = allowableMismatches;
  VecStr lowMismatches;
  for (const auto& i : iter::range(str1.length())) {
    std::string current = str1.substr(i, str1.length() - i) + str1.substr(0, i);
    int currentMinError = numberOfMismatches(current, str2);
    if (currentMinError < minError) {
      minError = currentMinError;
      lowMismatches.clear();
      lowMismatches.push_back(current);
    } else if (currentMinError == minError) {
      lowMismatches.push_back(current);
    }
  }
  return lowMismatches;
}
int numberOfMismatches(const std::string& str1, const std::string& str2) {
  if (str1.length() != str2.length()) {
    std::cout << "Strings should be the same length" << std::endl;
    std::cout << "Str1: " << str1 << " Str2: " << str2 << std::endl;
    return 0;
  }
  auto mis = std::make_pair(str1.begin(), str2.begin());
  int count = 0;
  while (mis.first != str1.end()) {
    mis = std::mismatch(mis.first, str1.end(), mis.second);
    if (mis.first != str1.end()) {
      ++mis.first;
      ++mis.second;
      ++count;
    }
  }
  return count;
}

std::size_t findFirstWhitespace(const std::string & str){
	return str.find_first_of(" \f\n\r\t\v");
}
bool trimAtFirstWhitespace(std::string & str){
	std::size_t firstWhitespace = findFirstWhitespace(str);
	if(firstWhitespace == std::string::npos	){
		return false;
	}else{
		str.erase(firstWhitespace);
		return true;
	}
}
std::vector<char> processAlphStrVecChar(const std::string & alphabetStr,
		const std::string & delim){
	std::vector<char> ans;
	VecStr toks = tokenizeString(alphabetStr, delim);
	for(const auto & t : toks){
		ans.emplace_back(t[0]);
	}
	return ans;
}
std::pair<std::vector<char>, std::vector<uint32_t>> processAlphStrVecCharCounts(const std::string & alphabetStr,
		const std::string & delim){
	std::vector<char> ansLets;
	std::vector<uint32_t> ansCounts;
	VecStr toks = tokenizeString(alphabetStr, delim);
	for(const auto & t : toks){
		ansLets.emplace_back(t[0]);
		if(t.length() > 2){
			ansCounts.emplace_back(std::stof(t.substr(1)));
		}else{
			ansCounts.emplace_back(1);
		}
	}
	return {ansLets, ansCounts};
}

VecStr processAlphStrVecStr(const std::string & alphabetStr, const std::string & delim){
	VecStr ans;
	VecStr toks = tokenizeString(alphabetStr, delim);
	for(const auto & t : toks){
		ans.emplace_back(t);
	}
	return ans;
}

/*
uint32_t firstMismatch(const std::string& str1,
                     const std::string& str2,
                     char ignore){

}
uint32_t lastMismtach(const std::string& str1,
                    const std::string& str2
                    char ignore){

}*/

std::string intToHex(int i) {
  std::stringstream stream;
  // stream << "0x";
  stream << std::setfill('0') << std::setw(2) << std::hex << i;
  return stream.str();
}
uint32_t hexToInt(const std::string& hString) {
  std::stringstream stream(hString);
  uint32_t ans;
  stream >> std::hex >> ans;
  return ans;
}



uint32_t countBeginChar(const std::string & str){
	if(str.length() == 0){
		return 0;
	}
	uint32_t ret = 1;
	for(auto pos : iter::range<uint64_t>(1, str.length())){
		if(str[pos] == str.front()){
			++ret;
		}else{
			break;
		}
	}
	return ret;
}

uint32_t countEndChar(const std::string & str){
	if(str.length() == 0){
		return 0;
	}
	uint32_t ret = 1;
	uint32_t pos = str.size() - 1;
	while(pos > 0 && str[pos - 1] == str.back() ){
		++ret;
		--pos;
	}
	return ret;
}
}  // namespace bib
