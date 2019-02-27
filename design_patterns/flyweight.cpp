//Flyweight pattern is basically used to achieve memory efficiency when a large number
//of objects are created and shared in a program...time efficieny might take a hit in 
//flyweight pattern...

#include<iostream>
#include<cstdlib>
#include<conio.h>
#include<ctime>
#include<chrono>
#include<string>
#include<unordered_map>

using namespace std;


enum Color {
	BLUE,
	RED,
	YELLOW,
	BLACK,
	CYAN,
	BROWN
};

namespace
{

	unordered_map<Color, string> mp_clr_str = 
	{
		{Color::BLUE, "blue"},
		{Color::RED, "red"},
		{Color::YELLOW, "yellow"},
		{Color::CYAN, "cyan" },
		{Color::BLACK, "black" },
		{Color::BROWN, "brown" },
	};
}

class Shape {
public:
	virtual void draw() {}
};



class Circle : public Shape {

	int radius;
	Color color;
public:
	Circle() {}
	Circle(Color c) :color(c) {}
	void draw()
	{
		cout << "Circle drawn\n";
	}
};

class CircleCreatorFactory : public Shape
{
	std::unordered_map<Color, Circle*> mCircle{};
public:
	void draw(){}

	Shape* createCircle(Color color)
	{
		if (mCircle.find(color) == mCircle.end())
		{
			// create the object
			Circle *c = new Circle(color);
			mCircle.insert(std::make_pair(color, c));
			cout << "New object created for color: " << mp_clr_str[color]<<endl;
		}
		else
		{
			cout << "Returning already stored value for color: " << mp_clr_str[color] << endl;
			return mCircle[color];
		}
		return nullptr;
	}
};



int main() {
	CircleCreatorFactory *factory = new CircleCreatorFactory();
	Circle *c  = static_cast<Circle*>(factory->createCircle(Color::BLACK)); 
	c = static_cast<Circle*>(factory->createCircle(Color::CYAN));
	c = static_cast<Circle*>(factory->createCircle(Color::BLACK));
	_getch();
	return 0;
}
