#include "index.h"
#include "link.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <regex>
#include "exporter.h"
#include "page.h"

using namespace std;

Index wikiIndex;
Exporter exporter("wiki");

void worker(string page) {
  regex titleRegex("<title>([^<>]*)</title>");
  regex redirectRegex("<redirect +title=\"([^\"]*)\" */>");
  regex textRegex("<text[^<>]*>([^<>]*)</text>");
  smatch m;
  bool haveRedirect = false;
  string title, redirect, text;

  if (regex_search(page, m, titleRegex))
    title = m[1];

  if (regex_search(page, m, redirectRegex))
    redirect = m[1], haveRedirect = true;
  if(haveRedirect)
  {
    Page page(title, redirect);
    exporter.exportPage(page.exportPage());
  }
  else {
    int textStart = page.find("<text"); // this is not bug
    int textEnd = page.find("</text>");

    bool start = false;
    for (int i = textStart; i < textEnd; i++) {
      if (start)
        text += page[i];
      if (page[i] == '>')
        start = true;
    }

    auto links = LinkRecognizer::parseLinks(text);
    Page page(title);

    for(auto link: links)
      page.addLink(link);

    exporter.exportPage(page.exportPage());
  }
}

void process()
{
  ifstream in("enwiki-20190501-pages-articles-multistream.xml");

  string page;
  bool inPage = false;

  const string startTag = "<page>";
  const string endTag = "</page>";
  int nextMatching = 0;

  int numberOfPages = 0;

  auto begin = chrono::high_resolution_clock::now();

  while (!in.eof()) {
    char c;
    c = in.get();

    if (!inPage) {
      if (c == startTag[nextMatching]) {
        nextMatching++;
        if (nextMatching == startTag.length()) {
          inPage = true;
          nextMatching = 0;
        }
      } else
        nextMatching = 0;
    } else {
      if (c == endTag[nextMatching]) {
        nextMatching++;
        if (nextMatching == endTag.length()) {
          inPage = false;
          nextMatching = 0;

          numberOfPages++;
          worker(page);
          page = "";
          auto end = chrono::high_resolution_clock::now();
          auto dur = end - begin;
          auto ms = chrono::duration_cast<chrono::milliseconds>(dur).count();
          if (numberOfPages % 1000 == 0)
            cout << "Number of pages: " << numberOfPages
                 << " PPS: " << 1.0 * numberOfPages / (ms / 1000) << endl;
        }
      } else
        nextMatching = 0;
    }

    if (inPage)
      page += c;
  }
  in.close();
}

int main() {
  process();
  exporter.finalize();
  cout << "Max ID: " << Page::nextIndex << endl;
}
