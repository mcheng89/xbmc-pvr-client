#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <string>
#include <vector>

std::string httpDigestHeaders(std::string username, std::string password, std::string method, std::string path);
std::string httpRequest(std::string host, int port, std::string path, std::string headers="", std::string request="");

std::vector<std::string> split(const std::string& s, const std::string& delim, const bool keep_empty);

#endif // UTILS_H_INCLUDED
