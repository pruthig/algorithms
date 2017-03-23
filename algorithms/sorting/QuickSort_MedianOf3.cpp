//Quicksort algo is similar to merge sort in terms of efficiency
//but it is in-place sorting algorithm

//changes rqrd for median of three - check the 

#include<iostream>
#include<string>
#include<sstream>
#include<fstream>

using namespace std;

void quicksort(int s, int e, int *p);
int partition(int s, int e, int *p);
void swap(int x, int y, int *p);
int median(int a[], int p, int r);

int n_c = 0;

int main(){

int buf[10000];
string str = "";
stringstream ss;
int i = 0;

ifstream ifs("/home/pruthi/Desktop/QuickSort.txt");

while(getline(ifs, str)){
std::istringstream iss(str);
iss >> buf[i];
++i;
}

quicksort(0, 9999, buf);
cout<<"After sorting numbers are\n";

cout<<"Number of comparisons are ; "<<n_c<<endl;

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
	n_c = n_c + (e-s);
	int i = s;
	int middle = (s+e)/2;


	int index = median(p, s, e);//findLargest(s, e, p);

	swap(s, index, p);
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


int median(int a[], int p, int r)
{
    int m = (p+r)/2;
    if(a[p] < a[m])
    {
        if(a[p] >= a[r])
            return p;
        else if(a[m] < a[r])
            return m;
    }
    else
    {
        if(a[p] < a[r])
            return p;
        else if(a[m] >= a[r])
            return m;
    }
    return r;
}

void swap(int x, int y, int *p){
	int t = *(p+x);
	*(p+x) = *(p+y);
	*(p+y) = t;
}
