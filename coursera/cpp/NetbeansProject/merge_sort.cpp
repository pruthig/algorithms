/*
 * Merge Sort
 * © Gaurav Pruthi <gaurav.pruthi88@gmail.com>
 */
#include <iostream>
#include <fstream>
#include <ctime>

using namespace std;

void merge_conquer(int arr[], int start_index, int end_index);
void merge_divide(int arr[], int start_index, int end_index);


void merge_sort_main() {
    int arr[100000] = { 0 }; 
    std::ifstream infile("IntegerArray.txt");
    
    for(int j=0; j<100000; j++)
     infile >> arr[j];
    
    int array_size = sizeof(arr)/sizeof(int);
    
    // Calculate the performance
    int start_s=clock();
    // pass starting and end index of array
    merge_divide(arr, 0, array_size-1);
    int stop_s=clock();
    cout << "Merge Sort on 1 lakh elements: Time elapsed in ms: " << (stop_s-start_s)/double(CLOCKS_PER_SEC)*1000; 
    //cout<<"Sorted array is : ";
    //for(int i = 0; i < array_size; ++i) {
    //    cout<<" "<<raw_arr[i]<<",";
    //}
    
}

// 2,5,7,12 - 0, 3
void merge_divide(int arr[], int start_index, int end_index) {
    
    if(start_index == end_index)
        return;
    
    int mid = (start_index + end_index)/2;
    merge_divide(arr, start_index, mid);
    merge_divide(arr, mid+1, end_index);
    merge_conquer(arr, start_index, end_index);

}

//0, 0, 1  --- 2,5
void merge_conquer(int arr[], int start_index, int end_index) {
    // We have to merge the two arrays
    int mid = (start_index + end_index)/2;
    int *dummy = new int[end_index-start_index + 1];
    int i = start_index;
    int j = mid+1;
    int p = 0;

    while(i != mid+1 && j != end_index+1) {
        if(arr[i] < arr[j] || arr[i] == arr[j]) {
            dummy[p] = arr[i];
            ++i; ++p;
        }
        else  {
            dummy[p] = arr[j];
            ++j; ++p;
        }
        
    }
    if(i != mid+1) {
        for(int k = i; k<=mid; ++k,++p)
            dummy[p] = arr[k];
    }
    if(j != end_index+1) {
        for(int k = j; k<=end_index; ++k,++p)
            dummy[p] = arr[k];
    }
    
    for(int l = start_index, dum = 0; l <= end_index ; ++dum, ++l) {
        arr[l] = dummy[dum];
    }
    
}