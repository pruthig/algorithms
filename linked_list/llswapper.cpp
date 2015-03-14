//this program will swap pair of nodes pointer wise
//not data wise

#include<iostream>
#include<cstdlib>


using namespace  std;

void insertElement(int n);
void listSwapper(struct linked* prev, struct linked* node);
void printList();


struct linked{
int node;
struct linked *next;
};

struct linked *head = NULL;

int main()
{
for(int i=1;i<=6;i++){
insertElement(i);
}
listSwapper(head, head->next->next);
printList();
cout<<endl;

return 0;
}

//Take two pointers ... one will move by 1 pos while another by 2...
void listSwapper(struct linked* prev, struct linked* node){
if(node == NULL || prev == NULL || node->next == NULL)
	return;
	
listSwapper(node, node->next->next);
struct linked* t1= prev->next;
struct linked* t2 = node->next;
node->next = t1;
prev->next = node;
t1->next = t2;
}


void insertElement(int n){
int element;
if(head == NULL){
struct linked** dummy = &head;
struct linked *newNode = (struct linked*)malloc(sizeof(struct linked));
newNode->next = NULL;
newNode->node = n;
*dummy = newNode;
return;
}
struct linked* dummy = head;
while(dummy->next != NULL)
dummy = dummy->next;

struct linked *newNode = (struct linked*)malloc(sizeof(struct linked));
newNode->next = NULL;
newNode->node = n;
//insert now
dummy->next = newNode;
}


void printList(){
struct linked* dummy = head;
cout<<endl;
while(dummy != NULL){
	cout<<dummy->node;
	dummy = dummy->next;
}
}





