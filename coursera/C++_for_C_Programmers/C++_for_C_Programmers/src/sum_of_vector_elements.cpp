// Author: Gaurav Pruthi
// Description: This program sums up the elements from 0 to 39 using vector
// data type and prints the result.
// Created on 4 March, 2017, 11:37 AM
 
#include <iostream>
#include <vector>

using namespace std;

//
// This functions sums up the values stored in vector and stores the result in 'result' variable
//
inline void sum_of_vector_elements(int& result, vector<int> const &int_vector)
{
  result = 0;
  for(auto i : int_vector)
  {
    result  += i;
  }
}

//
// The main method of program 
//
 void sum_of_vector_elements_main() {
    
  // 'N' defines the number of elements to be inserted in vector.
  const int N = 40;
  
  // Initialize a vector of integers.
  vector<int> vector_int(N);
  int sum = 0;            // variable to store result of summation

  for(int i = 0; i < N ; ++i ) 
  {
    vector_int[i] = i;
  }

  sum_of_vector_elements(sum, vector_int);
  
  // Display the sum.
  cout<<"Sum is : "<<sum<<endl;
}

