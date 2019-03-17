// CoR pattern is used when we have multiple handlers to serve requests, with each request being served by a single handler. The
// handlers are arranged in chain such that when a given handler cannot serve requests, it relegates the request to next handler.
// We have an interface that declares methods setNext() and process() implemented by each. Each handler inheriting from that interface 
// and contains pointer to next handler.

#include<iostream>
#include<string>
#include<conio.h>

using namespace std;

class Chain
{
public:
	virtual void setNext(Chain *c) = 0;
	// Allowable operations are 'add' and 'sub'
	virtual void process(string str) = 0;
};

class processor_one : public Chain
{
	Chain *chain;
public:
	processor_one()
	{
		chain = nullptr;
	}
	void setNext(Chain *c)
	{
		chain = c;
	}
	void process(string str)
	{
		cout << "Asked to process: " << str << endl;

		if (str == "add")
			cout << "Add processed\n";
		else
		{
			cout << "I cannot handle, passing on control\n";
			if (chain)
				chain->process(str);
			else
				cout << "Sorry no handler available\n";
		}
	}
};

class processor_two : public Chain
{
	Chain *chain;
public:
	processor_two()
	{
		chain = nullptr;
	}
	void setNext(Chain *c)
	{
		chain = c;
	}
	void process(string str)
	{
		cout << "Asked to process: " << str << endl;

		if (str == "sub")
			cout << "Subtraction processed\n";
		else
		{
			cout << "I cannot handle, passing on control\n";
			if (chain)
				chain->process(str);
			else
				cout << "Sorry no handler available\n";
		}
	}

};


int main()
{
	processor_one *p1 = new processor_one();
	processor_two *p2 = new processor_two();
	p1->setNext(p2);
	p1->process("add");
	p1->process("modulus");
	_getch();
	return 0;
}
