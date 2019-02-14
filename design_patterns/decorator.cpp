// Decorators provide a flexible alternative to subclassing for extending functionality.
// The decorator pattern can be used to make it possible to extend the functionality of a certain object at runtime.

#include<iostream>
#include<string>


class Beverage
{
private:
    int cost = 0;
public:
    Beverage(int c):cost(c){}
    Beverage(){}
    virtual int getCost() = 0;
};


class Espresso : public Beverage
{
    int cost;
public:
    Espresso(){}
    Espresso(int c):cost(c){}
    int getCost()
    {
        std::cout<<"Espresso getCost() called\n";
        return 11;
    }
};

class Coffee : public Beverage
{
    int cost;
public:
    Coffee(){}
    Coffee(int c):cost(c){}
    int getCost()
    {
        std::cout<<"Coffee getCost() called\n";
        return 12;
    }
};

class AddonDecorator : public Beverage
{
    int cost;
public:
    AddonDecorator(){}
    AddonDecorator(int c):cost(c){}
    int getCost()
    {
        return 0;
    }
};

class Caramel : public AddonDecorator
{
    Beverage *beverage;
public:
    Caramel(){}
    Caramel(Beverage *b = nullptr):beverage(b){}
    int getCost()
    {
        std::cout<<"Caramal getCost() called\n";
        return beverage->getCost() + 1;
    }
};

class Chocolate : public AddonDecorator
{
    Beverage *beverage;
public:
    Chocolate(){}
    Chocolate(Beverage *b = nullptr):beverage(b){}
    int getCost()
    {
        std::cout<<"Chocolate getCost() called\n";
        return beverage->getCost() + 2;
    }
};


int main()
{
    Beverage *bv = new Chocolate(new Caramel(new Espresso()));
    std::cout<<"Cost is: "<<bv->getCost()<<std::endl;
    return 0;
}
