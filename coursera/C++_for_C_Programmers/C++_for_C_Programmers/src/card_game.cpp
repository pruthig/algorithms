/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include<iostream>
#include<cassert>
#include <vector>
#include <algorithm>

using namespace std;

enum class suit : short {
    SPADE,
    HEART,
    DIAMOND,
    CLUB
};

class pips {
    int val;
public:
    pips(int value) { assert(value > 0 && value < 14); }
    pips(){}
    ~pips(){}
    int get_value() { return val; }
    friend ostream& operator<<(ostream& out, const pips& p);
};

class Card {
    pips p;
    suit s;
public:
    friend ostream& operator<<(ostream& out, const Card& c);
    pips get_pips() const { return p; }
    suit get_suit() const { return s; }
    Card(pips *p, suit s):s(s),p(*p){}
    //Card(suit s, int i):s(s),p(new pips(i)){}
    Card(){};
    ~Card(){}
};

ostream& operator<<(ostream& out, const Card& c) {
    out<<c.get_pips().get_value()<<(int)c.get_suit();
    return out;
}

void init_deck(vector<Card>& cards) {
    
    for (int i = 1 ; i < 14; ++i) {
        Card c(new pips(i), suit::CLUB);
        cards[i-1] = c;
    }
    for (int i = 1 ; i < 14; ++i) {
        Card c(new pips(i), suit::DIAMOND);
        cards[i+12] = c;
    }
    for (int i = 1 ; i < 14; ++i) {
        Card c(new pips(i), suit::HEART);
        cards[i+25] = c;
    }
    for (int i = 1 ; i < 14; ++i) {
        Card c(new pips(i), suit::SPADE);
        cards[i+38] = c;
    }
}

void print(vector<Card>& deck) {
    for(auto p : deck)
        cout<<" "<<p<<" ";
    cout<<endl;
}

bool is_flush(vector<Card>& deck) {
    suit s = deck[0].get_suit();
    for(auto a = deck.begin()+1; a != deck.end(); ++a) {
        if(a->get_suit() != s) 
            return false;
    }
    return true;
}

bool is_straight(vector<Card>& hand) {
    int pips_v[5], i = 0;
    for(auto p = hand.begin(); p != hand.end(); ++p) {
        pips_v[i++] = p->get_pips().get_value();
    }
    sort(pips_v, pips_v+5);
    if(pips_v[0] != 1)
        return ( pips_v[0] == pips_v[1]-1 && pips_v[1] == pips_v[2]-1 && pips_v[2] == pips_v[3]-1 && pips_v[3] == pips_v[4]-1);
    else
        return (pips_v[0] == pips_v[1]-1 && pips_v[1] == pips_v[2]-1) && (pips_v[2] == pips_v[3]-1 && pips_v[3] == pips_v[4]-1)
                || (pips_v[1] == 10) && (pips_v[2] == 11) && (pips_v[3] == 12) && (pips_v[4] == 13);
}
bool is_straight_flush(vector<Card>& deck) {
    return is_straight(deck) && is_flush((deck));
}

void card_game_main(){
    vector<Card> deck(52);

    init_deck(deck);
    int how_many = 0;
    int flush_count = 0;
    int str_count = 0;
    int str_flush_count = 0;
    
    cout<<"How many shuffles";
    cin>>how_many;
    
    for(int i = 0; i < how_many; ++i) {
        random_shuffle(deck.begin(), deck.end());
        vector<Card> hand;
        int j = 0;
        for(auto p = deck.begin(); j < 5; ++p) {
            hand[j++] = *p;
        }
        if(is_flush(hand)) flush_count++;
        if(is_straight(hand)) str_count++;
        if(is_straight_flush(hand)) str_flush_count++;
    }
    cout<<"Flushes :" << flush_count<< " Straights: "<<str_count<<" Straight and Flush "<<str_flush_count<<" out of " <<how_many<<endl;
    
}