// State pattern allows a object to alter its behaviour when its internal state changes
// Here we have a account context/class maintained by a Bank. State is an interface which will
// be inherited by concrete states. Account state will be determined by pointer to state contained in it.

#include<iostream>
#include<string>

using namespace std;

class Product {
    int price;
    string name;
    public:
        Product(int p, string n) {
            price = p;
            name = n;
        }
        int getPrice() {
            return price;
        }
        string getName() {
            return name;
        }
};

class state {
    public:
        virtual int get_discounted_amount(Product *p) = 0;
};

class silver_state : public state {
    public:
        int get_discounted_amount(Product *p) {
            return p->getPrice()*0.90;
        }
};

class gold_state : public state {
    public:
        int get_discounted_amount(Product *p) {
            return p->getPrice()*0.80;
        }
};

class platinum_state : public state {
    public:
        int get_discounted_amount(Product *p) {
            return p->getPrice()*0.70;
        }
};

class TobaccoShop {
    state *s;
    Product *p;
    int amount_deposited = 0;
    public:
        TobaccoShop() {
            s = nullptr;
            p = new Product(10, "tobacco");
        }
        void advance_deposit_amount(int amt) {
            amount_deposited = amt;
            if(amount_deposited>=1000 && amount_deposited<=5000)
                s = new silver_state;
            else if(amount_deposited>=5000 && amount_deposited<=10000)
                s = new gold_state;
            else if(amount_deposited>=10000)
                s = new platinum_state;
            else { /* do nothing */ }
        }
        int getTobaccoPrice() {
            if(!s)
                return p->getPrice();
            else
                return s->get_discounted_amount(p);
        }
};

int main() {
    TobaccoShop ts;
    cout<<"Price with no deposit amount is: "<<ts.getTobaccoPrice()<<endl;
    ts.advance_deposit_amount(20000);
    cout<<"Price after deposit amount is: "<<ts.getTobaccoPrice();
    return 0;
}
        