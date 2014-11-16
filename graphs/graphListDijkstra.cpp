/*
 * File:   graphListDijkstra.cpp
 * Author: gaurav.pruthi88@gmail.com
 *
 * Created on 15 Nov., 2014, 11:14 AM
 */


#include <cstdlib>
#include <vector>
#include <queue>
#include <iostream>
#include <climits>

#include <cstdio>

using namespace std;


struct vertex;

void deleteAllEdges(vertex *v);
vertex* p_createVertex(int l, int w);
void doOperation();

void updateEverywhere(int xLabel, int xWeight);
void deleteCorresponding(int originalParent, int originalChild);


struct vertex{
        int label;
        int vertex_weight;
		int edge_weight;
        vertex *next;
};




vector<vertex> p_V;

queue<vertex> q;


/*
 *
 */
int main()
{

	cout<<"Executing Prim Algorithm\n";
	FILE *f = fopen("H:\\stanford\\edges.txt", "r");
    if(f == NULL)
        return 0;


    
   
	int p_a, p_b, p_c; 
    fscanf(f, "%d %d", &p_a, &p_b);              //take the vertices and the edges
    //push NULL and NULL to access 1
    


	for(int i = 0 ; i <= 200; ++i){
		vertex t;
		t.label = i;
	
		if(i == 1)
			t.vertex_weight = 0;
		else
			t.vertex_weight = INT_MAX;

		t.edge_weight = 0;
		p_V.push_back(t);
	}

     
    while(!feof(f)){
			//todo
			//do all vertex weights of vertex - A as 0
            // tail, head, weight
            //fscanf(f, "%d %d %d", &p_a, &p_b, &p_c);
            struct vertex* temp = p_V.at(p_a).next;;
			//edge weight passed (p_A, p_C);
            p_V.at(p_a).next = p_createVertex(p_b,p_c);
            p_V.at(p_a).next->next = temp;


		    temp = p_V.at(p_b).next;;
			//edge weight passed (p_A, p_C);
            p_V.at(p_b).next = p_createVertex(p_a,p_c);
            p_V.at(p_b).next->next = temp;

              
        }

		doOperation();
		
        return 0;
}


vertex* p_createVertex(int l, int w){                //creates a vertex to be added to linked list, label and weight
        vertex* v = new vertex;
        v->label = l;
        v->vertex_weight = INT_MAX;
		v->edge_weight = w;
        v->next = NULL;
        return v;
}


//Exhaustive utility functions


void doOperation(){

	q.push(p_V.at(1));                //first vertex pushed to the Q

	while(!q.empty()){

			vertex v = q.front();
			q.pop();
			int parent = v.label;
			vertex *x = v.next;
			

			//vertex *x = &(p_v.at(i)->next;
			while(x != NULL){
				if(v.vertex_weight + x->edge_weight  < x->vertex_weight){
					x->vertex_weight = v.vertex_weight + x->edge_weight;
					updateEverywhere(x->label, x->vertex_weight);
				}
				q.push(p_V.at(x->label));
				deleteCorresponding(parent, x->label);
				x = x->next;
			}	

			deleteAllEdges(p_V.at(parent).next);
			p_V.at(parent).next = NULL;
	}
}

void deleteCorresponding(int originalParent, int originalChild){
	vertex *x1 = &(p_V.at(originalChild));
	vertex *x2 = x1->next;

	while(x2 != NULL){
		if(x2->label == originalParent){
			vertex *tmp = x2->next;
			x1->next = tmp;
			delete x2;
			return;
		}
		x1 = x1->next;
		x2 = x2->next;
		
	}
}



	
	
	
void updateEverywhere(int xLabel, int xWeight){
	bool first = true;

   for(vector<vertex>::iterator itrV = p_V.begin(); itrV != p_V.end(); ++itrV)
    {
					if(first == true){
						first = false;
						continue;
					}
		
                    vertex *x = &(*itrV); //returns pointer
                    for(; x != NULL; x = x->next)
                    {
						if(x->label == xLabel)	
							x->vertex_weight = xWeight;
                    }
    }
}
 
//delete all edges for a corresponding row
void deleteAllEdges(vertex *v){
	if(v == NULL)
		return;
	
	deleteAllEdges(v->next);
	delete v;
}

/*
bool allTraversed(){
	for(int i = 1;i <= 200; ++i)
		if(traversed[i] == false)
			return false;

	return true;
}

*/
