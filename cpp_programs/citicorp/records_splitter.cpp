// This program split the line items from a large file in small files with a given constraints as in -> A parent - child relation 
// exists between the line items and a child should be stored along with its parent in the same file. The best way to store such 
// data is in form of tree, set is one such impl. in STL but it consumes too much time for mass volume of data, better stick
// with hashtable/unordered_map
// Average time taken in parsing 1 million entries from file - 60-65 sec
// Average time taken to store in unordered map - 85 sec

#include "records_splitter.h"

using namespace std;

/* Constructor
SplitRecords::SplitRecords(){

}
*/

// Calculate the number of files required and add to set
void SplitRecords::populate_file_pool()
{
	num_files = size_of_entries / records_per_file;
	if (size_of_entries % records_per_file != 0)
		num_files++;

	for (int i = 0; i < num_files; ++i) {
		fstream *file;
		file = new fstream;
		file->rdbuf()->pubsetbuf(0, 0);
		file_pool.insert({ file, 0 });
	}
}

template <typename Stream>
void reopen(Stream& pStream, string pFile, std::ios_base::openmode pMode = ios_base::out) {
	pStream.close();
	pStream.clear();
	pStream.open(pFile, pMode);
}

bool SplitRecords::flush_write(string data) {
	if (f_iter_flush == file_pool.end())
		return false;
	if ((*f_iter_flush).second == records_per_file) {
		while ((*f_iter_flush).second == records_per_file && f_iter_flush != file_pool.end()) {
			// use next file
			++f_iter_flush;
		}
	}
	if (f_iter_flush == file_pool.end())
		return false;

	fstream *file = (*f_iter_flush).first;
	*file << data << endl;
	++((*f_iter_flush).second);
	return true;
}

// Main function that splits data to smaller files.
void SplitRecords::split_to_files()
{
	int id = 0;
	record_iter rec_iter = record_map.begin();
	file_iter f_iter = file_pool.begin();
	
	for ( ; f_iter != file_pool.end() && rec_iter != record_map.end(); ) 
	{
		// Take a map record
		auto &map_record = *rec_iter;
		list_iter l_iter = map_record.second.second.begin();  // map_record.second.second gives us list

		// Open the file for writing
		fstream *ofile = (*f_iter).first;
		ofile->open("file_part_" + to_string(id) + ".txt" , std::fstream::out | std::fstream::trunc | std::fstream::in);
		++id;
		// Add parent record, blank it out and update file entry count
		(*ofile) << map_record.second.first << endl;
		map_record.second.first.clear();
		(++(*f_iter).second);

		for (; l_iter != map_record.second.second.end() && (*f_iter).second < records_per_file; (++(*f_iter).second))
		{
			(*ofile) << (*l_iter) << endl;
			map_record.second.second.erase(l_iter++);			
		}
		// Delete the map entry of list, if completed
		if (map_record.second.second.size() == 0)
			record_map.erase(rec_iter++);
		else
			++rec_iter;
		ofile->flush();

		// Remove entry from file pool if file is full
		if ((*f_iter).second == records_per_file)
			file_pool.erase(f_iter++);
		else
			++f_iter;
	}

	f_iter_flush = file_pool.begin();
	// Flush leftover entries. The combined complexity of top and bottom loop is O(n)
	for (record_iter rec_iter = record_map.begin(); rec_iter != record_map.end(); ++rec_iter)
	{
		auto &map_record = *rec_iter;
		list_iter l_iter = map_record.second.second.begin();  // map_record.second.second gives us list
		
		// Add parent record if exists
		if (!map_record.second.first.empty()) {
			if (!flush_write(map_record.second.first))
				break;
		}

		for (; l_iter != map_record.second.second.end(); ++l_iter)
		{
			if (!flush_write(*l_iter)) break;
		}
	}

}

void SplitRecords::release_resources() {
	// free file pool
	
}

// Add parent ID to map
void SplitRecords::add_parent_id(unordered_map< string, std::pair< string, list<string>> > &record_map, string parent_id, string line) {
	if (record_map.find(parent_id) == record_map.end()){
		list<string> l;
		record_map.insert({ parent_id, make_pair(line, l) });
	}
}

void SplitRecords::populate_dataset(ifstream &infile){

	string line;
	while (std::getline(infile, line))
	{
		++size_of_entries;
		// Insert line into set
		int pos_last = line.rfind('~');
		string parent_id(line.substr(pos_last + 1));
		string find_id = line.substr(0, pos_last);
		string id(find_id.substr(find_id.rfind('~') + 1));
		if (parent_id == "0") {
			// Add parent_id whose parent is "0"
			add_parent_id(record_map, id, line);
		}
		else {
			// A part of assumption that all children have corresponding parents and entries are not repeated,
			// easily discernible too
			add_parent_id(record_map, parent_id, line); // Add parent_id if not present
			record_map.at(parent_id).second.push_back(line);
		}

	}
	//if (size_of_entries < )

}

void SplitRecords::set_records_per_file(int rpf){
	records_per_file = rpf;
}

int main() {
	int records_per_file = 0;
	string file_name;
	cout << "Please make sure that test file has been placed with the executable."<<endl<<endl;
	cout << "Please Enter the file name along with its extension" << endl;
	cin >> file_name;
	std::ifstream infile(file_name);
	if (!infile.good()) {
		cout << "File not found" << endl;
		system("pause");
		return 0;
	}
	cout << "Enter the number of records each file can hold:" << endl;
	cin >> records_per_file;
	cout << "Reading records...Please Wait" << endl;
	
	SplitRecords sr;
	sr.set_records_per_file(records_per_file);
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	sr.populate_dataset(infile);
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time elapsed reading records: " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << " seconds"<<std::endl;
	cout << endl<<"Finished reading from main file" << endl << "Writing to smaller files now" << endl;
	
	sr.populate_file_pool();

	begin = end;
	sr.split_to_files();
	end = std::chrono::steady_clock::now();
	std::cout << "Time elapsed writing to files: " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << " seconds" << std::endl;

	sr.release_resources();
	system("pause");
	return 0;
}