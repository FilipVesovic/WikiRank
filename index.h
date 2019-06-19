#include <fstream>
#include <set>
#include <vector>
#include <iostream>
#include <regex>
using namespace std;

class Index {
  set<string> index;

public:
  Index() {
    ifstream in("enwiki-20190501-pages-articles-multistream-index.txt");
    int current = 0;
    while (!in.eof()) {
      string line;
      getline(in, line);

      vector<int> loc;
      for (int i = 0; i < line.length(); i++)
        if (line[i] == ':')
          loc.push_back(i);

      if (loc.size() >= 2) {
        string title = line.substr(loc[1] + 1, line.length() - loc[1] - 1);
        transform(title.begin(), title.end(), title.begin(), ::tolower);
        index.insert(title);
      	current++;
      }

      if(current % 1000000 == 0)
        cout << "Index loaded: " << current << endl;
    }
    in.close();
    cout << "Index loaded!" << endl;
  }

  bool lookup(const string &s) const { return index.find(s) != index.end(); }
};
