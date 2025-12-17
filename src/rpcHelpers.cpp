#include <functional>
#include <thread>
#include <chrono>
#include <atomic>
#include <algorithm>
#include <string>

#include "xTools.h"

using namespace std;

void monitorWindowChanges(atomic<bool> &running,
                          function<void(const string&, const string&)> callback
) {
    string lastTitle;
    string lastName;

    while (running) {
        string title = getWindowTitle();
        string name  = getWindowName();

        // update if Window change
        if (title != lastTitle || name != lastName) {
            lastTitle = title;
            lastName  = name;
            callback(title, name);
        }

        this_thread::sleep_for(chrono::seconds(5));
    }
}

std::string to_lower(const std::string &str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return result;
}
