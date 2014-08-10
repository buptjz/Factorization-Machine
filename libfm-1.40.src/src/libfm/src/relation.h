/*
	Data and Links for Relations 

	Author:   Steffen Rendle, http://www.libfm.org/
	modified: 2013-07-04

	Copyright 2011-2013 Steffen Rendle, see license.txt for more information
*/

#ifndef RELATION_DATA_H_
#define RELATION_DATA_H_

#include <limits>
#include "../../util/matrix.h"
#include "../../util/fmatrix.h"
#include "../../fm_core/fm_data.h"
#include "../../fm_core/fm_model.h"
#include "Data.h"

class RelationData {
	protected:
		uint cache_size;
		bool has_xt;
		bool has_x;
	public:	
		RelationData(uint cache_size, bool has_x, bool has_xt) { 
			this->data_t = NULL;
			this->data = NULL;
			this->cache_size = cache_size;
			this->has_x = has_x;
			this->has_xt = has_xt;
			this->meta = NULL;
		}
		DataMetaInfo* meta;

		LargeSparseMatrix<DATA_FLOAT>* data_t;
		LargeSparseMatrix<DATA_FLOAT>* data;
	
		int num_feature;
		uint num_cases;
		uint attr_offset; 
 
		void load(std::string filename);	
		void debug();
};


class RelationJoin {
	public:
		DVector<uint> data_row_to_relation_row;
		RelationData* data;

		void load(std::string filename, uint expected_row_count) {
			bool do_binary = false;
			// check if binary or text format should be read
			{
				std::ifstream in (filename.c_str(), std::ios_base::in | std::ios_base::binary);
				if (in.is_open()) {
					uint file_version;
					uint data_size;
					in.read(reinterpret_cast<char*>(&file_version), sizeof(file_version));
					in.read(reinterpret_cast<char*>(&data_size), sizeof(data_size));
					do_binary = ((file_version == DVECTOR_EXPECTED_FILE_ID) && (data_size == sizeof(uint)));
					in.close();
				}
			}
			if (do_binary) {
				//std::cout << "(binary mode) " << std::endl;
				data_row_to_relation_row.loadFromBinaryFile(filename);
			} else {
				//std::cout << "(text mode) " << std::endl;
				data_row_to_relation_row.setSize(expected_row_count);
				data_row_to_relation_row.load(filename);
			}
			assert(data_row_to_relation_row.dim == expected_row_count);
		}
};

void RelationData::load(std::string filename) {

	std::cout << "has x = " << has_x << std::endl;
	std::cout << "has xt = " << has_xt << std::endl;
	assert(has_x || has_xt);

	//uint num_cases = 0;
	uint num_values = 0;
	uint this_cs = cache_size;
	if (has_xt && has_x) { this_cs /= 2; }
	
	if (has_x) {
		std::cout << "data... ";
		this->data = new LargeSparseMatrixHD<DATA_FLOAT>(filename + ".x", this_cs);
		this->num_feature = this->data->getNumCols();	
		num_values = this->data->getNumValues();
		num_cases = this->data->getNumRows();
	} else {
		data = NULL;
	}
	if (has_xt) {
		std::cout << "data transpose... ";
		this->data_t = new LargeSparseMatrixHD<DATA_FLOAT>(filename + ".xt", this_cs);
		this->num_feature = this->data_t->getNumRows();
		num_values = this->data_t->getNumValues();
		num_cases = this->data_t->getNumCols();
	} else {
		data_t = NULL;
	}
	
	if (has_xt && has_x) {
		assert(this->data->getNumCols() == this->data_t->getNumRows());
		assert(this->data->getNumRows() == this->data_t->getNumCols());
		assert(this->data->getNumValues() == this->data_t->getNumValues());
	}

	std::cout << "num_cases=" << this->num_cases << "\tnum_values=" << num_values << "\tnum_features=" << this->num_feature << std::endl;

	meta = new DataMetaInfo(this->num_feature);
		
	if (fileexists(filename + ".groups")) {
		meta->loadGroupsFromFile(filename + ".groups");
	}
}


void RelationData::debug() {
	if (has_x) {
		for (data->begin(); (!data->end()) && (data->getRowIndex() < 4); data->next() ) {
			for (uint j = 0; j < data->getRow().size; j++) {
				std::cout << " " << data->getRow().data[j].id << ":" << data->getRow().data[j].value;	
			}
			std::cout << std::endl;
		}
	}	
}

#endif /*RELATION_DATA_H_*/
