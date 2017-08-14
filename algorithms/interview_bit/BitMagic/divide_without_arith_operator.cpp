#include<iostream>
using namespace std;
        
void divide(int dividend, int divisor) {
    long long divi = dividend;
    long long divis = divisor;
    
    int sign = ((divi<0)^(divis<0)) ? -1 : 1;
    if(divis == 0 || (divi == INT_MIN && divis == -1))
    {
        return;
    }
    
    divi = labs(divi);
    divis = labs(divis);
    int ans = 0;
    
    
    while(divi >= divis)
    {
        int mul = 1;
        long long temp = divis;
        while(divi >= (temp << 1))
        {
            temp <<= 1;
            mul <<= 1;
        }
        divi -= temp;
        ans += mul;
    }
    cout<<"Quotient is: "<<((sign<0) ? -ans : ans)<<" and remainder is:  "<<divi<<endl;
}    

int main() {
	divide(77, 6);
	return 0;	
}
