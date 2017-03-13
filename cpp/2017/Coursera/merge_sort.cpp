/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include<iostream>

using std::cout;

void merge_conquer(int arr[], int start_index, int end_index);
void merge_divide(int arr[], int start_index, int end_index);


void merge_sort_main() {
    int raw_arr[] = { 5, 2, 7, 12, 3, 1, 32, 8, 47, 11 };
    int array_size = sizeof(raw_arr)/sizeof(int);
    // pass starting and end index of array
    merge_divide(raw_arr, 0, array_size-1);
    cout<<"Sorted array is : ";
    for(int i = 0; i < array_size; ++i) {
        cout<<" "<<raw_arr[i]<<",";
    }
    
}

// 2,5,7,12 - 0, 3
void merge_divide(int arr[], int start_index, int end_index) {
    
    if(start_index == end_index)
        return;
    
    int mid = (start_index + end_index)/2;
    merge_divide(arr, 0, mid);
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