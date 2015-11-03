//Best box size
//Length and Breadth will be same 
#include<iostream>
#include<cmath>
#include<cstdio>

using namespace std;

int main(){
	float p, s;
	int count = 0;
	cin>>count;

	for(int j = 1; j <= count; ++j)
	{
		cin>>p>>s;
		
		int sqrt_result = sqrt (p*p - 24*s);
		float length1 = (p + sqrt_result)/float(12);
		float length2 = (p - sqrt_result)/float(12);
		//cout<<"Length1 and Length2 are :"<<length1<<" "<<length2<<endl;

		float height1 =  (p - 8*length1)/float(4);
		float height2 =   (p - 8*length2)/float(4);
		//cout<<"Height1 and Height2 are :"<<height1<<" "<<height2<<endl;
		float result = (length1*length1*height1) > (length2*length2*height2) ? (length1*length1*height1) : (length2*length2*height2) ;

		
		printf("%.2f\n", float(result));
	}
	return 0;
}
	
	

	
