#include<iostream>
#include<vector>

using namespace std;

int findPosition(vector<double> &arr, double target) {
            int n = arr.size();
            int start = 0, end = n - 1;
            int mid;
            while(start <= end){
                mid = (start + end) / 2;
                if(target == arr[mid]){
                    return mid;
                }
                else if(target < arr[mid]){
                    end = mid - 1;
                }
                else{
                    start = mid + 1;
                }
            }
			return start;
}

int main() {
	vector<double> A{0,0,0,1,1,1,1,1,1,1,6,6};
	vector<double> arr(A.begin(), A.end());
	double n = 1;
	int start = 0, end = arr.size()-1;
	vector<int> res;
	int result = -1;

	int left = findPosition(arr, n-0.5);
	int right = findPosition(arr, n+0.5);
	if(left == right) {
		res.push_back(-1);
		res.push_back(-1);

	}
	else
	{
		res.push_back(left);
		res.push_back(--right);
	}

	for(auto arr : res)
		cout<<"arr : "<<arr;
}


