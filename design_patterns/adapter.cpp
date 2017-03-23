//Adapter design pattern uses a given interface from another interface which
//expects the functionality provided by the 1st interface

#include<bits/stdc++.h>

using namespace std;

class flowers{
vector<string> flowerList;
public:
vector<string> getFlowerList(){
	return flowerList;
}
void insertFlower(string flower){
	flowerList.push_back(flower);
}
};

class garden{
vector<string> plantList;
public:

flowers *fl;
vector<string> getFlowerList(){
	return fl->getFlowerList();
}
vector<string> getPlants(){
	return plantList;
}
void insertPlant(string plant){
	plantList.push_back(plant);
}
flowers* getFlowerRef(){
	return fl;
}
};



int main(){
	flowers f;
	f.insertFlower("rose");
	f.insertFlower("lotus");
	f.insertFlower("Lily");
	f.insertFlower("cauliflower");
	garden g;
	g.fl = &f;
	for(auto a : g.getFlowerList())
		cout<<"Flower :"<<a<<endl;

	return 0;
}


