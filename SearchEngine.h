#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <set> // Added for std::set

struct SearchResult
{
    std::string filename;
    int count;
    std::vector<std::string> lines;
};

class SearchEngine
{
    public:
        void loadDirectory(const std::string& dir);
        std::vector<SearchResult> search(const std::string& query);

    private:
        // word -> file -> set of line numbers
        std::unordered_map<std::string,std::unordered_map<std::string,std::vector<int>>> index;
        std::unordered_map<std::string,std::vector<std::string>> fileLines;
    };

#endif // SEARCHENGINE_H