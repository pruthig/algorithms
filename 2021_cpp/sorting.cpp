#include <iostream>
#include <unordered_map>
#include <climits>
#include <vector>
#include <algorithm>
#include <cstring>

using namespace std;

#define LOOP(i, s, e) for(int i=s;i<e;++i)
#define RLOOP(i, s, e) for(int i=s;i>=e;--i)


void insertion_sort(int arr[], int n) {
    LOOP(i, 1, n) {
        int j=i;
        while(j>0 && arr[j]<arr[j-1]) {
            int tmp = arr[j-1];
            arr[j-1] =  arr[j];
            arr[j] = tmp;
            --j;
        }
    }
}

// Bubble sort bubbles up the largest element
void bubble_sort(int arr[], int n) {
    RLOOP(i, n-1, 0) {
        LOOP(j, 0, i) {
            if(arr[j]>arr[j+1])
                swap(arr[j], arr[j+1]);
        }
    }
}

void selection_sort(int arr[], int n)
{
    for(int i=0;i<n;++i) {
        int min_idx = i;
        for(int j=i;j<n;++j) {
            if(arr[j]<arr[min_idx])
                min_idx = j;
        }
        swap(arr[i], arr[min_idx]);
    }
}

void merge(int arr[], int s, int e) {
    if(s==e)
        return;
    int len = e-s+1;
    int *temp_arr = new int[len];
    int mid = (s+e)/2;
    int i=s, j=mid+1, t_idx=0;
    while(i<=mid && j<=e) {
        if(arr[i]<=arr[j]) {
            temp_arr[t_idx] = arr[i];
            ++i; ++t_idx;
        }
        else {
            temp_arr[t_idx] = arr[j];
            ++j; ++t_idx;
        }
    }
    // copy leftover elements
    if(i != mid+1) {
        while(i <= mid) {
            temp_arr[t_idx] = arr[i];
            ++i; ++t_idx;
        }
    }
    else if(j != e+1) {
        while(j <= e) {
            temp_arr[t_idx] = arr[j];
            ++j; ++t_idx;
        }
    }
    // copy tmp to normal
    LOOP(i, 0, len) {
        arr[s+i] = temp_arr[i];
    }
}

void merge_sort(int arr[], int s, int e) {
    if(s==e)
        return;
    int mid = (s+e)/2;
    merge_sort(arr, s, mid);
    merge_sort(arr, mid+1, e);
    merge(arr, s, e);
}

int partition(int arr[], int s, int e) {
    int partition_element = arr[e-1];
    int j=s;
    LOOP(i, s, e) {
        if(arr[i] < partition_element) {
            swap(arr[i], arr[j]);
            ++j;
        }
    }
    swap(arr[e-1], arr[j]);
    return j;
}

void quick_sort(int arr[], int s, int e) {
    if(s<0 || e<0 || s>=e)
        return;
    int pos = partition(arr, s, e);
    quick_sort(arr, s, pos);
    quick_sort(arr, pos+1, e);
}

int kthsmallest_partition(int arr[], int s, int e, int k) {
    int partition_element = arr[e-1];
    int j=s;
    LOOP(i, s, e) {
        if(arr[i] < partition_element) {
            swap(arr[i], arr[j]);
            ++j;
        }
    }
    swap(arr[e-1], arr[j]);
    return j;
}

int kthsmallest(int arr[], int s_, int e_, int k) {
    if(k>=e_)
        return -1;
    int pos = -1;
    int s = s_, e = e_;
    while(pos != k) {
        if(s>=e)
            return arr[s];
        pos = kthsmallest_partition(arr, s, e, k);
        if(pos>k)
            e = pos;
        else
            s = pos+1; 
    }
    return arr[pos];
}

void triplet_sum_in_array(int arr[], int n, int m) {
    sort(arr, arr+n);
    for(int i=0;i<n-2;++i) {
        int j=i+1, k=n-1;
        while(j<k) {
           int tmp_sum = arr[i]+arr[j]+arr[k];
           //cout<<"tmp_sum and i/j/k is" <<tmp_sum<<" "<<arr[i]<<" "<<arr[j]<<" "<<arr[k];
           if(tmp_sum==m){
               cout<<"Sum found with digits: "<<arr[i]<<" "<<arr[j]<<" "<<arr[k]<<endl;
               return;
           }
           else if(tmp_sum>k)
               --k;
           else
               ++j;
        }
    }
    cout<<"No triplet found"<<endl;
}

