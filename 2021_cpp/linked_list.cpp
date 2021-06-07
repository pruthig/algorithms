#include <iostream>
#include <unordered_map>
#include <climits>
#include <vector>
#include <queue>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <set>
#include <iomanip>
#include <map>

using namespace std; 

struct Node {
    int data;
    Node *next;
    Node *prev; // will be unused for singly linked list
    Node *arb;  // Specially for arbitrary pointer question..
    Node(int d) {
        data = d;
        next = NULL;
        prev = NULL;
        arb = NULL;
    }
};

struct Node* reverseList(struct Node *head)
{
    if(!head)
        return head;
    Node *cur = head;
    Node *prev = nullptr;
    Node *next = cur->next;
    
    while(cur) {
        cur->next = prev;
        if(next == nullptr)
            return cur;
        prev = cur;
        cur = next;
        next = cur->next;
    }
}

Node* detectLoop(Node* head) {
    if(head == nullptr)
        return nullptr;
    Node *slow = head;
    Node *fast = head;
    do {
        slow = slow->next;
        if(fast->next == nullptr)
            return nullptr;
        fast = fast->next->next;
        if(slow == fast)
            return slow;

    } while(slow && fast);
    
    return nullptr;
}

int countNodesinLoop(struct Node *head) {
    Node *loopPoint = detectLoop(head);
    if(!loopPoint)
        return 0;
        
    int count = 1;
    Node *counter = loopPoint->next;
    while(counter != loopPoint) {
        ++count;
        counter = counter->next;
    }
    return count;
}

void removeLoop(Node* head) {
    Node *loopPoint = detectLoop(head);
    if(!loopPoint)
        return;
    Node *slow = head;
    Node *prev = loopPoint;
    if(loopPoint == slow) {
        slow = loopPoint->next;
        while(slow->next != loopPoint)
            slow = slow->next;
        slow->next = nullptr;
        return;
    }
    while(loopPoint != slow) {
        slow = slow->next;
        prev = loopPoint;
        loopPoint = loopPoint->next;
    }
    prev->next = nullptr;
    
}

// Rotate a linked list by 'k' nodes
Node* rotate(Node* head, int k) {
    if(!head)
        return head;
    Node *last = head;
    while(last->next != nullptr)
        last = last->next;
    int i=0;
    Node *cur = head;
    Node *rotation_node = nullptr, *new_head = nullptr;
    for(; cur != nullptr; ++i, cur = cur->next) {
        if(i == k-1) {
            rotation_node = cur;
            if(rotation_node == last)
                return head;
            new_head = rotation_node->next;
            rotation_node->next = nullptr;
            last->next = head;
            return new_head;
        }
        
    }
    return head;
}

/* Function to check if linked list is Palindrome */
bool isPalindrome(Node *head) {
    Node *cur = head;
    string straight = "", reversed = "";
    while(cur->next != NULL) {
        straight += to_string(cur->data);
        cur = cur->next;
    }
    Node *new_head = reverseList(head);
    cur = new_head;
    while(cur->next != NULL) {
        reversed += to_string(cur->data);
        cur = cur->next;
    }
    return (straight == reversed);
}

class compareNodes {
public:
    bool operator()(Node *n1, Node *n2) {
        return (n1->data) > (n2->data);
    }
};

Node * mergeKSortedLists(Node *lsts[], int K)
{
    priority_queue<Node*, vector<Node*>, compareNodes> pq{};
    for(int i=0;i<K;++i) {
        pq.push(lsts[i]);
    }
    Node *head = nullptr, *cur = nullptr;
    
    while(!pq.empty()) {
        Node* nd = pq.top();
        pq.pop();
        if(!head) {
            head = new Node(nd->data);
        }
        else {
            cur = new Node(nd->data);
            cur->next = head;
            head = cur;
        }
        if(nd->next) {
            pq.push(nd->next);
        }
    }
    return reverseList(head);
}
        
