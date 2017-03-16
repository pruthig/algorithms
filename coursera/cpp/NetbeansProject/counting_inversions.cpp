/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <iostream>
#include <fstream>
#include <ctime>

using namespace std;

void c_i_conquer(int arr[], int start_index, int end_index);
void c_i_divide(int arr[], int start_index, int end_index);

long long inversion_count = 0;
int x = 0;
void counting_inversions_main() {
    int raw_arr[100000] = { 0 }; 
    std::ifstream infile("IntegerArray.txt");
    
    for(int j=0; j<100000; j++)
     infile >> raw_arr[j];

    cout<<"Completed reading numbers from file"<<endl;
    
    int array_size = sizeof(raw_arr)/sizeof(int);
    int start_s=clock();
    
    // pass starting and end index of array
    c_i_divide(raw_arr, 0, array_size-1);
    int stop_s=clock();
    cout << "Time elapsed in ms: " << (stop_s-start_s)/double(CLOCKS_PER_SEC)*1000 << endl;   
    cout<<"Total inversions are : ";
        cout<<" "<<inversion_count;
}

// 2,5,7,12 - 0, 3
void c_i_divide(int arr[], int start_index, int end_index) {
    if(start_index == end_index)
        return;
    int mid = (start_index + end_index)/2;
    c_i_divide(arr, start_index, mid);
    c_i_divide(arr, mid+1, end_index);
    c_i_conquer(arr, start_index, end_index);

}

//0, 0, 1  --- 2,5
void c_i_conquer(int arr[], int start_index, int end_index) {
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
            inversion_count += mid-i+1;
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