#include<iostream>
#include<fstream>
#include<cstdlib>
#include<cmath>
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

int main(int argc, char **argv) {
  cout<<"DNA Based Encryption\n\n";
  
  string bases[] = {"A", "T", "G", "C"};
  CryptoKey k;
  string dna = "", plaintext = "";
  int codon_size = 2;
  int max_bits_per_num = 2 * codon_size;

  cout<<"Cryptographic Key:\n";
  k.keygen(2, 1);
  for(auto itr=k.key.begin(); itr!=k.key.end(); itr++)
    cout<<itr->first<<"\t"<<itr->second<<endl;
  cout<<endl;
  
  if(argc < 2)
    return 0;

  ifstream in(argv[1], ios::in | ios::binary);
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
  cout<<"Plaintext:\n"<<plaintext<<endl;
  cout<<"DNA String\n"<<dna<<endl<<endl;
  
  string codon, binary_cipher;
  int converted;
  for(int i=0; i<dna.length(); i+=codon_size) {
    codon = dna.substr(i, codon_size);
    converted = k.key.find(codon)->second;
    cout<<codon<<"\t"<<converted<<endl;
    binary_cipher += int_to_binary(converted, max_bits_per_num);
  }
  cout<<endl;
  cout<<"Binary Ciphertext:\n"<<binary_cipher<<endl;
  return 0;
}
