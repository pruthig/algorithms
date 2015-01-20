//This program will find the next permutation 
//Algo 
/* 
Start from right pick the smallest number 
now from the right elements pick the smallest  
Swap these two 
and sort them in non-decreasing order 
*/ 

#include<iostream>
#include<cstdlib>
#include<string>
#include<algorithm>
#include<climits>

using namespace std;



typedef struct list{
int node;
list* left;
list* right;
}list;

void createList(int t);
void printList();
list* getNodePointer(int index);
void nextPermute();
list* newNode(int t);

list *hd;
list *dummy = NULL;

int main()
{
	int num = 0;
	cout<<"\nEnter the number";
	cin>>num;
	if(num > 10)	
	{
		createList(num);
		nextPermute();
	}
	//printList();
	return 0;
}



void createList(int num)
{
	list *dummyNode = NULL ;
	list *dummy = NULL;
	//modified
	while(num != 0){
		dummyNode = newNode(num%10);
		dummyNode->right = dummy;
		if(dummy)
			dummy->left = dummyNode;
		dummy = dummyNode;
		num = num/10;
	}
	hd = dummy;

	
}

list* newNode(int t)
{
	list *s = (list*)malloc(sizeof(list));
	s->left = NULL;
	s->right = NULL;
	s->node = t;
	return s;
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


void nextPermute()
{
	list* maxNode = NULL;
	string s = "";
	string srt = "";

	struct list *dummy = hd;
	if(dummy == NULL || dummy->right == NULL){
		return;
	}

	//goto right extreme
	while(dummy->right != NULL)
		dummy = dummy->right;

	//find whose left is min than current one.
	while(dummy->left->node >= dummy->node)
	{
		dummy=dummy->left;
		if(dummy == NULL || dummy->left == NULL)
			return;
	}

	//now dummy points to the node whose left is first smallest
	list *pointerToLeftIsMin = dummy;

	//now move dummy fwd to get next smallest
	int reqNode = pointerToLeftIsMin->left->node;  //it is 2,  dummy points to 4
	int max = INT_MAX;
	//Next min will be find in units from pointerToLeftIsMin to Last element..then numbers from start till left min are 
	//appended and rest of them are sorted
	while(dummy != NULL)
	{
		if(dummy->node < max && dummy->node > reqNode){     
			max = dummy->node;
			maxNode = dummy;
		}
		dummy = dummy->right;
	}

	//swap the two
	int tmp = pointerToLeftIsMin->left->node; 
	pointerToLeftIsMin->left->node = maxNode->node;
	maxNode->node = tmp;
	//swapped

	dummy = hd;
	while(dummy != pointerToLeftIsMin){
		s = s.append(to_string(dummy->node));
		dummy = dummy->right;
	}
	cout<<"Base string is "<<s<<endl;
	dummy = pointerToLeftIsMin;
	while(dummy != NULL){
		srt = srt.append(to_string(dummy->node));
		dummy = dummy->right;
	}

	//sort string
	std::sort(srt.begin(), srt.end());
	s = s.append(srt);
	cout<<"Result is "<<s<<endl;

	return;
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
