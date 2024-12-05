#include <windows.h>
#include <iostream>
#include "repl/repl.h"

std::string getCurrentUsername() {
    DWORD bufferSize = 256;

    if (wchar_t buffer[256]; GetUserNameW(buffer, &bufferSize)) {
        std::wstring wideStr(buffer);
        return std::string(wideStr.begin(), wideStr.end());
    }

    // Fallback to environment variable
    if (const char *username = std::getenv("USERNAME")) {
        return std::string(username);
    }

    return "unknown";
}

int main() {
    try {
        const std::string username = getCurrentUsername();
        std::cout << "Hello " << username << "! This is the Monkey programming language!\n";
        std::cout << "Feel free to type in commands\n";
        Repl::start(std::cin, std::cout);
    } catch (const std::runtime_error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
