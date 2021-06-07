#include <iostream>
#include <unordered_map>
#include <climits>
#include <vector>
#include <algorithm>
#include <cstring>

using namespace std;
/*
int bin_search(int *arr, int start, int end, int element) {
    if(start<=end) {
        int mid = start + (end-start)/2;
        if(element == arr[mid])
            return 1;
        else if(element<arr[mid])
            return bin_search(arr, start, mid-1, element);
        else
            return bin_search(arr, mid+1, end, element);
    }
    return -1;
}
*/
/*
int kadane(int a[], int n) {
    int sum=0,max_till_here=0;
    for(int i=0;i<n;++i) {
        max_till_here+=a[i];
        if(max_till_here<0) {
            max_till_here=0;
            continue;
        }
        if(max_till_here>sum) {
            sum = max_till_here;
        }
    }
    return sum;
}
*/
/*
void vector_reverse(vector<int>& vec, int n) {
    for(int i=0;i<n/2;++i){
        std::swap(vec[i], vec[n-i-1]);
    }
}
*/
/*
void reverse(vector<int>& arr, int start, int end) {
    if(start>=end)
        return;
    int mid = start+(end-start)/2;
    for(int i=start;i<=mid;++i) {
        cout<<"Start,Mid,End: "<<start<<" "<<mid<<" "<<end<<endl;
        std::swap(arr[i], arr[end+start-i]);
    }
}
void reverseInGroups(vector<int>& arr, int n, int k){
    // code here
    // n=5,k=3
    int start=0;
    while(start+k-1<=n-1) {
        reverse(arr, start, start+k-1);
        start+=k;
    }
    reverse(arr, start, n-1);
}
*/
/*
void arr_rotation_counter_clockwise(vector<int>& vec, int index, int N, int D) {
    if(index >= N)
        return;
    int val = vec[index];
    arr_rotation_counter_clockwise(vec, index+1, N, D);
    vec[(index+N-D)%N] = val;
}
*/
/*
void convertToWave(int *arr, int n){
    bool greater = true;
    for(int i=0;i<n-1;++i) {
        if(greater) {
            if(arr[i]<arr[i+1])
                swap(arr[i], arr[i+1]);
        }
        else {
            if(arr[i]>arr[i+1])
                swap(arr[i], arr[i+1]);
        }
        greater = !greater;
    }
}
*/
/*
void frequencycount(vector<int>& arr,int n) {
  // Subtract 1 from every element so that the elements
  // become in range from 0 to n-1
  for (int i = 0; i < n; i++)
    arr[i] = arr[i] - 1;

  // Use every element arr[i] as index and add 'n' to
  // element present at arr[i]%n to keep track of count of
  //  occurrences of arr[i]
  for (int i = 0; i < n; i++)
    arr[arr[i] % n] += n;

  // To print counts, simply print the number of times n
  // was added at index corresponding to every element
  for (int i = 0; i < n; i++)
    arr[i] = arr[i] / n;
}
*/
/*
int equilibriumPoint(int arr[], int n) {

  int total_sum = 0, cur_sum_before_this = 0, leftover_sum = 0;

  for (int i = 0; i < n; ++i)
    total_sum += arr[i];

  for (int i = 0; i < n; ++i) {
    leftover_sum = total_sum - (cur_sum_before_this + arr[i]);
    if (cur_sum_before_this == leftover_sum) {
      return i+1;
    }
    cur_sum_before_this += arr[i];
  }
  return -1;
}
*/
/*
int trapped_water(int arr[], int n) {
	int trapped_water = 0;
	int *lmax = new int[n];
	int *rmax = new int[n];
	
	lmax[0]=arr[0];
	for(int i=1;i<n;++i) {
		lmax[i] = max(lmax[i-1],  arr[i]);
	}
	rmax[n-1]=arr[n-1];
	for(int i=n-2;i>=0;--i) {
		rmax[i] = max(rmax[i+1],  arr[i]);
	}
	for(int i=0;i<n;++i)
		trapped_water += abs( min(lmax[i], rmax[i])-arr[i] );
		
	return trapped_water;
}
*/
/*
int stock_buy_sell(int arr[], int n) {
	int *aux = new int[n];
	memset(aux, 0, n);
	aux[0] = 0;
	if(arr[0]<arr[1]) {
		aux[1] = arr[1]-arr[0];
	}
	else {
		aux[1] = 0;
	}
	for(int i=2;i<n;++i) {
		int mx = INT_MIN;
		for(int j=0;j<i;++j) {
			// let j be 2nd array
			if(j!=0) {
				if(aux[j-1] == 0)
					continue;
			}
			if(arr[i]<arr[j])
				continue;
			int remnant = j>0?aux[j-1]:0;
			int sum = arr[i]-arr[j];
			if(sum+remnant > mx)
				mx = sum+remnant;
		}
		aux[i] = mx;
	}
	return aux[n-1];
}
*/
/* Function to find first smallest positive
 missing number in the array
int missingNumber(int a[], int n) {

    // Your code here
    for(int i=0;i<n;++i) {
        if(a[i]>=1 && a[i]<=n) {
            int elem=a[i];
            while(elem>=1 && elem<=n && a[elem-1] != elem) {
                int bk = a[elem-1];
                a[elem-1] = elem;
                elem = bk;
            }
        }
    }
    for(int i=0;i<n;++i) {
        if(a[i] != i+1)
            return i+1;
    }
    return n+1;
}
*/
/*
int findMaxDiff(int A[], int n)
{
  if(!n) return 0;
  int *l = new int[n];
  int *r =new int[n];
  int l_min=A[0];
  int r_min = A[n-1];

  l[0]=0;
  r[n-1]=0;

  for(int i=1;i<n;++i){
      int element = A[i]; int dif = INT_MAX;
      int res = INT_MAX;
      for(int j=0;j<i;++j) {
          if(A[j] >= A[i])
            continue;
          if(abs(A[j]-element) < dif) {
            dif = abs(A[j]-element);
            res = A[j];
          }
      }
      if(dif == INT_MAX)
        l[i] = 0;
      else
        l[i]=res;
  }
  for(int i=n-2;i>=0;--i){
      int element = A[i]; int dif = INT_MAX;
      int res = INT_MAX;
      for(int j=n-1;j>i;--j) {
          if(A[j] >= A[i])
            continue;
          if(abs(A[j]-element) < dif) {
            dif = abs(A[j]-element);
            res = A[j];
          }
      }
      if(dif == INT_MAX)
        r[i] = 0;
      else
      r[i]=res;
  }
  int mx_dif = INT_MIN;
  for(int i=0;i<n;++i) {
      cout<<"l[i] and r[i] are: "<<l[i]<<" "<<r[i]<<endl;
      if(abs(l[i]-r[i]) > mx_dif)
        mx_dif = abs(l[i]-r[i]);
  }
  return mx_dif;
}			
*
/*
ector<long long int> productExceptSelf(vector<long long int>& nums, int n) {
   long double res=0.0, tmp=0.0;
   vector<long long int> t_vec{};
   for(int i=0;i<n;++i) {
       //cout<<"log is "<<log(nums[i])<<"   res after logs is "<<res<<endl;
       res += log(nums[i]);
   }
   for(int i=0;i<n;i++) {
       tmp = res - logl(nums[i]);
       //cout<<"tmp is: "<<exp(tmp)<<endl;
       t_vec.push_back((long long int)exp(tmp));
   }
   return t_vec;
}
*/

