#pragma once
/*
 * readDistGraph.hpp
 *
 *  Created on: Dec 16, 2016
 *      Author: nick
 */




#include "bibseq/objects/dataContainers/graphs/UndirWeightedGraph.hpp"

namespace bibseq {


template<typename DIST>
class readDistGraph : public njhUndirWeightedGraph<DIST, std::shared_ptr<seqInfo>>{
public:

/**@b construct with reads form reads and the distance in distances, the keys being the read names
 *
 * @param distances a map or map, structure is readname1, readname2, dist
 * @param reads the reads
 */
	template<typename T>
	readDistGraph(const std::unordered_map<std::string, std::unordered_map<std::string, DIST>> & distances,
			const std::vector<T> & reads){
	  std::vector<std::string> readNames;
		for(const auto & pos : iter::range(reads.size())){
	  	this->addNode(reads[pos].seqBase_.name_,
	  			std::make_shared<seqInfo>(reads[pos].seqBase_));
	  	readNames.emplace_back(reads[pos].seqBase_.name_);
	  }
	  for(const auto & first : distances){
	  	for(const auto & second : first.second){
	  		if(!bib::in(first.first, readNames) || !bib::in(second.first, readNames)){
	  			std::stringstream ss;
	  			ss << "Error finding " << first.first << " or " << second.first << std::endl;
	  			ss << "in " << vectorToString(readNames, ", ") << std::endl;
	  			throw std::runtime_error{bib::bashCT::boldRed(ss.str())};
	  		}else{
	  			this->addEdge(first.first,
	  				  				second.first,
	  				  				second.second);
	  		}
	  	}
	  }
	}

	/**@b Construct with a distance matrix and a vector of reads that were used to create the distances
	 *
	 * @param distances The distance matrix, the matrix should at least have the diagonal values (each row has as many elements as it's row position
	 * @param reads the reads the distance graph is describing
	 * @todo do a size check for the distance matrix
	 */
	template<typename T>
	readDistGraph(const std::vector<std::vector<DIST>> & distances,
			const std::vector<T> & reads){
	  for(const auto & pos : iter::range(reads.size())){
	  	this->addNode(reads[pos].seqBase_.name_,
	  			std::make_shared<seqInfo>(reads[pos].seqBase_));
	  }
	  for(const auto & pos : iter::range(distances.size())){
	  	for(const auto & subPos : iter::range<uint64_t>(distances[pos].size())){
	  		this->addEdge(reads[pos].seqBase_.name_,
	  				reads[subPos].seqBase_.name_,
	  				distances[pos][subPos]);
	  	}
	  }
	}

	template<typename T, typename... Args>
	readDistGraph(const std::vector<T> & reads, uint32_t numThreads,
			std::function<DIST(const T & e1, const T& e2, Args...)> func,
			const Args&... args) {
			auto distances = getDistanceCopy(reads, numThreads, func, args...);

		  for(const auto & pos : iter::range(reads.size())){
		  	this->addNode(reads[pos].seqBase_.name_,
		  			std::make_shared<seqInfo>(reads[pos].seqBase_));
		  }
		  for(const auto & pos : iter::range(distances.size())){
		  	for(const auto & subPos : iter::range<uint64_t>(distances[pos].size())){
		  		this->addEdge(reads[pos].seqBase_.name_,
		  				reads[subPos].seqBase_.name_,
		  				distances[pos][subPos]);
		  	}
		  }
		}

	template<typename T>
	readDistGraph(const std::vector<std::vector<DIST>> & distances,
			const std::vector<std::unique_ptr<T>> & reads){
	  for(const auto & pos : iter::range(reads.size())){
	  	this->addNode(reads[pos]->seqBase_.name_,
	  			std::make_shared<seqInfo>(reads[pos]->seqBase_));
	  }
	  for(const auto & pos : iter::range(distances.size())){
	  	for(const auto & subPos : iter::range<uint64_t>(distances[pos].size())){
	  		this->addEdge(reads[pos]->seqBase_.name_,
	  				reads[subPos]->seqBase_.name_,
	  				distances[pos][subPos]);
	  	}
	  }
	}

