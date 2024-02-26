#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#define ADDR_BITS 256
#define DATA_BITS 256

#define HARD_LOCATIONS 8192

#define STORE_HAMMING_DISTANCE 95 /*0-255*/
#define RETRIEVE_HAMMING_DISTANCE 95 /*0-255*/

using namespace std;

struct HardLocation {
    bool address[ADDR_BITS];
    int data[DATA_BITS];
};

// hammingDistance(bool* a, bool* b) - return the hamming distance between two addresses
int hammingDistance(bool* a, bool* b) {
    int distance = 0;
    for (int i = 0; i < ADDR_BITS; i++) {
        if (a[i] != b[i]) {
            distance++;
        }
    }
    return distance;
}

class SDM {
public:
    // alloc on heap
    HardLocation* hardLocations[HARD_LOCATIONS];

    SDM() {
        // alloc on heap
        for (int i = 0; i < HARD_LOCATIONS; i++) {
            hardLocations[i] = new HardLocation();
        }

        // initialize hard locations
        for (int i = 0; i < HARD_LOCATIONS; i++) {
            for (int j = 0; j < ADDR_BITS; j++) {
                hardLocations[i]->address[j] = rand() % 2;
            }
            for (int j = 0; j < DATA_BITS; j++) {
                hardLocations[i]->data[j] = 0;
            }
        }
    }

    void dumpLocations() {
        for (int i = 0; i < HARD_LOCATIONS; i++) {
            cout << "location " << i << ": ";
            for (int j = 0; j < ADDR_BITS; j++) {
                cout << hardLocations[i]->address[j];
            }
            cout << endl;

            cout << "data: ";
            for (int j = 0; j < DATA_BITS; j++) {
                cout << hardLocations[i]->data[j] << " ";
            }
            cout << endl;
        }
    }

    // findLocations(int hamming_distance) - return a vector of references to the hard locations
    // that are within the hamming distance of the input address
    vector<HardLocation*> findLocations(bool* address, int hamming_distance) {
        vector<HardLocation*> locations;

        cout << "finding locations" << endl;
        for (int i = 0; i < ADDR_BITS; i++) {
            cout << address[i];
        }
        cout << endl;

        for (int i = 0; i < HARD_LOCATIONS; i++) {
            if (hammingDistance(address, hardLocations[i]->address) <= hamming_distance) {
                locations.push_back(hardLocations[i]);
            }
        }

        if (locations.size() == 0) {
            return findLocations(address, hamming_distance + 1);   
        }

        cout << "==============found " << locations.size() << " locations===========" << endl;

        return locations;
    }

    // store
    void store(bool* address, bool* data) {
        vector<HardLocation*> locations = findLocations(address, STORE_HAMMING_DISTANCE);
        for (auto location : locations) {
            for (int i = 0; i < DATA_BITS; i++) {
                // todo: prevent overflow
                if (data[i]) {
                    location->data[i]++;
                } else {
                    location->data[i]--;
                }
            }
        }
    }

    // retrieve
    bool* retrieve(vector<bool*> addresses) {
        int* data = new int[DATA_BITS];
        for (int i = 0; i < DATA_BITS; i++) {
            data[i] = 0;
        }

        for (auto address : addresses) {
            vector<HardLocation*> locations = findLocations(address, RETRIEVE_HAMMING_DISTANCE);
            for (auto location : locations) {
                for (int i = 0; i < DATA_BITS; i++) {
                    // todo: prevent overflow
                    data[i] += location->data[i];
                }
            }
        }

        // cout << "retrieved raw data: ";
        // for (int i = 0; i < DATA_BITS; i++) {
        //     cout << data[i] << " ";
        // }
        // cout << endl;

        // postprocess
        bool* processed_data = new bool[DATA_BITS];

        for (int i = 0; i < DATA_BITS; i++) {
            if (data[i] > 0) {
                processed_data[i] = true;
            } else if (data[i] < 0) {
                processed_data[i] = false;
            } else {
                processed_data[i] = rand() % 2;
            }
        }

        // free
        delete[] data;

        return processed_data;
    }

};

std::string loadText() {
    std::ifstream file("tinyshakespeare.txt");
    std::string str;
    std::string file_contents;
    while (std::getline(file, str))
    {
        file_contents += str;
        file_contents.push_back('\n');
    }
    return file_contents;
}

bool* genAddress(bool* seed, int offset) {
    bool* modified_seed = new bool[ADDR_BITS];

    for (int i = 0; i < ADDR_BITS; i++) {
        modified_seed[i] = seed[i];
    }

    // int doublings = offset / ADDR_BITS;

    // for (int i = 0; i < doublings; i++) {
    //     bool new_bit = modified_seed[0] ^ modified_seed[1]; // use taps at 0 and 1 to create new bit

    //     for (int j = 0; j < ADDR_BITS - 1; j++) {
    //         modified_seed[j] = modified_seed[j + 1]; // shift all bits to left
    //     }
    //     modified_seed[ADDR_BITS - 1] = new_bit; // insert the new bit to the rightmost position
    // }

    // flatten
    for (int i = 0; i < ADDR_BITS; i++) {
        modified_seed[i] = (modified_seed[i] > 0) ? 1 : 0;
    }

    bool* address = new bool[ADDR_BITS];
    for (int i = 0; i < ADDR_BITS; i++) {
        address[i] = modified_seed[(i + offset) % ADDR_BITS];
    }

    // flatten
    for (int i = 0; i < ADDR_BITS; i++) {
        address[i] = (address[i] > 0) ? 1 : 0;
    }

    return address;
}