// Reverse a doubly-linked-list
Node* reverseDLL(Node * head)
{
    if(!head)
        return nullptr;
    if(head->next == nullptr) {
        Node *prev = head->prev;
        head->prev = nullptr;
        head->next = prev;
        return head;
    }
    Node *tmp = head->next;
    Node *new_head = reverseDLL(head->next);
    
    Node *prev = head->prev;
    head->prev = tmp;
    if(prev == nullptr)
        head->next = nullptr;
    else
        head->next = prev;

    return new_head;
}

/*******************************************
*********** LRU Cache implementation *******
*******************************************/

struct custom_list {
    int key;
    int value;
    struct custom_list *prev;
    struct custom_list *next;
public:
    custom_list(int k, int v) {
        key = k;
        value = v;
        prev = nullptr;
        next = nullptr;
    }
    custom_list(std::pair<int, int> pr) {
        key = pr.first;
        value = pr.second;
        prev = nullptr;
        next = nullptr;
    }    
};

class LRUCache
{
private:
    static int list_size;
    static int capacity;
    static custom_list *head, *tail;
    // map with key, pointer 
    static map<int, custom_list*> mp; 
public:
    // constructor for cache
    LRUCache(int cap)
    {
        // Intialize the cache capacity with the given
    }
    
    static int get(int key)
    {
        // your code here
        if(mp.find(key) != mp.end()) {
            custom_list *lst_ptr = mp[key];
            int data = lst_ptr->value;
            if(lst_ptr == head)
                return data;

            
            // Correct prev and next pointers for attaching
            // to front
            lst_ptr->prev->next = lst_ptr->next;
            if(lst_ptr->next)
                    lst_ptr->next->prev = lst_ptr->prev;
            
            lst_ptr->prev = nullptr;
            lst_ptr->next = head;
            head->prev = lst_ptr;
            head = lst_ptr;

            return data;
            
        }
        else
            return -1;
    }

    // storing key, value pair
    static void set(int key, int value)
    {
        // your code here 
        if(mp.find(key) != mp.end())
            return;
            

        if(list_size == capacity) {
            if(capacity == 1) {
                delete head;
                head = nullptr;
            }
            else {
                custom_list *prev_node = tail->prev;
                tail->prev->next = nullptr;
                mp.erase(tail->key);
                delete(tail);
                tail = prev_node;
            }

        }
        custom_list *new_node = new custom_list(make_pair(key, value));
        new_node->next = head;
        if(head)
            head->prev = new_node;
        else {
            head = new_node;
            tail = new_node;
        }
        mp.insert(make_pair(key, new_node));
        ++list_size;
    }
};

int LRUCache::list_size = 0;
int LRUCache::capacity = 0;
map<int, custom_list*> LRUCache::mp;
custom_list* LRUCache::head = nullptr;
custom_list* LRUCache::tail = nullptr;

/*************************************
/******End - LRU Cache Implementation
**************************************/

// Clone linked list with node having arbitrary pointer as an additional member
Node *cloneArbitraryPointerList(Node *head1) {
    // Your code here
    // Map tp store mapping of list 1 nodes
    // with list 2 nodes..
    unordered_map<Node*, Node*> ump{};
    Node *cur1 = head1, *cur2 = nullptr, *head2 = nullptr;
    while(cur1 != nullptr) {
        Node *new_node = new Node(cur1->data);
        if(!head2) {
            head2 = new_node;
            cur2 = head2;
        }
        else {
            cur2->next = new_node;
            cur2 = cur2->next;
        }
        ump.insert(std::make_pair(cur1, cur2));
        cur1 = cur1->next;
    }
    // Traver se again
    cur1 = head1;
    cur2 = head2;
    while(cur1) {
        cur2->arb = ump[cur1->arb];
        cur1 = cur1->next;
        cur2 = cur2->next;
    }
    return head2;
}

int main() {
    return 0;
}

// Notes

/*
To check if linked list is palindrome
> You can use stack.
> Reverse 2nd half of list and match with 1st half.
> Use recursion and start increment head pointer as stack unfolds
> To check the intersection point of linked list without comparing data and addresses is to create a circular loop in first list and find the loop in second linked list.
> 
