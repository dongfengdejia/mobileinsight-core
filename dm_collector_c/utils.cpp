#include "utils.h"

#include <cstring>

// Find multiple IDs coressponding to name, and append all IDs to out_vector
// Return how many IDs are found
int find_ids(const ValueName id_to_name[], int n, const char *name,
             IdVector &out_vector) {
    int cnt = 0;
    for (int i = 0; i < n; i++) {
        if (strcmp(name, id_to_name[i].name) == 0) {
            out_vector.push_back(id_to_name[i].val);
            cnt++;
        }
    }
    return cnt;
}

const char *search_name(const ValueName id_to_name[], int n, int val) {
    for (int i = 0; i < n; i++) {
        if (id_to_name[i].val == val) {
            return id_to_name[i].name;
        }
    }
    return NULL;
}
std::string make_string(const char *str, size_t length) {
    char hex[10] = {};
    std::string output = "0x";
    for (size_t i = 0; i < length; i++) {
        sprintf(hex, "%02x", str[i] & 0xFF);
        output += hex;
    }
    return output;
}
