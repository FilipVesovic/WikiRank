#ifndef LINK_RECOGNIZER
#define LINK_RECOGNIZER

#include <regex>
#include <iostream>

using namespace std;

extern Index wikiIndex;

class LinkRecognizer
{
  static string strip(const string &s)
  {
    string ret;
    int firstIndex = -1, lastIndex = -1;
    for(int i = 0; i < s.length(); i++)
    {
      if(firstIndex == -1 && s[i] != ' ')firstIndex = i;
      if(s[i] != ' ')lastIndex = i;
    }
    for(int i = firstIndex; i <= lastIndex; i++)
      ret += s[i];
    return ret;
  }
public:

  static vector<string> findLinks(const string &s)
  {
    vector<string> result;
    string link;
    bool inLink = false;

    int startLoc;
    const string startTag = "[[";
    const string endTag = "]]";
    int nextMatching = 0;

    for(int i = 0; i < s.length(); i++)
    {
      char c = s[i];

      if (!inLink) {
        if (c == startTag[nextMatching]) {
          nextMatching++;
          if (nextMatching == startTag.length()) {
            inLink = true;
            startLoc = i - 1;
            nextMatching = 0;
            continue;
          }
        } else
          nextMatching = 0;
      } else {
        if (c == endTag[nextMatching]) {
          nextMatching++;
          if (nextMatching == endTag.length()) {
            inLink = false;
            nextMatching = 0;
            link.pop_back();
            if(!(startLoc - 1 >= 0 && s[startLoc - 1] == '='))
              result.push_back(link);
            link = "";
          }
        } else
          nextMatching = 0;
      }

      if (inLink)
        link += c;
    }
    return result;
  }


  static vector<string> parseLinks(const string &text) {
    vector<string> links;
    vector<string> possibleLinks = findLinks(text);

    for(auto &linkText : possibleLinks)
    {
      transform(linkText.begin(), linkText.end(), linkText.begin(), ::tolower);

      int delimiters = count(linkText.begin(), linkText.end(), '|');
      if (delimiters < 2) {
        string original = linkText;

        // normal link
        if (delimiters == 1)
          linkText = linkText.substr(0, linkText.find('|'));

  			size_t hash = linkText.find('#');

  			// remove section linking
        if (hash != string::npos)
          linkText = linkText.substr(0, hash);

        replace(linkText.begin(), linkText.end(), '_', ' ');
        linkText = strip(linkText);

  			if (!wikiIndex.lookup(linkText)) {
        size_t colon = linkText.find(':');

        // ignore links with namespaces except from en(languge)
        if (colon != string::npos) {
          string colLeft = linkText.substr(0, colon);
          if (colLeft != "en" && colLeft != "")
            continue;
  				if (colLeft == "") {
            linkText = linkText.substr(1,linkText.length());
          }

          else if (colLeft == "en")
            linkText = linkText.substr(colon + 1, linkText.length() - colon - 1);
        }
  		  }

      linkText = strip(linkText);
      // skip template, help bug with :
      if (linkText.find("category:") == 0 || linkText.find("wikipedia:") == 0 || linkText.find("template:") == 0
        || linkText.find("help:") == 0 || linkText.find("portal:") == 0 || linkText.find("mediawiki:") == 0 || linkText.find("file:") == 0 )
        continue;

        // handle "biology (kingdom)" for ex
        if (!wikiIndex.lookup(linkText)) {
          size_t paropen = linkText.find('(');
          size_t parclose = linkText.find(')');
          if (paropen != string::npos && parclose != string::npos &&
              paropen < parclose)
            linkText =
                linkText.substr(0, paropen) +
                linkText.substr(parclose + 1, linkText.length() - parclose - 1);
        }

        // handle "Seattle, Washington"
        if (!wikiIndex.lookup(linkText)) {
          size_t comma = linkText.find(',');
          if (comma != string::npos)
            linkText = linkText.substr(0, comma);
        }
        linkText = strip(linkText);

        // if nothing's left => skip (most likely internal linking/sections)
        if (linkText == "")
          continue;

  			if(wikiIndex.lookup(linkText))
          links.push_back(linkText);
      }
    }
    return links;
  }

};

#endif
