#include<iostream>
#include<algorithm>
#include<stack>
#include<vector>

using namespace std;

struct treeStruct{
	int element;
	struct treeStruct *left;
	struct treeStruct *right;
};

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
	mainPtr = newNode(10);
	mainPtr->left = newNode(6);
	mainPtr->right = newNode(12);
	
	mainPtr->left->left = newNode(2);
	mainPtr->left->right = newNode(9);
	
	mainPtr->right->left = newNode(11);
	mainPtr->right->right = newNode(18);
	
	mainPtr->right->right->left = newNode(15);
	mainPtr->right->right->right = newNode(20);
	
	return mainPtr;
}

void PostorderTraversalWithoutRecur(struct treeStruct *ptr)
{
	vector<int> vec{};
	
	if(ptr == nullptr)
		return;
	stack<treeStruct*> st{};
	treeStruct *temp = ptr;
	while(temp) {
		vec.push_back(temp->element);
		st.push(temp);
		temp = temp->right;
	}
	while(!st.empty()) {
		treeStruct *p = st.top();
		treeStruct *p_rt = p->left;
		st.pop();
		while(p_rt) {
			vec.push_back(p_rt->element);
			st.push(p_rt);
			p_rt = p_rt->right;
		}
	}
	reverse(vec.begin(), vec.end());
	cout<<"Values are: ";
	for(auto a : vec)
		cout<<" "<<a;
	cout<<endl;
}
	
	

int main()
{
	struct treeStruct *rootPtr = treeCreator();
	PostorderTraversalWithoutRecur(rootPtr);
	return 0;
}



