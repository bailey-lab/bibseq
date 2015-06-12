

#include "consensusHelper.hpp"


namespace bibseq {


void consensusHelper::genConsensusFromCounters(seqInfo & info,
		const std::map<uint32_t, charCounterArray> & counters,
		const std::map<uint32_t, std::map<uint32_t, charCounterArray>> & insertions,
		const std::map<int32_t, charCounterArray> & beginningGap) {
	info.seq_.clear();
	info.qual_.clear();
	// first deal with any gaps in the beginning
	double fortyPercent = 0.40 * info.cnt_;
	for (const auto & bCount : beginningGap) {
		uint32_t bestQuality = 0;
		char bestBase = ' ';
		bCount.second.getBest(bestBase, bestQuality);
		if (bestBase == '-' || bCount.second.getTotalCount() < fortyPercent) {
			continue;
		}
		info.seq_.push_back(bestBase);
		info.qual_.emplace_back(bestQuality / bCount.second.getTotalCount());
	}
	//read.seqBase_.outPutFastq(std::cout);
	// the iterators to over the letter counter maps
	for (const auto & count : counters) {
		uint32_t bestQuality = 0;
		char bestBase = ' ';
		// if there is an insertion look at those if there is a majority of reads
		// with that insertion
		auto search = insertions.find(count.first);
		if (search != insertions.end()) {
			for (auto & counterInsert : search->second) {
				bestQuality = 0;
				bestBase = ' ';
				counterInsert.second.getBest(bestBase, bestQuality,
						std::round(info.cnt_));
				if (bestBase == ' ') {
					continue;
				} else {
					info.seq_.push_back(bestBase);
					info.qual_.emplace_back(bestQuality);
				}
			}
		}
		count.second.getBest(bestBase, bestQuality);
		if (bestBase == '-' || count.second.getTotalCount() < fortyPercent) {
			continue;
		}
		info.seq_.push_back(bestBase);
		info.qual_.emplace_back(
				bestQuality / count.second.getTotalCount());
	}
}


}  // namespace bibseq
