//merge two sorted linked lists

#include<iostream>
#include<cstdlib>


using namespace  std;

void insertElement(struct linked** , int);
void merge(struct linked* a1, struct linked* a2);
void printElement(struct linked*);


struct linked{
int node;
struct linked *next;
};


int main()
{
struct linked *head1 = NULL; 
struct linked *head2 = NULL; 


for(int i = 3; i <= 7; ++i)
	insertElement(&head1, i);

for(int i = 4; i<=12; i+=2)
	insertElement(&head2, i);

//a2 is smaller( 2nd one ) to be considered coz the other head will be useless and head1 will point to the only list..
if(head1->node <= head2->node){
	merge(head2, head1);
	printElement(head1);
}
else{
	merge(head1, head2);
	printElement(head2);
}

return 0;
}


void insertElement(struct linked** l, int el){
struct linked* dummy = *l;

while(dummy != NULL && dummy->next != NULL)
	dummy = dummy->next;

struct linked *newNode = (struct linked*)malloc(sizeof(struct linked));
newNode->next = NULL;
newNode->node = el;

//insert now
if(*l == NULL)
	*l = newNode;
else
	dummy->next = newNode;
}

//a2 goes with linked list , a1 is unlinked part
void merge(struct linked* a1, struct linked* a2){

if(a2->next == NULL){
	//
	a2->next = a1;
	return;
}
else if(a1->node <= a2->next->node){
	struct linked* t = a2->next;
	a2->next = a1;
	a1 = t;
	a2 = a2->next;


	merge(a1, a2);
}
else{
	a2 = a2->next;
	merge(a1, a2);
	}
}

void printElement(struct linked *h){
	struct linked* tmp = h;
	while( tmp != NULL){
		cout<<tmp->node<<", ";
		tmp = tmp -> next;
	}
	cout<<endl;
}









