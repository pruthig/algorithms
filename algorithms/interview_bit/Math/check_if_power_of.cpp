#include<iostream>
#include<cstdio>
#include<cmath>

using namespace std;

double intlog(double base, double x) {
    return (log(x) / log(base));
}

bool isPower(int A) {
    if(A==1)
        return true;
    for(int i = 2; i <= sqrt(A); ++i) {
        double ret = intlog(i, A);
        float p = (float)ret;
        if(round(p) == p) {
            return true;
        }
    }
    return false;

}


int main() {
int a;
cout<<"Enter: \n";
cin>>a;
if(isPower(a))
    cout<<"true\n";
else
    cout<<"False\n";

return 0;
}
