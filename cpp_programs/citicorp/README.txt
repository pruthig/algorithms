======  README ======

Problem 1:
Build instructions 
> Place the files: records_splitter.cpp, records_splitter.h and dataset file(containing records) in a directory
> This program was tested using g++ version 4.9.1 and VS 2013, make sure either g++ or VS 2013 has been installed on your PC
> For VS 2013, you can create a project and include header and cpp file, while for g++ follow the below steps
> Generate the executable using command: g++ -o records_splitter -std=c++11 records_splitter.cpp 
> Run the executable : ./records_splitter 
> You will be prompted for dataset file, input the same and check for results.

In the problem description, it was mentioned that Parent-Child entry should be in the same file. To achieve the same, unordered_map from STL has been used, reason being for its O(1) lookup time which gives O(n) time in insertion
of records. The unordered_map maps the parent node to the list of child nodes. A file pool is created before data is fed into the files. The map is traversed and each record of it (parent node and its children) is written to the file. File for each of the record is written till either max file entries limit is reached or list is exhausted. The nodes which are left are flushed later with their P-C relationship intact, though records can sparse out.

Assumptions:
It is assumed that there exists a parent for each child.

Limitation and scope of improvement:
Repeated parents entries are not considered because of time constraint.
Verification of record is not being done. It is assumed that dataset is sane and each record is as per the problem statement.
If a Parent-Child set cannot be accommodated in a given file, it is stored in different file.

Problem 2:

> The build setup is  the same as described in the Problem-1
> Compile/Link the file using the following command: g++ -o multiple_threads -std=c++11 multiple_threads.cpp

In this problem, a vector of threads is created, their number dependent on the input. For synchronization, A condition variable, mutex and an atomic variable is used. The solution is implemented using wait-signal mechanism in which all threads
wait till they get notified. Each thread is designated by an id which is used to wake them up. Initially, first thread is woken which awakes another thread and the step recurses.



Please revert in case something is missing or wrongly assumed. I'll fix that and revert back.