#include <bits/stdc++.h>
using namespace std;

/* 
// For a given array arr[],
//returns the maximum j – i such that
//arr[j] > arr[i] 
int maxIndexDiff(int arr[], int n)
{
	int maxDiff;
	int i, j;

	int* LMin = new int[(sizeof(int) * n)];
	int* RMax = new int[(sizeof(int) * n)];

	LMin[0] = arr[0];
	for (i = 1; i < n; ++i)
		LMin[i] = min(arr[i], LMin[i - 1]);

	RMax[n - 1] = arr[n - 1];
	for (j = n - 2; j >= 0; --j)
		RMax[j] = max(arr[j], RMax[j + 1]);

	i = 0, j = 0, maxDiff = -1;
	while (j < n && i < n) {
		if (LMin[i] < RMax[j]) {
			maxDiff = max(maxDiff, j - i);
			j = j + 1;
		}
		else
			i = i + 1;
	}
	return maxDiff;
}
*/

/*
int getPairsCount(int arr[], int n, int k) {
    unordered_map<int, int> s{};
    int count = 0;
    for(int i=0;i<n;++i) {
        if(s.find(k-arr[i]) != s.end())
            count += s[k-arr[i]];
        if(s.find(arr[i]) != s.end())
            s[arr[i]]++;
        if(s.find(arr[i]) == s.end())
            s[arr[i]] = 1;
    }
    return count;
}
*/
/*
int findZeroes(int arr[], int n, int m) {
    int max_n = INT_MIN;
    int start=0, n0 = 0;
    int i=0;
    while(i<n) {
        if(arr[i] == 0)
            ++n0;
        if(n0 > m) {
            while(arr[start++] != 0); //start at fine position
            --n0;
        }
        if(i-start+1 > max_n)
            max_n = i-start+1;
        ++i;

    }
    return max_n;
}  
*/
/* 
// Find majority element. Majority element is the element whose count is >N/2 in array
int majorityElement(int a[], int size) {
    int major_element = a[0];
    int count = 1;
    bool found = false;
    for(int i=1;i<size;++i) {
        if(a[i] == major_element) {
            ++count;
            found = true;
        }
        else {
            if(count >1)
                --count;
            else {
               major_element = a[i];
               found = false;
               count = 1;
            }
        }
    }
    //double check
    int c = 0;
    for(int i=0;i<size;++i) {
        if(a[i] == major_element)
            ++c;
    }
    if(c > size/2)
        return major_element;
    else
        return -1;
}
*/
/*
vector<int> SortBinaryArray(vector<int> a)
{
    int j=0;
    for(int i=0;i<a.size();++i) {
        if(a[i] == 0) {
                swap(a[i], a[j]);
                ++j;
        }
    }
    return a;
}
*/
/*
int minDist(int a[], int n, int x, int y) {
    // code here
    int pos_x=-1,pos_y=-1,min_dif=INT_MAX;
    for(int i=0;i<n; ++i) {
        if(a[i] == x) {
            pos_x=i;
            //cout<<"pos_x is: "<<pos_x<<endl;
        }
        if(a[i] == y) {
            pos_y=i;
            //cout<<"pos_y is: "<<pos_y<<endl;
        }
        if(pos_x != -1 && pos_y != -1 &&
            abs(pos_y - pos_x) < min_dif) 
            min_dif = abs(pos_y - pos_x);
    }
    if(pos_x == -1 || pos_y == -1)
        return -1;
    else
        return min_dif;
}
*/
/*
int FindMaxSum(int arr[], int n)
{
    // Your code here
    int *sum = new int[n];
    memset(sum, 0, n);
    sum[0] = arr[0];
    sum[1] = arr[1];
    sum[2] = arr[0] + arr[2];
    for(int i=3;i<n;++i) {
        sum[i] = arr[i] + max(sum[i-2], sum[i-3]);
    }
    return max(sum[n-1], sum[n-2]);
}
*/
/*
void rearrange_alternate(int arr[], int n) {
  int min_index = 0, max_index = n-1;
  int max_element = arr[n-1] + 1;

  // Update the element.. for odd_index, update the minimum element
  // and for even index update for the max element;

  for(int i=0;i<n;++i) {
    if(i%2 == 0) {
      arr[i] += (arr[max_index]%max_element)*max_element;
      max_index--;
    }
    else {
      arr[i] += (arr[min_index]%max_element)*max_element;
      min_index++;
    }
  }
  // Update the array
  for(int i=0;i<n;++i) {
    arr[i] = arr[i]/max_element;
  }
}
*/
/*
// Find a starting point where the truck can start to get through the complete circle 
// without exhausting its petrol in between
struct petrolPump
{
    int petrol;
    int distance;
};

int tour(petrolPump p[],int n)
{
   //Your code here
   int res_index = -1;
   int i=0;
   for(;i<n;++i) {
       if(p[i].petrol - p[i].distance < 0)
            continue;
        else {
            int cur_sum = p[i].petrol - p[i].distance;
            int j = 1;
            for(;j<n;++j) {
                cur_sum += p[(i+j)%n].petrol - p[(i+j)%n].distance;
                if(cur_sum<0)
                    break;
            }
            if(j == n && cur_sum >= 0)
                return i;
        }
   }
   return res_index;
}
*/

