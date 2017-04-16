/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <iostream>

using namespace std;

typedef enum {
    MON, TUE, WED, THU, FRI, SAT, SUN
}Day;

ostream& operator<<(ostream& out, const Day& d) {
    //int d = static_cast<int>(d);
    switch(d) {
        case MON:
            cout<<"Monday";
            break;
        case TUE:
            cout<<"Tuesday";
            break;
        case WED:
            cout<<"Wednesday";
            break;
        case THU:
            cout<<"Thursday";
            break;
        case FRI:
            cout<<"Friday";
            break;
        case SAT:
            cout<<"Saturday";
            break;
        case SUN:
            cout<<"Sunday";
            break;
        
    }
	return out;
}

void operator++( Day& d) {
    d =  static_cast<Day>( (static_cast<int>(d)+1)%7 );
}


void operator_overloading_main() {
    Day d = MON;
    ++d;
    cout<<"Day is: "<<d<<" ";
}