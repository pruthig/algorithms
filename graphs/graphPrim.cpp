/*
 * File:   graphPrim.cpp
 * Author: g.pruthi
 *
 * Created on 10 July, 2014, 11:14 AM
 */


#include <cstdlib>
#include <vector>
#include <set>
#include <iostream>
#include <list>
#include <algorithm>
#include <chrono>


using namespace std;


struct vertex;

int p_findResultantWeight();
bool p_findMinAndDeleteInVAndAddInX() ;
vertex* p_createVertex(int l, int w);
bool vInX(int c);


struct vertex{
        int label;
        int weight;
        vertex *next;
};





vector<vertex*> p_V;
vector<vertex*> p_X;
//create a marked array
bool *p_markedX ;
int p_a, p_b, p_c;                             //input from file
int p_nodes = 0;                         //no. of vertices







/*
 *
 */
void prim() {


	cout<<"Executing Prim Algorithm\n";
	FILE *f = fopen("H:\\stanford\\edges.txt", "r");
    if(f == NULL)
        return;


    
    
    fscanf(f, "%d %d", &p_a, &p_b);              //take the vertices and the edges
    //push NULL and NULL to access 1
    p_V.push_back(NULL);
    p_X.push_back(NULL);
    
	p_nodes = p_a;

    //create a vetices
    for(int i=1; i<=p_nodes; i++){
        vertex *tmp = NULL;
        p_V.push_back(tmp);
    }
         
    //create a vetices
	for(int i=1; i<=p_nodes; i++){
        vertex *tmp = NULL;
        p_X.push_back(tmp);
    }


    p_markedX = new bool[p_a+1];
    p_markedX[0] = true;


    for(int i=1; i<=p_nodes; i++)
            p_markedX[i] = false;                      //initially no element is marked..
        
    while(!feof(f)){
            // tail, head, weight
            fscanf(f, "%d %d %d", &p_a, &p_b, &p_c);
            struct vertex* temp = p_V.at(p_a);
            p_V.at(p_a) = p_createVertex(p_b,p_c);
            p_V.at(p_a)->next = temp;

			temp = p_V.at(p_b);
            p_V.at(p_b) = p_createVertex(p_a,p_c);
            p_V.at(p_b)->next = temp;
              
        }


    bool l = false;
        while (l == false){
            l = p_findMinAndDeleteInVAndAddInX();
        }
		cout<<endl<<"Net result is : "<<p_findResultantWeight()<<endl;
		
        system("pause");
        return;
}


vertex* p_createVertex(int l, int w){                //creates a vertex to be added to linked list, label and weight
        vertex* v = new vertex;
        v->label = l;
        v->weight = w;
        v->next = NULL;
        return v;
}


//Exhaustive utility functions




//Add nodes in V from input code...put here






bool p_addEdgeInLL(int i, int j, int weight){
              


    //return if both are already there in set
    if ( p_markedX[i] == true &&    p_markedX[j] == true)
                    return false;




    bool first = false, second = false;
    //search if i, j and j, i is not added
    struct vertex* startPointer = p_X.at(i);   //pointer to first element
    struct vertex* endPointer = p_X.at(j);
    //cout<<"Coming here uncle1";
    while(startPointer != NULL){
            if(startPointer->label == j)
                    first = true;
            startPointer = startPointer->next;
    }


    while(endPointer != NULL){
            if(endPointer->label == i)
                    second = true;
            endPointer = endPointer->next;
    }


    struct vertex* temp = NULL;
    //cout<<"Coming here uncle2";
    
    if(first == false && second == false)
        {
                //Add the node as first
                    if(p_X.at(i) == NULL)
                            p_X.at(i) = p_createVertex(j, weight);
                    else{
                            temp = p_createVertex(j, weight);
                            temp->next = p_X.at(i);
                            p_X.at(i) = temp;
                    }
                    p_markedX[i] = true;
                    p_markedX[j] = true;
                    //cout<<"Value of deleted labels :  "<<i<<", "<<j<<" with weight : "<<weight<<endl;
                    return true;


    }
    else
                return false;
    //cout<<"Coming here uncle3";
}


//find min edge , remove it from V and add that to X with its weight...
bool p_findMinAndDeleteInVAndAddInX() // vector<vertex> &vecV,  int& record_l, int& record_r, int min=999){              
{


    /*
    //Assume that all are filled
    bool allNull = true;                 //check if atleast one is there then mark it true
    bool allFilled = true;
    for(int i=1; i<=nodes; i++){
            if(X.at(i) != NULL){
                    allNull = false;                           //there are somenodes and not all are false
                    for(int j=1; j<=499; j++){
                            if(X.at(j) == NULL){
                                    allFilled = false;
                                    break;
                            }
                    }


            }
    }




    if(allFilled == true)
            return true;
    */
            //both are there in the already taken set
    bool markCheck = true;
    for(int i = 1; i<=p_nodes; i++){
            if(p_markedX[i] == false){
                    markCheck = false;
                    break;
            }
    }
    if(markCheck == true)
            return true;


    int m = 9999;
    //struct vertex* prev;   //reqd for deletion
    int record_l = 0;
	int record_r = 0;
    vector<vertex>::iterator itrV;

    int count = 1;
    for(; count <= p_nodes; ++count){
		if(count != 1){
			if(vInX(count) == false)
				continue;
		}
        vertex* v = p_V.at(count);
        vertex *x = v;
        for(; x != NULL; x = x->next){

            if(x->weight < m){
                    record_l = count;                   //start label
                    record_r = x->label;               //second label
                    m = x->weight;                    //make x = x->next if not automatically done by loop
            }
        }
    }
    //Deletion code
    struct vertex* temp = p_V.at(record_l);
    struct vertex* p = NULL;
    
    if(temp == NULL){
            //no work
    }
    else if(temp->weight == m) {
            //this is the only node meant for deletion
            p = temp->next;
            delete temp;
            p_V.at(record_l) = p;
    }
    else{
            while(temp->next->weight != m)
                    temp = temp->next;


            struct vertex* p = temp->next;
            temp->next = p->next;


            delete p;
    }


          //Now add the same in X...that updates the vertex too
           p_addEdgeInLL(record_l, record_r, m);
          return false;
             
    
}


int p_findResultantWeight()
{


    long long sum = 0;
    vector<vertex>::iterator itrV;
    for(vector<vertex*>::iterator itrV = p_X.begin(); itrV != p_X.end(); ++itrV)
    {
                    vertex *x = *itrV; //returns pointer
                    for(; x != NULL; x = x->next)
                    {
                    //continue with the outer loop..
                            sum = sum + x->weight;
                    }
    }
    return sum;
}

bool vInX(int c)   {//c being position

	int count = 1;

	//2 loops to find the same
    for(; count <= p_nodes; ++count){

		vertex* v = p_X.at(count);
    vertex *x = v;
    for(; x != NULL; x = x->next){

		if(x->label == c)
			return true;
        }
    }
	
	return false;

}
