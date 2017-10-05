//program prints out the elements at given level using simple recursion
#include<iostream>
#include<queue>
#include<stack>

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

void printNodes(struct treeStruct *ptr){
	if(ptr == NULL){
		return;
	}
	printNodes(ptr->left);  //6 ka left completed
	cout<<ptr->element<<" ";
	printNodes(ptr->right);
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

void alternateLevelOrderTraverse(struct treeStruct *ptr){
	if(ptr == NULL)
		return;
	vector<vector<int>> main_vector{};
	queue<treeStruct*> q_tree{};
	q_tree.push(ptr);
	int count = 1;
	bool flag = false;  // false means we will insert from right to left
	while(!q_tree.empty()) {
		int tmp_count = 0;
		vector<int> vec_tmp{};
		stack<treeStruct*> tmp_stck{};
		for(int i = 0; i < count; ++i) {

			treeStruct *tmp = q_tree.front();
			vec_tmp.push_back(tmp->element);
			q_tree.pop();
			if(flag){
				if(tmp->left) {
					++tmp_count;
					tmp_stck.push(tmp->left);
				}
				if(tmp->right) {
					++tmp_count;
					tmp_stck.push(tmp->right);
				}
			}
			else {
				if(tmp->right) {
					++tmp_count;
					tmp_stck.push(tmp->right);
				}
				if(tmp->left) {
					++tmp_count;
					tmp_stck.push(tmp->left);
				}
			}
		}
		//push stack to queue
		while (!tmp_stck.empty()) {
			q_tree.push(tmp_stck.top());
			tmp_stck.pop();
		}
		
		flag = !flag;
		main_vector.push_back(vec_tmp);
		count = tmp_count;
	}
	
	// print the nodes
	for(auto& a : main_vector) {
		for(auto& b : a) {
			cout<<b<<" ";
		}
		cout<<endl;
	}

}

int main(){
	int lvl = 0;
	struct treeStruct *rootPtr = treeCreator();
	alternateLevelOrderTraverse(rootPtr);
	printNodes(rootPtr);
	return 0;
}



