#include<vector>
#include<cstring>
#include<iostream>
#include<cmath>

using namespace std;

int pwr(int a, int b) {
	int y = 1;

	while (true) {
		if ((b & 1) != 0) y = a*y;
		b = b >> 1;
		if (b == 0) return y;
		a *= a;
	}
}

int find_power(int res[], int x, int power, int mod_b);

int main() {
	int x = 79161127;
	int n = 99046373;
	int mod_b = 57263970;
	int l = pwr(2, floor(log(x) / log(2)));
	int *res = (int*)calloc(1, sizeof(int)*(n + 1));
	int temp_n = n;
	int modulus = 1;
	while (temp_n >= 1) {
		int power = pwr(2, (int)floor(log(temp_n) / log(2)));
		temp_n = temp_n - power;
		modulus *= find_power(res, x, power, mod_b);
		modulus = modulus%mod_b;
	}
	cout << "Modulus is: " << modulus;
	return 0;
}

int find_power(int res[], int x, int power, int mod_b)
{
	if (res[power] != 0)
		return res[power];

	if (power == 1) {
		res[1] = (x%mod_b);
		return res[1];
	}
	res[power] = ((find_power(res, x, power / 2, mod_b) % mod_b)*(find_power(res, x, power / 2, mod_b) % mod_b)) % mod_b;
	return res[power];
}