	/**@b Construct with a distance matrix and a vector of reads that were used to create the distances, for shared pointer vecs
	 *
	 * @param distances The distance matrix, the matrix should at least have the diagonal values (each row has as many elements as it's row position
	 * @param reads the reads the distance graph is describing
	 * @todo do a size check for the distance matrix
	 */
	template<typename T>
	readDistGraph(const std::vector<std::vector<DIST>> & distances,
			const std::vector<std::shared_ptr<T>> & reads){
	  for(const auto & pos : iter::range(reads.size())){
	  	this->addNode(reads[pos]->seqBase_.name_,
	  			std::make_shared<seqInfo>(reads[pos]->seqBase_));
	  }
	  for(const auto & pos : iter::range(distances.size())){
	  	for(const auto & subPos : iter::range<uint64_t>(distances[pos].size())){
	  		this->addEdge(reads[pos]->seqBase_.name_,
	  				reads[subPos]->seqBase_.name_,
	  				distances[pos][subPos]);
	  	}
	  }
	}


	Json::Value toJsonMismatchGraphAll(bib::color backgroundColor,
			std::unordered_map<std::string, bib::color> nameColors ){
	  Json::Value graphJson;
	  graphJson["backgroundColor"] = "#" +  backgroundColor.hexStr_ ;
	  auto & nodes = graphJson["nodes"];
	  auto & links = graphJson["links"];
	  uint32_t nCount = 0;
	  std::unordered_map<std::string, uint64_t> nameToNewPos;
		uint32_t mismatchesAllowed = 1;
		this->turnOffEdgesAbove(mismatchesAllowed);
		this->determineGroups();
		while(this->numberOfGroups_ > 1){
			++mismatchesAllowed;
			this->resetVisitedNodes();
			this->resetVisitedEdges();
			this->turnOffEdgesAbove(mismatchesAllowed);
			this->determineGroups();
		}

		bib::randomGenerator gen;
	  uint64_t pos = 0;
	  double minReadCnt = std::numeric_limits<double>::max();
	  double maxReadCnt = std::numeric_limits<double>::lowest();
	  for(const auto & n : this->nodes_){
	  	if(n->value_->cnt_ < minReadCnt){
	  		minReadCnt = n->value_->cnt_;
	  	}
	  	if(n->value_->cnt_ > maxReadCnt){
	  		maxReadCnt = n->value_->cnt_;
	  	}
	  }
	  //scale<double> cntScale({minReadCnt, maxReadCnt},{50.0, 1000.0});
	  scale<double> cntScale({0, maxReadCnt},{50.0, 1000.0});
	  for(const auto & n : this->nodes_){
  		nameToNewPos[n->name_] = pos;
  		++pos;
	  	//std::cout << n->name_ << " : " << n->group_  << " : " << n->value_ << std::endl;
	  	nodes[nCount]["name"] = bib::json::toJson(n->name_);
	  	nodes[nCount]["group"] = bib::json::toJson(n->group_);
	  	nodes[nCount]["color"] = "#" + nameColors[n->name_].hexStr_;
	  	nodes[nCount]["size"] = cntScale.get(n->value_->cnt_);
	  	++nCount;
	  }
	  uint32_t lCount=0;
		for(const auto & e : this->edges_){
			if(e->on_){
				if(e->dist_ == 0){
					links[lCount]["source"] = bib::json::toJson(nameToNewPos[this->nodes_[this->nameToNodePos_[e->nodeToNode_.begin()->first]]->name_]);
					links[lCount]["target"] = bib::json::toJson(nameToNewPos[e->nodeToNode_.begin()->second.lock()->name_]);
					links[lCount]["value"] = bib::json::toJson(1);
					links[lCount]["on"] = bib::json::toJson(e->on_);
					auto lColor = getColsBetweenExcludeClosest(nameColors[this->nodes_[this->nameToNodePos_[e->nodeToNode_.begin()->first]]->name_],
							nameColors[e->nodeToNode_.begin()->second.lock()->name_], 1);
					links[lCount]["color"] = "#" + lColor.front().hexStr_;
					++lCount;
				}else{
					std::string lastName = this->nodes_[this->nameToNodePos_[e->nodeToNode_.begin()->first]]->name_;
					auto lColors = getColsBetweenExcludeClosest(nameColors[this->nodes_[this->nameToNodePos_[e->nodeToNode_.begin()->first]]->name_],
							nameColors[e->nodeToNode_.begin()->second.lock()->name_], e->dist_ + 1);
					for(const auto & mis : iter::range(e->dist_)){
						std::string newName = this->nodes_[this->nameToNodePos_[e->nodeToNode_.begin()->first]]->name_
								+ estd::to_string(mis) + e->nodeToNode_.begin()->second.lock()->name_;
			  		nameToNewPos[newName] = pos;
			  		++pos;
				  	nodes[nCount]["name"] = bib::json::toJson(newName);
				  	nodes[nCount]["group"] = bib::json::toJson(e->nodeToNode_.begin()->second.lock()->group_);
				  	nodes[nCount]["color"] = "red";
				  	nodes[nCount]["size"] = 10;
				  	++nCount;
						links[lCount]["source"] = bib::json::toJson(nameToNewPos[lastName]);
						links[lCount]["target"] = bib::json::toJson(nameToNewPos[newName]);
						links[lCount]["value"] = bib::json::toJson(1);
						links[lCount]["on"] = bib::json::toJson(true);
						links[lCount]["color"] = "#" + lColors[mis].hexStr_;
						++lCount;
						lastName = newName;
					}

					links[lCount]["source"] = bib::json::toJson(nameToNewPos[lastName]);
					links[lCount]["target"] = bib::json::toJson(nameToNewPos[e->nodeToNode_.begin()->second.lock()->name_]);
					links[lCount]["value"] = bib::json::toJson(1);
					links[lCount]["on"] = bib::json::toJson(true);
					links[lCount]["color"] = "#" + lColors[e->dist_].hexStr_;
					++lCount;
				}
			}
		}
		return graphJson;
	}

