#include <iostream>
#include <unordered_map>
#include <climits>
#include <vector>
#include <queue>
#include <algorithm>
#include <stack>
#include <cmath>
#include <set>
#include <map>
#include <cmath>

using namespace std; 

struct Node
{
    int data;
    struct Node* left;
    struct Node* right;
    
    Node(int x){
        data = x;
        left = right = NULL;
    }
};

class MedianOfRunningStream{
    priority_queue<int, vector<int>, greater<int>> min_heap{};
    priority_queue<int> max_heap{};
    
    public:
    // Function to insert heap
    void insertHeap(int &x)
    {
        if(max_heap.empty()) {
            max_heap.push(x);
            return;
        }
        if(x<=max_heap.top())
            max_heap.push(x);
        else
            min_heap.push(x);
        balanceHeaps();
    }
    
    // Function to balance heaps
    void balanceHeaps()
    {
        if(abs((int)min_heap.size()-(int)max_heap.size()) <= 1)
            return;
        if(min_heap.size() < max_heap.size()) {
            int mxtop = max_heap.top();
            max_heap.pop();
            min_heap.push(mxtop);
        }
        else {
            int mintop = min_heap.top();
            min_heap.pop();
            max_heap.push(mintop);
        }
    }
    
    // Function to return getMedian
    double getMedian()
    {
        int minsz = min_heap.size();
        int maxsz = max_heap.size();
        if(!minsz && !maxsz)
            return -1;
        if(minsz>maxsz)
            return min_heap.top();
        else if(maxsz>minsz)
            return max_heap.top();
        else
            return (min_heap.top()+max_heap.top())/2;
    }
};

vector <int> nearlySorted(int arr[], int num, int K){
    vector<int> vec{};
    if(K>num)
        return vec;
    priority_queue<int, vector<int>, greater<int>> pq{};
    if(num == K)
        K -= 1;
    for(int i=0;i<=K;++i) {
        pq.push(arr[i]);
    }
    for(int i=K+1;i<num;++i) {
        int elem = pq.top();
        pq.pop();
        vec.push_back(elem);
        pq.push(arr[i]);
    }
    
    while(!pq.empty()) {
        int elem = pq.top();
        vec.push_back(elem);
        pq.pop();
    }
    return vec;   
}

int kMostFrequent(int arr[], int n, int k) 
{ 
    unordered_map<int, int> mp{};
    priority_queue<int> pq{};
    for(int i=0;i<n;++i) {
        if(mp.find(arr[i]) != mp.end()) {
            mp[arr[i]]++;
        }
        else
            mp[arr[i]] = 1;
    }	
    for(auto a: mp) {
        pq.push(a.second);
    }
    int sum = 0;
    for(int i=0; !pq.empty() && i<k;++i) {
        sum += pq.top();
        pq.pop();
    }
    return sum;
} 

long long minCost(long long arr[], long long n) {
    priority_queue<long long, vector<long long>, greater<long long>> pq{};
    for(int i=0;i<n;++i) {
        pq.push(arr[i]);
    }
    long long  sum = 0;
    while(pq.size() != 1) {
        long long e1 = pq.top();
        pq.pop();
        long long e2 = pq.top();
        pq.pop();
        pq.push(e1+e2);
        sum += (e1+e2);
    }
    return sum;
}

/* create a functionality for scheduling meetings. You need to determine and block the minimum number of meeting rooms for these meetings.
meeting times can be listed as follows: {{2, 8}, {3, 4}, {3, 9}, {5, 11}, {8, 20}, {11, 15}}. We observe that three meetings overlap: {2, 8}, {3, 4}, and {3, 9}. Therefore, at least these three will require separate rooms. */
//Steps to solve this:
//1. Sort the meetings by startTime.
//2. Allocate the first meeting to a room. Add the endTime as an entry in the heap.
//3. Traverse the other meetings and check if the meeting at the top has ended.
//4. If the room is free, extract this element, add it to the heap again with the ending time of the current meeting we want to process. If it is not free,allocate a new room and add it to our heap.
//5. After processing the list of meetings, the size of the heap will tell us how many rooms are allocated. This should be the minimum number of rooms we need.

int main() {
    int arr[] = { 3, 1, 4, 4, 5, 2, 6, 1 };
    int n = sizeof(arr)/sizeof(arr[0]);
    cout<<"K most occuring element with queue size 2 is: "<<kMostFrequent(arr, n, 2)<<endl;
    return 0;
}
