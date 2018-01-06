// Given an array, the program is to find the sequence in the form of buy...sell using which
// max profit can be obtained
#include<iostream>
#include<vector>

using namespace std;


int maxProfit(const vector<int> &A) {
    int i = 0;
    int n = A.size();
    if(n == 0)
        return 0;
    int buy = 0, sell = 0, res = 0;
    while(i < n) {
        // Find the local minima
        while(A[i+1] <= A[i] && i < n-1) {
            ++i; continue;
        }

        buy = A[i];
        // Find local maxima    
        while(A[i+1] >= A[i] && i < n-1) {
            ++i; continue;
        }

        /*if(i == n-1  && A[n-1] > buy) {
            res += sell - A[n-1];
            break;
        }
        */

        sell = A[i];

        res = res + (sell - buy);
        ++i;
        buy = 0; sell = 0;
    }
    return res;
}
            

int main() {
    vector<int> v{ 2, 3, 4, 15, 6, 3, 24, 34};
    int res = maxProfit(v);
    cout<<"Result is: "<<res<<endl;
    return 0;
}
