#ifndef XTOOLS_H
#define XTOOLS_H

#include <string>

std::string to_lower(const std::string &str);
std::string sanitize_asset(std::string s);

std::string getWindowManagerName();
std::string getWindowTitle();
std::string getWindowClass();

#endif 
