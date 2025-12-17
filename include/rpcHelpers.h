#ifndef RPCHELPERS_H
#define RPCHELPERS_H

#include <atomic>
#include <functional>
#include <string>

void monitorWindowChanges(std::atomic<bool>& running, std::function<void(const std::string&, const std::string&)> callback);
std::string to_lower(const std::string &str);

#endif
