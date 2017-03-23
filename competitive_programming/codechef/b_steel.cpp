//Grade the steel
#include<iostream>

using namespace std;



int main(){

int hard, ten;
double carb;
bool i = false, ii = false, iii = false;

int count, grade = 5;
cin>>count;

    for(int j = 1; j <= count; ++j){
        grade = 0;
        cin >> hard >> carb >> ten;
        if(hard>50) 
            i = true;
        if(carb<0.7f)
            ii = true;
        if(ten>5600)
            iii = true;

        if(i && ii && iii)
            grade = 10;
        else if(i && ii && !iii) 
            grade = 9;
        else if(i && !ii && iii)
            grade = 7;
        else if(!i && ii && iii)
            grade = 8;
        else if( (i && !ii && !iii ) || ( !i && ii && !iii ) || ( !i && !ii && iii))
            grade = 6;
        else 
            grade = 5;
        
        cout<<grade<<endl;
        grade = 5;
        i = false; ii = false; iii = false;
    }
return 0;
}
    
    

