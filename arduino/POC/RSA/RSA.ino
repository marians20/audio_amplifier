#include <math.h>
#include <string.h>

long power(long a, long b) {
  long temp = 1;
  for (long i = 0; i < b; ++i) {
    temp = temp * a;
  }
  return temp;
}

int size_string (unsigned char* string )
{   int i;
  for (i = 0; string[i] != '\0'; ++i);
  return i;
}

int size_string (char* string )
{   int i;
  for (i = 0; string[i] != '\0'; ++i);
  return i;
}


char* to_string(long num) {
  char* str;
  if (num == 0) {
    str = new char[2];
    str[0] = '0';
    str[1] = '\0';
    return str;
  }

  long num_of_digits = (long) (log10(num)) + 1;
  long rem;
  
  str = new char[num_of_digits + 1];
  
  str[num_of_digits] = '\0';
  for (long i = num_of_digits- 1; i >= 0; --i) {
    
    rem = num % ((long)(10));
   
    char rem_char = (char)(rem + 48); // '0'
    str[i] = rem_char;
    num = num / 10;
  }
  
  return str;
}

long to_num(char* str) {
  int len = size_string(str);
  long num=0;
  int number_is_zero = 1;
  for (int i = 0; i < len; ++i) {
    if (str[i] != '0') {
      number_is_zero = 0;
      break;  
    }
  }
  if (number_is_zero == 1)
    return 0;
  for (int i = 0; i < len; ++i) {
    if (str[i] != '0')
      num += power(10, len - i - 1) * ( str[i] - '0' ); 
  }
  return num;
}

void Serial_print_array(long* arr, long length, char *name) {
  for (int i = 0; i < length; ++i) {
    Serial.write(name);
    Serial.write("[");
    Serial.write(to_string(i));
    Serial.write("] = ");
    Serial.println(to_string(arr[i]));
  }
}

void Serial_print_array(unsigned char* str, long length, char *name) {
  for (int i = 0; i < length; ++i) {
    Serial.write(name);
    Serial.write("[");
    Serial.write(to_string(i));
    Serial.write("] = ");
    Serial.println(str[i]);
  }
}

long raiseto_mod(long a, long b, long c) {
  long temp = 1;
  for (long i = 0; i < b; ++i) {
    temp = (temp * a) % c;
  }
  return temp;
}

extern bool isprime(long n, int iterations);

char* dec2bin(int n) {
  long r;
        int c = 0, i, b[100];
  while(n > 0)   
  {   
    r = n % 2 ;   
      b[c] = r ;   
      n = n / 2 ;   
      c++ ; 
    }

  char* str;
  for(i = 0 ; i >= c-1 ; i++)   {
      str[c-i-1] = b[0] + 48;
  }
         
        
  return str;
}

long* string2ascii_int_list(char* string) {
  long* ascii_int_list;
  int length = strlen(string);
  ascii_int_list = new long [length];
  for (long i = 0; i < length; ++i) {
    ascii_int_list[i] = (int)(string[i]);
    ascii_int_list[i] = (long) (ascii_int_list[i]);
    Serial.write("ascii_int_list["); Serial.print(i);
    Serial.write("] = ");
    Serial.println(ascii_int_list[i]);
  }
   
  return ascii_int_list;
}

char* ascii_int_list2string(int* ascii_int_list) {
  char* string;
  int length = sizeof(ascii_int_list) / sizeof(ascii_int_list[0]);
  string = new char[length];
  for (int i = 0; i < length; ++i) {
    string[i] = (char) (ascii_int_list[i]);
  }
  return string;
} 

long getprime(long lower, long upper) {
  long num;
  while(1) {
    num = random(lower,upper);
    if(isprime(num, 2))
      break;  
  }
  return num;
}

