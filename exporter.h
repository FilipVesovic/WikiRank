#ifndef EXPORTER
#define EXPORTER

#include <fstream>
#include <iostream>

using namespace std;

class Exporter
{
  int fileMaximum = 100000;
  int currentEntry = 0;
  int currentFile = 0;
  ofstream out;
  string name;
public:

  string getFilename()
  {
    return name + to_string(currentFile);
  }

  Exporter(const string &name):name(name){
    out.open(getFilename());
  }

  void exportPage(const string &s)
  {
    currentEntry++;
    if(currentEntry == fileMaximum)
    {
      currentFile++;
      currentEntry = 0;
      out.close();
      out.open(getFilename());
    }
    out << s;
  }

  void finalize()
  {
    out.close();
  }
};

#endif