/*
// Program to search an element in row-wise
// and column-wise sorted matrix
int searchInColAndRowWiseSortedMatrix(int mat[4][4], int n, int x)
{
	if (n == 0)
		return -1;

	int smallest = mat[0][0], largest = mat[n - 1][n - 1];
	if (x < smallest || x > largest)
		return -1;

	// set indexes for top right element
	int i = 0, j = n - 1;
	while (i < n && j >= 0)
	{
		if (mat[i][j] == x)
		{
			cout << "n Found at "
				<< i << ", " << j;
			return 1;
		}
		if (mat[i][j] > x)
			j--;
	
		// Check if mat[i][j] < x
		else
			i++;
	}

	cout << "n Element not found";
	return 0;
}
*/

/*
// Function to find minimum swaps
// to group all 1's together
int minSwaps(int arr[], int n)
{

    int numberOfOnes = 0;

    // find total number of all 1's in the array
    for (int i = 0; i < n; i++) {
        if (arr[i] == 1)
        numberOfOnes++;
    }

    // length of subarray to check for
    int x = numberOfOnes;

    int count_ones = 0, maxOnes;

    // Find 1's for first subarray of length x
    for(int i = 0; i < x; i++){
        if(arr[i] == 1)
        count_ones++;
    }
        
    maxOnes = count_ones;
        
    // using sliding window technique to find
    // max number of ones in subarray of length x
    for (int i = 1; i <= n-x; i++) {
        
        // first remove leading element and check
        // if it is equal to 1 then decreament the
        // value of count_ones by 1
        if (arr[i-1] == 1)
        count_ones--;
        
        // Now add trailing element and check
        // if it is equal to 1 Then increment
        // the value of count_ones by 1
        if(arr[i+x-1] == 1)
        count_ones++;
        
        if (maxOnes < count_ones)
        maxOnes = count_ones;
    }

    // calculate number of zeros in subarray
    // of length x with maximum number of 1's
    int numberOfZeroes = x - maxOnes;

    return numberOfZeroes;
}

*/

