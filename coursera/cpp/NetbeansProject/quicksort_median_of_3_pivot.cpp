/*
 * This program sorts the elements in an array using QuickSort with last element chosen as pivot
 * © Gaurav Pruthi <gaurav.pruthi88@gmail.com>
 */

#include <algorithm>
#include <iostream>
#include <fstream>
#include <ctime>

using namespace std;

long long number_of_comparisons = 0;

static int quicksort_combine_head_on_last_pivot(int arr[], int start_index, int end_index);
// Currently implemented as median of 3.
static int quicksort_combine_no_head_on(int arr[], int start_index, int end_index);
static void quicksort_divide(int arr[], int start_index, int end_index);

void quicksort_last_pivot_main(){
    int arr[10000] = { 0 }; 
    std::ifstream infile("QuickSort.txt");
    
    for(int j=0; j<10000; j++)
     infile >> arr[j];

    cout<<"Completed reading numbers from file"<<endl;
    int array_size = sizeof(arr)/sizeof(int);
 
    // calculate performance
    int start_s=clock();
    quicksort_divide(arr, 0, array_size-1);
    int stop_s=clock();
    cout << "Quick Sort on 10 thousand elements: Time elapsed in ms: " << (stop_s-start_s)/double(CLOCKS_PER_SEC)*1000 << endl; 
    cout<<"Number of comparisons "<<number_of_comparisons<<endl;
    
}

void quicksort_divide(int arr[], int start_index, int end_index) {
    if(start_index == end_index)
        return;
    int pivot_position = quicksort_combine_no_head_on(arr, start_index, end_index);
    if(pivot_position>start_index)
        quicksort_divide(arr, start_index, pivot_position-1);
    if(pivot_position<end_index)
        quicksort_divide(arr, pivot_position+1, end_index);
}


// Works in O(n) time
// 'i' points to start and 'j' points to the end of array.
// Increment 'i' till a[i] < pivot and decrement 'j' till a[j] > element, after that swap.
static int quicksort_combine_head_on_last_pivot(int arr[], int start_index, int end_index) {
    int i = start_index;
    int j = end_index-1;
    int pivot = arr[end_index];
    while(i<j) {
        while (arr[i]<pivot) {
            ++i;
            number_of_comparisons++;
        }
        while (arr[j]>pivot) {
            --j;
            number_of_comparisons++;
        }
        if(i<j) {
            swap(arr[i], arr[j]);
            i++; j--;
        }
        else {
            break;
        }
    }
    number_of_comparisons++;
    // If pivot is small we can throw that away at end..so swap next element ( i+1 )
    if(pivot > arr[i]) {
        swap(arr[end_index], arr[i+1]);
        return i+1; //return the pivot position
    }
    else {
        swap(arr[end_index], arr[i]);
        return i; //return the pivot position
    }
}


// Works in O(n) time
// 'i' and 'j' points to start of the array
// with 'i' acts as wedge between the two partitions and the 'j' indicates cursor index, the right
// of which is yet to be discovered.
static int quicksort_combine_no_head_on(int arr[], int start_index, int end_index) {
    int i = start_index+1;
    int j = start_index+1;
    int median = max(std::min(arr[start_index],arr[end_index]), min(max(arr[start_index],arr[end_index]),arr[start_index + (end_index-start_index)/2]));
    
    // Use median of 3 rule to minimize the number of comparisons.
    if(median == arr[end_index]) {
        swap(arr[start_index], arr[end_index]);
    }
    else if(median == arr[start_index + (end_index-start_index)/2]) {
        swap(arr[start_index], arr[start_index + (end_index-start_index)/2]);
    }
    else {
        /* do nothing*/
    }
    
    int pivot = arr[start_index];
    while(j <= end_index) {
        number_of_comparisons++;
        if( arr[j]<pivot) {
            swap(arr[i], arr[j]);
            ++i;
        }
        ++j;
    }
    swap(arr[i-1], arr[start_index]);
    return i-1;
}


