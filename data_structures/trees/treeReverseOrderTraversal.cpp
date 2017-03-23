//program prints out the reverse of tree using 1Q and 2Stacks.

#include<iostream>
#include<stack>
#include<queue>


using namespace std;

typedef struct treeStruct{
int element;
struct treeStruct *left;
struct treeStruct *right;
}treeStruct;


struct treeStruct* newNode(int data)
{
struct treeStruct *newElement = new(struct treeStruct);
newElement->left = NULL;
newElement->right = NULL;
newElement->element = data;
return newElement;
}

struct treeStruct* treeCreator(){
//create the main pointer
struct treeStruct *mainPtr;
mainPtr = newNode(1);
mainPtr->left = newNode(2);
mainPtr->right = newNode(3);

mainPtr->left->left = newNode(4);
mainPtr->left->right = newNode(5);

mainPtr->right->left = newNode(6);
mainPtr->right->right = newNode(7);

return mainPtr;
}

stack<int> s_partial;
stack<int> s_full;
queue<treeStruct *> q;

void reverseLevelOrder(struct treeStruct *ptr)
{
	int size = 0, tmp = 0;

	if(ptr != NULL)
	{
		q.push(ptr);
		size++;
	}
	else
		return;
	while(1)
	{
		tmp = 0;
		while(size != 0)
		{
			ptr = q.front();
			s_partial.push(ptr->element);
			q.pop();   size--;
			if(ptr == NULL)
				return;
			s_partial.push(ptr->left);  tmp++;
			s_partial.push(ptr->right); tmp++;
		
		}//pop and put in full stack
		while(!s_partial.size())
		{
			s_full.push(s_partial.front());
			s_partial.pop();
		}
		size = tmp;
	}


}

int main()
{
	int lvl = 0;
	struct treeStruct *rootPtr = treeCreator();
	printNodes(rootPtr);
	while(s_partial.size() != 0)
	{
		cout<<s_full.front()<<" , ";
		s_full.pop();
	}
	return 0;
}



