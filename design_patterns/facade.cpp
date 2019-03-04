//Facade means front
//and this pattern works in a similar way...rather than providing hell lot of interfaces of all subsystems
//it provides a single unified interface for complete system..
//To achieve this it employs  composition wherein it encapsulates 
//objects of all systems...
#include<iostream>

using namespace std;

//Take the system of building a house..
//We need the following 
//1. Get approval from authority
//2. Hire an architect and get the map drawn.
//3. Hire masons and labourers
//3. Buy raw material
//4. Start building...

//Here follows the 5 classes to achieve this..
class authority{
public:
    void approveHousing(){
        cout<<"Approve the construction of house\n";
    }
    void cityPlan(){
        //this might have certain other encapsulated objects or methods.
    }
    void sanitationPlan(){
        //sanitation plan for whole city
    }
};

class Architect{
public:
    void drawMap(){
        cout<<"Draws map\n";
    }
    void chargeMoney(){
        //depending on the area and complexity.
    }
};



class contractor{
public:
    void quotePrice(){
        //Depending on the number of labourers..
    }
    void supplyLabourers(){
        cout<<"Supply laboureres depending on the requirement\n";
    }
};

class rawMaterial{
public:
    void buyCement(){
        cout<<"Buy Cement\n";
    }
    void buyBricks(){
        cout<<"Buy bricks\n";
    }
    void buyIron(){
        cout<<"Buy steel bars\n";
    }
    void buyAll(){
        buyCement();
        buyBricks();
        buyIron();
    }
};

class builder{
public:
    void startBuilding(){
        //Procedure how to build the house...
        cout<<"Start building\n";
    }
};


//Now lets take a common CLASS that represents the compositer
class Facade{
    authority a; 
    Architect at;
    contractor ct;
    rawMaterial rm; 
    builder bd;
public:
    void doAll(){
        a.approveHousing();
        at.drawMap();
        ct.supplyLabourers();
        rm.buyAll();
        bd.startBuilding();
    }
};

int main(){
    authority a; 
    Architect at;
    contractor ct;
    rawMaterial rm; 
    builder bd;

    //Below 5 calls show the subsystem calls ... rather we can create an object that encapsulates all or some in a single method.
    a.approveHousing();
    at.drawMap();
    ct.supplyLabourers();
    rm.buyAll();
    bd.startBuilding();

    cout<<"\nAll steps now in one call\n";
    Facade f;
    f.doAll();

    return 0;
}