/*
// Find minimum difference b/w length of tallest and shortest tower
private static int doIt(final int[] arr, final int k) {
    java.util.Arrays.sort(arr);
    final int n = arr.length;
    int result = arr[n - 1] - arr[0];
    for (int i = 1; i < n; ++i) {
        final int min = Math.min(arr[0] + k, arr[i] - k);
        final int max = Math.max(arr[n - 1] - k, arr[i - 1] + k);
        result = Math.min(result, max - min);
    }
    return result;
}
*/

/*
// This is implementation of fisher yates shuffling algorithm
void fisher_yates_shuffling(int arr[], int n) {
    srand(time(NULL));
    for(int i=n;i>=1;--i) {
        int rand_index = (rand()%i+1);
        swap(arr[rand_index], arr[i]);
    }
}
*/

/*
// Qn. : Apply A*x*x + B*x + C for each element x in the array and sort the modified array.
// The equation given is parabolic. So the result of applying it to
// a sorted array will result in an array that will have a maximum/minimum 
// with the sub-arrays to its left and right sorted.
void sortArray(int arr[], int n, int A, int B, int C) {
	for (int i = 0; i < n; i++)
		arr[i] = A*arr[i]*arr[i] + B*arr[i] + C;

	// Find maximum element in resultant array
	int index, maximum = INT_MIN;
	for (int i = 0; i< n; i++)
		if (maximum < arr[i]) {
			index = i;
			maximum = arr[i];
		}

	// Use maximum element as a break point
	// and merge both subarrays usin simple
	// merge function of merge sort
	int i = 0, j = n-1;
	int new_arr[n], k = 0;
	while (i < index && j > index)
		if (arr[i] < arr[j])
			new_arr[k++] = arr[i++];
		else
			new_arr[k++] = arr[j--];
	
	// Merge remaining elements
	while (i < index)
		new_arr[k++] = arr[i++];
	while (j > index)
		new_arr[k++] = arr[j--];

	new_arr[n-1] = maximum;

	// Modify original array
	for (int i = 0; i < n ; i++)
		arr[i] = new_arr[i];
}
*/

