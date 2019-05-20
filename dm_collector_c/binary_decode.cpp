#include <iostream>
#include <string>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <cstdio>
#include "utils.h"
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>
#include "unistd.h"
using namespace std;

std::string exec(const char *cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}
std::string toAHex(unsigned char * Msg, int size){
    char buf[2200] = {0};
    for(int i = 0; i < size; i++){
        sprintf(buf, "%s\\x%02x",buf, Msg[i]);
    }
    return buf;
}
std::string binary_decode(unsigned int type, const char * Msg, int size){
	if(!type)
		return make_string(Msg, size);
    union{
        unsigned int size = 0;
        unsigned char e[4];
    }tmp;
	tmp.size = type;
    unsigned char buf[520];
    buf[0] = tmp.e[3];
    buf[1] = tmp.e[2];
    buf[2] = tmp.e[1];
    buf[3] = tmp.e[0];
    tmp.size = size;
    buf[4] = tmp.e[3];
    buf[5] = tmp.e[2];
    buf[6] = tmp.e[1];
    buf[7] = tmp.e[0];
    memcpy(buf + 8, Msg, size);
	string Hex = toAHex((unsigned char*)buf, size + 8);


    string cmd = "/bin/echo -ne '" + Hex + "' | ws_dissector";
    string reply = exec(cmd.c_str());

    return reply;
}