long* rsa_encrypt(char* plaintext, long* key) {
  //long n = key[0];
  //long e = key[1];
  long* plaintext_list;
  long* ciphertext_list;
  int length = strlen(plaintext);
        
  plaintext_list = string2ascii_int_list(plaintext);
        
  ciphertext_list = new long[length];
  Serial.write("inside hell called rsa_encrypt\n");
  
  for (int i = 0; i < length; ++i) {
    Serial.write("plaintext_list["); Serial.print(i); Serial.print("] = "); Serial.println((plaintext_list[i]));
    ciphertext_list[i] = raiseto_mod(plaintext_list[i], key[1], key[0]);
  }

  return ciphertext_list;

}

long* rsa_decrypt(long* ciphertext_list, long* key, int length) {
  //long n = key[0];
  //long d = key[1];
  long* plaintext_list;
  //int length = sizeof(ciphertext_list)/sizeof(ciphertext_list[0]);
  plaintext_list = new long[length];
  for(int i = 0; i < length; ++i) {
    plaintext_list[i] = raiseto_mod(ciphertext_list[i], key[1], key[0]);
        }    
  return plaintext_list;
}

/* The following functions can be used for generating a pair of RSA keys. */

long gcd(long a, long b){
  // Returns greatest common denominator by using recursive Euclidean algorithm
  if (b == 0){
          
    return a; 
  }
  return  gcd(b, a % b);
}

long* extended_gcd(long a, long b){
  // Recursive extended Euclidean algorithm
  long *temp, *temp2;
  if (a==0){
    temp = new long[3];
    temp[0] = b; temp[1]= 0; temp[2]= 1;
                
                
    return temp;
    }
  else {
    temp = extended_gcd(b % a, a);
    temp2 = new long[3];
    temp2[0] = temp[0];
    temp2[1] = temp[2] - (long)(b/a)*temp[1];
    temp2[2] = temp[1];
                
                
    return temp2;
  }
}

long modinv(long a,long m){
  // Returns the inverse of a modulo m
  long *temp;
  temp = extended_gcd(a,m);
  long x = temp[1], g = temp[0]; // y = temp[2];
  if (g==1)           
    return (x % m);
  
}

long getrelprime(long num){
  // Returns the smallest relative prime number to num
  for(long i=2; i< num; i++){
    if (gcd(i,num) == 1){                
      return i;
    }
  }
}

// 
long* findkq(long n){
  // Find k and q such that n = (2**k)*q 
  long result;
  result = n % (long)(2);
  long n_new = n-1;
  long k = 0;
  long q = 0;
  while (n_new) {
    n_new /= 2;
    k += 1;
    if (n_new % (2) != 0){
      q = n_new;
      break;
    }
  }
  long *temp;
  temp = new long[2];
  temp[0] = k;
  temp[1] = q;
  return temp ;
}

int test3(long n, long k, long q){
  
  long a = random(2, n-2); // long min = 2, max = n-2; a=rand() % (max - min + 1) + min; 
  
  if (raiseto_mod(a, q, n) == 1){
                
                
                
    return 0; // "inconclusive";
  }
  for (long j = 0; j < k; j++){
    if (raiseto_mod(a,power(2,j),n) == n-1)                  
      return 0; //"inconclusive";
  }
  return 1; //"composite";
}

bool isprime(long n, int iterations = 1){
  // Primality test using Miller-Rabin algorithm
  if (n % 2 == 0){
          
          return 0;
  }
  long *t1 = findkq(n);
  //long k = t1[0];
  //long q = t1[1];

  for (int i = 0; i < iterations; i++){
    int result = test3(n,t1[0],t1[1]);
    if (result == 1) {
      return 0;
    }
  }
            
  return 1;
}

long* generate_keys(long lower, long upper) {
  long n, phi_n, p, q, e, d;
  
  while(1) {
    
    p = getprime(lower, upper);
    q = getprime(lower, upper);
    n = p * q;
    phi_n = (p-1) * (q-1);
    e = getrelprime(phi_n);
    d = modinv(e, phi_n);
    if (d>= 2)
      break;
  }

  long *Key;      
  Key = new long[3];
  Key[0] = n; Key[1] = e; Key[2] = d;
  
  // Public Key is {n,e}
  // Private Key is {n, d}            
  
  return Key;
}

long RSA_Own_Private_Key[2] = {5723, 3341};
const int MOST_NO_OF_DIGITS = 10;
