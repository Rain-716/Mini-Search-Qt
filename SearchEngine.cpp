#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <set>
#include "SearchEngine.h"

namespace fs=std::filesystem;

static std::string toLower(const std::string& s)
{
    std::string r;
    r.reserve(s.size());
    for (char c : s){
        r+=std::tolower(c);
    }
    return r;
}

void SearchEngine::loadDirectory(const std::string& dir)
{
    index.clear();
    fileLines.clear();
    for (auto& p : fs::directory_iterator(dir)){
        if (!p.is_regular_file()){
            continue;
        }
        std::string fname=p.path().filename().string();
        std::ifstream in(p.path());
        std::string line;
        int lineno=0;
        while (std::getline(in,line)){
            lineno++;
            fileLines[fname].push_back(line);
            std::istringstream iss(line);
            std::string word;
            while (iss>>word){
                std::string w=toLower(word);
                index[w][fname].push_back(lineno);
            }
        }
    }
}

std::vector<SearchResult> SearchEngine::search(const std::string& query)
{
    std::istringstream iss(query);
    std::vector<std::string> terms;
    std::string t;
    while (iss>>t){
        terms.push_back(toLower(t));
    }
    if (terms.empty()){
        return {};
    }
    // find files containing all terms
    std::unordered_map<std::string,int> fileCount;
    std::unordered_map<std::string,std::set<int>> fileLinesSet;
    bool first=true;
    for (auto& term : terms){
        auto it=index.find(term);
        if (it==index.end()){
            return {};
        }
        if (first){
            for (auto& [fname,lines] : it->second){
                fileCount[fname]=lines.size();
                fileLinesSet[fname]=std::set<int>(lines.begin(),lines.end());
            }
            first=false;
        }
        else {
            std::unordered_map<std::string,int> newCount;
            std::unordered_map<std::string,std::set<int>> newLines;
            for (auto& [fname,lines] : it->second){
                if (fileCount.count(fname)){
                    newCount[fname]=fileCount[fname]+lines.size();
                    std::set<int> merged=fileLinesSet[fname];
                    merged.insert(lines.begin(),lines.end());
                    newLines[fname]=std::move(merged);
                }
            }
            fileCount.swap(newCount);
            fileLinesSet.swap(newLines);
        }
    }
    // assemble results
    std::vector<SearchResult> results;
    for (auto& [fname,cnt] : fileCount){
        SearchResult r;
        r.filename=fname;
        r.count=cnt;
        // extract unique lines in order
        for (int ln : fileLinesSet[fname]){
            r.lines.push_back(fileLines[fname][ln-1]);
        }
        std::sort(r.lines.begin(),r.lines.end());
        results.push_back(r);
    }
    // sort by count desc, filename asc
    std::sort(results.begin(),results.end(),[](auto& a,auto& b)
    {
        if (a.count!=b.count){
            return a.count>b.count;
        }
        return a.filename<b.filename;
    });
    return results;
}
