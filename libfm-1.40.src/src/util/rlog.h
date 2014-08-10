/*
	Logging into R compatible files

	Author:   Steffen Rendle, http://www.libfm.org/
	modified: 2010-12-10

	Copyright 2010 Steffen Rendle, see license.txt for more information
*/

#ifndef RLOG_H_
#define RLOG_H_
#include <iostream>
#include <fstream>
#include <assert.h>
#include <map>

class RLog {
	private:
		std::ostream* out;
		std::vector<std::string> header;
		std::map<std::string,double> default_value;
		std::map<std::string,double> value;
	public:
		RLog(std::ostream* stream) { 
			out = stream;
			header.clear();
			default_value.clear();
			value.clear();
		};	
		
		void log(const std::string& field, double d) {
			value[field] = d;
		}
		
		void init() {
			if (out != NULL) {
				for (uint i = 0; i < header.size(); i++) {
					*out << header[i];
					if (i < (header.size()-1)) {
						*out << "\t";
					} else {
						*out << "\n";
					}
				}			
				out->flush();
			}
			for (uint i = 0; i < header.size(); i++) {
				value[header[i]] = default_value[header[i]];	
			}
		}
		
		void addField(const std::string& field_name, double def) {
			//std::cout << field_name << std::endl; std::cout.flush();
			std::vector<std::string>::iterator i = std::find(header.begin(), header.end(), field_name);
			if (i != header.end()) {
				throw "the field " + field_name + " already exists";
			}
			header.push_back(field_name);
			default_value[field_name] = def;
		}
		
		void newLine() {
			if (out != NULL) {
				for (uint i = 0; i < header.size(); i++) {
					*out << value[header[i]];
					if (i < (header.size()-1)) {
						*out << "\t";
					} else {
						*out << "\n";
					}
				}
				out->flush();
				value.clear();	
				for (uint i = 0; i < header.size(); i++) {
					value[header[i]] = default_value[header[i]];	
				}
			}
		}
};
	

#endif /*RLOG_H_*/