	Json::Value toJsonMismatchGraph(uint32_t groupCutOff,
				uint32_t mismatchesAllowed,
				bib::color backgroundColor, double hueStart, double hueStop,
		    double lumStart, double lumStop,
		    double satStart, double satStop){
		Json::Value graphJson;
		graphJson["backgroundColor"] = "#" +  backgroundColor.hexStr_ ;
		auto & nodes = graphJson["nodes"];
		auto & links = graphJson["links"];
		uint32_t nCount = 0;
		std::unordered_map<std::string, uint64_t> nameToNewPos;
		this->turnOffEdgesAbove(mismatchesAllowed);
		this->determineGroups();

		bib::randomGenerator gen;
		uint64_t pos = 0;
		double minReadCnt = std::numeric_limits<double>::max();
		double maxReadCnt = std::numeric_limits<double>::lowest();
		std::unordered_map<uint32_t, bib::color> groupColors;
	  std::unordered_map<uint32_t, uint32_t> groupCounts;
	  uint32_t numOfCutOffGroups = 0;
	  for(const auto & n : this->nodes_){
	  	++groupCounts[n->group_];
	  }
	  std::vector<uint32_t> groups;
		for(const auto & g : groupCounts){
			if(g.second >= groupCutOff){
				++numOfCutOffGroups;
				groups.emplace_back(g.first);
			}
		}
		//printOutMapContents(groupCounts,"\t", std::cout);
		//printVector(groups);
		auto gColors = bib::getColsBetweenInc(hueStart, hueStop,
	  		lumStart, lumStop,
	  		satStart, satStop,
	  		groups.size());

		for(const auto & pos : iter::range(groups.size())){
			groupColors[groups[pos]] = gColors[pos];
		}

		for(const auto & n : this->nodes_){
			if(n->value_->cnt_ < minReadCnt){
				minReadCnt = n->value_->cnt_;
			}
			if(n->value_->cnt_ > maxReadCnt){
				maxReadCnt = n->value_->cnt_;
			}
		}
		//scale<double> cntScale({minReadCnt, maxReadCnt},{50.0, 1000.0});
		scale<double> cntScale({0, maxReadCnt},{50.0, 1000.0});
		for(const auto & n : this->nodes_){
	  	if(groupCounts[n->group_] >= groupCutOff){
				nameToNewPos[n->name_] = pos;
				++pos;
				//std::cout << n->name_ << " : " << n->group_  << " : " << n->value_ << std::endl;
				nodes[nCount]["name"] = bib::json::toJson(n->name_);
				nodes[nCount]["group"] = bib::json::toJson(n->group_);
				nodes[nCount]["color"] = "#" + groupColors[n->group_].hexStr_;
				nodes[nCount]["size"] = cntScale.get(n->value_->cnt_);
				++nCount;
	  	}
		}
		uint32_t lCount=0;
		for(const auto & e : this->edges_){
			if(e->on_ && groupCounts[e->nodeToNode_.begin()->second.lock()->group_] >= groupCutOff){
				if(e->dist_ == 0){
					links[lCount]["source"] = bib::json::toJson(nameToNewPos[this->nodes_[this->nameToNodePos_[e->nodeToNode_.begin()->first]]->name_]);
					links[lCount]["target"] = bib::json::toJson(nameToNewPos[e->nodeToNode_.begin()->second.lock()->name_]);
					links[lCount]["value"] = bib::json::toJson(1);
					links[lCount]["on"] = bib::json::toJson(e->on_);
					auto lColor = getColsBetweenExcludeClosest(groupColors[this->nodes_[this->nameToNodePos_[e->nodeToNode_.begin()->first]]->group_],
							groupColors[e->nodeToNode_.begin()->second.lock()->group_], 1);
					links[lCount]["color"] = "#" + lColor.front().hexStr_;
					++lCount;
				}else{
					std::string lastName = this->nodes_[this->nameToNodePos_[e->nodeToNode_.begin()->first]]->name_;
					auto lColors = getColsBetweenExcludeClosest(groupColors[this->nodes_[this->nameToNodePos_[e->nodeToNode_.begin()->first]]->group_],
							groupColors[e->nodeToNode_.begin()->second.lock()->group_], e->dist_ + 1);
					for(const auto & mis : iter::range(e->dist_)){
						std::string newName = this->nodes_[this->nameToNodePos_[e->nodeToNode_.begin()->first]]->name_
								+ estd::to_string(mis) + e->nodeToNode_.begin()->second.lock()->name_;
						nameToNewPos[newName] = pos;
						++pos;
						nodes[nCount]["name"] = bib::json::toJson(newName);
						nodes[nCount]["group"] = bib::json::toJson(e->nodeToNode_.begin()->second.lock()->group_);
						nodes[nCount]["color"] = "red";
						nodes[nCount]["size"] = 10;
						++nCount;
						links[lCount]["source"] = bib::json::toJson(nameToNewPos[lastName]);
						links[lCount]["target"] = bib::json::toJson(nameToNewPos[newName]);
						links[lCount]["value"] = bib::json::toJson(1);
						links[lCount]["on"] = bib::json::toJson(true);
						links[lCount]["color"] = "#" + lColors[mis].hexStr_;
						++lCount;
						lastName = newName;
					}

					links[lCount]["source"] = bib::json::toJson(nameToNewPos[lastName]);
					links[lCount]["target"] = bib::json::toJson(nameToNewPos[e->nodeToNode_.begin()->second.lock()->name_]);
					links[lCount]["value"] = bib::json::toJson(1);
					links[lCount]["on"] = bib::json::toJson(true);
					links[lCount]["color"] = "#" + lColors[e->dist_].hexStr_;
					++lCount;
				}
			}
		}
		return graphJson;
	}

