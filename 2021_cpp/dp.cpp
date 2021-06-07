#include <iostream>
#include <unordered_map>
#include <climits>
#include <vector>
#include <algorithm>
#include <cstring>

using namespace std;


// Check if an array can be partitioned into two parts such that 
// the sum of elements in both parts is the same.
/* recursive solution:
if (arr[n - 1] > sum) return isSubsetSum(arr, n - 1, sum);
else return isSubsetSum (arr, n-1, sum/2) || isSubsetSum (arr, n-1, sum/2 - arr[n-1])
*/   
int IsEqualPartitionPossible(int arr[], int N)
{
    if(N == 0)
        return 0;
    int sum = 0;
    for (int i = 0; i < N; i++)
        sum += arr[i];
    int required_sum = sum/2;
    if(sum%2 == 1)
        return 0;
    // No create table of (N+1, required_sum + 1) 
    int **dp = new int*[N+1];
    for(int i=0;i<=N;++i) {
        dp[i] = new int[required_sum + 1];
        memset(dp[i], 0, required_sum + 1);
    }
    for(int i=0;i<=(required_sum); ++i) {
        dp[0][i] = 0;
    }
    for(int i=0;i<=N; ++i) {
        dp[i][0] = 1;
    }

    for(int i=1;i<=N;++i) {
        for(int j=1;j<=required_sum;++j) {
            // We have got the sum already so picked as such and continue
            if(dp[i-1][j] == 1) {
                dp[i][j]  = 1;
                continue;
            }
            if(arr[i-1] > j) {
                continue;
            }
            else {
                // Picked this item
                dp[i][j] = dp[i-1][j-arr[i-1]];
                /*
                In case of 0-1 knapsack following statement will be used
                max(dp[i-1][j], arr[i] + dp[i-1][j-arr[i-1]]
                In case of unbounded knapsack following statement will be used
                max(dp[i-1][j], arr[i] + dp[i][j-arr[i-1]]
                */
            }
        }
    }
    return dp[N][required_sum];
}

/*
Variations:
>> To find minimum sum between 2 partitions - search for subset having sum = total_sum/2
   If found, min sum is 0, else look on left and search for first true, then let the position
   be dp[i][j] then (total_sum - dp[i][j]) will be total sum

>> For equal partitions also, use the same algorithm
>> Knapsack too is based on above idea.
*/

// 0-1 knapsack - subset sum, target sum, equal sum partition
// Unbounded knapsack - Rod-cutting, Coin change
// Fibonacci - 
// LCS - Longest common subsequence, Shortest common subsequence, Minimum insertion and deletion to make one string to another
// Kinda pattern matching, Longest Palindromic Subsequence (take string and do its reverse, not find common substring amongst them). For Insertion and Deletion also, find LCS and decrease it from total_length. Minimum number of deletion to make a string palindrome. Longest repetiting subsequence - duplicate that string again, now again find LCS given the fact that i!= j while matching a[i-1] == b[j-1].
//// Min no. of insertion in string to make it palindrome - duplicate and reverse. find lcs. total length of string - lcs lenght is the answer.
//// Is A present in B as subsequence.
// LIS -
// Kadane's algorithm - Maximum sum rectangle in a 2D matrix - Use 2 loops for traversing column-wise and for each start of left, memset(temp, 0, ROW) and do, for each left and right column:
// Calculate sum between current left and right for every row 'i'. For each left, memset temp to 0 and do following:
//         for (i = 0; i < ROW; ++i)
//                temp[i] += M[i][right];  
// after this run modified kadane's algorithm which will return max size sub-array with start and finish indices. O(n^3)
// Matrix Chain Multiplication - 
// DP on trees - 
// DP on grid - 
// Others - 


// rod-cutting problem



/*
>> Rod-cutting is also variation of aforementioned problem. It's kind of unbounded knapsack
wherein length of rod is equivalent to weights.
>> Coin change problem is also type of unbounded knapsack
*/

int min_jumps(int arr[], int *counter_arr, int count, int idx, int sz) {
            
    if(idx == sz-1)
        return count;
        
    if(counter_arr[idx] != -1)
        return counter_arr[idx];
    
    int min_c = INT_MAX;
    for(int j=1;j<=arr[idx];++j) {
        int cnt = min_jumps(arr, counter_arr, count+1, idx+j, sz);
        min_c = min(min_c, cnt);
    }
    counter_arr[idx] = min_c;
    return min_c;
}

int MaxCoinPickedValue(int arr[], int i, int j, int sz) {
    if(i==j || i<0 || i>=sz || j < 0 || j>=sz)
        return 0;
    else {
        int mx_val = max(arr[i]+MaxCoinPickedValue(arr, i+1, j, sz), arr[j]+MaxCoinPickedValue(arr, i+1, j, sz));
        return mx_val;
    }
}

