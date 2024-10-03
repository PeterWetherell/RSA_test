#include <iostream>
#include <random>
#include <vector>
#include <chrono>

using namespace std;

random_device rd;  // Initialize random number generator
mt19937 gen;

unsigned mult(unsigned a, unsigned b, unsigned n){ // a*b mod(n)
    unsigned r = 0;
    while (b > 0){
        if (b & 1){
            r = (r + a) % n;
        }
        a = (a << 1) % n;
        b >>= 1;
    }
    return r;
}

unsigned extendedGCD(unsigned q, unsigned r, int &x, int &y, unsigned n){
    if (r == 0){
        x = 1;
        y = 0;
        return q;
    }
    int x1, y1;
    unsigned gcd = extendedGCD(r, q%r, x1, y1, n);
    
    x = y1;

    y = x1 - (q/r)*y1;
    
    //cout << x << " " << y << endl;

    return gcd;
}

bool modInverse(int e, unsigned phi_n, unsigned& d) {
    int x, y;
    int g = extendedGCD(e, phi_n, x, y, phi_n);
    //cout << x << endl;
    while (x < 0){
        x = x + phi_n;
    }
    d = x % phi_n; // Ensure positive result
    return g != 1; // if g == 1 then they are coprime
}

unsigned fastExpentiation(unsigned M, unsigned e, unsigned n) { //return M^e mod n
    unsigned r = 1;
    M %= n;
    while (e > 0){
        if (e & 1){ //if current bit is set
            r = mult(r,M,n); //Make sure they don't get too large
        }
        M = mult(M,M,n); //square a each time
        e >>= 1; //shift b right by 1
    }
    return r;
}

bool millerRabin (int n, int k){
    if (n <= 1 || n == 4) return false;
    if (n <= 3) return true;

    int s = 0; //n-1 = 2^s*d basically we take all the zeros out from in front of d and replace it with s
    int d = n-1;
    while ((d & 1) == 0){
        s ++;
        d >>= 1;
    }
    for (int i = 0; i < k; i ++){
        int a = 2 + (gen() % (n-3)); //rand from 2 to n-2
        int x = fastExpentiation(a, d, n);

        if (x == 1 || x == n - 1) continue; // If x is a trivial root, continue

        // Check for nontrivial roots
        bool foundNontrivialRoot = false;
        for (int j = 0; j < s; j ++){
            int x = (x*x) % n;
            if (x == n - 1) {
                foundNontrivialRoot = true; // Found a trivial root
                break; // Break out of the loop
            }
        }

        if (!foundNontrivialRoot) {
            return false; // Composite
        }
    }
    return true; // probably prime
}

// Function to find a strong prime
int generateStrongPrime(int bits) {
    int prime = 0;

    while (true) {
        // Generate a random odd number of the desired bit length
        uniform_int_distribution<> dist((1 << (bits - 1)), (1 << bits) - 1);
        prime = dist(gen);
        if (prime % 2 == 0) prime++; // Ensure it's odd

        // Test for primality
        if (millerRabin(prime, 20)) {
            return prime;
        }
    }
}

// Function to try to do the fermat factor attack
void fermatAttack(int n){ //Goal: find a, b such that a^2 - b^2 = n
    int a = ceil(sqrt(n)); //start with larger a than required
    int b = 0;
    int bsquared = b*b;
    unsigned k = a*a - n; // n = a^2 - b^2 -> b^2 = a^2 - n
    while(k != bsquared){
        if (bsquared > k){ // We need to grow k
            k += 2*a + 1; // (a+1)^2 - n = a^2 - n + 2a + 1 and (k = a^2 - n) so k' = k + 2a +1
            a ++;
        }
        else{ // We need to grow b
            bsquared += 2*b + 1; // (b+1)^2 = b^2 + 2b + 1
            b ++;
        }
    }
    cout  << "p and q: " << (a-b) << " " << (a+b) << endl;
}

int main(){

    unsigned seed = rd() ^ chrono::steady_clock::now().time_since_epoch().count();
    gen = mt19937(seed);

    unsigned M = 100;

    int primeSize = 15;

    unsigned e = 65537;
    unsigned p = 0, q = 0, n = 0, phi = 0, d = 0;
    do {
        //p = 38603;
        //q = 36383;
        p = generateStrongPrime(primeSize);
        q = generateStrongPrime(primeSize);
        if (p == q){
            continue;
        }
        n = p*q;
        phi = (p-1)*(q-1);
    }while (modInverse(e,phi,d)); // while it hasn't generated a valid d

    cout << "Primes (p,q): " << p << " " << q << endl;
    cout << "Public Key (n,e): " << n << " " << e << endl;
    cout << "Private Key d: " << d << endl;

    cout << "phi too large: " << (((1<<31) & phi) != 0) << endl;

    unsigned C = fastExpentiation(M,e,n); //Verifies we can encrypt
    cout << C << endl;

    unsigned M_e = fastExpentiation(C,d,n); //Verifies we can decrypt
    cout << M_e << endl;

    fermatAttack(n);
}