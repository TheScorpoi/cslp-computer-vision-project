#include <iostream>
#include <fstream>

using std::cout; using std::ofstream;
using std::endl; using std::string;
using std::fstream;

int main()
{
    string text("Some huge text to write to\n");
    string filename2("../../output/tmp2.txt");
    fstream outfile;

    outfile.open(filename2, std::ios_base::out);
    if (!outfile.is_open()) {
        cout << "failed to open " << filename2 << '\n';
    } else {
        outfile.write(text.data(), text.size());
        cout << "Done Writing!" << endl;
    }

    return EXIT_SUCCESS;
}
