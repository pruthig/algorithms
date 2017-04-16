/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include<iostream>
#include <list>
#include<vector>
#include <queue>

using namespace std;

const int size = 6;
static std::vector< std::list<int> > graph(size);


static void construct_graph() {
    //list<int> l = graph.at(0);
   
    graph.at(1).push_back(2);
    
    graph.at(2).push_back(1);
    graph.at(2).push_back(3);
    graph.at(2).push_back(4);
    
    graph.at(3).push_back(2);
    graph.at(3).push_back(5);
    
    graph.at(4).push_back(2);
    
    graph.at(5).push_back(3);
    graph.at(5).push_back(6);
    
    graph.at(6).push_back(5);
   
}

bool is_connected() {
    // size+1 since array starts from 0
    bool marked[size+1] = { false, };
    std::queue<int> queue;
    std::list<int> temp_list;
    queue.push(1);
    
    while(!queue.empty()) {
        int x = queue.front();
        queue.pop();
        temp_list = graph.at(x);
        marked[x] = true; // Mark it and push its neighbors in list
        while(!temp_list.empty()) {
            if(marked[temp_list.back()] = false) {
                queue.push(temp_list.back());
            }
            temp_list.pop_back();          // remove the popped element
        }
        
    }
    
    
    
}
void dijkstra_main(){
    //std::vector< std::list<int> > graph[size];  // A data structure to hold a tree
    construct_graph();
}
