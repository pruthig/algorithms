/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Ex.cpp
 * Author: pruthi
 *
 * Created on 5 March, 2017, 11:00 AM
 */

#include <iostream>
#include "generic.h"
using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {

    int match;
    
    while(1)
    {
        cout<<"\n\nWhat do u want to test"<<'\n';
        cout<<"1. Vector Example"<<'\n';
        cout<<"2. Operator Overloading"<<'\n';
        cout<<"3. Merge Sort"<<'\n';
        cout<<"4. QuickSort with last element chosen as pivot"<<'\n';
        cout<<"5. QuickSort with random element chosen as pivot"<<'\n';
        cout<<"6. Card Game"<<'\n';
        cout<<"7. Dijkstra Algorithms"<<'\n';
        cout<<"8. Count inversions in an array"<<'\n';
        cout<<"\nEnter your choice\n";
        cin>>match;

        switch(match) {
            case 1:
                sum_of_vector_elements_main();
                break;
            case 2:
                operator_overloading_main();
                break;
            case 3:
                merge_sort_main();
                break;
            case 4:
                quicksort_last_pivot_main();
                break;
            case 5:
                quicksort_randomized_pivot_main();
                break;
            case 6:
                card_game_main();
                break;
            case 7:
                dijkstra_main();
                break;
            case 8:
                counting_inversions_main();
                break;

            default:
                exit(0);

        }
    }
    

    return 0;
}