int mcm_dp_arr[6][6];

// Matrix chain multiplication
int mcm(int arr[], int i, int j, int sz) {
    if(i>=j || i<0 || j>=sz) {
        //mcm_dp_arr[i][j] = 0;
        return 0;
    }
    if(j==i+1)
        return arr[i-1]*arr[i]*arr[j];

    if(mcm_dp_arr[i][j] != -1)
        return mcm_dp_arr[i][j];
    
    int mn = INT_MAX;
    for(int k=i;k<j;++k) {
        int res = mcm(arr, i, k, sz) + mcm(arr, k+1, j, sz) + arr[i-1]*arr[k]*arr[j];
        mn = min(res, mn);
    }
    mcm_dp_arr[i][j] = mn;
    return mn;
}
/*
 MCM - Palindrome partition from i to j, k can take up any position.
 Implement palindrome function.
*/
int palindrome_partition(string s, int i, int j) {
    if(i>=j) {
        return 0;
    }
    if(mcm_dp_arr[i][j] != -1)
        return mcm_dp_arr[i][j];
    
    //@ToDo
    //if(isPalindrome(s, i, j) == 0)
    //    return 0;
        
    int mn = INT_MAX;
    for(int k=i;k<=j-1;++k) {
        int res = palindrome_partition(s, i, k-1) + 1 + palindrome_partition(s, k+1, j);
        mn = min(res, mn);
    }
    mcm_dp_arr[i][j] = mn;
    return mn;
}

// Notes:
// LIS can be solved in nlogn using patience sorting
// Variations of LIS: 
// Maximum Sum Increasing Subsequence: Given an array of n positive integers. Write a program to find the maximum sum subsequence of the given array such that the intgers in the subsequence are sorted in increasing order. For example, if input is {1, 101, 2, 3, 100, 4, 5}, then output should be {1, 2, 3, 100}.
// The Longest Chain You are given pairs of numbers. In a pair, the first number is smaller with respect to the second number. Suppose you have two sets (a, b) and (c, d), the second set can follow the first set if b < c. So you can form a long chain in the similar fashion. Find the longest chain which can be formed
// Box Stacking: You are given a set of n types of rectangular 3-D boxes, where the i^th box has height h(i), width w(i) and depth d(i) (all real numbers). You want to create a stack of boxes which is as tall as possible, but you can only stack a box on top of another box if the dimensions of the 2-D base of the lower box are each strictly larger than those of the 2-D base of the higher box. Create an array 3*n which will include all rotations of all boxes and run LIS based on base area -> length*width
// Building bridges: 
// Consider a 2-D map with a horizontal river passing through its center. There are n cities on the southern bank with x-coordinates a(1) … a(n) and n cities on the northern bank with x-coordinates b(1) … b(n). You want to connect as many north-south pairs of cities as possible with bridges such that no two bridges cross. When connecting cities, you can only connect city a(i) on the northern bank to city b(i) on the southern bank. Maximum number of bridges that can be built to connect north-south pairs with the aforementioned constraints.
// Solution: Sort the points w.r.t Southern coordinates and then find LIS of northern coordinates
// if there are duplicates in southern coordinates, sort the corresponding pair of northern coordinates.
// Adobe question:
//You are given a set of n rectangular envelopes, where the ith envelope has length l(i) and width w(i) (all natural numbers). You have to create a stack of envelopes which is as tall as possible, but you can only stack an envelope on top of another envelope if the dimensions of the lower envelope are each strictly larger than those of the higher envelope. For simplicity, you cannot rotate an envelope. And, you are not allowed to use multiple instances of the same envelope.

int main() {
    int arr[] = { 1, 2, 1, 1, 6 };
    int sz = sizeof(arr)/sizeof(arr[0]);
    int *counter_arr = new int[sz];
    for(int i=0;i<sz;++i)
        counter_arr[i] = -1;
        
    //cout<<IsEqualPartitionPossible(arr, 4)<<endl;
    //cout<<MaxCoinPickedValue(arr, 0, sizeof(arr)-1, sizeof(arr)/sizeof(arr[0]))<<endl;
    /* min_jumps(arr, counter_arr, 0, 0, sz);
    int mn_jmps = INT_MAX;
    for(int i=0;i<sz; ++i) {
        if(counter_arr[i]<mn_jmps) {
            mn_jmps = counter_arr[i];
        }
    }
    cout<<"Min jumps required are: "<<mn_jmps<<endl;
    */
    memset(mcm_dp_arr, -1, sizeof(mcm_dp_arr));
    
    cout<<"Minimum cost in matrix chain multiplication is: "<<mcm(arr, 1, sz-1, sz)<<endl; //mcm_dp_arr[sz][sz]<<endl;
    return 0;
}
    
