#include<iostream>
using namespace std;

struct NaturalNum
{
    NaturalNum()
    {
        static int i=1;
        cout<<i++<<" ";
    }
} N[10];

main(){}
