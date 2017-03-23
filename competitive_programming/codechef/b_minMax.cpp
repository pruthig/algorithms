//This problem finds the solution where we find the smallest elements and repeatedly removes the 
//adjacent largest
#include<iostream>
#include<algorithm>
#include<vector>

using namespace std;



int main(){
vector<long int> vec;
int count;
long int element, nElements = 0;

cin>>count;
    //testCase count
    for(int j = 1; j <= count; ++j){
        //Take in the number of elements...
        cin>>nElements;
        for(int i = 1; i <= nElements; ++i){
            cin>>element;
            vec.push_back(element);
        }
        //find min in vector 
        vector<long int>::iterator itr = std::min_element( vec.begin(), vec.end());
        cout<<((long long)(*itr)*(long long)(nElements - 1))<<endl;
        
    }
return 0;
}
    
    

