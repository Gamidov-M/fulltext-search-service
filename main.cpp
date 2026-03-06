#include <iostream>

int main() {
    std::cout << "C++: " << __cplusplus << "\n";

#ifdef __GNUC__
    std::cout << "GCC: " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << "\n";
#endif
    return 0;
}