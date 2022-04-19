#include "Application.h"

using namespace std;

// https://gist.github.com/statico/6809850727c708f08458
#ifdef _WIN32
#include <windows.h>
// Use discrete GPU by default.
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

int main(int, char**) {
    setlocale(LC_ALL, "RUS");

    try {
        Application application;
        application.Exec();
    }
    catch (const exception& err) {
        cerr << err.what() << "\nPress enter to continue..." << endl;
        getchar();
        return 1;
    }

    return 0;
}
