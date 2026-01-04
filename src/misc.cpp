#include <string>
#include <algorithm>
#include <cctype>

using namespace std;

string normalize(string s) {
    string out;
    for (char c : s) {
        if (isalnum(static_cast<unsigned char>(c))) {
            out += tolower(c);
        }
    }
    return out;
}

string capitalizeFirstLetter(const string &s) {
    if (s.empty()) {
        return s;
    }

    string result = s;
    result[0] = toupper(result[0]);
    return result;
}

string to_lower(const string &str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return tolower(c); });
    return result;
}

string sanitize_asset(string s) {
    s = to_lower(s);
    s.erase(remove_if(s.begin(), s.end(),
        [](char c){ return !isalnum(c) && c != '_'; }),
        s.end());
    return s;
}
