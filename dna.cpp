#include<iostream>
#include<fstream>
#include<cstdlib>
#include<cmath>
#include<sys/time.h>
#include<map>
#include<string>
#include<vector>
using namespace std;

class CryptoKey {
  private:
    static string bases[4]; 
    vector<string> codons;
    void generate_codons(int size, string str);
    void swap(int *a, int *b);
  public:
    map<string, int> key;
    void keygen(int codon_size, int random_seed);
};

string CryptoKey::bases[4] = {"A", "T", "G", "C"};

void CryptoKey::swap(int *a, int *b) {
  int temp = *a;
  *a = *b;
  *b = temp;
}

void CryptoKey::generate_codons(int size, string str) {
  if(size == 0)
    codons.push_back(str);
  else
    for(int i=0; i<4; i++)
      generate_codons(size-1, str + bases[i]);
}

void CryptoKey::keygen(int codon_size, int random_seed) {
  // generate a key using DNA strings of the given codon length 
  int key_size = pow(4, codon_size);
  int value[key_size];
  for(int i=0; i<key_size; i++) 
    value[i] = i;

  // Set random seed
  srand(random_seed);
  // Fischer - Yates Shuffle
  for(int i=key_size-1; i>0; i--) 
    swap(&value[i], &value[rand() % (i+1)]);
  // Permute base pairs for all codons of given size
  generate_codons(codon_size, "");
  
  for(int i=0; i<key_size; i++)
    key.insert(pair<string, int>(codons[i], value[i]));
}

string int_to_binary(int c, int num_bits) {
  string bin = "";
  int bit;
  for(int i=num_bits-1; i>=0; i--) {
    bit = (c >> i) & 0x01;
    if(bit)
      bin += "1";
    else
      bin += "0"; 
  }
  return bin;
}

// encrypt a file with DNA encryptor
string encrypt_file(string in_file, string out_file, CryptoKey k) {
  string bases[] = {"A", "T", "G", "C"};
  string dna = "", plaintext = "";
  int codon_size = 2;
  int max_bits_per_num = 2 * codon_size;

  ifstream in(in_file, ios::in | ios::binary);
  char buffer[1];
  while(in.read(buffer, 1)) {
    plaintext += buffer[0];
    // append correct base to dna
    dna += bases[(buffer[0]>>6) & 0x03];
    dna += bases[(buffer[0]>>4) & 0x03];
    dna += bases[(buffer[0]>>2) & 0x03];
    dna += bases[buffer[0] & 0x03];
    //cout<<buffer[0]<<"\t"<<dna<<endl;
  }
  in.close();
  cout<<"Plaintext:\n"<<plaintext<<endl;
  cout<<"DNA String\n"<<dna<<endl<<endl;
  
  string codon, binary_cipher;
  int converted;
  for(int i=0; i<dna.length(); i+=codon_size) {
    codon = dna.substr(i, codon_size);
    converted = k.key.find(codon)->second;
    cout<<codon<<"\t"<<converted<<endl;
    binary_cipher += int_to_binary(converted, max_bits_per_num);
    // out.write(4 bits of converted) to store actual bits
  }
  cout<<endl;
  cout<<"Binary Ciphertext:\n"<<binary_cipher<<endl<<endl;

  ofstream out(out_file, ios::out | ios::binary);
  int write_num;
  for(int i=0; i<dna.length(); i+=4) {
    // first 4 bits
    codon = dna.substr(i, 2);
    converted = k.key.find(codon)->second;
    write_num = (converted << 4) & 0xf0;
    // next 4 bits
    codon = dna.substr(i+2, 2);
    converted = k.key.find(codon)->second;
    write_num += converted & 0x0f;
    // write char
    buffer[0] = (char) write_num;  // only lower 8 bits
    out.write(buffer, 1);
  }
  out.close();
  
  return binary_cipher;
}

int match_letter(char base) {
  return base == 'A'? 0: base == 'T'? 1: base =='G'? 2: 3;
}

// decrypt a file encrypted with DNA encryptor
string decrypt_file(string in_file, string out_file, CryptoKey k) {
  int codon_size = 2;
  int max_bits_per_num = 2 * codon_size;
  
  map<int, string> reverse_key;   // reverse lookup
  for(auto itr=k.key.begin(); itr!=k.key.end(); itr++)
    reverse_key.insert(pair<int, string>(itr->second, itr->first));

  cout<<"Reversed key for decryption:\n";
  for(auto itr=reverse_key.begin(); itr!=reverse_key.end(); itr++)
    cout<<itr->first<<"\t"<<itr->second<<endl;
  cout<<endl;

  ifstream in(in_file, ios::in | ios::binary);
  char buffer[1];
  int received;
  string dna;
  while(in.read(buffer, 1)) {
    // append correct codon to dna
    received = (int) (buffer[0]>>4 & 0x0f);
    dna += reverse_key.find(received)->second;
    received = (int) (buffer[0] & 0x0f);
    dna += reverse_key.find(received)->second;
  }
  cout<<"DNA String\n"<<dna<<endl;
  in.close();

  string bases[] = {"A", "T", "G", "C"};
  char letter = 0;
  string plaintext = "";
  for(int i=0; i<dna.length(); i+=4) {
    letter = (match_letter(dna[i]) << 6) & 0xc0;
    letter += (match_letter(dna[i+1]) << 4) & 0x30;
    letter += (match_letter(dna[i+2]) << 2) & 0x0c;
    letter += match_letter(dna[i+3]) & 0x03;
    plaintext += letter;
  }
  cout<<"Plaintext:\n"<<plaintext<<endl;

  ofstream out(out_file, ios::out | ios::binary);
  for(int i=0; i<plaintext.length(); i++) {
    buffer[0] = plaintext[i];
    out.write(buffer, 1);
  }
  out.close();

  return plaintext;
}

int main(int argc, char **argv) {
  cout<<"DNA Based Encryption\n\n";

  CryptoKey k;
  cout<<"Cryptographic Key:\n";
  k.keygen(2, 1);
  for(auto itr=k.key.begin(); itr!=k.key.end(); itr++)
    cout<<itr->first<<"\t"<<itr->second<<endl;
  cout<<endl;
  
  if(argc < 2)
    return 0;
  
  int start = clock();
  encrypt_file(argv[1], "test.crypt", k);
  decrypt_file("test.crypt", "outtest.png", k);
  int end = clock();

  float time_ms = (end-start) * 1000000 / CLOCKS_PER_SEC;
  cout<<"System clock rate: "<<CLOCKS_PER_SEC<<endl<<endl;
  cout<<"Time Elapsed: "<<time_ms<<" us"<<endl<<endl;  

  return 0;
}