// Find all permutations of a given string
/*
void permute(string orig_str, string buf, int idx, int len) {
    int *arr = new int[len];
    if(idx == len) {
        cout<<buf<<endl;
        return;
    }
    for(int i=0;i<len;++i) {
        if(arr[i] == -1)
            continue;
        arr[i]=-1;
        permute(orig_str, buf+orig_str[i], idx+1, len);
        arr[i]=0;
    }
}
*/

// Another way to permute
/*
void permute(string a, int l, int r) {
    if (l == r)
        cout<<a<<endl;
    else {
        // Permutations made
        for (int i = l; i <= r; i++) {
            swap(a[l], a[i]);    // Swapping done
            permute(a, l+1, r);  // Recursion called
            swap(a[l], a[i]);   //backtrack
        }
    }
}
*/

/* Given an array, check if that can represent a pre-order traversal. For this, use the idea of next greater element */

// Spiral order traversal of array
void spiralOrderTraversal() {
	int a[][4] = {
					{ 1, 2, 3, 4 },
					{ 5, 6, 7, 8 },
					{ 9, 10, 11, 12 },
					{ 13, 14, 15, 16}
				};
	
	int top=0, down=3, left=0,right=3;
	int direction = 0;
	while(top<=down && left <= right) {
		if(direction == 0) {
			for(int i=left; i<=right;++i) {
				cout<<a[top][i]<<" ";
			}
			++top;
		}
		else if(direction == 1) {
			for(int i=top; i<=down;++i) {
				cout<<a[i][right]<<" ";
			}
			--right;		
		}
		else if(direction == 2) {
			for(int i=right; i>=left;--i) {
				cout<<a[down][i]<<" ";
			}
			--down;		
		}

		else if(direction == 3) {
			for(int i=down; i>=top;--i) {
				cout<<a[i][left]<<" ";
			}
			++left;		
		}
		direction = (direction+1)%4;
	}
}

void expandingSpiralFromCenter() {
    int x = 0; // current position; x
    int y = 0; // current position; y
    int d = 0; // current direction; 0=RIGHT, 1=DOWN, 2=LEFT, 3=UP
    int c = 0; // counter
    int s = 1; // chain size

	int a[][4] = {
					{ 1, 2, 3, 4 },
					{ 5, 6, 7, 8 },
					{ 9, 10, 11, 12 },
					{ 13, 14, 15, 16}
				};
	int size = 4;
    // starting point
    x = ((int)floor(size/2.0))-1;
    y = ((int)floor(size/2.0))-1;

    for (int k=1; k<=(size-1); k++)
    {
        for (int j=0; j<(k<(size-1)?2:3); j++)
        {
            for (int i=0; i<s; i++)
            {
                std::cout << a[x][y] << " ";
                c++;

                switch (d)
                {
                    case 0: y = y + 1; break;
                    case 1: x = x + 1; break;
                    case 2: y = y - 1; break;
                    case 3: x = x - 1; break;
                }
            }
            d = (d+1)%4;
        }
        s = s + 1;
    }
}

