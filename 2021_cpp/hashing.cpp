#include <unordered_map>
#include <climits>
#include <iostream>
#include <vector>
#include <set>
#include <list>
#include <map>

using namespace std;

// Check if two arrays are equal or not
bool check(vector<int> arr, vector<int> brr, int n) {
    //code here
    unordered_map<int,  int> mp{};
    for(int i=0;i<n;++i) {
        if(mp.find(arr[i]) != mp.end()) {
            mp[arr[i]]++;
        }
        else
            mp[arr[i]] = 1;
    }
    for(int i=0;i<n;++i) {
        if(mp.find(brr[i]) != mp.end()) {
            if(mp[brr[i]]>1)
                mp[brr[i]] = mp[brr[i]]-1;
            else
                mp.erase(brr[i]);
        }
    }
    if(mp.empty())    
        return true;
    else
        return false;
}

// Subarray range with given sum
int subArraySum(int arr[], int n, int sum)
{
    //Your code here
    unordered_map<int, int> mp{};
    int occurence = 0;
    int cur_sum = 0;
    mp[0] = 1;
    for(int i=0;i<n;++i) {
        cur_sum += arr[i];
        if(mp.find(cur_sum-sum) != mp.end())
            occurence += mp[cur_sum-sum];
        if(mp.find(cur_sum) != mp.end()) {
            mp[cur_sum]++;
        }
        else
            mp[cur_sum] = 1;
    }
    return occurence;
}

int firstRepeated(int arr[], int n) {
    //code here
    unordered_map<int,  int> mp{};
    for(int i=0;i<n;++i) {
        if(mp.find(arr[i]) != mp.end()) {
            return mp[arr[i]];
        }
        mp.insert(std::make_pair(arr[i], i));
    }
    return -1;
}
// Subarray with 0 sum
bool subArrayExists(int arr[], int n)
{
    set<int> st{};
    if((n ==1 && arr[0] == 0 ) || 
       (n > 1 && arr[0] + arr[1] == 0))
        return true;
    st.insert(arr[0]);
    for(int i=1;i<n;++i) {
        arr[i] = arr[i] + arr[i-1];
        if(st.find(arr[i]) != st.end())
            return true;
        st.insert(arr[i]);
    }
    return false;
}

// Count Zero Sum Subarrays
int findSubarray(vector<int> arr, int n ) {
    unordered_map<int, int> mp{};
    int cur_sum = 0;

    int counter  = 0;
    for(int i=0;i<n;++i) {
        cur_sum += arr[i];
        if(cur_sum == 0)
            ++counter;
        if(mp.find(cur_sum) != mp.end()) {
            counter += mp[cur_sum];
            ++mp[cur_sum];
        }
        else
            mp[cur_sum] = 1;
    }
    return counter;
}

// Subarrays with equal 1s and 0s
int subarrays_with_equal_1s_0s_count(int arr[], int n ) {
    unordered_map<int, int> mp{};
    int cur_sum = 0;
    int counter  = 0;
    for(int i=0;i<n;++i)
        if(arr[i] == 0)
            arr[i] = -1;
            
    for(int i=0;i<n;++i) {
        cur_sum += arr[i];
        if(cur_sum == 0)
            ++counter;
        if(mp.find(cur_sum) != mp.end()) {
            counter += mp[cur_sum];
            ++mp[cur_sum];
        }
        else
            mp[cur_sum] = 1;
    }
    return counter;
}

// Sort an array according to the other
vector<int> sortA1ByA2(vector<int> A1, int N, vector<int> A2, int M) {
    map<int, int> mp{};
    vector<int> res{};
    for(int i=0;i<N;++i) {
        if(mp.find(A1[i]) != mp.end()) {
            ++mp[A1[i]];
        }
        else
            mp[A1[i]] = 1;
    }
    // Now loop over 2nd array
    for(int j=0;j<M;++j) {
        if(mp.find(A2[j]) != mp.end()) {
            for(int k=0;k<mp[A2[j]]; ++k)
                res.push_back(A2[j]);
            mp.erase(A2[j]);
        }
    }
    for(auto a:mp) {
        for(int i=0;i<a.second;++i)
            res.push_back(a.first);
        mp.erase(a.first);
    }
    return res;
} 

// Sorting Elements of an Array by Frequency 
vector<int> sortByFreq(int arr[],int n)
{
    // this map is about number -> frequency
    map<int, int> mp{};
    vector<int> res{};

    for(int i=0;i<n;++i) {
        if(mp.find(arr[i]) != mp.end()) {
            mp[arr[i]]++;
        }
        else
            mp[arr[i]] = 1;
    }
    // now use another map for frequency->number
    map<int, list<int> > mp2{};
    for(auto a: mp) {
        mp2[a.second].push_back(a.first);
    }
    //map<int, list<int>>::reverse_iterator iter{};
    for(auto iter=mp2.rbegin(); iter != mp2.rend(); ++iter) {
        list<int> lst = iter->second;
        int k = iter->first;
        for(auto a: lst) {
            for(int i=0;i<k;++i) {
                res.push_back(a);
            }
        }
    }
    return res;
}

// Find the length of the longest sub-sequence such that elements in the 
// subsequence are consecutive integers
int findLongestConseqSubseq(int arr[], int N)
{
  //Your code here
  set<int> st{};
  int mx = INT_MIN;
  int cur_sum = 0, sum = 0;
  for(int i=0; i<N;++i) {
      if(mx<arr[i])
        mx = arr[i];
      st.insert(arr[i]);
  }
  for(int i=0;i<mx;++i) {
      if(st.find(i+1) != st.end()) {
          cur_sum += (i+1);
      }
      else {
        if(sum<cur_sum)
            sum = cur_sum;
        cur_sum = 0;
      }
  }
  if(sum<cur_sum)
    sum = cur_sum;
  return sum;
}

int main() {
    int arr[] = { 1, 3, 2, 10, 9, 11, 25, 12};
    int n = sizeof(arr)/sizeof(arr[0]);
    //cout<<firstRepeated(arr, n)<<endl;
    //cout<<subArrayExists(arr, n)<<endl;
    cout<<findLongestConseqSubseq(arr, n);
    return 0;
}
