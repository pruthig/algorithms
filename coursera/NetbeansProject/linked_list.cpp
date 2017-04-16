/*
 * Copyright 2017 Gaurav Pruthi
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include<iostream>
#include <cassert>

using namespace std;

class ListNode {
private:
    int value;
    ListNode *next;
public:
    ListNode():value(0), next(NULL){}
    ListNode(int val):value(val),next(NULL){}
    int getValue() {
        return value;
    }
    void setValue(int val) {
        value = val;
    }
    ListNode* getNext() { return next; }
    void setNext(ListNode *nxt) { next = nxt; }

};

class LinkedList {
private:
    ListNode *head;
public:
    LinkedList();
    ~LinkedList();
    void printList();
    int getSize();
    int getNthElement(int n);
    int getMiddle();

    void prependElement(int value);
    void appendElement(int value);

    ListNode* getHead();
    void setHead(ListNode *head);

    void reverseList(ListNode *node, ListNode *nextNode);

    bool checkLoopInList();
    bool checkPalindrome();
};


LinkedList::LinkedList() {
    head = NULL;
}

LinkedList::~LinkedList() {
    ListNode *next = NULL;
    ListNode *cursor = getHead();
    while(cursor != NULL) {
        next = cursor->getNext();
        delete cursor;
        cursor = next;
    }
    head = NULL;
}

ListNode* LinkedList::getHead() {
    return head;
}

void LinkedList::setHead(ListNode *head) {
    this->head = head;
}

void LinkedList::prependElement(int value) {
    ListNode *cursor = getHead();
    
    ListNode *newNode = new ListNode(value);
    if(cursor == NULL ) {
        setHead(newNode);
    }
    else {
        ListNode *newNode = new ListNode(value);
        newNode->setNext(head);
        setHead(newNode);
    }
}

void LinkedList::appendElement(int value) {
    ListNode *cursor = getHead();
    
    ListNode *newNode = new ListNode(value);
    if(cursor == NULL ) {
        setHead(newNode);
    }
    else {
        while(cursor->getNext() != NULL) 
            cursor = cursor->getNext();
        cursor->setNext(newNode);
    }
}

int LinkedList::getNthElement(int n) {
    int size = getSize();
    if(size < n) {
        cout<<"Size of list is smaller than requested element\n";
    }
    else {
        ListNode *cursor = getHead();
        while(n--)
            cursor = cursor->getNext();
        return cursor->getValue();
    }
    assert(0);
}

int LinkedList::getMiddle() {
    if(!getHead()){
        cout<<"Head is NULL"<<endl;
        assert(0);
    }
    ListNode *slow = getHead();
    ListNode *fast = getHead();

    while(fast && fast->getNext()) {
        slow = slow->getNext();
        fast = fast->getNext()->getNext();
    }
    return slow->getValue();
}

int LinkedList::getSize() {
    int counter = 0;
    ListNode *cursor = getHead();
    while(cursor != NULL ) {
        counter++;
        cursor = cursor->getNext();
    }
    return counter;
}


void LinkedList::printList() {
    ListNode *cursor = getHead();
    while(cursor != NULL ) {
        cout<<cursor->getValue();
        cursor = cursor->getNext();
        if(cursor)
            cout<<",";
    }
    cout<<endl;
}


void LinkedList::reverseList(ListNode *node, ListNode *nextNode) {
    if(nextNode->getNext() == NULL) {
        nextNode->setNext(node);
        this->setHead(nextNode);
        return;
    }
    reverseList(nextNode, nextNode->getNext());
    if(node == NULL) {
        nextNode->setNext(NULL);
        return;
    }

    nextNode->setNext(node);
}

// Floyd Cycle detection algorithm-using 2 pointers-slow and fast
bool LinkedList::checkLoopInList() {
    if(!getHead()){
        cout<<"Head is NULL"<<endl;
        assert(0);
    }
    ListNode *slow = getHead();
    ListNode *fast = getHead();

    while(slow && fast && fast->getNext()) {
        slow = slow->getNext();
        fast = fast->getNext()->getNext();
        if(fast == slow)
            return true;
    }
    return false;
}

bool LinkedList::checkPalindrome() {
    // Its a recursive approach where in we use an extra pointer which
    // will start recursing when we reach to the end of list
    // left traversing -----> <----- stack_provided_right_pointer
	return false;
}
int linked_list_main() {
    LinkedList *list = new LinkedList;

    list->appendElement(1);
    list->appendElement(2);
    list->appendElement(3);
    list->appendElement(4);
    list->appendElement(5);
    cout<<"Size of list is: "<<list->getSize()<<endl;
    cout<<"0th element is: "<<list->getNthElement(0)<<endl;
    cout<<"Middle of linked list is "<<list->getMiddle()<<endl;
    cout<<"Printing the list:"<<endl;
    list->printList();

    // Reverse the list
    cout<<"Head of list before reversal is: "<<list->getHead()<<endl;
    if(list->getHead())
        list->reverseList(NULL, list->getHead());
    cout<<"Head of list after reversal is:  "<<list->getHead()<<endl;
    cout<<"Printing list after reversal: "<<endl;
    list->printList();
    cout<<"Reversing again**"<<endl;
    if(list->getHead())
        list->reverseList(NULL, list->getHead());
    
    cout<<"Printing list after 2nd reversal: "<<endl;
    list->printList();

    std::cout << std::boolalpha;
    cout<<"Checking bool functio returned: "<<list->checkLoopInList()<<endl;
    cout<<"Going for destruction"<<endl;
    
    list->~LinkedList();
    return 0;
}

