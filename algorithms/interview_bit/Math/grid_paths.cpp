#include<iostream>
#include<cmath>

using namespace std;

int fact(int sum, int small) {
	double t_small = small;
	double t_sum = sum;
	double total = 1;
	for( ;t_small; --t_sum, --t_small) {
		total = total*(double(t_sum/t_small));
	}
	return round(total);
}

int main() {
	int A = 9, B = 10;
	int x = A-1, y = B-1;
	double total = (x > y)?fact(x+y, y):fact(x+y, x);
	cout<<"Total is: "<<total<<endl;
	return 0;
}
