/*
 * This program sorts the elements in an array using QuickSort with pivot being generated at random.
 * © Gaurav Pruthi <gaurav.pruthi88@gmail.com>
 */

#include <algorithm>
#include <iostream>
#include <fstream>
#include <ctime>
#include <random>

using namespace std;

static int order_statistics_combine(int arr[], int start_index, int end_index);
static int order_statistics_divide(int arr[], int start_index, int end_index, int pos);

int main(){
    int arr[] = { 5, 4, 12, 1, 9, 6, 10, 3 };  // Sorted is: 1 3 4 5 6 9 10 12
    int pos = 0;
    
    int array_size = sizeof(arr)/sizeof(int);
    cout<<"Enter the kth order statistic"<<endl;
    cin>>pos;
    if(pos < 0 || pos > array_size-1) {
        cout<<"Invalid input"<<endl;
        return 0;
    }

    int number = order_statistics_divide(arr, 0, array_size-1,pos-1);
    cout<<"Number is: "<<number<<endl;
    return 0;
}

int order_statistics_divide(int arr[], int start_index, int end_index, int pos) {
    
    int pivot_position = order_statistics_combine(arr, start_index, end_index);
    
    if(pivot_position == pos)
        return arr[pos];
    else if(pos < pivot_position)
        return order_statistics_divide(arr, start_index, pivot_position-1, pos);
    else
        return order_statistics_divide(arr, pivot_position+1, end_index,  pos);
}

// Works in O(n) time
// 'i' and 'j' points to start of the array
// with 'i' acts as wedge between the two partitions and the 'j' indicates cursor index, the right
// of which is yet to be discovered.
static int order_statistics_combine(int arr[], int start_index, int end_index) {
    int i = start_index+1;
    int j = start_index+1;
    
    int pivot = arr[start_index];
    while(j <= end_index) {
        if( arr[j]<pivot) {
            swap(arr[i], arr[j]);
            ++i;
        }
        ++j;
    }
    swap(arr[i-1], arr[start_index]);
    return i-1;
}

