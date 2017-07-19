#include<vector>
#include<deque>
#include<iostream>


using namespace std;

class A{
	public:
		void show(deque<int> d) {
			cout<<"In integer";
		}
		void show(deque<float> d) {
			cout<<"In float\n";
		}
};

int main() {
	deque<int> dInt;
	A a;
	a.show(dInt);
	return 0;
}
