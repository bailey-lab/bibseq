#pragma once
/*

 * swisProt.hpp
 *
 *  Created on: Apr 8, 2014
 *      Author: nickhathaway
 */
#include "bibseq/utils.h"

namespace bibseq {
class reference{
	/*
  """Holds information from one reference in a SwissProt entry.

  Members:
  number      Number of reference in an entry.
  positions   Describes extent of work.  List of strings.
  comments    Comments.  List of (token, text).
  references  References.  List of (dbname, identifier).
  authors     The authors of the work.
  title       Title of the work.
  location    A citation for the work.

  """
  */
public:
	uint32_t number_;
	std::vector<std::string> positions_;
	std::unordered_map<std::string, std::string> comments_;
	std::unordered_map<std::string, std::string> references_;
	std::vector<std::string> authors_;
	std::string title_;
	std::string location_;
};
class swisProt {
  /*"""Holds information from a SwissProt record.

  Members:
  entry_name        Name of this entry, e.g. RL1_ECOLI.
  data_class        Either 'STANDARD' or 'PRELIMINARY'.
  molecule_type     Type of molecule, 'PRT',
  sequence_length   Number of residues.

  accessions        List of the accession numbers, e.g. ['P00321']
  created           A tuple of (date, release).
  sequence_update   A tuple of (date, release).
  annotation_update A tuple of (date, release).

  description       Free-format description.
  gene_name         Gene name.  See userman.txt for description.
  organism          The source of the sequence.
  organelle         The origin of the sequence.
  organism_classification  The taxonomy classification.  List of strings.
                           (http://www.ncbi.nlm.nih.gov/Taxonomy/)
  taxonomy_id       A list of NCBI taxonomy id's.
  host_organism     A list of names of the hosts of a virus, if any.
  host_taxonomy_id  A list of NCBI taxonomy id's of the hosts, if any.
  references        List of Reference objects.
  comments          List of strings.
  cross_references  List of tuples (db, id1[, id2][, id3]).  See the docs.
  keywords          List of the keywords.
  features          List of tuples (key name, from, to, description).
                    from and to can be either integers for the residue
                    numbers, '<', '>', or '?'

  seqinfo           tuple of (length, molecular weight, CRC32 value)
  sequence          The sequence.

  """*/
public:
	//members
  std::string entryName_;
  std::string dataClass_;
  std::string moleculeType_;
  uint32_t sequenceLength_;

  std::vector<std::string >accessions_;
  std::tuple<std::string, std::string> created_;
  std::tuple<std::string, std::string> sequenceUpdate_;
  std::tuple<std::string, std::string> annotationUpdate_;

  std::unordered_map<std::string, std::string> description_;
  std::string geneName_;
  std::string organism_;
  std::string organelle_;
  std::vector<std::string> organismClassification_;
  std::vector<std::string> taxonomyId_;
  std::vector<std::string> hostOrganism_;
  std::vector<std::string> hostTaxonomyId_;
  std::vector<reference> references_;
  std::vector<std::string> comments_;
  std::vector<std::vector<std::string>> crossReferences_;
  std::vector<std::string> keywords_;
  struct feature{
  	std::string name_;
  	std::string from_;
  	std::string to_;
  	std::string description_;
  };
  std::vector<feature> features_;
  struct sequenceInfo{
  	std::string length_;
  	std::string molecularWeight_;
  	std::string CRC32Value_;
  };
  sequenceInfo seqinfo_;
  std::string sequence_;

  void readIdLine(const std::string & line);
  virtual void printDescription(std::ostream & out, bool deep);
};

} /* namespace bib */




//class Reference(object):


