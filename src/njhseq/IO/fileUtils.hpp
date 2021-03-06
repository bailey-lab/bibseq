#pragma once
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
//
//  fileUtils.hpp
//
//  Created by Nicholas Hathaway on 9/15/13.
//

#include <map>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <boost/filesystem.hpp>
#include "njhseq/common/typedefs.hpp"
#include "njhseq/IO/IOUtils.hpp"

namespace njhseq {





int getdir(const std::string &dir,
           std::map<std::string, std::pair<std::string, bool>> &files);

std::map<std::string, std::pair<std::string, bool>> listFilesInDir(
    const std::string &directoryName, bool recursive);


// opening text files
void openTextFile(std::ofstream &file, bfs::path filename,
		std::string fileExtention, bool overWrite, bool exitOnFailure);
void openTextFile(std::ofstream &file, const OutOptions & options);
void openTextFile(std::ofstream &file, bfs::path filename,
		const OutOptions & outOptions);
void openTextFile(std::ofstream &file, bfs::path filename,
		std::string fileExtention, const OutOptions & outOptions);

// runLog stuff
void startRunLog(std::ofstream &runLog, const MapStrStr &inputCommands);

std::map<std::string, std::pair<std::string, bool>> getFiles(
    const std::string &directoryName, const std::string &contains,
    const std::string &filesOrDirectories, bool specific, bool recursive);

std::map<std::string, std::pair<std::string, bool>> getFiles(
    const std::string &directoryName, const VecStr &contains,
    const std::string &filesOrDirectories, bool specific, bool recursive);

VecStr collectFilenames(VecStr contains, VecStr doesNotContain,
                        bool recursive = true);
VecStr collectFilenamesWithExtention(const std::string &directory,
                                     const std::string &extention,
                                     bool recursive = true);
std::string cleanOutPerLine(const std::string &in, uint32_t width,
                            uint32_t indentAmount);
std::string cleanOut(const std::string &in, uint32_t width,
                     uint32_t indentAmount);

VecStr getNewestDirs(const std::string & dirName, const std::string & con);


template<typename OPT>
std::streambuf* determineOutBuf(std::ofstream & outFile,
		const OPT & opts) {
	return njh::files::determineOutBuf(outFile, opts.outFilename_,
			opts.outExtention_, opts.overWriteFile_, opts.append_,
			opts.exitOnFailureToWrite_);
}



void concatenateFiles(const std::vector<bfs::path> & fnps, const OutOptions & outopts);


}  // namespace njhseq