	Json::Value toJsonMismatchGraph(bib::color backgroundColor, double hueStart, double hueStop,
			    double lumStart, double lumStop,
			    double satStart, double satStop){
			Json::Value graphJson;
			graphJson["backgroundColor"] = "#" +  backgroundColor.hexStr_ ;
			auto & nodes = graphJson["nodes"];
			auto & links = graphJson["links"];
			uint32_t nCount = 0;
			std::unordered_map<std::string, uint64_t> nameToNewPos;

			uint64_t pos = 0;
			double minReadCnt = std::numeric_limits<double>::max();
			double maxReadCnt = std::numeric_limits<double>::lowest();
			std::unordered_map<uint32_t, bib::color> groupColors;
		  std::unordered_map<uint32_t, uint32_t> groupCounts;
		  for(const auto & n : this->nodes_){
		  	++groupCounts[n->group_];
		  }
		  std::vector<uint32_t> groups;
			for(const auto & g : groupCounts){
					groups.emplace_back(g.first);
			}
			auto gColors = bib::getColsBetweenInc(hueStart, hueStop,
		  		lumStart, lumStop,
		  		satStart, satStop,
		  		groups.size());

			for(const auto & pos : iter::range(groups.size())){
				groupColors[groups[pos]] = gColors[pos];
			}

			for(const auto & n : this->nodes_){
				if(n->value_->cnt_ < minReadCnt){
					minReadCnt = n->value_->cnt_;
				}
				if(n->value_->cnt_ > maxReadCnt){
					maxReadCnt = n->value_->cnt_;
				}
			}
			//scale<double> cntScale({minReadCnt, maxReadCnt},{50.0, 1000.0});
			scale<double> cntScale({0, maxReadCnt},{50.0, 1000.0});
			for(const auto & n : this->nodes_){
				nameToNewPos[n->name_] = pos;
				++pos;
				//std::cout << n->name_ << " : " << n->group_  << " : " << n->value_ << std::endl;
				nodes[nCount]["name"] = bib::json::toJson(n->name_);
				nodes[nCount]["group"] = bib::json::toJson(n->group_);
				nodes[nCount]["color"] = "#" + groupColors[n->group_].hexStr_;
				nodes[nCount]["size"] = cntScale.get(n->value_->cnt_);
				++nCount;
			}
			uint32_t lCount=0;
			for(const auto & e : this->edges_){
				if(e->on_){
					if(e->dist_ == 0){
						links[lCount]["source"] = bib::json::toJson(nameToNewPos[this->nodes_[this->nameToNodePos_[e->nodeToNode_.begin()->first]]->name_]);
						links[lCount]["target"] = bib::json::toJson(nameToNewPos[e->nodeToNode_.begin()->second.lock()->name_]);
						links[lCount]["value"] = bib::json::toJson(1);
						links[lCount]["on"] = bib::json::toJson(e->on_);
						auto lColor = getColsBetweenExcludeClosest(groupColors[this->nodes_[this->nameToNodePos_[e->nodeToNode_.begin()->first]]->group_],
								groupColors[e->nodeToNode_.begin()->second.lock()->group_], 1);
						links[lCount]["color"] = "#" + lColor.front().hexStr_;
						++lCount;
					}else{
						std::string lastName = this->nodes_[this->nameToNodePos_[e->nodeToNode_.begin()->first]]->name_;
						auto lColors = getColsBetweenExcludeClosest(groupColors[this->nodes_[this->nameToNodePos_[e->nodeToNode_.begin()->first]]->group_],
								groupColors[e->nodeToNode_.begin()->second.lock()->group_], e->dist_ + 1);
						for(const auto & mis : iter::range(e->dist_)){
							std::string newName = this->nodes_[this->nameToNodePos_[e->nodeToNode_.begin()->first]]->name_
									+ estd::to_string(mis) + e->nodeToNode_.begin()->second.lock()->name_;
							nameToNewPos[newName] = pos;
							++pos;
							nodes[nCount]["name"] = bib::json::toJson(newName);
							nodes[nCount]["group"] = bib::json::toJson(e->nodeToNode_.begin()->second.lock()->group_);
							nodes[nCount]["color"] = "red";
							nodes[nCount]["size"] = 10;
							++nCount;
							links[lCount]["source"] = bib::json::toJson(nameToNewPos[lastName]);
							links[lCount]["target"] = bib::json::toJson(nameToNewPos[newName]);
							links[lCount]["value"] = bib::json::toJson(1);
							links[lCount]["on"] = bib::json::toJson(true);
							links[lCount]["color"] = "#" + lColors[mis].hexStr_;
							++lCount;
							lastName = newName;
						}

						links[lCount]["source"] = bib::json::toJson(nameToNewPos[lastName]);
						links[lCount]["target"] = bib::json::toJson(nameToNewPos[e->nodeToNode_.begin()->second.lock()->name_]);
						links[lCount]["value"] = bib::json::toJson(1);
						links[lCount]["on"] = bib::json::toJson(true);
						links[lCount]["color"] = "#" + lColors[e->dist_].hexStr_;
						++lCount;
					}
				}
			}
			return graphJson;
		}
};

}  // namespace bibseq

