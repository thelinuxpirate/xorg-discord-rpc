#ifndef MISC_H
#define MISC_H

#include <string>

std::string normalize(std::string s);
std::string capitalizeFirstLetter(const std::string &s);
std::string to_lower(const std::string &str);
std::string sanitize_asset(std::string s);

#endif
