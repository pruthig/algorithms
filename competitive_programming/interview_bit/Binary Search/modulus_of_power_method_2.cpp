
#include<iostream>

using namespace std;

int find_power(int x, int n, int d);

int main() {
	int x = 7, n = 3, d = 2;
	cout<<"Modulus of power : "<<find_power(x, n, d)<<endl;
	return 0;
}

int find_power(int x, int n, int d)
{

        if(x==0)
        return 0;
    if(n==0)
        return 1;
    long long a=1;
    long long y=x%d;
    if(y<0) y=y+d;
    while(n>0){
        if(n&1){
            a=(a*y)%d;
        }
        y=y*y;
        y=y%d;
        if(y<0)
            y+=d;
        n=n>>1;
    }
    return (int)a;
}
