#include<iostream>
#include<vector>
#include<typeinfo>

using namespace std;

class Vehicle {
	public:
		virtual void name() {
			cout<<"I am a generic Vehicle\n";
		}
};

class Bus : public Vehicle {
	public:
		virtual void name() {
			cout<<"I am bus\n";
		}
};

class Car : public Vehicle {
	public:
		virtual void name() {
			cout<<"I am car\n";
		}
};

void processCars(vector<Vehicle*>  &vehicle_list) {
	for(auto a : vehicle_list) {
		if(typeid(*a) == typeid(Car))
			cout<<"Car detected with type(id) name: "<<typeid(*a).name()<<endl;
	}
	
}

int main() {
	Vehicle *c[4];
	vector<Vehicle*> vehicle_list;
	c[0] = new Car();
	c[1] = new Car();
	c[2] = new Bus();
	c[3] = new Car();
	for(int i = 0; i < 4; ++i)
		vehicle_list.push_back(c[i]);
	processCars(vehicle_list);
	return 0;
}
