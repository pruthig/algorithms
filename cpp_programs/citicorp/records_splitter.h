#ifndef _RECORDS_SPLITTER
#define _RECORDS_SPLITTER

#include<fstream> 
#include<iostream>
#include <string>
#include <chrono>
#include <list>
#include <sstream>
#include <map>
#include <unordered_set>
#include<unordered_map>
#include <algorithm>

using namespace std;

class SplitRecords{
private:
	// create iterators alias
	using file_iter = unordered_map<fstream*, int>::iterator;
	using record_iter = unordered_map< string, std::pair< string, list<string>> >::iterator;
	using list_iter = list<string>::iterator;

	// Main dataset
	unordered_map< string, std::pair< string, list<string>> > record_map;

	// Create file pool
	unordered_map<fstream*, int> file_pool;
	int records_per_file = 0;
	int records_flushed = 0;
	int size_of_entries = 0;
	int num_files = 0;
	// File iterator for flushing the remaining data
	file_iter f_iter_flush;

public:
	void populate_file_pool();
	bool flush_write(string data);
	void split_to_files();
	void add_parent_id(unordered_map< string, std::pair< string, list<string>> > &record_map, string parent_id, string line);
	void release_resources();
	void populate_dataset(ifstream &infile);
	void set_records_per_file(int rpf);
};
#endif