#include <iostream>
#include <unordered_map>
#include <climits>
#include <vector>
#include <algorithm>
#include <cstring>

using namespace std;
/*
int count_ones_in_binary_array(int arr[], int start, int end, int n) {
    if(start<end) {
        int mid=(start+end)/2;
        if(arr[mid] == 0) {
            if(mid+1<=n && arr[mid+1]==1) 
                return n-mid;
            else
                return count_ones_in_binary_array(arr, mid+1, end, n);
        }
        else {
            if(mid >= 1 && arr[mid-1] == 0)
                return n-mid+1;
            else
                return count_ones_in_binary_array(arr, start, mid-1, n);
        }
    }
    return 0;
}
int find_floor(int x, int arr[], int start, int end, int n) {
    if(start<=end && start >= 0 && end < n) {
        int mid = (start+end)/2;
        if(arr[mid] == x) {
            return mid;
        }
        else if(arr[mid] < x) {
            if(mid+1 >=n || (mid+1 < n && arr[mid+1]>x))
                return mid;
            else
                return find_floor(x, arr, mid+1, end, n);
        }
        else {
            // x < arr[mid]
            if(mid-1>=0 && arr[mid-1] <=x)
                return mid-1;
            else
                return find_floor(x, arr, start, mid-1, n);
        }
    }
    return -1;
}

void twoRepeated (int arr[], int N)
{
    int first=0, second=0;
    for(int i=0;i<N;++i) {
        if(arr[abs(arr[i])-1] < 0) {
            if(first==0) {
                first = arr[i];
                cout<<first<<endl;
                continue;
            }
            else {
                second=arr[i];
                cout<<second<<endl;
                break;
            }
        }
        arr[abs(arr[i])-1] = -arr[abs(arr[i])-1];
    }    
}

// find the maximum number of consecutive steps you can put forward such
// that you gain an increase in altitude with each step
int maxStep(int A[], int N){
    
   int steps = 0, max_steps = 0;
   if(N==1)
    return 0;
   for(int i=1;i<N;++i) {
       if(A[i]>A[i-1]) {
            ++steps;
            continue;
       }
       else {
           if(steps>max_steps)
                max_steps = steps;
            steps=0;
       }
        
   }
   if(steps>max_steps)
    max_steps = steps;
   return max_steps;
}
int main() {
    int arr[] = { 1, 2, 1, 3, 4, 3 };
    int n = sizeof(arr)/sizeof(arr[0]);
    //cout<<"Count of 1s is: "<<count_ones_in_binary_array(arr, 0, n-1, n-1)<<endl;
    //cout<<"Floor of array is: "<<find_floor(1, arr, 0, n-1, n)<<endl;
    //cout<<"Two repeated elements are: "<<twoRepeated(arr, 6)<<endl;
    //cout<<"Maximum consecutive steps count is:  "<<maxStep(arr, n)<<endl;
    return 0;
}
