#include<iostream>
#include<string>
#include<cctype>
using namespace std;

// trim from left
inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from right
inline std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from left & right
inline std::string& trim(std::string& s, const char* t = " \t\n\r\f\v")
{
    return ltrim(rtrim(s, t), t);
}

int main() {
	string s = "dhdjd88kjwehyfuwiefw897er89789";
	string trimmed = trim(s);
	int i = 0;
	for(; i < trimmed.size(); ++i)
		if(!isdigit(s[i]))
			break;
	
	string carved = trimmed.substr(0, i);
	if(carved.length() <= 0)
		return 0;
	else {
		long long lp = stoll(carved);
		if(lp > INT_MAX)
			return INT_MAX;
		else if(lp < INT_MIN)
			return INT_MIN;
		else
			return lp;
	}
	return 0;
}
