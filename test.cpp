#include <fstream>
#include <iostream>
using namespace std;
int main()
{
    // Create and open a text file
    ofstream MyFile("filename.txt");
    // Write to the file

    MyFile << "Files can be tricky, but it is fun enough!";
    // Close the file

    MyFile.close();

    int x = 6;

    if (x == 5)

    {
        MyFile << "Files can be tricky, but it is fun enough!";

        int y = 6;
    }

    if (x == 7)
    {
    }

    int yy = 9;

    if (x == 5)
    {
    }
}
