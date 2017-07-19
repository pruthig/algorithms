#include<iostream>
#include<vector>
#include<algorithm>
using namespace std;

struct Interval {
public:
	Interval(int i, int j):left(i), right(j){}
	Interval(){}
	int left;
	int right;
};


struct less_than_key
{
    inline bool operator() (const Interval& struct1, const Interval& struct2)
    {
        return (struct1.left < struct2.left);
    }
};

vector<Interval>  merge(vector<Interval> in) {
	vector<Interval> input(in);
	sort(input.begin(), input.end(), less_than_key());
	int i = 0;
	vector<Interval> solution;
	if(input.size() == 0)
		return solution;
	int cur_left = input[0].left;
	int cur_right = input[0].right;
	
	if(input.size() == 1)
	{
		Interval int_l(cur_left, cur_right);
		solution.push_back(int_l);
		return solution;
	}
	while(i<input.size()-1) {
		if(cur_right < input[i+1].left)
		{
			Interval int_v(cur_left, cur_right);
			solution.push_back(int_v);
			i++;
			cur_left = input[i].left;
			cur_right = input[i].right;
			if(i == input.size()-1)
				break;
		}
		else if(input[i+1].right < cur_left)
		{
			cur_left = input[i+1].left;
			i++;
			if(i == input.size()-1)
				break;
		}
		else if(cur_left <= input[i+1].left && cur_right >= input[i+1].right)
		{
			i++;continue;
		}
		else if(cur_left >= input[i+1].left && cur_right <= input[i+1].right)
		{
			cur_left = input[i+1].left;
			cur_right = input[i+1].right;
			i++;continue;
		}
		else if(cur_left >= input[i+1].left && cur_left < input[i+1].right && cur_right >= input[i+1].right) {
			cur_left = input[i+1].left;
			i++;
		}	
		else {
			cur_right = input[i+1].right;
			i++;
		}
		
	}
	Interval int_l(cur_left, cur_right);
	solution.push_back(int_l);
	return solution;
	
}

int main() {
	vector<Interval> vecInt{ {30, 63}, {66, 94}, {36, 87}, {16, 86}, {26, 85}, {24, 50}, {17, 84}, {5, 25}, {67, 81}, {23, 54}, {84, 99}, {48, 85}, {23, 28}, {3, 86}, {63, 79}, {18, 73}, {6, 68}, {34, 40}, {61, 66}, {60, 96}, {95, 99}, {1, 10}, {4, 82}, {19, 78}, {23, 61}, {30, 45}, {53, 87}, {10, 42}, {80, 93}, {33, 73}, {64, 65}, {29, 71}, {73, 89}, {2, 98}, {62, 67}, {84, 98}, {43, 58}, {20, 45}, {86, 92}, {22, 100}, {72, 74}, {5, 52}, {48, 56}, {69, 93}, {8, 98}, {37, 47}, {19, 45}, {22, 99}, {34, 97}, {21, 80}, {58, 77}, {48, 66}, {59, 91}, {18, 33}, {2, 7}, {8, 92}, {12, 32}, {17, 83}, {11, 16}, {60, 75}, {9, 11}, {3, 61}, {4, 18}, {53, 68}, {17, 39}, {18, 93}, {15, 55}, {4, 34}, {48, 85}, {61, 65}, {59, 77}, {15, 37}, {62, 82}, {4, 78}, {80, 96}, {4, 42}, {15, 48}, {27, 45}  }; 
	//Interval i1(1, 3);
	//Interval i1(2, 6);
	//Interval i1(8, 10);
	vector<Interval> solution = merge(vecInt);
	for(auto a : solution) {
		cout<<a.left<<" , "<<a.right<<endl;
	}
	return 0;	
}
