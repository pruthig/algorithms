/* Abstract factory creates family of related factory which, in turn, create relevant objects which, generally, are derived from a common interface.
IN the below program the class AbstractGUIFactory acts as generic factory class which is used to create family of items 
*/
#include<iostream>

using namespace std;

enum Interface {
    WINDOWS,
    LINUX
};

class AbstractItem {
public:
    virtual void print() = 0;
};

class WinItem : public AbstractItem {
public:
    void print() {
        cout<<"This is Windows item\n";
    }
};

class LinItem : public AbstractItem {
public:
    void print() {
        cout<<"This is linux item\n";
    }
};


class AbstractGUIFactory {
public:
    virtual AbstractItem* createItem() = 0;
};

class WinGUIFactory : public AbstractGUIFactory {
public:
    AbstractItem* createItem() {
        return new WinItem;
    }
};

class LinGUIFactory : public AbstractGUIFactory {
public:
    AbstractItem* createItem() {
        return new LinItem;
    }

};

AbstractGUIFactory* getFactory(Interface interface) {
    if(interface == WINDOWS)
        return new WinGUIFactory;
    else 
        return new LinGUIFactory;
}

int main() {
    AbstractGUIFactory* factory = getFactory(WINDOWS);
    factory->createItem()->print();
    return 0;
}
