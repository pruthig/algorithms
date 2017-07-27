#include<iostream>
#include <chrono>
#include <thread>
#include<string>
#include<algorithm>

using namespace std;

int factorial(int n)
{
	if (n == 1 || n == 0)
		return 1;
	else {
		int fact = n*factorial(n - 1);
		if (fact > 1000003)
			fact %= 1000003;
		return fact;
	}
}

int main() {
	string modified = "ZCSFLVHXRYJQKWABGT";
	string original = modified;
	int total = 0;
	int str_size = modified.size();
	sort(original.begin(), original.end());
	// Now start parsing
	for (int i = 0; i < modified.size(); ++i) {
		if (modified[i] == original[i])
			continue;
		// Now modification detected.. find number of characters greater than original[i] and less than modified[i]
		int count = 0;
		for (int j = i + 1; j < original.size(); ++j) {
			if (original[j] > original[i] && original[j] < modified[i])
				++count;
		}
		// Count number of character left
		int to_forward = (count + 1)*(factorial(str_size - i - 1));
		total += to_forward;
		if (total > 1000003)
			total %= 1000003;
		// find the modified's current character in original string and replace the original's one
		swap(original[original.find(modified[i], i + 1)], original[i]);
		sort(original.begin() + i + 1, original.end());

	}
	cout << "Permutation count: " << total+1;
	std::this_thread::sleep_for(std::chrono::milliseconds(10000));
}
