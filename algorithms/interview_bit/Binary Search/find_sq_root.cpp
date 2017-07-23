#include<iostream>
#include<cmath>
#include<string>

using namespace std;

double find_sq_root(double number, double start, double end, int counter) {

	double mid = double((start+end)/2);
	++counter;
	if(counter >= 100)
		return floor(mid);
	double mid_sq = double(mid)*double(mid);
	if(number == double(mid*mid))
		return mid;
	else if(double(mid*mid) > number)
		return find_sq_root(number, start, mid, counter);
	else
		return find_sq_root(number, mid, end, counter);
}

int main() {
	double number = 0;
	cout<<"Enter the number\n";
	cin>>number;
	double sq_rt = find_sq_root(number, 0, number, 0);
	cout<<"sqrt is: "<<(double)sq_rt<<endl;
	return 0;
	
}
