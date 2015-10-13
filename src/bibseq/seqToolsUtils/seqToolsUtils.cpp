#include "seqToolsUtils.hpp"
#include "bibseq/objects/seqObjects/cluster.hpp"
#include "bibseq/seqToolsUtils/distCalc.hpp"
#include "bibseq/helpers/kmerCalculator.hpp"
#include "bibseq/IO/readObjectIOOpt.hpp"
#include "bibseq/objects/dataContainers/graphs/UndirWeightedGraph.hpp"

//////tools for dealing with multipleSampleCollapse
namespace bibseq {




Json::Value genMinTreeData(const std::vector<readObject> & reads, aligner & alignerObj){
	std::function<uint32_t(const readObject &, const readObject &, aligner, bool)> misFun =
			getMismatches<readObject>;
	uint32_t numThreads = 2;
	readDistGraph<uint32_t> graphMis(reads, numThreads, misFun, alignerObj, true);

	std::vector<std::string> popNames;
	for (const auto & n : graphMis.nodes_) {
		popNames.emplace_back(n->name_);
	}
	auto nameColors = getColorsForNames(popNames);
	return graphMis.toJsonMismatchGraphAll(bib::color("#000000"), nameColors);
}

Json::Value genMinTreeData(const std::vector<readObject> & reads) {
	uint64_t maxLength = 0;
	readVec::getMaxLength(reads, maxLength);
	aligner alignerObj(maxLength, gapScoringParameters(5, 1),
			substituteMatrix::createDegenScoreMatrix(2, -2));
	std::function<uint32_t(const readObject &, const readObject &, aligner, bool)> misFun =
			getMismatches<readObject>;
	return genMinTreeData(reads, alignerObj);
}

uint32_t countSeqs(const readObjectIOOptions & opts, bool verbose){
	uint32_t ret = 0;
	if(bib::files::bfs::exists(opts.firstName_)){
		readObjectIOOpt reader(opts);
		reader.openIn();
		readObject read;
		while(reader.readNextRead(read)){
			ret += ::round(read.seqBase_.cnt_);
		}
	}else if(verbose){
		std::cout << "File: " << opts.firstName_ << "doesn't exist, returning 0" << std::endl;
	}
	return ret;
}


ExtractionInfo collectExtractionInfo(const std::string & dirName, const std::string & indexToDir, const std::string & sampNames){
	std::string nameDelim = "_extractor_";
	auto dirs = getNewestDirs(dirName, nameDelim);
	table indexNamesTab(indexToDir,"\t", true);
  table mainTableExtractionProfile;
  table mainTableExtractionStats;
	std::unordered_map<std::string, std::string> nameToIndex;
	for (const auto & row : indexNamesTab.content_) {
		if (row.size() != 2) {
			std::cerr << "Error in parsing " << indexToDir
					<< ", should have two columns, not " << row.size() << std::endl;
			std::cerr << vectorToString(row, "\t") << std::endl;
			exit(1);
		}
		auto lastPeriod = row[1].rfind(".");
		nameToIndex[row[1].substr(0, lastPeriod)] = row[0];
	}
	uint32_t count = 0;
	VecStr oldProfileColNames;

	VecStr oldStatsColNames;
	table inTab(sampNames, "whitespace", false);
	//goes sample name -> pairs of midname - index name
	std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> sampleDirWithSubDirs;
	for(const auto & rowPos : iter::range(inTab.content_.size())){
		const auto & row = inTab.content_[rowPos];
		if(row.empty() || row[0].front() == '#'){
			continue;
		}
		if (row.size() < 3) {
			throw std::runtime_error { bib::err::F()
					<< "setUpSampleDirs: rows should have at least 3 columns, row: "
					<< rowPos << "has " << row.size() };
		}
		for(const auto & colPos : iter::range<uint32_t>(2,row.size())){
			if(row[colPos] == "" || allWhiteSpaceStr(row[colPos])){
				continue;
			}
			//Discrepancy  between people naming MID01 and MID1 so replacing MID0 with MID
			sampleDirWithSubDirs[row[1]].emplace_back(replaceString(row[colPos], "MID0", "MID"), row[0]);
		}
	}

	//extraction info by index by mid;
	std::unordered_map<std::string, std::unordered_map<std::string, VecStr>> extractionInfo;

	for (const auto &dir : dirs) {
		table inProfileTab(dir + "/extractionProfile.tab.txt", "\t", true);
		//i have accidentally added one more tab than is needed to
		//extractionProfile which makes it so all rows have an empty
		//at the end that correspond to any column names
		for (auto & row : inProfileTab.content_) {
			if (row.back() == "") {
				row.erase(row.begin() + row.size() - 1);
			}
		}
		table inStatsTab(dir + "/extractionStats.tab.txt", "\t", true);
		std::string dirName = bib::files::getFileName(dir);
		auto pos = dirName.find(nameDelim);
		std::string indexName = nameToIndex[dirName.substr(0, pos)];
		if (count == 0) {
			oldStatsColNames = inStatsTab.columnNames_;
			oldProfileColNames = inProfileTab.columnNames_;
		}
		inStatsTab.addColumn(VecStr { indexName }, "IndexName");
		inProfileTab.addColumn(VecStr { indexName }, "IndexName");
		for(const auto & row : inProfileTab.content_){
			auto midName = row[inProfileTab.getColPos("name")];
			auto midPos = midName.rfind("MID");
			midName = replaceString(midName.substr(midPos), "MID0", "MID");
			extractionInfo[row[inProfileTab.getColPos("IndexName")]][midName] = row;
		}
		if (count == 0) {
			mainTableExtractionStats = inStatsTab;
			mainTableExtractionProfile = inProfileTab;
		} else {
			mainTableExtractionStats.rbind(inStatsTab);
			mainTableExtractionProfile.rbind(inProfileTab);
		}
		++count;
	}

	table outSampleInfo(catenateVectors(VecStr{"Sample"}, mainTableExtractionProfile.columnNames_));
	for(const auto & samp : sampleDirWithSubDirs){
		for(const auto & indexMidNames : samp.second){
			auto addingRow = extractionInfo[indexMidNames.second][indexMidNames.first];
			if(addingRow.empty()){
				addingRow = std::vector<std::string>(mainTableExtractionProfile.content_.front().size(), "0");
			}
			addingRow[mainTableExtractionProfile.getColPos("IndexName")] = indexMidNames.second;
			addingRow[mainTableExtractionProfile.getColPos("name")] = indexMidNames.first;
			outSampleInfo.content_.emplace_back(catenateVectors(VecStr{samp.first}, addingRow));
		}
	}

	auto outSampleColName = catenateVectors(VecStr{"Sample"}, catenateVectors(VecStr{"IndexName"}, oldProfileColNames));
	auto profileColNames = catenateVectors(VecStr{"IndexName"}, oldProfileColNames);
	auto statsColName = catenateVectors(VecStr{"IndexName"}, oldStatsColNames);


	outSampleInfo = outSampleInfo.getColumns(outSampleColName);
	outSampleInfo.sortTable("Sample", false);
	mainTableExtractionProfile = mainTableExtractionProfile.getColumns(profileColNames);
	mainTableExtractionStats = mainTableExtractionStats.getColumns(statsColName);
	outSampleInfo.columnNames_[outSampleInfo.getColPos("name")] = "MidName";
	outSampleInfo.setColNamePositions();
	return ExtractionInfo(mainTableExtractionStats, mainTableExtractionProfile, outSampleInfo);
}

ExtractionInfo collectExtractionInfoDirectName(const std::string & dirName, const std::string & indexToDir, const std::string & sampNames){
	std::string nameDelim = "_extractor_";
	VecStr dirs;
	table indexNamesTab(indexToDir,"\t", true);
  table mainTableExtractionProfile;
  table mainTableExtractionStats;
	std::unordered_map<std::string, std::string> nameToIndex;
	for (const auto & row : indexNamesTab.content_) {
		if (row.size() != 2) {
			std::cerr << "Error in parsing " << indexToDir
					<< ", should have two columns, not " << row.size() << std::endl;
			std::cerr << vectorToString(row, "\t") << std::endl;
			exit(1);
		}
		dirs.emplace_back(row[1]);
		nameToIndex[row[1]] = row[0];
	}
	uint32_t count = 0;
	VecStr oldProfileColNames;

	VecStr oldStatsColNames;
	table inTab(sampNames, "whitespace", false);
	//goes sample name -> pairs of midname - index name
	std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> sampleDirWithSubDirs;
	for(const auto & rowPos : iter::range(inTab.content_.size())){
		const auto & row = inTab.content_[rowPos];
		if(row.empty() || row[0].front() == '#'){
			continue;
		}
		if (row.size() < 3) {
			throw std::runtime_error { bib::err::F()
					<< "setUpSampleDirs: rows should have at least 3 columns, row: "
					<< rowPos << "has " << row.size() };
		}
		for(const auto & colPos : iter::range<uint32_t>(2,row.size())){
			if(row[colPos] == "" || allWhiteSpaceStr(row[colPos])){
				continue;
			}
			//Discrepancy  between people naming MID01 and MID1 so replacing MID0 with MID
			sampleDirWithSubDirs[row[1]].emplace_back(replaceString(row[colPos], "MID0", "MID"), row[0]);
		}
	}

	//extraction info by index by mid;
	std::unordered_map<std::string, std::unordered_map<std::string, VecStr>> extractionInfo;

	for (const auto &dir : dirs) {
		auto fullDirName = bib::files::appendAsNeededRet(dirName, "/") + dir;
		table inProfileTab(fullDirName + "/extractionProfile.tab.txt", "\t", true);
		//i have accidentally added one more tab than is needed to
		//extractionProfile which makes it so all rows have an empty
		//at the end that correspond to any column names
		for (auto & row : inProfileTab.content_) {
			if (row.back() == "") {
				row.erase(row.begin() + row.size() - 1);
			}
		}
		table inStatsTab(fullDirName + "/extractionStats.tab.txt", "\t", true);
		std::string indexName = nameToIndex[dir];
		if (count == 0) {
			oldStatsColNames = inStatsTab.columnNames_;
			oldProfileColNames = inProfileTab.columnNames_;
		}
		inStatsTab.addColumn(VecStr { indexName }, "IndexName");
		inProfileTab.addColumn(VecStr { indexName }, "IndexName");
		for(const auto & row : inProfileTab.content_){
			auto midName = row[inProfileTab.getColPos("name")];
			auto midPos = midName.rfind("MID");
			midName = replaceString(midName.substr(midPos), "MID0", "MID");
			extractionInfo[row[inProfileTab.getColPos("IndexName")]][midName] = row;
		}
		if (count == 0) {
			mainTableExtractionStats = inStatsTab;
			mainTableExtractionProfile = inProfileTab;
		} else {
			mainTableExtractionStats.rbind(inStatsTab);
			mainTableExtractionProfile.rbind(inProfileTab);
		}
		++count;
	}

	table outSampleInfo(catenateVectors(VecStr{"Sample"}, mainTableExtractionProfile.columnNames_));
	for(const auto & samp : sampleDirWithSubDirs){
		for(const auto & indexMidNames : samp.second){
			auto addingRow = extractionInfo[indexMidNames.second][indexMidNames.first];
			if(addingRow.empty()){
				addingRow = std::vector<std::string>(mainTableExtractionProfile.content_.front().size(), "0");
			}
			addingRow[mainTableExtractionProfile.getColPos("IndexName")] = indexMidNames.second;
			addingRow[mainTableExtractionProfile.getColPos("name")] = indexMidNames.first;
			outSampleInfo.content_.emplace_back(catenateVectors(VecStr{samp.first}, addingRow));
		}
	}

	auto outSampleColName = catenateVectors(VecStr{"Sample"}, catenateVectors(VecStr{"IndexName"}, oldProfileColNames));
	auto profileColNames = catenateVectors(VecStr{"IndexName"}, oldProfileColNames);
	auto statsColName = catenateVectors(VecStr{"IndexName"}, oldStatsColNames);


	outSampleInfo = outSampleInfo.getColumns(outSampleColName);
	outSampleInfo.sortTable("Sample", false);
	mainTableExtractionProfile = mainTableExtractionProfile.getColumns(profileColNames);
	mainTableExtractionStats = mainTableExtractionStats.getColumns(statsColName);
	outSampleInfo.columnNames_[outSampleInfo.getColPos("name")] = "MidName";
	outSampleInfo.setColNamePositions();
	return ExtractionInfo(mainTableExtractionStats, mainTableExtractionProfile, outSampleInfo);
}



void setUpSampleDirs(
    const std::string& sampleNamesFilename,
		const std::string& mainDirectoryName,
		bool separatedDirs) {
	auto topDir = bib::replaceString(mainDirectoryName, "./", "");
	table inTab(sampleNamesFilename, "whitespace", false);
	std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> sampleDirWithSubDirs;
	for(const auto & rowPos : iter::range(inTab.content_.size())){
		const auto & row = inTab.content_[rowPos];
		if(row.empty() || row[0].front() == '#'){
			continue;
		}
		if (row.size() < 3) {
			throw std::runtime_error { bib::err::F()
					<< "setUpSampleDirs: rows should have at least 3 columns, row: "
					<< rowPos << "has " << row.size() };
		}
		for(const auto & colPos : iter::range<uint32_t>(2,row.size())){
			if(row[colPos] == "" || allWhiteSpaceStr(row[colPos])){
				continue;
			}
			sampleDirWithSubDirs[row[1]].emplace_back(row[colPos], row[0]);
		}
	}
	std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> indexsWithFullSampPathNames;
	auto cwd = get_cwd();
	for(const auto & sDirs : sampleDirWithSubDirs){
		std::cout << bib::bashCT::bold << "Making Sample Dir: " << bib::bashCT::green << topDir + sDirs.first
				<< bib::bashCT::reset << std::endl;
		std::string sampDir = "";
		if(separatedDirs){
			try {
				std::string indexDirName = sDirs.second.front().second;
				if(!bib::files::bfs::exists(bib::files::join(topDir,sDirs.second.front().second))){
					 bib::files::makeDir(topDir, sDirs.second.front().second);
				}
				sampDir = bib::files::makeDir(topDir, indexDirName + "/" + sDirs.first);
			} catch (std::exception & e) {
				std::stringstream ss;
				ss << bib::bashCT::boldRed(e.what()) << std::endl;
				throw std::runtime_error{ss.str()};
			}
		}else{
			try {
				sampDir = bib::files::makeDir(topDir, sDirs.first);
			} catch (std::exception & e) {
				std::stringstream ss;
				ss << bib::bashCT::boldRed(e.what()) << std::endl;
				throw std::runtime_error{ss.str()};
			}
		}

		for(const auto & midNames : sDirs.second){
			std::cout << bib::bashCT::bold << "Making Mid Dir: " << bib::bashCT::blue << sampDir + midNames.first
					<< bib::bashCT::reset << std::endl;
			std::string midDir = "";
			try {
				midDir = bib::files::makeDir(sampDir,midNames.first);
			} catch (std::exception & e) {
				std::stringstream ss;
				std::cerr << bib::bashCT::boldRed(e.what()) << std::endl;
				throw std::runtime_error{ss.str()};
			}
			bib::files::appendAsNeeded(cwd, "/");
			indexsWithFullSampPathNames[midNames.second].emplace_back(midNames.first, cwd + midDir);
		}
	}

  std::string indexDir = bib::files::makeDir(mainDirectoryName, "locationByIndex");
  for (const auto& indexIter : indexsWithFullSampPathNames) {
    std::ofstream indexFile;
    openTextFile(indexFile, indexDir + replaceString(indexIter.first),
                 ".tab.txt", false, false);
    for (const auto& spIter : indexIter.second) {
      indexFile << spIter.first << "\t"
                << replaceString(spIter.second, "./", "") << std::endl;
    }
  }
}

std::string genHtmlStrForPsuedoMintree(std::string jsonFileName){
	std::string ret = "<!DOCTYPE html>"
	"<meta charset=\"utf-8\">"
	"<body>"
	"<script src=\"http://d3js.org/d3.v3.min.js\"></script>"
	"<script src=\"http://ajax.googleapis.com/ajax/libs/jquery/2.1.1/jquery.min.js\"></script>"
	"<script src=\"http://bib8.umassmed.edu/~hathawan/js/psuedoMinTree.js\"></script>"
	"<button id=\"save_svg\">Save as Svg</button>"
	"<svg id = \"main\"></svg>"
	"<script>"
	"var jsonDat = \""+ jsonFileName + "\";"
	"var add = \"#main\";"
	"drawPsuedoMinTree(jsonDat, add);"
	"var bName = \"#save_svg\";"
	"addSvgSaveButton(bName, add);"
	"</script>"
	"</body>";
	return ret;
}


std::pair<uint32_t, uint32_t> getMaximumRounds(double cnt){
	uint32_t count = 0;
	double currentCnt = cnt;
	while((currentCnt/3.0) > 1){
		currentCnt = currentCnt/3.0;
		++count;
	}

	return {count,  static_cast<uint32_t>(currentCnt) };
}


VecStr getRecurrenceStemNames(const VecStr& allNames) {
  VecStr recurentNames = getStringsContains(allNames, "R");
  VecStr recurentNamesNoRs;
  for (const auto& sIter : recurentNames) {
    recurentNamesNoRs.push_back(replaceString(sIter, "R", ""));
  }
  return getUniqueStrings(recurentNamesNoRs);
}
std::vector<char> getAAs(bool noStopCodon){
	std::vector<char> ans = getVectorOfMapKeys(aminoAcidInfo::infos::allInfo);
	if(noStopCodon){
		removeElement(ans, '*');
	}
	return ans;
}


void processRunCutoff(uint32_t& runCutOff, const std::string& runCutOffString,
                      int counter) {
	auto toks = tokenizeString(runCutOffString, ",");
	if(toks.size() == 1){
	  if (runCutOffString.back() == '%') {
	    runCutOff = std::round(
	        std::stof(runCutOffString.substr(0, runCutOffString.length() - 1)) *
	        counter / 100);
	  } else {
	    runCutOff = std::stoi(runCutOffString);
	  }
	}else{
	  if (toks[0].back() == '%') {
	    runCutOff = std::round(
	        std::stod(toks[0].substr(0, toks[0].length() - 1)) *
	        counter / 100.0);
	  } else {
	    runCutOff = std::stoi(toks[0]);
	  }
	  int32_t hardCutOff = std::stoi(toks[1]);
	  if(hardCutOff > runCutOff){
	  	runCutOff = hardCutOff;
	  }
	}
}

uint32_t processRunCutoff(const std::string& runCutOffString,
                      uint64_t counter){
	uint32_t ret = 0;
	processRunCutoff(ret, runCutOffString, counter);
  return ret;
}


void makeMultipleSampleDirectory(const std::string& barcodeFilename,
                                 const std::string& mainDirectoryName) {
  seqUtil seqHelper = seqUtil();
  VecStr barcodes = seqHelper.getBarcodesInOrderTheyAppear(barcodeFilename);
  if (barcodes.size() % 2 != 0) {
  	std::stringstream ss;
    ss << "need to have an even amount of barcodes, currently have "
              << barcodes.size() << std::endl;
    throw std::runtime_error{ss.str()};
  }
  auto longestSharedName = seqUtil::findLongestSharedSubString(barcodes);
  VecStr combinedNames;
  std::string directoryName = bib::files::makeDir("./", mainDirectoryName);
  for (size_t i = 0; i < barcodes.size(); i += 2) {
    auto firstName = replaceString(barcodes[i], longestSharedName, "");
    auto secondName = replaceString(barcodes[i + 1], longestSharedName, "");
    auto combinedName = longestSharedName + firstName + secondName;
    combinedNames.push_back(combinedName);
    auto combinedDirectoryName = bib::files::makeDir(directoryName, combinedName);
    bib::files::makeDir(combinedDirectoryName, barcodes[i]);
    bib::files::makeDir(combinedDirectoryName, barcodes[i + 1]);
  }
  // std::cout<<vectorToString(combinedNames)<<std::endl;
}
void makeSampleDirectoriesWithSubDirectories(
    const std::string& barcodeFilename, const std::string& mainDirectoryName) {

  std::map<std::pair<std::string, std::string>, VecStr> directoryNames;
  table inTab(barcodeFilename, "\t");
  for (const auto& fIter : inTab.content_) {

    auto indexAndSampleName = std::make_pair(fIter[0], fIter[1]);
    int count = 0;
    for (const auto& lIter : fIter) {
      if (count == 0) {
        // TODO: this shouldn't be needed, as [] always return valid object
        directoryNames[indexAndSampleName] = {};
      } else if (count == 1) {

      } else {
        directoryNames[indexAndSampleName].emplace_back(lIter);
      }
      ++count;
    }
  }

  VecStr actualDirectoryNames;
  std::map<std::string, std::vector<std::pair<std::string, std::string>>>
      indexMap;
  for (const auto& dIter : directoryNames) {
    actualDirectoryNames.push_back(dIter.first.second);
    if (indexMap.find(dIter.first.first) == indexMap.end()) {
      // TODO: this shouldn't be needed, as [] always return valid object
      indexMap[dIter.first.first] = {};
    }
    for (const auto& sIter : dIter.second) {
      auto currentWorkingDirectory = get_cwd();
      indexMap[dIter.first.first]
          .push_back({sIter, currentWorkingDirectory + "/" + mainDirectoryName +
                                 dIter.first.second + "/" + sIter + "/"});
      actualDirectoryNames.push_back(dIter.first.second + "/" + sIter);
    }
  }

  std::cout << "Making following directories" << std::endl;
  std::cout << vectorToString(actualDirectoryNames, "\n");
  std::cout << std::endl;
  std::string indexDir = bib::files::makeDir(mainDirectoryName, "locationByIndex");
  for (const auto& indexIter : indexMap) {
    std::ofstream indexFile;
    openTextFile(indexFile, indexDir + replaceString(indexIter.first),
                 ".tab.txt", false, false);
    for (const auto& spIter : indexIter.second) {
      indexFile << spIter.first << "\t"
                << replaceString(spIter.second, "./", "") << std::endl;
    }
  }
  for (const auto& e : actualDirectoryNames) {
  	bib::files::makeDir(mainDirectoryName, e);
  }
}

void processKrecName(readObject& read, bool post) {
  if (post) {
    auto toks = tokenizeString(read.seqBase_.name_, "|");
    read.seqBase_.cnt_ = 20000 * (atof(toks[1].c_str()) / 100.00);
  } else {
    auto toks = tokenizeString(read.seqBase_.name_, "_");
    read.seqBase_.cnt_ = atoi(toks[1].c_str());
  }
  read.updateName();
}



/////////tools for finding additional location output
std::string makeIDNameComparable(const std::string& idName) {
  std::string ans = replaceString(idName, "ID0", "");
  ans = replaceString(ans, "M0", "M");
  return replaceString(ans, "ID", "");
}
std::string findIdNameFromFileName(const std::string& filename) {
  auto periodPos = filename.rfind(".");
  if (periodPos != std::string::npos && !isdigit(filename[periodPos - 1])) {
    periodPos = filename.rfind("_");
  }
  auto lastMPos = filename.rfind("M");
  if(lastMPos == std::string::npos){
  	return filename;
  }
  return filename.substr(lastMPos, periodPos - lastMPos);
}
std::string processFileNameForID(const std::string& fileName) {
  return makeIDNameComparable(findIdNameFromFileName(fileName));
}

std::string findAdditonalOutLocation(const std::string& locationFile,
                                     const std::string& fileName) {
	table inTab(locationFile, "\t");
  MapStrStr additionalOutNames;
  for (const auto& fIter : inTab.content_) {
    additionalOutNames[makeIDNameComparable(fIter[0])] = fIter[1];
  }

  return additionalOutNames[processFileNameForID(fileName)];
}

VecStr getPossibleDNASubstringsForProtein(const std::string& seq,
                                          const std::string& protein,
                                          const std::string& seqType) {

  VecStr ans;
  std::vector<std::unordered_map<size_t, size_t>> positions;
  std::map<char, VecStr> codonMap;
  if (seqType == "DNA") {
  	for(const auto & aa : aminoAcidInfo::infos::allInfo){
  		codonMap[aa.first] = aa.second.dnaCodons_;
  	}
  } else if (seqType == "RNA") {
  	for(const auto & aa : aminoAcidInfo::infos::allInfo){
  		codonMap[aa.first] = aa.second.rnaCodons_;
  	}
  } else {
  	std::stringstream ss;
    ss << "Unrecognized seqType : " << seqType << std::endl;
    throw std::runtime_error{ss.str()};
  }
  std::vector<size_t> firstCodonPositions;
  for (const auto& codon : codonMap.at(protein.front())) {
    addOtherVec(firstCodonPositions, findOccurences(seq, codon));
  }
  for (const auto& c : protein) {
    if (c == protein.front()) {
      continue;
    }
    std::vector<size_t> currentAminoAcid;
    for (const auto& codon : codonMap.at(c)) {
      addOtherVec(currentAminoAcid, findOccurences(seq, codon));
    }
    std::unordered_map<size_t, size_t> currentPositons;
    for (const auto& pos : currentAminoAcid) {
      currentPositons[pos] = pos;
    }
    positions.push_back(currentPositons);
  }
  std::vector<std::vector<size_t>> possiblePositions;
  for (const auto& firstPos : firstCodonPositions) {
    if (positions.back().find(firstPos + 3 * positions.size()) ==
        positions.back().end()) {
      continue;
    }
    std::vector<size_t> currentPositions;
    currentPositions.push_back(firstPos);
    bool madeItToTheEnd = true;
    size_t count = 1;
    for (const auto& codonPositions : positions) {
      if (codonPositions.find(firstPos + count * 3) != codonPositions.end()) {
        currentPositions.push_back(firstPos + count * 3);
      } else {
        madeItToTheEnd = false;
        break;
      }
      ++count;
    }

    if (madeItToTheEnd) {
      possiblePositions.push_back(currentPositions);
    }
  }
  for (const auto& possible : possiblePositions) {
    ans.emplace_back(getStringFromSubstrings(seq, possible, 3));
  }
  return ans;
}
VecStr findPossibleDNA(const std::string& seq, const std::string& protein,
                       const std::string& seqType, bool checkComplement) {
  VecStr ans = getPossibleDNASubstringsForProtein(seq, protein, seqType);
  if (checkComplement) {
    std::string complementSeq = seqUtil::reverseComplement(seq, "DNA");
    VecStr complementAns =
        getPossibleDNASubstringsForProtein(complementSeq, protein, seqType);
    for (auto& str : complementAns) {
      str = seqUtil::reverseComplement(str, "DNA");
    }
    addOtherVec(ans, complementAns);
  }
  return ans;
}
VecStr getAllCycloProteinFragments(const std::string& protein) {
  VecStr ans;
  for (auto i : iter::range(1, (int)protein.length())) {
    for (auto j : iter::range(protein.length())) {
      std::string currentFragment = "";
      if (j + i > protein.size()) {
        currentFragment = protein.substr(j, protein.size() - j) +
                          protein.substr(0, i - (protein.size() - j));
      } else {
        currentFragment = protein.substr(j, i);
      }
      ans.push_back(currentFragment);
    }
  }
  ans.push_back(protein);
  return ans;
}

VecStr getAllLinearProteinFragments(const std::string& protein) {
  VecStr ans;
  ans.push_back("");
  for (auto i : iter::range(1, (int)protein.length())) {
    for (auto j : iter::range(protein.length())) {
      std::string currentFragment = "";
      if (j + i > protein.size()) {

      } else {
        currentFragment = protein.substr(j, i);
      }
      ans.push_back(currentFragment);
    }
  }
  ans.push_back(protein);
  return ans;
}
std::multimap<int, std::string> getProteinFragmentSpectrum(
    const std::string& protein) {
  std::multimap<int, std::string, std::less<int>> ans;
  ans.insert({0, ""});
  VecStr fragments = getAllCycloProteinFragments(protein);
  for (const auto frag : fragments) {
    ans.insert({seqUtil::calculateWeightOfProteinInt(frag), frag});
  }
  return ans;
}

std::vector<int> getRealPossibleWeights(const std::vector<int>& spectrum) {
  std::vector<int> possibleWeights;
  for (auto i : spectrum) {
    if (i < 187) {
      possibleWeights.push_back(i);
    }
  }
  removeDuplicates(possibleWeights);
  // possibleWeights = getUnique(possibleWeights);
  std::vector<int> realPossibleWeights;
  for (const auto& pw : possibleWeights) {
    if (aminoAcidInfo::infos::weightIntToAminoAcid.find(pw) !=
        aminoAcidInfo::infos::weightIntToAminoAcid.end()) {
      realPossibleWeights.push_back(pw);
    }
  }
  return realPossibleWeights;
}


VecStr organizeLexicallyKmers(const VecStr& input, int colNum) {
  VecStr secondInputAsNumbers;

  MapStrStr stone;
  char count = 'A';
  for (const auto& sIter : input) {
    stone[std::string(1, count)] = sIter;
    secondInputAsNumbers.push_back(std::string(1, count));
    ++count;
  }
  uint64_t reserveSize = 0;
  for (auto i = 1; i <= colNum; ++i) {
    reserveSize += Factorial(i);
  }
  std::vector<VecStr> secondAns;
  secondAns.reserve(reserveSize);

  for (auto i = 1; i <= colNum; ++i) {
    auto temp = permuteVector(secondInputAsNumbers, i);
    secondAns.insert(secondAns.end(), temp.begin(), temp.end());
  }
  VecStr thirdAns;
  for (const auto& iter : secondAns) {
    thirdAns.push_back(vectorToString(iter, ""));
  }
  std::sort(thirdAns.begin(), thirdAns.end());
  for (auto& iter : thirdAns) {
    translateStringWithKey(iter, stone);
  }
  return thirdAns;
}
// doesn't actually work don't use
VecStr organizeLexicallyKmers(const std::string& input, size_t colNum) {
  std::string transformedString = "";

  MapStrStr stone;
  char count = 'A';
  for (const auto& c : input) {
    stone[std::string(1, count)] = c;
    transformedString.push_back(count);
    ++count;
  }
  uint64_t reserveSize = 0;
  for (size_t i = 1; i <= colNum; ++i) {
    reserveSize += Factorial(i);
  }
  VecStr secondAns;
  secondAns.reserve(reserveSize);
  VecStr fragments = getAllCycloProteinFragments(transformedString);
  VecStr trimmedFragments;
  for (const auto& frag : fragments) {
    if (frag.size() <= colNum) {
      trimmedFragments.push_back(frag);
    }
  }
  std::sort(trimmedFragments.begin(), trimmedFragments.end());
  printVector(trimmedFragments);
  for (const auto& frag : trimmedFragments) {
    auto temp = fastPermuteVectorOneLength(frag);
    printVector(temp);
    addOtherVec(secondAns, temp);
  }
  std::sort(secondAns.begin(), secondAns.end());
  for (auto& iter : secondAns) {
    translateStringWithKey(iter, stone);
  }
  return secondAns;
  /*
   exit(1);
   for (auto i = 1; i <= colNum; ++i) {
   auto temp = permuteVector(secondInputAsNumbers, i);
   secondAns.insert(secondAns.end(), temp.begin(), temp.end());
   }
   VecStr thirdAns;
   for (const auto& iter : secondAns) {
   thirdAns.push_back(vectorToString(iter, ""));
   }
   std::sort(thirdAns.begin(), thirdAns.end());

   return thirdAns;*/
}
uint64_t smallestSizePossible(uint64_t weight) {
  return std::floor(weight / 186.00);
}
int64_t getPossibleNumberOfProteins(
    int64_t weight, std::unordered_map<int64_t, int64_t>& cache) {
  if (0 == weight) {
    return 1;
  }
  if (0 > weight) {
    return 0;
  }
  int64_t count = 0;
  for (auto w : aminoAcidInfo::infos::weightIntToAminoAcid) {
    auto q = weight - w.first;
    // std::cout << q << std::endl;
    if (cache.find(q) == cache.end()) {
      auto t = getPossibleNumberOfProteins(q, cache);
      cache[q] = t;
    }
    count += cache[q];
  }
  return count;
}

probabilityProfile randomlyFindBestProfile(const VecStr& dnaStrings,
                                           const std::vector<VecStr>& allKmers,
                                           int numberOfKmers,
                                           randomGenerator& gen) {
  VecStr randomMers;
  // bool needToSeed=true;
  std::vector<int> randomKmersNums =
      gen.unifRandVector(0, numberOfKmers, dnaStrings.size());
  uint32_t pos = 0;
  for (const auto& i : randomKmersNums) {
    randomMers.push_back(allKmers[pos][i]);
    ++pos;
  }
  probabilityProfile bestProfile(randomMers);
  while (true) {
    VecStr currentMers;
    for (const auto& dString : dnaStrings) {
      currentMers.push_back(bestProfile.mostProbableKmers(dString)[0].k_);
    }
    probabilityProfile currentProfile(currentMers);
    if (currentProfile.score_ < bestProfile.score_) {
      bestProfile = currentProfile;
    } else {
      return bestProfile;
    }
  }
  return bestProfile;
}

probabilityProfile randomMotifSearch(const VecStr& dnaStrings, int kLength,
                                     int numberOfRuns, bool gibs, int gibsRuns,
                                     randomGenerator& gen) {
  std::vector<VecStr> allKmers;
  int numOfKmers = 0;
  for (const auto& dString : dnaStrings) {
    allKmers.push_back(kmerCalculator::getAllKmers(dString, kLength));
    numOfKmers = (int)allKmers.back().size();
  }
  probabilityProfile bestProfile =
      randomlyFindBestProfile(dnaStrings, allKmers, numOfKmers, gen);
  for (int i = 0; i < numberOfRuns; ++i) {
    if (gibs) {
    	probabilityProfile currentProfile = randomlyFindBestProfileGibs(dnaStrings, allKmers,
                                                   numOfKmers, gibsRuns, gen);
      if (currentProfile.score_ < bestProfile.score_) {
        bestProfile = currentProfile;
      }
    } else {
    	probabilityProfile currentProfile =
          randomlyFindBestProfile(dnaStrings, allKmers, numOfKmers, gen);
      if (currentProfile.score_ < bestProfile.score_) {
        bestProfile = currentProfile;
      }
    }

  }
  return bestProfile;
}
probabilityProfile randomlyFindBestProfileGibs(
    const VecStr& dnaStrings, const std::vector<VecStr>& allKmers,
    int numberOfKmers, int runTimes, randomGenerator& gen) {
  VecStr randomMers;
  std::vector<int> randomKmersNums =
      gen.unifRandVector(0, numberOfKmers, dnaStrings.size());
  uint32_t pos = 0;
  for (const auto& i : randomKmersNums) {
    randomMers.push_back(allKmers[pos][i]);
    ++pos;
  }
  probabilityProfile bestProfile(randomMers);
  // run random selection j times
  for (int j = 0; j < runTimes; ++j) {
    // pick random kmer to replace and get new profile without that kmer
    size_t randomStringNum =
        gen.unifRand(0, (int)bestProfile.dnaStrings_.size());
    VecStr currentMotifs;
    for (auto i : iter::range(bestProfile.dnaStrings_.size())) {
      if (i != randomStringNum) {
        currentMotifs.push_back(bestProfile.dnaStrings_[i]);
      }
    }
    probabilityProfile currentProfile(currentMotifs);
    // now select a new kmer by preferentially picking a more probable kmer but
    // still randomly
    std::multimap<double, std::string> kmersByProb;
    double cumProb = 0.0;
    for (const auto& ks : allKmers[randomStringNum]) {
      double currentProb = currentProfile.getProbabilityOfKmer(ks);
      kmersByProb.insert({currentProb, ks});
      cumProb += currentProb;
    }
    double randomProb = gen.unifRand(0.0, cumProb);
    double sumOfProbs = 0;
    std::string randomKmer = "";
    for (const auto& kByProb : kmersByProb) {
      sumOfProbs += kByProb.first;
      if (sumOfProbs > randomProb) {
        currentProfile.add(kByProb.second);
        currentProfile.updateScore();
        randomKmer = kByProb.second;
        break;
      }
    }
    // if new randomly replaced kmer creates a better profile repalce
    // best profile but keep order of strings so that they are correctly
    // replaced latter
    if (currentProfile.score_ < bestProfile.score_) {
      bestProfile.dnaStrings_[randomStringNum] = randomKmer;
      bestProfile = probabilityProfile(bestProfile.dnaStrings_);
    }
  }
  return bestProfile;
}
std::vector<std::vector<int>> growNextCycle(
    const std::vector<std::vector<int>>& previousCycle,
    const std::vector<int>& possibleWeights) {
  std::vector<std::vector<int>> nextCycle;
  nextCycle.reserve(previousCycle.size() * possibleWeights.size());
  for (const auto& pw : possibleWeights) {
    for (auto cycle : previousCycle) {
      cycle.push_back(pw);
      nextCycle.push_back(cycle);
    }
  }
  return nextCycle;
}

bool trimCycle(std::vector<std::vector<int>>& nextCycle,
               std::vector<std::vector<int>>& matchesSpectrum,
               const std::map<int, int>& spectrumToWeights,
               const std::vector<int> specVec) {
  int pos = (int)nextCycle.size();
  std::vector<int> positions;
  for (const auto& cycle : iter::reverse(nextCycle)) {
    --pos;
    auto currentFragments = getAllVecLinearFragments(cycle);
    std::map<int, int> fragmentWeightCounts;
    for (const auto& fragment : currentFragments) {
      auto currentSum = vectorSum(fragment);
      ++fragmentWeightCounts[currentSum];
    }
    bool faulted = false;
    for (const auto& weight : fragmentWeightCounts) {
      if (spectrumToWeights.find(weight.first) == spectrumToWeights.end() ||
          spectrumToWeights.at(weight.first) < weight.second) {
        nextCycle.erase(nextCycle.begin() + pos);
        positions.push_back(pos);
        faulted = true;
        break;
      }
    }
    if (faulted) {
      continue;
    }
    std::vector<int> theoSpectrum;
    theoSpectrum.reserve(currentFragments.size());
    auto currentCycloFragments = getAllVecCycloFragments(cycle);
    for (const auto& fragment : currentCycloFragments) {
      auto currentSum = vectorSum(fragment);
      theoSpectrum.push_back(currentSum);
    }
    std::sort(theoSpectrum.begin(), theoSpectrum.end());
    if (theoSpectrum == specVec) {
      matchesSpectrum.push_back(cycle);
      nextCycle.erase(nextCycle.begin() + pos);
    }
  }
  return !nextCycle.empty();
}

std::vector<std::vector<int>> getPossibleProteinsForSpectrum(
    const std::string& spectrum, bool useAllWeights) {
  std::vector<int> specVec = stringToVector<int>(spectrum);
  std::map<int, int> spectrumToWeights;
  for (const auto& i : specVec) {
    ++spectrumToWeights[i];
  }
  std::vector<int> possibleWeights;
  if (useAllWeights) {
    possibleWeights = getVectorOfMapKeys(aminoAcidInfo::infos::weightIntToAminoAcid);
    // for(const auto & weight : )
  } else {
    possibleWeights = getRealPossibleWeights(specVec);
  }

  std::vector<std::vector<int>> initialCycle;
  for (auto pw : possibleWeights) {
    initialCycle.emplace_back(std::vector<int>(1, pw));
  }
  bool keepGrowing = true;
  std::vector<std::vector<int>> matchesSpectrum;
  int count = 0;
  while (keepGrowing) {
    initialCycle = growNextCycle(initialCycle, possibleWeights);
    keepGrowing =
        trimCycle(initialCycle, matchesSpectrum, spectrumToWeights, specVec);
    std::sort(initialCycle.begin(), initialCycle.end());
    ++count;
  }
  return matchesSpectrum;
}

int scoreSpectrumAgreement(const std::map<int, int>& spectrum,
                           const std::map<int, int>& currentSpectrum) {
  int ans = 0;
  for (const auto& weight : currentSpectrum) {
    // std::cout<<"Weight: "<<weight.first<<" count:
    // "<<weight.second<<std::endl;
    if (spectrum.find(weight.first) == spectrum.end()) {

    } else if (spectrum.at(weight.first) <= weight.second) {
      ans += spectrum.at(weight.first);
      // std::cout<<"ansFirstOption: "<<ans<<std::endl;
    } else if (spectrum.at(weight.first) > weight.second) {
      ans += weight.second;
      // std::cout<<"ansSecondOption: "<<ans<<std::endl;
    } else {
      // this shouldn't happen
      std::cout << "ERROR!!" << std::endl;
    }
  }
  return ans;
}

std::multimap<int, std::vector<int>, std::greater<int>> growNextCycleScore(
    const std::multimap<int, std::vector<int>, std::greater<int>>&
        previousCycle,
    const std::vector<int>& possibleWeights,
    const std::map<int, int>& spectrumCounts, int parentMass, bool linear) {
  std::multimap<int, std::vector<int>, std::greater<int>> nextCycle;
  // nextCycle.reserve(previousCycle.size()*possibleWeights.size());
  for (const auto& pw : possibleWeights) {
    for (auto cycle : previousCycle) {
      cycle.second.push_back(pw);
      std::vector<std::vector<int>> currentFragments;
      if (linear) {
        currentFragments = getAllVecLinearFragments(cycle.second);
      } else {
        currentFragments = getAllVecCycloFragments(cycle.second);
      }
      std::map<int, int> fragmentWeightCounts;
      bool tooBig = false;
      // std::cout<<std::endl;
      for (const auto& fragment : currentFragments) {
        // std::cout<<"fragment: ";printVector(fragment);
        auto currentSum = vectorSum(fragment);
        // std::cout<<"Current sum: "<<currentSum<<std::endl;
        if (currentSum > parentMass) {
          tooBig = true;
        }
        ++fragmentWeightCounts[currentSum];
      }
      if (!tooBig) {
        int currentScore =
            scoreSpectrumAgreement(spectrumCounts, fragmentWeightCounts);
        // std::cout<<"currentScore: "<<currentScore<<std::endl;
        nextCycle.insert({currentScore, cycle.second});
      }
      // exit(1);
    }
  }
  return nextCycle;
}

std::multimap<int, std::vector<int>, std::greater<int>> trimCycleScore(
    std::multimap<int, std::vector<int>, std::greater<int>>& nextCycle,
    std::multimap<int, std::vector<int>, std::greater<int>>& matchesSpectrum,
    int parentMass, int leaderBoardNumber, int& currentLeader) {
  std::multimap<int, std::vector<int>, std::greater<int>> ans;
  int pos = 0;
  int lastScore;
  for (const auto& cycle : nextCycle) {
    if (pos == 0) {
      lastScore = cycle.first;
    }
    ++pos;
    if (pos > leaderBoardNumber && cycle.first != lastScore) {

    } else {
      int mass = vectorSum(cycle.second);
      if (mass == parentMass && cycle.first == currentLeader) {
        matchesSpectrum.insert(cycle);
      } else if (mass == parentMass && cycle.first >= currentLeader) {
        matchesSpectrum.clear();
        matchesSpectrum.insert(cycle);
        currentLeader = cycle.first;
      }
      lastScore = cycle.first;
      ans.insert(cycle);
    }
  }
  return ans;
}
std::multimap<int, std::vector<int>, std::greater<int>>
getPossibleProteinsForSpectrum(const std::string& spectrum,
                               int leaderBoardNumber, bool verbose,
                               bool useAllWeights, bool convolution,
                               int convolutionCutOff, bool linear) {
  std::vector<int> specVec = stringToVector<int>(spectrum);
  std::sort(specVec.begin(), specVec.end());
  int parentMass = specVec.back();
  std::map<int, int> spectrumToWeights;
  for (const auto& i : specVec) {
    ++spectrumToWeights[i];
  }
  /*
   for(const auto & specW : spectrumToWeights){
   std::cout<<specW.first<< ":"<<specW.second<<std::endl;
   }*/
  std::vector<int> possibleWeights;
  if (convolution) {
    possibleWeights = topConvolutionWeights(specVec, convolutionCutOff);
    std::cout << "convolutionCutOff: " << convolutionCutOff << std::endl;
    std::cout << "convolutionPossibleWeights: ";
    printVector(possibleWeights);
  } else if (useAllWeights) {
    possibleWeights = std::vector<int>(144);
    std::iota(possibleWeights.begin(), possibleWeights.end(), 57);
  } else {
    possibleWeights = getVectorOfMapKeys(aminoAcidInfo::infos::weightIntToAminoAcid);
  }

  std::multimap<int, std::vector<int>, std::greater<int>> initialCycle;
  for (auto pw : possibleWeights) {
    initialCycle.insert({0, std::vector<int>(1, pw)});
  }
  bool keepGrowing = true;
  std::multimap<int, std::vector<int>, std::greater<int>> matchesSpectrum;
  int currentLeader = 0;
  int count = 0;
  while (keepGrowing) {
    std::cout << "on cycle " << count << std::endl;
    initialCycle = growNextCycleScore(initialCycle, possibleWeights,
                                      spectrumToWeights, parentMass, linear);
    if (verbose) {
      for (const auto& cycle : initialCycle) {
        std::cout << cycle.first << " " << vectorToString(cycle.second)
                  << std::endl;
      }
    }
    initialCycle = trimCycleScore(initialCycle, matchesSpectrum, parentMass,
                                  leaderBoardNumber, currentLeader);
    if (initialCycle.empty()) {
      keepGrowing = false;
    }
    /*if (initialCycle.empty() || !matchesSpectrum.empty()) {
      keepGrowing=false;
    }*/
    ++count;
  }
  return matchesSpectrum;
}
std::map<int, int> getConvolutionWeights(std::vector<int> experimentalSpectrum,
                                         int multiplicityCutOff, int lowerBound,
                                         int upperBound) {
  std::sort(experimentalSpectrum.begin(), experimentalSpectrum.end());
  std::map<int, int, std::greater<int>> countsOfDifferences;
  for (const auto& i : iter::range(experimentalSpectrum.size())) {
    for (const auto& j : iter::range(i + 1, experimentalSpectrum.size())) {
      int currentDifference = experimentalSpectrum[j] - experimentalSpectrum[i];
      if (currentDifference >= lowerBound && currentDifference <= upperBound) {
        ++countsOfDifferences[currentDifference];
      }
    }
  }
  std::map<int, int> ans;
  for (const auto& count : countsOfDifferences) {
    if (count.second >= multiplicityCutOff) {
      ans.insert(count);
    }
  }
  return ans;
}
std::vector<int> convolutionWeights(std::vector<int> experimentalSpectrum,
                                    int multiplicityCutOff, int lowerBound,
                                    int upperBound) {
  std::map<int, int> counts = getConvolutionWeights(
      experimentalSpectrum, multiplicityCutOff, lowerBound, upperBound);
  return getVectorOfMapKeys(counts);
}
std::vector<int> topConvolutionWeights(std::vector<int> experimentalSpectrum,
                                       int mItems, int lowerBound,
                                       int upperBound) {
  std::map<int, int> allCounts =
      getConvolutionWeights(experimentalSpectrum, 1, lowerBound, upperBound);
  std::multimap<int, int, std::greater<int>> byCounts;
  for (const auto& count : allCounts) {
    byCounts.insert({count.second, count.first});
  }
  int counting = 0;
  int lastCount = byCounts.begin()->first;
  // int testCount=0;
  /*for(const auto & count : byCounts){
    testCount++;
    std::cout<<"testCount: "<<testCount<<" currentCount: "<<count.first<<"
  weight: "<<count.second<<std::endl;
  }*/
  for (const auto& count : byCounts) {
    ++counting;
    if (counting > mItems && count.first != lastCount) {
      break;
    }
    lastCount = count.first;
  }
  // std::cout<<"lastcount: "<<lastCount<<std::endl;
  return convolutionWeights(experimentalSpectrum, lastCount, lowerBound,
                            upperBound);
}
int64_t getMinCoins(int64_t change, const std::vector<int64_t>& coins,
                    std::unordered_map<int64_t, int64_t>& cache) {
  if (0 == change) {
    return 0;
  }
  int64_t count = change;
  for (auto coin : coins) {
    if (change - coin < 0) {

    } else {
      int64_t q = change - coin;
      if (cache.find(q) == cache.end()) {
        auto t = getMinCoins(q, coins, cache) + 1;
        cache[q] = t;
      }
      if (cache[q] < count) {
        count = cache[q];
      }
    }
  }
  return count;
}

void processAlnInfoInput(aligner& alignerObj,
		const std::string& alnInfoDirName) {
	if (alnInfoDirName != "") {
		//std::cout << bib::bashCT::boldRed("here") << std::endl;
		//std::lock_guard<std::mutex> lock(alignment::alnCacheDirSearchLock);
		//std::cout << bib::bashCT::boldRed("here1") << std::endl;
		alignment::alnCacheDirSearchLock.lock();

		auto fullPath = bib::files::normalize(alnInfoDirName).string();
		auto search = alignment::alnCacheDirLocks.find(fullPath);
		if (search == alignment::alnCacheDirLocks.end()) {
			alignment::alnCacheDirLocks.emplace(std::piecewise_construct,
					std::make_tuple(fullPath), std::tuple<>());
		}
		auto realSearch = alignment::alnCacheDirLocks.find(fullPath);
		std::lock_guard<std::mutex> lock(realSearch->second);
		alignment::alnCacheDirSearchLock.unlock();
		//std::cout << bib::bashCT::boldRed("here1.1") << std::endl;
		int directoryStatus = mkdir(alnInfoDirName.c_str(),
		S_IRWXU | S_IRWXG | S_IRWXO);
		//std::cout << bib::bashCT::boldRed("here1.2") << std::endl;
		if (directoryStatus != 0) {
			//std::cout << bib::bashCT::boldRed("here1.3") << std::endl;
			auto readInHoler = alnInfoMasterHolder(alnInfoDirName,
					alignerObj.parts_.gapScores_, alignerObj.parts_.scoring_, false);
			//std::cout << bib::bashCT::boldRed("here1.4") << std::endl;
			//add local alignments
			//std::cout << "Read in previous alns infos " << std::endl;
			//std::cout << bib::bashCT::boldRed("here1.5") << std::endl;
			for (const auto & lHolder : readInHoler.localHolder_) {
				//std::cout << lHolder.first << std::endl;
				alignerObj.alnHolder_.localHolder_[lHolder.first] = lHolder.second;
			}
			//std::cout << bib::bashCT::boldRed("here1.6") << std::endl;
			//add global alignments
			for (const auto & gHolder : readInHoler.globalHolder_) {
				//std::cout << gHolder.first << std::endl;
				alignerObj.alnHolder_.globalHolder_[gHolder.first] = gHolder.second;
			}
			//std::cout << bib::bashCT::boldRed("here1.7") << std::endl;
			//alignerObj.alnHolders_ = alnInfoMasterHolder(alnInfoDirName);
		} else {
			//std::cout << bib::bashCT::boldRed("here1.8") << std::endl;
			chmod(alnInfoDirName.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}
	}
	//std::cout << bib::bashCT::boldRed("here3") << std::endl;
}




std::vector<uint32_t> getWindowQuals(const std::vector<uint32_t>& qual,
                                     uint32_t qWindowSize, uint32_t pos) {
  uint32_t lowerBound = 0;
  uint32_t higherBound = qual.size();
  if (pos > qWindowSize) {
    lowerBound = pos - qWindowSize;
  }
  if (pos + qWindowSize + 1 < higherBound) {
    higherBound = pos + qWindowSize + 1;
  }
  return getSubVector(qual, lowerBound, higherBound - lowerBound);
}
std::vector<double> likelihoodForBaseQ(
    const std::vector<uint32_t>& qual,
    std::unordered_map<double, double>& likelihoods) {
  std::vector<double> ans;
  for (const auto& i : iter::range(qual.size())) {
    ans.emplace_back(likelihoods.at(qual[i]));
  }
  return ans;
}
std::vector<double> likelihoodForMeanQ(
    const std::vector<uint32_t>& qual, uint32_t qWindowSize,
    std::unordered_map<double, double>& likelihoods) {
  std::vector<double> ans;
  for (const auto& i : iter::range(qual.size())) {
    double currentMean = vectorMean(getWindowQuals(qual, qWindowSize, i));
    ans.emplace_back(likelihoods.at(roundDecPlaces(currentMean, 2)));
  }
  return ans;
}
std::vector<double> likelihoodForMedianQ(
    const std::vector<uint32_t>& qual, uint32_t qWindowSize,
    std::unordered_map<double, double>& likelihoods) {
  std::vector<double> ans;
  for (const auto& i : iter::range(qual.size())) {
    double currentMedian = vectorMedian(getWindowQuals(qual, qWindowSize, i));
    ans.emplace_back(likelihoods.at(roundDecPlaces(currentMedian, 2)));
  }
  return ans;
}
std::vector<double> likelihoodForMinQ(
    const std::vector<uint32_t>& qual, uint32_t qWindowSize,
    std::unordered_map<double, double>& likelihoods) {
  std::vector<double> ans;
  for (const auto& i : iter::range(qual.size())) {
    double currentMin = vectorMinimum(getWindowQuals(qual, qWindowSize, i));
    ans.emplace_back(likelihoods.at(roundDecPlaces(currentMin, 2)));
  }
  return ans;
}
double getChangeInHydro(const char& firstAA, const char& secondAA) {
  return std::abs(aminoAcidInfo::infos::allInfo.at(firstAA).acidHydrophobicity_ -
                  aminoAcidInfo::infos::allInfo.at(secondAA).acidHydrophobicity_);
}
cluster createClusterFromSimiliarReads(std::vector<readObject> similiarReads,
                                       aligner& alignerObj) {
  if (similiarReads.size() > 0) {
    sort(similiarReads);
    cluster output(similiarReads.front());
    for (const auto& readPos : iter::range<uint32_t>(1, similiarReads.size())) {
      output.addRead(similiarReads[readPos]);
    }
    output.calculateConsensus(alignerObj, true);
    return output;
  } else {
    return cluster();
  }
}
/*
std::vector<double> getHydroChanges(const std::string& originalCodon,
                                    const VecStr& mutantCodons,
                                    const std::map<std::string, char>& code) {
  std::vector<double> ans;
  if (originalCodon.size() != 3) {
    std::cout << "codon needs to be 3 bases" << std::endl;
    std::cout << originalCodon << std::endl;
    std::cout << originalCodon.size() << std::endl;
    return ans;
  }
  char originalAA = code.at(originalCodon);
  if (originalAA == '*') {
    return ans;
  }
  for (const auto& codon : mutantCodons) {
    char mutantAA = code.at(codon);
    if (mutantAA == '*') {
      continue;
    }
    ans.emplace_back(getChangeInHydro(originalAA, mutantAA));
  }
  return ans;
}*/


VecStr createDegenStrs(const std::string & str){
	VecStr ans;

	char first = *str.begin();
	std::cout << first << std::endl;
	std::cout << std::endl;
	if(degenBaseExapnd.find(first) != degenBaseExapnd.end()){
		for(const auto & nextChar : degenBaseExapnd.at(first)){
			ans.emplace_back(std::string(1,nextChar));
		}
	}else{
		ans.emplace_back(std::string(1,first));
	}

	printVector(ans);
	uint32_t charCount = 0;
	for(const auto & c : str){
		++charCount;
		if(charCount == 1){
			continue;
		}
		if(degenBaseExapnd.find(c) != degenBaseExapnd.end()){
			VecStr adding;
			for(auto & current : ans){
				uint32_t count = 0;
				std::string copy = current;
				for(const auto & nextChar : degenBaseExapnd.at(c)){
					if(count == 0){
						current.push_back(nextChar);
					}else{
						adding.emplace_back(copy);
						adding.back().push_back(nextChar);
					}
					++count;
				}
			}
			addOtherVec(ans,adding);
		}else{
			for(auto & current : ans){
				current.push_back(c);
			}
		}
	}
	return ans;
}

table getErrorFractionsCoded(const table & errorTab){
	//error tab should have column names readName\trefName\treadCnt\tinsertions\tdeletions\tmismatchs
	std::vector<double> readCounts = vecStrToVecNum<double>(errorTab.getColumn("readCnt"));
	double readTotal = vectorSum(readCounts);
	//goes insert, deletion, mismatch
	std::map<std::tuple<uint32_t, uint32_t, uint32_t>, double> errorMap;
	for(const auto & row : errorTab.content_){
		uint32_t currentIn = std::stoul(row[3]);
		uint32_t currentDel = std::stoul(row[4]);
		uint32_t currentMis = std::stoul(row[5]);
		uint32_t inCode = 0;
		uint32_t delCode = 0;
		uint32_t misCode = 0;
		if(currentIn > 0){
			inCode = 1;
		}
		if(currentDel > 0){
			delCode = 1;
		}
		if(currentMis > 0){
			misCode = 1;
		}
		double currentRCnt = std::stod(row[2]);
		errorMap[std::make_tuple(inCode, delCode, misCode)] += currentRCnt;
	}
	table outErrorTabTest(VecStr{"insertion", "deletion", "mismatch", "cnt"});
	for(const auto & currentError : errorMap){
		outErrorTabTest.content_.emplace_back(VecStr{to_string(std::get<0>(currentError.first)),
			to_string(std::get<1>(currentError.first)), to_string(std::get<2>(currentError.first)),
		to_string(currentError.second/readTotal)});
	}
	return outErrorTabTest;
}
table getErrorFractions(const table & errorTab){
	//error tab should have column names readName\trefName\treadCnt\tinsertions\tdeletions\tmismatchs
	std::vector<double> readCounts = vecStrToVecNum<double>(errorTab.getColumn("readCnt"));
	double readTotal = vectorSum(readCounts);
	double insertionsCnt = 0;
	double deletionsCnt = 0;
	double misCnt = 0;
	double onlyIndelCnt = 0;
	double bothDelInCnt = 0;
	double onlyMismatch = 0;
	double mismatchIndelCnt = 0;
	double allThree = 0;
	double errorFree = 0;
	for(const auto & row : errorTab.content_){
		uint32_t currentIn = std::stoul(row[3]);
		uint32_t currentDel = std::stoul(row[4]);
		uint32_t currentMis = std::stoul(row[5]);
		double currentRCnt = std::stod(row[2]);

		if(currentIn> 0 ){
			insertionsCnt += currentRCnt;
		}
		if(currentDel > 0 ){
			deletionsCnt += currentRCnt;
		}
		if(currentMis > 0 ){
			misCnt += currentRCnt;
		}
		if((currentIn + currentDel) > 0 && currentMis == 0){
			onlyIndelCnt += currentRCnt;
		}
		if(currentIn > 0  && currentDel > 0){
			bothDelInCnt += currentRCnt;
		}
		if(currentIn == 0  && currentDel == 0 && currentMis > 0){
			onlyMismatch += currentRCnt;
		}
		if((currentIn + currentDel) > 0 && currentMis > 0){
			mismatchIndelCnt += currentRCnt;
		}
		if(currentIn > 0  && currentDel > 0 && currentMis > 0){
			allThree += currentRCnt;
		}
		if(currentIn == 0  && currentDel == 0 && currentMis == 0){
			errorFree += currentRCnt;
		}
	}
	table outErrorTab(VecStr{"errorFree", "insertions", "deletions", "mismatches",
		"onlyIndel", "onlyMismatch", "mismatchIndel", "deletionAndInsert", "allThree"});
	outErrorTab.content_.emplace_back(numVecToVecStr(std::vector<double>{errorFree/readTotal,
		insertionsCnt/readTotal, deletionsCnt/readTotal, misCnt/readTotal, onlyIndelCnt/readTotal,
	onlyMismatch/readTotal, mismatchIndelCnt/readTotal, bothDelInCnt/readTotal, allThree/readTotal}));
	return outErrorTab;
}
table getIndelSizeStats(const table & indelTab){
	// indel tab should have columnname s readName\trefName\treadCnt\tindel\tsize
	//std::vector<double> readCounts = vecStrToVecNum<double>(indelTab.getColumn("readCnt"));
	//double readTotal = vectorSum(readCounts);
	std::vector<uint32_t> insertSizes;
	std::vector<uint32_t> delSizes;
	for(const auto & row : indelTab.content_){
		if(row[3] == "insertion"){
			addOtherVec(insertSizes, std::vector<uint32_t>(std::stoi(row[2]),std::stoi(row[4])));
		}else if (row[3] == "deletion"){
			addOtherVec(delSizes, std::vector<uint32_t>(std::stoi(row[2]),std::stoi(row[4])));
		}else{
			std::cout << " getAverageIndelSize " << std::endl;
			std::cout << "this should not be happening" << std::endl;
		}
	}
	auto insertStats = getStatsOnVec(insertSizes);
	auto delStats = getStatsOnVec(delSizes);
	table indelSizeStatsTab(VecStr{"indel", "minSize", "maxSize", "meanSize", "medianSize"});
	indelSizeStatsTab.content_.emplace_back(VecStr{"insertion", to_string(insertStats["min"]),
		to_string(insertStats["max"]), to_string(insertStats["mean"]), to_string(insertStats["median"])});
	indelSizeStatsTab.content_.emplace_back(VecStr{"deletion", to_string(delStats["min"]),
		to_string(delStats["max"]), to_string(delStats["mean"]), to_string(delStats["median"])});
	return indelSizeStatsTab;
}
table getErrorDist(const table & errorTab){
	//error tab should have column names readName\trefName\treadCnt\tinsertions\tdeletions\tmismatchs
	std::unordered_map<uint32_t, uint32_t> insertCounts;
	std::unordered_map<uint32_t, uint32_t> delCounts;
	std::unordered_map<uint32_t, uint32_t> misCounts;
	for(const auto & row : errorTab.content_){
		uint32_t currentIn = std::stoul(row[3]);
		uint32_t currentDel = std::stoul(row[4]);
		uint32_t currentMis = std::stoul(row[5]);
		if(currentIn> 0 ){
			insertCounts[currentIn] += std::stoi(row[2]);
		}
		if(currentDel > 0 ){
			delCounts[currentDel] += std::stoi(row[2]);
		}
		if(currentMis > 0 ){
			misCounts[currentMis] += std::stoi(row[2]);
		}
	}

	table insertTab(insertCounts, VecStr{"errorNum", "count"});
	insertTab.addColumn(VecStr{"insertion"}, "error");

	table delTab(delCounts, VecStr{"errorNum", "count"});
	delTab.addColumn(VecStr{"deletion"}, "error");

	table misTab(misCounts, VecStr{"errorNum", "count"});
	misTab.addColumn(VecStr{"mismatch"}, "error");

	delTab.rbind(insertTab);
	delTab.rbind(misTab);
	delTab.sortTable("errorNum", true);
	delTab.sortTable("error", true);
	return delTab;
}
table getErrorStats(const table & errorTab){
	//error tab should have column names readName\trefName\treadCnt\tinsertions\tdeletions\tmismatchs
	std::vector<uint32_t> misNum;
	std::vector<uint32_t> insertNums;
	std::vector<uint32_t> delNums;
	for(const auto & row : errorTab.content_){
		uint32_t currentIn = std::stoul(row[3]);
		uint32_t currentDel = std::stoul(row[4]);
		uint32_t currentMis = std::stoul(row[5]);
		if(currentIn> 0 ){
			addOtherVec(insertNums, std::vector<uint32_t>(std::stoi(row[2]), currentIn));
		}
		if(currentDel > 0 ){
			addOtherVec(delNums, std::vector<uint32_t>(std::stoi(row[2]), currentDel));
		}
		if(currentMis > 0 ){
			addOtherVec(misNum, std::vector<uint32_t>(std::stoi(row[2]), currentMis));
		}
	}
	auto insertStats = getStatsOnVec(insertNums);
	auto delStats = getStatsOnVec(delNums);
	auto misStats = getStatsOnVec(misNum);
	table errorNumStatsTab(VecStr{"error", "minNum", "maxNum", "meanNum", "medianNum"});
	errorNumStatsTab.content_.emplace_back(VecStr{"insertion", to_string(insertStats["min"]),
		to_string(insertStats["max"]), to_string(insertStats["mean"]), to_string(insertStats["median"])});
	errorNumStatsTab.content_.emplace_back(VecStr{"deletion", to_string(delStats["min"]),
		to_string(delStats["max"]), to_string(delStats["mean"]), to_string(delStats["median"])});
	errorNumStatsTab.content_.emplace_back(VecStr{"mismatch", to_string(misStats["min"]),
			to_string(misStats["max"]), to_string(misStats["mean"]), to_string(misStats["median"])});

	return errorNumStatsTab;
}
table getIndelDistribution(const table & indelTab){
	std::unordered_map<uint32_t, uint32_t> insertCounts;
	std::unordered_map<uint32_t, uint32_t> delCounts;
	for(const auto & row : indelTab.content_){
		if(row[3] == "insertion"){
			insertCounts[std::stoi(row[4])] += std::stoi(row[2]);
		}else if (row[3] == "deletion"){
			delCounts[std::stoi(row[4])] += std::stoi(row[2]);
		}else{
			std::cout << " getAverageIndelSize " << std::endl;
			std::cout << "this should not be happening" << std::endl;
		}
	}
	table insertTab(insertCounts, VecStr{"indelSize", "count"});
	insertTab.addColumn(VecStr{"insertion"}, "indel");
	table delTab(delCounts, VecStr{"indelSize", "count"});
	delTab.addColumn(VecStr{"deletion"}, "indel");
	delTab.rbind(insertTab);
	delTab.sortTable("indelSize", true);
	delTab.sortTable("indel", true);
	return delTab;
}

table getSeqPosTab(const std::string & str){
	table out(VecStr{"char", "pos"});
	for(const auto & pos : iter::range(str.size())){
		out.content_.emplace_back(toVecStr(str[pos], pos));
	}
	return out;
}

uint32_t processCutOffStr(const std::string& runCutOffString,
  uint64_t readCount){
	uint32_t runCutOff;
  if (runCutOffString.back() == '%') {
    runCutOff = std::round(
        std::stod(runCutOffString.substr(0, runCutOffString.length() - 1)) *
        readCount / 100.0);
  } else {
    runCutOff = std::stoi(runCutOffString);
  }
  return runCutOff;
}




uint32_t getAlnPos(const std::string & seq, uint32_t realSeqPos) {
  uint32_t offSet = 0;
  for (uint32_t i = 0; i < seq.size(); i++) {
    if ((i - offSet) == realSeqPos) {
      return i;
    }
    if (seq[i] == '-') {
      ++offSet;
    }
  }
  return seq.size();
}

uint32_t getRealPos(const std::string & seq, uint32_t seqAlnPos) {
  uint32_t offSet = 0;
  for (uint32_t i = 0; i < seqAlnPos; i++) {
    if (seq[i] == '-') {
      ++offSet;
    }
  }
  return seqAlnPos - offSet;
}
}  // namespace bib
