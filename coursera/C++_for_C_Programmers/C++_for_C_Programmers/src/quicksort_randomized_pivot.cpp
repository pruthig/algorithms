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

static int quicksort_combine(int arr[], int start_index, int end_index);
static void quicksort_divide(int arr[], int start_index, int end_index);

void quicksort_randomized_pivot_main(){
    int arr[100000] = { 0 }; 
    std::ifstream infile("IntegerArray.txt");
    
    for(int j=0; j<100000; j++)
     infile >> arr[j];

    cout<<"Completed reading numbers from file"<<endl;
    int array_size = sizeof(arr)/sizeof(int);
    srand (time(NULL));
    // calculate performance
    int start_s=clock();
    quicksort_divide(arr, 0, array_size-1);
    int stop_s=clock();
    cout << "Quick Sort on 1 lakh elements: Time elapsed in ms: " << (stop_s-start_s)/double(CLOCKS_PER_SEC)*1000 << endl; 
    
    //cout<<"Sorted array is : ";
    //for(int i = 0; i < array_size; ++i) {
    //    cout<<" "<<arr[i]<<",";
    //}
}

void quicksort_divide(int arr[], int start_index, int end_index) {
    if(start_index == end_index)
        return;
    int pivot_position = quicksort_combine(arr, start_index, end_index);
    if(pivot_position>start_index)
        quicksort_divide(arr, start_index, pivot_position-1);
    if(pivot_position<end_index)
        quicksort_divide(arr, pivot_position+1, end_index);
}


// Works in O(n) time
static int quicksort_combine(int arr[], int start_index, int end_index) {
    int i = start_index;
    int j = end_index-1;
    int random_number = start_index + rand() % (end_index-start_index+1);
    //cout<<"Start element, End element, random"<<start_index<<"    "<<end_index<<"    "<<random_number<<endl;
    swap(arr[random_number], arr[arr[end_index]]);
    int pivot = arr[random_number];
    while(i<j) {
        while (arr[i]<pivot)
            ++i;
        while (arr[j]>pivot)
            --j;
        if(i<j) {
            swap(arr[i], arr[j]);
            i++; j--;
        }
        else {
            break;
        }
    }
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

