#ifndef PAGE
#define PAGE

#include <map>
#include <vector>
#include <iostream>
#include <regex>

using namespace std;

class Page
{
  string name;
  int id;
  vector<int> links;
  bool redirect;

public:
  Page() {}
  Page(const string& name) : name(name), redirect(false), id(obtainID(name)) {}
  Page(const string& name, const string &redirected) : name(name), redirect(true), id(obtainID(name)) {
    links.push_back(obtainID(redirected));
  }

  friend bool operator<(const Page &a, const Page &b) { return a.id < b.id; }

  void addLink(const string& name) {
    links.push_back(obtainID(name));
  }

  void addLink(int id) {
    links.push_back(id);
  }

  int getId() const { return id; }
  string getName() const { return name; }

  string exportPage()
  {
    string ret;
    ret += to_string(id) + "|" + name + "|" + to_string(redirect?1:0) + "|" + to_string(links.size()) + "|";
    for(auto link : links)
      ret += to_string(link) + "|";
    ret += "\n";
    return ret;
  }

public:
  static int nextIndex;
  static map<string, int> indexer;

  static int obtainID(string name)
  {
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    auto it = indexer.find(name);
    if(it != indexer.end()) return (*it).second;

    indexer[name] = nextIndex;
    return nextIndex++;
  }

  friend ostream& operator<<(ostream& os, const Page& p);
};


#endif
