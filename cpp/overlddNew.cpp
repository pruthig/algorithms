#include<iostream>
#include<cstdlib>



using namespace std;

class A{
	void* operator new(size_t s){
		int *n = new int[s];
		return void*(n);
	}
};

int main()
{
		A a = new A;
		return 0;

}
