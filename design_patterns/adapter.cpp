// An adapter pattern converts the interface of a class into another interface the clients expect. 
// Adapter lets classes work together that couldn't otherwise because of incompatible interfaces.
#include <iostream>
#include <string>

/* Adaptee (source) interface */
class EuropeanSocketInterface
{
    public:
        virtual int voltage() = 0;
};

/* Adaptee */
class Socket : public EuropeanSocketInterface
{
    public:
        int voltage() { return 230; }
};

/* Target interface */
class USASocketInterface {
    public:
        virtual int voltage() = 0;
};

/* The Adapter */
class Adapter : public USASocketInterface {
    EuropeanSocketInterface* socket;
    public:
        void plugIn(EuropeanSocketInterface* outlet) {
            socket = outlet;
        }
        int voltage() { return 110; }
};

/* Client */
class ElectricKettle {
    USASocketInterface* power;

    public:
        void plugIn(USASocketInterface* supply) {
            power = supply;
        }

        void boil() {
            if (power->voltage() > 110)
            {
                std::cout << "Kettle is on fire!" << std::endl;
                return;
            }
        }
};

int main()
{
    Socket* socket = new Socket;
    Adapter* adapter = new Adapter;
    ElectricKettle* kettle = new ElectricKettle;

    /* Pluging in. */
    adapter->plugIn(socket);
    kettle->plugIn(adapter);

    /* Having coffee */
    kettle->boil();

    return 0;
}

