#include<iostream>
#include<exception>

using namespace std;

struct Node {
    int data;
    Node *next;
    Node *prev;
    Node(int i=0):data(i),next(nullptr),prev(nullptr){}
};


class LinkedListIterator {
    private:
        Node *curr;
    public:
        LinkedListIterator(Node *x = nullptr) {
            curr = x;
        }
        void setCurrent(Node *n) {
            curr = n;
        }
        void operator++() {
            if(curr == nullptr)
                throw exception();
            curr = curr->next;
        }
        void operator++(int n) {
            if(curr == nullptr)
                throw exception();
            curr = curr->next;
        }
        void operator--() {
            if(curr == nullptr)
                throw exception();
            curr = curr->prev;
        }
        void operator--(int n) {
            if(curr == nullptr)
                throw exception();
            curr = curr->prev;
        }
        int operator*() {
            if(curr == nullptr)
                throw exception();
            return curr->data;
        }
};

class LinkedList {
	private:
		Node *head;
		LinkedListIterator *lli;
	public:
		LinkedList() {
			head = nullptr;
			lli = nullptr;
		}
		void append(int i) {
			Node *n = new Node(i);
			if(!head) {
				head = n;
				return;
			}
			n->next = head;
			head->prev = n;
			head = n;
		}
		void remove() {
			// removal logic
		}
		LinkedListIterator& begin() {
			lli = new LinkedListIterator();
			lli->setCurrent(head);
			return *lli;
			
		}
		LinkedListIterator& end() {
			lli = new LinkedListIterator();
			if(!head) return *lli;
			Node *last = head;
			while(last->next != nullptr) last = last->next;
			lli->setCurrent(last);
			return *lli;
		}
		friend class LinkedListIterator;
};

int main() {
	LinkedList ll;
	ll.append(12);
	LinkedListIterator beg; // = ll.begin();
	cout<<(*beg);
	return 0;
}
		
		
