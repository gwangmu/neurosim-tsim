#pragma once

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <vector>
#include <string>

using namespace std;

namespace String
{
    static inline string TrimLeft (string s) 
    { 
        s.erase(s.begin(), find_if(s.begin(), s.end(), 
                    not1(ptr_fun<int, int>(isspace))));
        return s; 
    }

    static inline string TrimRight (string s) 
    {
        s.erase(find_if(s.rbegin(), s.rend(), 
                    not1(ptr_fun<int, int>(isspace))).base(), s.end()); 
        return s; 
    }

    static inline string Trim (string s) { return TrimLeft (TrimRight (s)); }

    static inline vector<string> Tokenize (const string &str, const string &delimiters = " ") 
    {
        vector<string> tokens;

        string::size_type lastPos = str.find_first_not_of (delimiters, 0);
        string::size_type pos     = str.find_first_of (delimiters, lastPos);

        while (string::npos != pos || string::npos != lastPos) {
            tokens.push_back (Trim (str.substr (lastPos, pos - lastPos))); 
            lastPos = str.find_first_not_of (delimiters, pos);
            pos = str.find_first_of (delimiters, lastPos); 
        } 

        return tokens;
    }
}