// genaddress2 - just reverse genaddress
bool* genAddress2(bool* seed, int offset) {
    bool* genaddress = genAddress(seed, offset);
    // swap each
    for (int i = 0; i < ADDR_BITS / 2; i++) {
        bool temp = genaddress[i];
        genaddress[i] = genaddress[ADDR_BITS - 1 - i];
        genaddress[ADDR_BITS - 1 - i] = temp;
    }

    return genaddress;
}

std::string dataToAscii(bool* data) {
    std::string ascii = "";
    for (int i = 0; i < DATA_BITS; i+=8) {
        int byte = 0;
        for (int j = 0; j < 8; j++) {
            byte += data[i + j] * (1 << j);
        }
        ascii += (char)byte;
    }
    return ascii;
}

void testGenAddress(bool* seed) {
    for (int i = 0; i < 1024; i++) {
        bool* address = genAddress(seed, i);
        cout << "address " << i << ": ";
        for (int j = 0; j < ADDR_BITS; j++) {
            cout << address[j];
        }
        cout << endl;
    }
}

void PRINT(int shift, SDM &sdm, bool* seed) {
    cout << "shift: " << shift << endl;

    bool* address = genAddress(seed, shift);
    // print address
    cout << "address: ";
    for (int i = 0; i < ADDR_BITS; i++) {
        cout << address[i];
    }
    bool* data = new bool[DATA_BITS];
    for (int i = 0; i < DATA_BITS; i++) {
        data[i] = 0;
    }
    // retrieve
    bool* retrieved_data = sdm.retrieve(vector<bool*>({address}));
    cout << "retrieved data: ";
    for (int i = 0; i < DATA_BITS; i++) {
        cout << retrieved_data[i] << " ";
    }
    cout << endl;
    // ascii
    cout << "retrieved ascii: " << dataToAscii(retrieved_data) << endl;
}

int main() {
    SDM sdm = SDM();

    const int LIMIT = 8192 * 1;

    // std::string text = loadText();
    std::string text = "";
    for(int i=0; i<LIMIT; i++){
        text += to_string(i) + " ";
    }

    int NUM_SEEDS = LIMIT / (DATA_BITS / 8);
    vector<bool*> seeds;
    for (int i = 0; i < NUM_SEEDS; i++) {
        bool* seed = new bool[ADDR_BITS];
        for (int j = 0; j < ADDR_BITS; j++) {
            seed[j] = rand() % 2;
        }
        seeds.push_back(seed);
    }

    int shift = -1;
    for (int i = 0; i < LIMIT; i+=DATA_BITS/8) {
        shift++;
        bool* address = genAddress(seeds[i / (DATA_BITS / 8)], shift);
        // bool* address2 = genAddress2(seeds[i / (DATA_BITS / 8)], shift);

        bool* data = new bool[DATA_BITS];
        for (int j = 0; j < DATA_BITS; j++) {
            data[j] = 0;
        }

        // get the characters from this position
        std::string sub = text.substr(i, DATA_BITS/8);

        for (int j = 0; j < DATA_BITS / 8; j++) {
            int byte = sub[j];
            for (int k = 0; k < 8; k++) {
                data[j * 8 + k] = byte % 2;
                byte /= 2;
            }
        }

        sdm.store(address, data);
        // sdm.store(address2, data);

        delete[] data;
        delete[] address;
        // delete[] address2;
    }

    sdm.dumpLocations();

    // TEST

    int incorrect = 0;

    std::string retrieved_text = "";

    shift = -1;
    for (int i = 0; i < LIMIT; i+=DATA_BITS/8) {
        shift++;
        bool* address = genAddress(seeds[i / (DATA_BITS / 8)], shift);
        bool* address2 = genAddress2(seeds[i / (DATA_BITS / 8)], shift);

        cout << "shift: " << shift << endl;
        
        bool* retrieved_data = sdm.retrieve(vector<bool*>({address}));

        std::string retrieved_str = dataToAscii(retrieved_data);

        cout << retrieved_str << endl;

        // get the bytes from the original text at this position
        std::string original_str = text.substr(i, DATA_BITS/8);

        // compare each byte
        for (int j = 0; j < DATA_BITS/8; j++) {

            // std::cout << "original: " << original_str[j] << " retrieved: " << retrieved_str[j] << std::endl;

            if (retrieved_str[j] != original_str[j]) {
                incorrect++;
            }
        }

        retrieved_text += retrieved_str;

        delete[] retrieved_data;
        delete[] address;
        delete[] address2;
    }

    cout << endl;

    // cout << "retrieved text: " << retrieved_text << endl;

    cout << "incorrect: " << incorrect << " / " << LIMIT << endl;

    cout << "retrieved text length: " << retrieved_text.length() << endl;

    // testGenAddress(seed);

    // cout << "-------------------------" << endl;

    // PRINT(0, sdm, seed);
    // PRINT(256, sdm, seed);

    return 0;
}