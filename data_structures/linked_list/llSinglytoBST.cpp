//Creating the singly linked sorted list that later will be 
//converted to a Binary Search Tree

#include<iostream>
#include<cstdlib>

using namespace std;



typedef struct list{
int node;
list* next;
list* left;
list* right;
}list;

void createList();
void printList();
list* getNodePointer(int index);
list* convertToTree(int l, int r);
void traverseNodes(list *ptr);


list *hd;
int length = 9;

int main()
{
	createList();
	printList();
	list *head  = convertToTree(0, 8);
	cout<<"Printing the tree nodes\n";
	traverseNodes(head);
	return 0;
}

list* convertToTree(int l, int r){
        int mid = (l+r)/2;

	if( l > r)
		return NULL;
        if(l == r || l == mid || r == mid)
                return getNodePointer(mid);
        
        getNodePointer(mid)->left   = convertToTree( l, mid-1);
        getNodePointer(mid)->right  = convertToTree ( mid+1, r );
        return getNodePointer(mid);
}


void createList()
{
	hd = (struct list*)malloc(sizeof(list));
	hd->node = 2;
	hd->left = NULL;
	hd->right = NULL;

	hd->next = (struct list*)malloc(sizeof(list));
	hd->next->node = 4;
	hd->next->left = NULL;
	hd->next->right = NULL;

	hd->next->next = (struct list*)malloc(sizeof(list));
	hd->next->next->node = 6;
	hd->next->next->left = NULL;
	hd->next->next->right = NULL;

	hd->next->next->next = (struct list*)malloc(sizeof(list));
	hd->next->next->next->node = 8;
	hd->next->next->next->left = NULL;
	hd->next->next->next->right = NULL;

	hd->next->next->next->next = (struct list*)malloc(sizeof(list));
	hd->next->next->next->next->node = 12;
	hd->next->next->next->next->left = NULL;
	hd->next->next->next->next->right = NULL;

	hd->next->next->next->next->next = (struct list*)malloc(sizeof(list));
	hd->next->next->next->next->next->node = 15;
	hd->next->next->next->next->next->left = NULL;
	hd->next->next->next->next->next->right = NULL;

	hd->next->next->next->next->next->next = (struct list*)malloc(sizeof(list));
	hd->next->next->next->next->next->next->node = 20;
	hd->next->next->next->next->next->next->left = NULL;
	hd->next->next->next->next->next->next->right = NULL;

	hd->next->next->next->next->next->next->next = (struct list*)malloc(sizeof(list));
	hd->next->next->next->next->next->next->next->node = 24;
	hd->next->next->next->next->next->next->next->left = NULL;
	hd->next->next->next->next->next->next->next->right = NULL;

	hd->next->next->next->next->next->next->next->next = (struct list*)malloc(sizeof(list));
	hd->next->next->next->next->next->next->next->next->node = 30;
	hd->next->next->next->next->next->next->next->next->left = NULL;
	hd->next->next->next->next->next->next->next->next->right = NULL;


}

list* getNodePointer(int index)
{
	list* d = hd;
	while(index != 0){
		d = d->next;
		index--;
	}
	return d;
}
void printList()
{
	list *h = hd;
	cout<<"Printing the list\n";
	while(h != NULL)
	{
		cout<<h->node<<", ";
		h = h->next;
	}
	cout<<endl;
}

void traverseNodes(list *ptr)
{
        if(ptr == NULL)
                return;
        traverseNodes(ptr->left);
        cout<<ptr->node<<" , ";
        traverseNodes(ptr->right);
}
