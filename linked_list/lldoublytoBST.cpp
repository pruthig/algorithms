//Creating the doubly linked sorted list that later will be 
//converted to a Binary Search Tree

#include<iostream>
#include<cstdlib>

using namespace std;



typedef struct list{
int node;
list* left;
list* right;
}list;

void createList();
void printList();
list* getNodePointer(int index);
void convertToTree(int l, int r);


list *hd;

int main()
{
	createList();
	printList();
	convertToTree(0, 6);
	return 0;
}

void convertToTree(int l, int r){
	int mid = (l+r)/2;

	if(l == r){
		getNodePointer(mid)->left = NULL;
		getNodePointer(mid)->right = NULL;
		cout<<"\nReturning values "<<getNodePointer(mid)->node<<endl;
		return;
	}
	getNodePointer(mid)->left = getNodePointer( (l+mid-1)/2 );
	getNodePointer(mid)->right = getNodePointer( (mid+1+r)/2 );
        cout<<" l, mid-1 "<<l<<", "<<(mid-1)<<endl;	
	convertToTree(l, mid-1);
	cout<<" mid+1, r "<<mid+1<<", "<<r<<endl;	

	convertToTree(mid+1, r);
	return;
}




void createList()
{
	hd = (struct list*)malloc(sizeof(list));
	hd->node = 4;
	hd->left = NULL;
	hd->right = (struct list*)malloc(sizeof(list)); 

	hd->right->left = hd;
	hd->right->node = 6;
	hd->right->right = (struct list*)malloc(sizeof(list)); 

	hd->right->right->left = hd->right;
	hd->right->right->node = 8;
	hd->right->right->right = (struct list*)malloc(sizeof(list)); 

	hd->right->right->right->left = hd->right->right;
	hd->right->right->right->node = 12;
	hd->right->right->right->right = (struct list*)malloc(sizeof(list)); 


	hd->right->right->right->right->left = hd->right->right->right;
	hd->right->right->right->right->node = 14;
	hd->right->right->right->right->right = (struct list*)malloc(sizeof(list)); 

	hd->right->right->right->right->right->left = hd->right->right->right->right;
	hd->right->right->right->right->right->node = 20;
	hd->right->right->right->right->right->right = (struct list*)malloc(sizeof(list)); 

	hd->right->right->right->right->right->right->left = hd->right->right->right->right->right;
	hd->right->right->right->right->right->right->node = 24;
	hd->right->right->right->right->right->right->right = NULL;

}

list* getNodePointer(int index)
{
	list* d = hd;
	while(index != 0){
		d = d->right;
		index--;
	}
	return d;
}
void printList()
{
	list *h = hd;
	while(h != NULL)
	{
		cout<<h->node<<", ";
		cout<<endl;
		h = h->right;
	}
}
