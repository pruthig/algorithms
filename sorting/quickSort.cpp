//Quicksort algo is similar to merge sort in terms of efficiency
//but it is in-place sorting algorithm

#include<iostream>
#include<string>

using namespace std;

void quicksort(int s, int e, int *p);
int partition(int s, int e, int *p);

int main(){

int a[]  = {6, 45, 3, 100, -98, 3, 566};

quicksort(0, 6, a);
cout<<"After sorting numbers are\n";
for(int i = 0; i <= 6; ++i)
	cout<<a[i]<<", ";
cout<<endl;

return 0;
}

void quicksort(int s, int e, int *p){
	if(s == e || s<0 || e<0 || s>e || e<s)
		return;
	int pivot = partition(s, e, p);
	cout<<"Pivot returned is "<<pivot<<endl;

	quicksort(s, pivot-1, p);
	quicksort(pivot+1, e, p);
}


int partition(int s, int e, int *p){
	int i = s;
	int j = s + 1;
	int pvt = p[s];

	for(int j = i + 1; j <= e; ++j){
		if(p[j] < pvt){
			++i;
			int t = p[i];
			p[i] = p[j];
			p[j] = t;
		}
	}
	int t = p[i];
	p[i] = p[s];
	p[s] = t;

	return i;
}
