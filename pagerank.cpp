#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

using Probability = long double;

const int MAX_N = 40000000;

int numberOfPages;

int currentState = 0;
Probability *state[2];

string *names;
vector<int> *incoming;
int *outcoming;
bool *valid;

Probability randomHopProbability = 0.1;

inline Probability absolute(Probability x) {
  return (x>=0)?x:-x;
}

bool cmp(int a, int b) {
  return state[currentState][a] > state[currentState][b];
}

struct Page {
  string name;
  int id;
  bool redirect;
  vector<int> links;
};

void addEdge(int from, int to) {
  incoming[to].push_back(from);
  outcoming[from]++;
}

void addPage(const Page &page) {
  names[page.id] = page.name;
  for (auto id : page.links)
    addEdge(page.id, id);
}

void loadFromFile(ifstream &in) {
  string line;
  while (getline(in, line)) {
    string tmp;
    int k = 0, n;
    Page p;
    for (int i = 0; i < line.length(); i++) {
      char c = line[i];
      if (c == '|') {
        if (k == 0)
          p.id = stoi(tmp);
        else if (k == 1)
          p.name = tmp;
        else if (k == 2)
          p.redirect = (stoi(tmp))>0?true:false;
        else if (k == 3)
          n = stoi(tmp);
        else
          p.links.push_back(stoi(tmp));
        k++;
        tmp = "";
      } else
        tmp += c;
    }
    if(tmp!="")
      p.links.push_back(stoi(tmp));

    addPage(p);
  }
}

bool load(const string &path) {
  ifstream in(path);
  bool exists = in.good();

  if (exists) {
    cout << "loading " << path << "..." << endl;
    loadFromFile(in);
  }
  in.close();
  return exists;
}

string printName(const string &name)
{
  string ret = name;
  bool prevIsSpace = true;
  for(int i = 0;i < ret.length(); i++)
  {
    bool needUpper = prevIsSpace;

    if(ret[i]>='a' && ret[i] <= 'z' && needUpper)
      ret[i] = ret[i]-'a'+'A';
    else if(ret[i] >= 'A' && ret[i] <= 'Z' && !needUpper)
      ret[i] = ret[i] - 'A' +'a';

    prevIsSpace = !(ret[i]>='a' && ret[i] <= 'z') && !(ret[i] >= 'A' && ret[i] <= 'Z');
  }
  return ret;
}

void finalize() {
  cout << "Checkpointing..." << endl;
  vector<int> v;
  for (int i = 0; i < numberOfPages; i++)
    if (valid[i])
      v.push_back(i);
  sort(v.begin(), v.end(), cmp);
  ofstream out("out");
  for (int i = 0; i < numberOfPages; i++) {
    int id = v[i];
    out << printName(names[id]) << "|" << state[currentState][id] << endl;
  }
  out.close();
  cout << "Checkpoint saved.." << endl;
}

void iteration() {
  int nextState = 1 - currentState;
  Probability stayProbability = 1.0 - randomHopProbability;

  for (int i = 0; i < MAX_N; i++)
    state[nextState][i] = 0.0;

  Probability unusedProbability = 0.0;

  for (int i = 0; i < MAX_N; i++) {
    if (!valid[i])
      continue;
    for (auto from : incoming[i]) {
      Probability probPerConnection = stayProbability / outcoming[from];
      state[nextState][i] += probPerConnection * state[currentState][from];
    }
  }

  for (int i = 0; i < MAX_N; i++) {
    if (!valid[i])
      continue;
    if (outcoming[i] == 0)
      unusedProbability += state[currentState][i];
    else
      unusedProbability += randomHopProbability * state[currentState][i];
  }
  cout << unusedProbability << endl;

  Probability prob = unusedProbability / numberOfPages;

  for (int i = 0; i < MAX_N; i++)
    if (valid[i])
      state[nextState][i] += prob;

  double diff = 0.0;
  for (int i = 0; i < numberOfPages; i++)
    diff += absolute(state[nextState][i] - state[currentState][i]);
  cout << "Diff: " << diff << endl;
  currentState = nextState;
}

void cleanup() {
  for (int i = 0; i < MAX_N; i++)
    if (outcoming[i] > 0 || incoming[i].size() > 0)
      valid[i] = true, numberOfPages++;
}

int main() {
  state[0] = new Probability[MAX_N];
  state[1] = new Probability[MAX_N];
  names = new string[MAX_N];
  incoming = new vector<int>[MAX_N];
  outcoming = new int[MAX_N];
  valid = new bool[MAX_N];

  string file = "../data/wiki";
  int i = 0;
  while (load(file + to_string(i++)));

  cleanup();

  for (int i = 0; i < MAX_N; i++)
    if (valid[i])
      state[currentState][i] = 1.0 / numberOfPages;

  for (int i = 0; i < MAX_N; i++)
    incoming[i].shrink_to_fit();

  cout << "Initalization finished.." << endl;
  int iterationCount = 0;
  while (true) {
    auto begin = chrono::high_resolution_clock::now();
    iteration();
    iterationCount++;
    auto end = chrono::high_resolution_clock::now();
    auto dur = end - begin;
    auto ms = chrono::duration_cast<chrono::milliseconds>(dur).count();
    cout << "Iteration #" << iterationCount << " Time: " << ms << endl;
    if (iterationCount % 10 == 0)
      finalize();
  }
  return 0;
}
