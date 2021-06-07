#include <queue>
#include <utility>

string solution(int A, int B, int C) {
    // write your code in C++14 (g++ 6.2.0)
    std::priority_queue<std::pair<int, char>> pq{};
    if(A>0)
        pq.push(std::make_pair(A, 'a'));
    if(B>0)
        pq.push(std::make_pair(B, 'b'));
    if(C>0)
        pq.push(std::make_pair(C, 'c'));

    string res = "";
    while(!pq.empty()) {
        std::pair<int, char> pair_1 = pq.top();
        pq.pop();
        res += pair_1.second;
        pair_1.first--;
        if(!pq.empty()) {
            std::pair<int, char> pair_2 = pq.top();
            pq.pop();
            if(pair_1.first>pair_2.first) {
                res += pair_1.second;
                pair_1.first--;
                res += pair_2.second;
                pair_2.first--;
            }
            if(pair_2.first>0)
                pq.push(pair_2);
        }
        else {
            if(pair_1.first>0)
                res+= pair_1.second;
                pair_1.first--;
                return res;
        }
        if(pair_1.first>0)
            pq.push(pair_1);
            
    }
    //cout<<"result is"<<res<<endl;
    return res;
}
    
