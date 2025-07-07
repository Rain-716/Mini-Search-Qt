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
        r+=static_cast<char>(std::tolower(static_cast<unsigned char>(c))); // Use static_cast for tolower
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
        if (!in.is_open()){
            // Handle error if file cannot be opened
            continue;
        }
        std::string line;
        int lineno=0;
        fileLines[fname]; // Ensure entry exists even for empty files

        while (std::getline(in,line)){
            lineno++;
            fileLines[fname].push_back(line);
            std::istringstream iss(line);
            std::string word;
            while (iss>>word){
                // Simple word tokenization, could be improved
                // Remove punctuation, etc.
                std::string processedWord;
                for (char c : word){
                    if (std::isalnum(static_cast<unsigned char>(c))){ // Only include alphanumeric characters
                        processedWord+=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                    }
                }

                if (!processedWord.empty()){
                    index[processedWord][fname].push_back(lineno);
                }
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
        // Apply the same processing to query terms as during indexing
        std::string processedTerm;
        for (char c : t){
            if (std::isalnum(static_cast<unsigned char>(c))){
                processedTerm+=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }
        }
        if (!processedTerm.empty()){
            terms.push_back(processedTerm);
        }
    }

    if (terms.empty()){
        return {};
    }

    // find files containing all terms
    std::unordered_map<std::string,int> fileTotalOccurrences; // Tracks total occurrences across all query terms in a file
    std::unordered_map<std::string,std::set<int>> fileUniqueLines; // Stores unique line numbers across all query terms in a file

    bool firstTerm=true;
    for (const auto& term : terms){
        auto it=index.find(term);
        if (it==index.end()){
            // If any term is not found, no files will contain all terms
            return {};
        }

        if (firstTerm){
            for (const auto& [fname,lines] : it->second){
                fileTotalOccurrences[fname]=lines.size();
                fileUniqueLines[fname].insert(lines.begin(),lines.end());
            }
            firstTerm=false;
        }
        else {
            std::unordered_map<std::string,int> newTotalOccurrences;
            std::unordered_map<std::string,std::set<int>> newUniqueLines;

            for (const auto& [fname,lines] : it->second){
                // Only consider files that were present in previous terms' results
                if (fileTotalOccurrences.count(fname)){
                    newTotalOccurrences[fname]=fileTotalOccurrences[fname]+lines.size();
                    std::set<int> mergedLines=fileUniqueLines[fname];
                    mergedLines.insert(lines.begin(),lines.end());
                    newUniqueLines[fname]=std::move(mergedLines);
                }
            }
            fileTotalOccurrences.swap(newTotalOccurrences);
            fileUniqueLines.swap(newUniqueLines);
        }
        // If at any point fileTotalOccurrences becomes empty, it means no file contains all terms so far
        if (fileTotalOccurrences.empty()){
            return {};
        }
    }

    // assemble results
    std::vector<SearchResult> results;
    for (const auto& [fname,totalCount] : fileTotalOccurrences){
        SearchResult r;
        r.filename=fname;
        r.count=totalCount;
        // extract unique lines in order
        for (int ln : fileUniqueLines[fname]){
            if (ln>0&&static_cast<size_t>(ln-1)<fileLines[fname].size()){
                r.lines.push_back(fileLines[fname][ln-1]);
            }
        }
        std::sort(r.lines.begin(),r.lines.end()); // Ensure lines are sorted for consistent output
        results.push_back(r);
    }

    // sort by count desc, filename asc
    std::sort(results.begin(),results.end(),[](const auto& a,const auto& b)
    {
        if (a.count!=b.count){
            return a.count>b.count;
        }
        return a.filename<b.filename;
    });
    return results;
}