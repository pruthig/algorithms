#include <iostream>
#include <unordered_map>
#include <climits>
#include <vector>
#include <queue>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <stack>
#include <cmath>
#include <cstdlib>
#include <set>
#include <map>

using namespace std; 


// A structure to represent a job 
struct Job 
{ 
    char id;    // Job Id 
    int dead; // Deadline of job 
    int profit; // Profit if job is over before or on deadline 
}; 

bool comparison(Job a, Job b)  { 
    return (a.profit > b.profit); 
} 

/* Huffman encoding: It is a lossless compression technique in which we encode the input characters. We have to make sure that we are not using 
  the same prefix for characters to avoid ambiguity. For Example, if a is assigned to 0, b is 01 and d is assigned 1. Then 01 can be either ad or b.
  For this we create a max-heap in which the added according to the frequency they are encountered. After adding all nodes, we loop over the nodes of heap 
  till 1 is left. In this operation, we extract 2 nodes and add their frequency and add that node back in heap, making the extracted nodes as children
   to this internal node. We do this extract and combined operation till 1 node is left. After that, we traverse the tree formed starting from the root, 
   maintaining an auxiliary array. While moving to the left child, write 0 to the array. While moving to the right child, write 1 to the array. Print the array when a leaf node is encountered.
/* Job sequencing problem */
vector<int> JobScheduling(Job arr[], int n) 
{ 
    sort(arr, arr+n, comparison);

    vector<int> res{};
    // find max deadline
    int mx_dead = 0;
    for(int i=0;i<n;++i)
        if(arr[i].dead > mx_dead)
            mx_dead = arr[i].dead;
    int *profit_arr = new int[mx_dead+1];
    memset(profit_arr, 0, mx_dead+1);
    
    for(int i=1;i<=mx_dead;++i)
        profit_arr[i] = 0;
        
    // Check slot for each job
    for(int i=0;i<n;++i) {
        for(int j=min(arr[i].dead, n);j>=1;--j) {
            if(profit_arr[j] == 0) {
                profit_arr[j] = arr[i].profit;
                break;
            }
        }

    }
    int count = 0, profit = 0;
    for(int i=1;i<=mx_dead;++i) {
        if(profit_arr[i] != 0) {
            ++count;
            profit += profit_arr[i];
        }
    }
    res.push_back(count);
    res.push_back(profit);
    return res;
}

/*  Activity Selection problem */
bool comparator(const pair<int, int>& p1, const pair<int, int>& p2) {
    return p1.second < p2.second;
}

int maxMeetings(int start[], int end[], int n) {
// Your code here
    vector<pair<int, int>> vec{};
    for(int i=0;i<n;++i)
        vec.push_back(std::make_pair(start[i], end[i]));
        
    sort(vec.begin(), vec.end(), comparator); 
    
    int cur_end = 0, count = 0;
    for(auto a : vec) {
        if(a.first > cur_end) {
            ++count;
            cur_end = a.second;
        }
    }
    return count;
}

int main() {
    return 0;
}