void sort_0_1_2(int arr[], int n) {
    int i=0, j=0, k=n-1;
    while(arr[k]==2 && k>=0) --k;
    
    while(i<k && j<=k) {
        if(arr[j]==0) {
            swap(arr[i], arr[j]);
            ++i; ++j;
        }
        else if(arr[j]==2) {
            swap(arr[j], arr[k]);
            while(arr[k]==2 && k>=0) --k;
        }
        else {
            ++j;
        }
    }
    cout<<"Sorted array is: "<<endl;
    for(int i=0;i<n;++i)
        cout<<arr[i]<<" ";
    cout<<endl;            
}

// Partition the array around the range [a, b] such that array is divided 
// into three parts.
void threeWayPartition(int arr[], int n, int a, int b)
{
    int i=0, j=0, k=n-1;
    while(arr[k]>b && k>=0) --k;
    
    while(i<k && j<=k) {
        if(arr[j]<a) {
            swap(arr[i], arr[j]);
            ++i; ++j;
        }
        else if(arr[j]>b) {
            swap(arr[j], arr[k]);
            while(arr[k]>b && k>=0) --k;
        }
        else {
            ++j;
        }
    }
    cout<<"threeWayPartition array is: "<<endl;
    for(int i=0;i<n;++i)
        cout<<arr[i]<<" ";
    cout<<endl;
}

void max_water_in_2_bldgs(int arr[], int n) {
    int *l_mx_arr = new int[n];
    int *r_mx_arr = new int[n];
    int l_mx_idx = 0;
    int r_mx_idx = n-1;

    l_mx_arr[0] = 0;
    r_mx_arr[n-1] = n-1;
    for(int i=1;i<n;++i) {
        if(arr[i]>arr[l_mx_idx]) {
            l_mx_arr[i] = i;
            l_mx_idx = i;
        }
        else
            l_mx_arr[i] = l_mx_idx;
    }
    for(int i=n-2;i>=0;--i) {
        if(arr[i]>arr[r_mx_idx]){
            r_mx_arr[i] = i;
            r_mx_idx = i;
        }
        else
            r_mx_arr[i] = r_mx_idx;
    }
    int mx_ = INT_MIN, cur_sum=0;
    for(int i=0;i<n;++i) {
        int num = abs(std::min(arr[l_mx_arr[i]], arr[r_mx_arr[i]]) - arr[i]);
        if(num>0)
            cur_sum += num;
        else {
            if(mx_<cur_sum)
                mx_ = cur_sum;
            cur_sum = 0;
        }
    }
    // To take care of last element
    if(mx_<cur_sum)
        mx_ = cur_sum;
    cout<<"Max water to be trapped in any 2 buildings is : "<<mx_<<endl;
    return;
}

bool find_triplets_sum_to_zero(int arr[], int n)
{ 
    int j=0, k=0;
    sort(arr, arr+n);
    for(int i=0;i<n-2;++i){
        j=i+1; k=n-1;
        while(j<k) {
            int sum = arr[i] + arr[j] + arr[k];
            cout<<"Sum is: "<<sum<<endl;
            if(sum == 0) {
                cout<<"Elements summing up 0 are: "<<arr[i]<<" "<<arr[j]<<" "<<arr[k]<<endl;
                return true;
            }
            else if(sum<0)
                ++j;
            else
                --k;
        }
    }
    cout<<"Returning false"<<endl;
    return false;
}

int main() {
    int arr[] = { 2, 1, 3, 4, 6, 5 };
    int n = sizeof(arr)/sizeof(arr[0]);
    //bubble_sort(arr, n);
    //quick_sort(arr, 0, n);
    //LOOP(i, 0, n)
    //    cout<<arr[i]<<" ";
    //cout<<"Kth smallest element is: "<<kthsmallest(arr, 0, n, 2)<<endl;
    //triplet_sum_in_array(arr, n, 10);
    //sort_0_1_2(arr, n);
    //threeWayPartition(arr, n, 44, 62);
    //cout<<"Does array contain triplet that sum up to zero : ";
    //if(find_triplets_sum_to_zero(arr, n)==0) cout<<"No"<<endl; else cout<<"Yes"<<endl;
    max_water_in_2_bldgs(arr, n);
    return 0;
}
        