int main() {
  int arr[] = { 1, 2, 3, 4, 5, 6 };
  int sz = sizeof(arr)/sizeof(arr[0]);
  vector<int> vec{ 2,3,2,3,5 };
  /*int val = bin_search(arr, 0, 5, 12); // args - array, size_array, element_to_search
  if(val == -1) cout<<"Element not found\n"; else cout<<"Element found\n";
  */
  /*int res = kadane(arr, 6); cout<<"Max contiguous subset is: "<<res<<endl; */
  // Reversed array is:
  /* vector_reverse(vec, vec.size());
  for(auto& a:vec) { cout<<a<<" "; } */
  // Reverse in groups
  /* reverseInGroups(vec, 5, 3); for(auto& a:vec) { cout<<a<<" "; } */
  // Array rotation
  /* arr_rotation_counter_clockwise(vec, 0, 5, 2);  for(auto& a:vec) { cout<<a<<" "; } */
  // Convert to wave like 1<=2>=3<=5>=13<=20
  /* convertToWave(arr, 5);*/
  // Count frequency of each element in an array
  /*frequencycount(vec, 5); for(int i=0;i<5;++i) cout<<vec[i]<<" "; */
  // Find equilibrium point, a point where left_sum_before==sum_after_it
  /* int res = equilibriumPoint(arr, 5); cout << "equilibriumPoint is: " << res << endl; */
  // Trapping walter
  /* int res = trapped_water(arr, sizeof(arr)/sizeof(arr[0])); cout<<"Trapped water is: "<<res<<endl; */
  // Maximize the profit by buying and selling stocks at appropriate dates
  /* int res = stock_buy_sell(arr, sizeof(arr)/sizeof(arr[0])); cout<<"Stock maximization is" <<res<<endl;*/
  //  Function to find first smallest positive  missing number in the array
  /* int res = missingNumber(int a[], int n); cout<<"Missing number is: "<<res; */
  // Finds the maximum absolute difference between nearest left and right smaller element of every element
  /* int mx_dif = findMaxDiff(arr, sizeof(arr)/sizeof(arr[0]));   cout<<"Max dif is: "<<mx_dif<<endl; */
  // Find the number of pairs of elements in the array whose sum is equal to K.
  /* int count = getPairsCount(arr, sizeof(arr)/sizeof(arr[0]), 5);   cout<<"Number of pairs are: "<<count<<endl; */
  //Find the maximum number of consecutive 1's produced by flipping at most M 0's.
  /* int count = findZeroes(arr, sizeof(arr)/sizeof(arr[0]), 3);    cout<<"Answer is: "<<count<<endl;  */
  // Find majorityElement. Majority element is the element whose count is > N/2
  /* cout<<"Majority element is: "<<majorityElement(arr, sizeof(arr)/sizeof(arr[0])); */
  // Sort the binary array of 0s and 1s
  // vector<int> res = SortBinaryArray(vec);  for(auto a:res) cout<<a<<" "; 
  // Find the minimum distance between two numbers
  /* cout<<"min dist is: "<<minDist(arr, 27, 78, 68)<<endl; */
  /* cout<<"Max non contiguous sum is: "<<FindMaxSum(arr, sizeof(arr)/sizeof(arr[0])); */
  // Rearrange the given array so that arr[i] becomes arr[arr[i]]
  /* arrange(arr, sizeof(arr)/sizeof(arr[0])); */
  // Rearrange the array so that element appears in form... max_elem, min_elem,
  // second_max, second_min... and so on
  /* rearrange_alternate(arr, sizeof(arr)/sizeof(arr[0]));  for(int i=0;i<sizeof(arr)/sizeof(arr[0]);++i) {  cout<<arr[i]<<" "; } */
  // Find a starting point where the truck can start to get through the complete circle without exhausting its petrol in between
  /* tour(<>); */
  // Search for an element in row-wise and column-wise sorted matrix
  /* int mat[4][4] = { { 10, 20, 30, 40 },
					{ 15, 25, 35, 45 },
					{ 27, 29, 37, 48 },
					{ 32, 33, 39, 50 } };
   searchInColAndRowWiseSortedMatrix(mat, 4, 29);
   */
   /*
   fisher_yates_shuffling(arr, sz);
   for(int i=0;i<sz;++i)
    cout<<arr[i]<<" ";
    */
   // Find all permutations of a string
   /*string s = "pinchu";
   string buf = "";
   permute(s, buf, 0, 6);
   */
   /* Spiral order traversal of array */
   //spiralOrderTraversal();
   /* Expanding spiral order traversal */
   expandingSpiralFromCenter();
   

  
  
  return 0;
}

// Notes:
// 1. To find minimum unsorted subarray that needs to be sorted to make the complete array sorted. First find the deviation
// from left, then deviation from right. Then find min and max in that range (s, e) because all elements in that range should
// be greater than the element immediate left to range and all elements in that range need to be lesser than the element
// which is immediate right after the range.

