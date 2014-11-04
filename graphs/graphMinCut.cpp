/*
* File:   graphMinCut.cpp
* Author: g.pruthi
*
* Created on 1 Nov, 2014, 1:14 PM
*/
#include<iostream>

#include<fstream>
#include<map>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<string>
#include<vector>
#include<sstream>




using namespace std;


struct vertex;

vertex* p_createVertex(int l);
bool vInX(int c);
void getUniqueVs(int *a, int *b);
void pickAndMerge();
void exchangeAllSecondWithFirst(int min, int max);
void removeSelfLoops(int m);
bool checkIfNULL(int t1, int t2);
void removeAllSelfLoops();
void UpdateEdgeCount();
void checkAndInsert(int x, int y);
int vertexCount();


struct vertex{
	int label;
	vertex *next;
};

vector<vertex*> p_V;
int g_NodeCount = 0;
std::vector< std::vector<int> > V;
std::vector <int> vec;


/*
*
*/
int main()
{
	ifstream ifs;
	ifs.open("/home/pruthi/Desktop/kargerMinCut.txt");
	bool first = true;

	string line = "";
	int i = 0;

	int x = 1;
	
	while (std::getline(ifs, line)) //Read a line 
	{
		std::stringstream ss(line);
		first = true;

		while (ss >> i) {//Extract integers from line
			if (first){
				first = false;
				continue;
			}
			vec.push_back(i);
		}
		V.push_back(vec);
		vec.clear();
	}
	//fill it NULL
	for (int i = 0; i <= 200; ++i)
		p_V.push_back(NULL);

	//create unique edges in the graph
	i = 1;

	for (const auto &x : V)
	{
		for (const auto &y : x){
			checkAndInsert(i, y);
			
		}
		++i;
	}
	UpdateEdgeCount();
	int p = 0;
	while ( ((p = vertexCount()) != 2)){
		pickAndMerge();
	}

	//ok loop
	p = 0;

	for (vector<vertex*>::iterator itrV = p_V.begin(); itrV != p_V.end(); ++itrV)
	{
		for (vertex *l = *itrV; l != NULL; l= l->next)
			++p;
		
	}


	cout << "Min cuts are :" << p << endl;
	cout << "ok";

	return 0;
}


vertex* p_createVertex(int l){                //creates a vertex to be added to linked list, label and weight
	vertex* v = new vertex;
	v->label = l;
	v->next = NULL;
	return v;
}

//taking an array of ints to achieve the same...
int vertexCount(){
	//201
	int arr[201] = {0};
	vector<vertex>::iterator itrV;
	bool first = true;
	int i = 0;
	for (vector<vertex*>::iterator itrV = p_V.begin(); itrV != p_V.end(); ++itrV, ++i)
	{
		if(first == true){
			first = false;
			continue;
		}
		if(*itrV != NULL)
			arr[i] = 1;
			
		vertex *x = *itrV; //returns pointer
		for (; x != NULL; x = x->next)
		{
			arr[x->label] = 1;
		}
	}
	int vc = 0;
	//200
	for(int i = 1; i<=200; i++)
		if(arr[i] == 1)
			vc++;
			
	return vc;
	
	
}



//Exhaustive utility functions
void pickAndMerge(){
	int min = 0, max = 0;
	//Now min max will be retreived from the map...
	getUniqueVs(&min, &max);
	//cout << "Min, Max obtained are " << min << "and " << max << endl;
	vertex *vMin = p_V.at(min);
	vertex *vMax = p_V.at(max);
	//no need to check since NULL check is there while extracting random nos.	
	while (vMin->next != NULL)
		vMin = vMin->next;

	//all elements inserted in min. 
	vMin->next = vMax;

	p_V.at(max) = NULL;
	exchangeAllSecondWithFirst(min, max);
	removeSelfLoops(min);
	UpdateEdgeCount();

}


//Add nodes in V from input code...put here
void removeSelfLoops(int m){

	struct vertex* startPointer = p_V.at(m);   //pointer to first element / min element
	struct vertex* temp = NULL;
	while (startPointer != NULL && startPointer->next != NULL){
		if (startPointer->label == m){
			struct vertex* temp = startPointer->next->next;
			startPointer->label = startPointer->next->label;
			delete startPointer->next;
			startPointer->next = temp;

		}
		if (startPointer->label != m)
			startPointer = startPointer->next;

	}
	//for head case
	if (p_V.at(m) != NULL && p_V.at(m)->label == m){
		temp = p_V.at(m);
		p_V.at(m) = temp->next;
		delete temp;
	}
	//tail case	
	startPointer = p_V.at(m);   //pointer to first element / min element
	while (startPointer != NULL && startPointer->next != NULL){
		if (startPointer->next->label == m){
			temp = startPointer->next;  
			startPointer->next = temp->next;
			delete temp;
		}
		startPointer = startPointer->next;
	}
}


void   exchangeAllSecondWithFirst(int min, int max){
	vector<vertex>::iterator itrV;
	for (vector<vertex*>::iterator itrV = p_V.begin(); itrV != p_V.end(); ++itrV)
	{
		vertex *x = *itrV; //returns pointer
		for (; x != NULL; x = x->next)
		{
			if (x->label == max)
				x->label = min;
		}
	}
}

//get and update total number of edges...
void UpdateEdgeCount(){
	int t = 0;
	for (vector<vertex*>::iterator itrV = p_V.begin(); itrV != p_V.end(); ++itrV)
	{
		vertex *x = *itrV; //returns pointer
		for (; x != NULL; x = x->next)
		{
			++t;
                }
	}
	
	g_NodeCount = t;
	//cout<<"Edge count is "<<g_NodeCount<<endl;
}

//NULL check
void getUniqueVs(int *mn, int *mx){
	UpdateEdgeCount();
	srand(time(NULL));
	int rndIndex;
	rndIndex = rand() % g_NodeCount + 1;           //Node count is number of vertices left unique in the graph...
	//cout<<" rand is "<<rndIndex<<endl;
	int lbl = 0;
	int count = 0;
	int i = 0;
	bool first = true;
	for (vector<vertex*>::iterator itrV = p_V.begin(); itrV != p_V.end(); ++itrV, ++i)
	{
		if(first == true){
			first = false;
			continue;
		}	
		vertex *x = *itrV; //returns pointer
		
		for (; x != NULL; x = x->next)
		{
			++count;
			if (count == rndIndex){
				lbl = x->label;
				goto bye;

			}
		}
	}
	bye:
	if (i < lbl){
		*mn = i;
		*mx = lbl;
	}
	else{
		*mn = lbl;
		*mx = i;
	}

}

void checkAndInsert(int index, int value){

	//double loop
	vector<vertex>::iterator itrV;
	int i = 0;
	bool first = true;
	for (vector<vertex*>::iterator itrV = p_V.begin(); itrV != p_V.end(); ++itrV, ++i)
	{	
		if(first == true){
			first = false;
			continue;
		}

		vertex *x = *itrV; //returns pointer
		for (; x != NULL; x = x->next)
		{
			if (i == value && x->label == index)
				return;
		}
	}


	vertex *temp = p_V.at(index);
	p_V.at(index) = p_createVertex(value);
	p_V.at(index)->next = temp;
}
