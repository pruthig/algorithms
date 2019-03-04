// Builder design pattern is used to create complex object by creating simple object step-by-step
#include<iostream>
#include<string>
#include<conio.h>

using namespace std;

class RobotPlanInterface
{
	virtual void Head(std::string val) = 0;
	virtual void Legs(std::string val) = 0;
	virtual void Arms(std::string val) = 0;
	virtual void Torso(std::string val) = 0;
};

class Robot : public RobotPlanInterface
{
private:
	string head, torso, arms, legs;
public:

	std::string Head() const { return head; }
	void Head(std::string val) { head = val; }
	std::string Torso() const { return torso; }
	void Torso(std::string val) { torso = val; }
	std::string Arms() const { return arms; }
	void Arms(std::string val) { arms = val; }
	std::string Legs() const { return legs; }
	void Legs(std::string val) { legs = val; }
};

class RobotBuilder
{
	virtual void buildHead() = 0;
	virtual void buildArms() = 0;
	virtual void buildTorso() = 0;
	virtual void buildLegs() = 0;
};

class OldRobotBuilder : public RobotBuilder
{
private:
	Robot robot;
public:
	void buildHead()
	{
		robot.Head("New head");
	}

	void buildArms()
	{
		robot.Arms("New arms");
	}
	void buildLegs()
	{
		robot.Legs("New Legs");
	}
	void buildTorso()
	{
		robot.Torso("New Torso");
	}
	Robot getRobot()
	{
		return robot;
	}

};

class RobotEngineer
{
private:
	OldRobotBuilder *robotBuilder;
public:
	RobotEngineer(RobotBuilder *rb)
	{
		robotBuilder = static_cast<OldRobotBuilder*>(rb);
	}
	void makeRobot()
	{
		robotBuilder->buildTorso();
		robotBuilder->buildLegs();
		robotBuilder->buildHead();
		robotBuilder->buildArms();
	}
	Robot getRobot()
	{
		return robotBuilder->getRobot();
	}
};

int main()
{
	RobotBuilder *rb = new OldRobotBuilder();
	RobotEngineer *robotEngineer = new RobotEngineer(rb);
	robotEngineer->makeRobot();
	Robot robot = robotEngineer->getRobot();

	cout << "Robot built\n";

	cout << "Head: " << robot.Head() << endl;
	cout << "Torso: " << robot.Torso() << endl;
	cout << "Legs: " << robot.Legs() << endl;
	cout << "Arms: " << robot.Arms() << endl;

	_getch();
	return 0;
}