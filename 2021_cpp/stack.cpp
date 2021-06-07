#include <iostream>
#include <climits>
#include <vector>
#include <queue>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <stack>

using namespace std;


/********** Implement queue with 2 stacks *************************/

class StackQueue{
private:   
    stack<int> s1;
    stack<int> s2;
public:
    void push(int x) {
        s1.push(x);
    }
    
    int pop() {
        if(!s2.empty()) {
            int elem = s2.top();
            s2.pop();
            return elem;
        }
        else {
            if(s1.empty())
                return -1;
            else {
                while(!s1.empty()) {
                    int elem = s1.top();
                    s1.pop();
                    s2.push(elem);
                }
                int to_return = s2.top();
                s2.pop();
                return to_return;
            }
        }
        
    }
};


/******** Implement stack with 2 Qs ***********/

class QueueStack{
private:
    queue<int> q1;
    queue<int> q2;
public:
    void push(int);
    int pop();
};


/* The method push to push element into the stack */
void QueueStack :: push(int x)
{
    if(q2.empty())
        q1.push(x);
    else
        q2.push(x);
}

/*The method pop which return the element poped out of the stack*/
int QueueStack :: pop()
{
    int elem = 0;
    if(!q1.empty()) {
        while(q1.size() > 1) {
            q2.push(q1.front());
            q1.pop();
        }
        elem = q1.front();
        q1.pop();
        return elem;
    }
    else if(!q2.empty()) {
        while(q2.size() > 1) {
            q1.push(q2.front());
            q2.pop();
        }
        elem = q2.front();
        q2.pop();
        return elem;
    }
    else
        return -1;
}

// Remove all adjacent duplicates using stack
string removeAdjacentDuplicates(string s) {
    stack<char> stk{};
    char ch{};
    for(int i=0; i<s.length(); ++i) {
        if(!stk.empty()) {
            ch = stk.top();
            if(s[i] == ch)
                continue;
        }
        stk.push(s[i]);
    }
    s = "";
    while(!stk.empty()) {
        s += stk.top();
        stk.pop();
    }
    std::reverse(s.begin(), s.end());
    return s;
}

// Function to return if the paranthesis are balanced or not
bool ispar(string s)
{
    stack<char> stk{};
    for(int i=0;i<s.length();++i) {
        if(s[i] == '{' || s[i] == '(' || s[i] == '[')
            stk.push(s[i]);
        else {
            if(s[i] == '}') {
                if(stk.empty() || stk.top() != '{')
                    return false;
                else
                    stk.pop();
            }
            if(s[i] == ')') {
                if(stk.empty() || stk.top() != '(')
                    return false;
                else
                    stk.pop();
            }
            if(s[i] == ']') {
                if(stk.empty() || stk.top() != '[')
                    return false;
                else
                    stk.pop();
            }
        }
    }
    if(!stk.empty())
        return false;
    else
        return true;
}

// Delete middle element of a stack without explicit use
// of data structure
void deleteMiddle(stack<int>&s, int k, int sizeOfStack) {
    if(s.empty())
        return;
    int elem = s.top();
    s.pop();
    deleteMiddle(s, k+1, sizeOfStack);
    if(k != (sizeOfStack/2) )
        s.push(elem);
}

// Evaluation of Postfix Expression
int evaluatePostfix(string S)
{
    stack<int> stk{};
    
    for(int i=0;i<S.length();++i) {
        if(isdigit(S[i]))
                stk.push(S[i]-'0');
        else {
            int elem_1 = stk.top();
            stk.pop();
            int elem_2 = stk.top();
            stk.pop();
            if(S[i] == '+')
                stk.push(elem_2 + elem_1);
            else if(S[i] == '-')
                stk.push(elem_2 - elem_1);
            else if(S[i] == '*')
                stk.push(elem_2 * elem_1);
            else
                stk.push(elem_2 / elem_1);
        }
    }
    if(!stk.empty())
        return stk.top();
}

string find_binary(int num) {
    
    int res = log2(num);
    string str{};
    
    while(res >= 0) {
        if(num >= pow(2, res)) {
            str += '1';
            num -= pow(2, res);
        }
        else
            str += '0';
        --res;
    }
    return str;
}

// Reverse First K elements of Queue
queue<int> reverseKElementsInQueue(queue<int> q, int k)
{
    //add code here.
    queue<int> q_res(q);
    stack<int> st{};
    
    if(k>=q_res.size())
        return q_res;
    
    for(int i=0;i<k;++i) {
        int elem = q_res.front();
        q_res.pop();
        st.push(elem);
    }
    while(!st.empty()) {
        int elem = st.top();
        st.pop();
        q_res.push(elem);
    }
    int sz = q_res.size();
    for(int i=0;i<sz-k;++i) {
        int elem = q_res.front();
        q_res.pop();
        q_res.push(elem);
    }
    return q_res;
}
// Maximum Rectangular Area in a Histogram
long long getMaxHistogramArea(int arr[], int n){

    stack<std::pair<int, int>> stk{};
    int *l_index = new int[n];
    int *r_index = new int[n];
    
    
    l_index[0] = 0;
    stk.push(std::make_pair(arr[0], 0));
    for(int i=1; i<n; ++i) {
        std::pair<int, int> pr{};
        if(!stk.empty() && arr[i] <= stk.top().first) {
            while(!stk.empty() && arr[i] <= stk.top().first) {
                pr = stk.top();
                stk.pop();
            }
            l_index[i] = pr.second; // store min index in result array
            stk.push(make_pair(arr[i], pr.second));
        }
        else {
            l_index[i] = i;
            stk.push(make_pair(arr[i], i));
            
        }
    }
    while(!stk.empty())
        stk.pop();
    r_index[n-1] = n-1;
    stk.push(make_pair(arr[n-1], n-1));
    for(int i=n-2; i>=0; --i) {
        std::pair<int, int> pr{};
        if(!stk.empty() && arr[i] <= stk.top().first) {
            while(!stk.empty() && arr[i] <= stk.top().first) {
                pr = stk.top();
                stk.pop();
            }
            r_index[i] = pr.second; // store min index in result array
            stk.push(make_pair(arr[i], pr.second));
        }
        else {
            r_index[i] = i;
            stk.push(make_pair(arr[i], i));
            
        }
    }
    long long mx_val = 1, value = 0;
    for(int i=0;i<n;++i) {
        value = (long long)arr[i]*(abs(r_index[i]-l_index[i])+1);
        if(value > mx_val)
            mx_val = value; 
    }

    return (long long)mx_val;
}

int main() {
    int arr[] = { 6, 2, 5, 4, 5, 1, 6};
    int n = sizeof(arr)/sizeof(arr[0]);
    // string res = removeAdjacentDuplicates("abbcccddeeeeeeee");
    // cout<<"Resultant string is: "<<res<<endl;
    // cout<<"Result of postfix expression evaluation: "<<evaluatePostfix("231*+9-")<<endl;
    //cout<<"Max hist value is: "<<getMaxHistogramArea(arr, n)<<endl;
    //cout<<"Binry number is: "<<find_binary(2)<<endl;
    queue<int> q{};
    for(int i=1;i<=5;i++)
        q.push(i);
    queue<int> res = reverseKElementsInQueue(q, 3);
    while(!res.empty()) {
        int elem = res.front();
        cout<<elem<<" ";
        res.pop();
    } 
    
    return 0;
}