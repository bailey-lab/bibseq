/*
 * MasterTableCache.cpp
 *
 *  Created on: Feb 7, 2016
 *      Author: nick
 */
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
#include "MasterTableCache.hpp"

namespace bibseq {

MasterTableCache::MasterTableCache(const TableIOOpts & tabOpts,
		const std::string & dir, bool recursive, uint32_t depth,
		const std::regex & pattern) :
		tabOpts_(tabOpts), dir_(dir), recursive_(recursive), depth_(depth), pattern_(
				pattern) {
	//ensure the table is can be written to overwrite outdate info
	tabOpts_.out_.overWriteFile_ = true;

}

void MasterTableCache::collectFiles() {
	files_ = bib::files::listAllFiles(dir_, recursive_, { pattern_ }, depth_);
}
bool MasterTableCache::needsUpdate() const {
	bool needsUpdate = false;
	if (!bib::files::bfs::exists(tabOpts_.out_.outFilename_)) {
		needsUpdate = true;
	} else {
		auto masterWriteTime = bib::files::last_write_time(
				tabOpts_.out_.outFilename_);
		for (const auto & file : files_) {
			if (bib::files::normalize(file.first)
					== bib::files::normalize(tabOpts_.out_.outFilename_)) {
				continue;
			}
			auto currentFileWriteTime = bib::files::last_write_time(file.first);
			if (masterWriteTime < currentFileWriteTime) {
				needsUpdate = true;
				break;
			}
		}
	}
	return needsUpdate;
}

void MasterTableCache::updateTab() {
	collectFiles();
	if (needsUpdate()) {
		masterTab_ = table();
		for (const auto & file : files_) {
			if (bib::files::normalize(file.first)
					== bib::files::normalize(tabOpts_.out_.outFilename_)) {
				continue;
			}
			table fileTab = table(file.first.string(), tabOpts_.outDelim_,
					tabOpts_.hasHeader_);
			if (masterTab_.content_.empty()) {
				masterTab_ = fileTab;
			} else {
				masterTab_.rbind(fileTab, false);
			}
		}
	}
}

void MasterTableCache::writeTab(){
	updateTab();
	masterTab_.outPutContents(tabOpts_);
}


}  // namespace bibseq

