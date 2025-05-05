#include "memory_database.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

std::vector<Newsgroup> InMemoryDatabase::listNewsgroups() const {
    std::vector<Newsgroup> list;
    for(const auto& group : newsgroups) {
        list.push_back(group.second);
    }
    return list;
}

int InMemoryDatabase::getNextNewsgroupId() const {
    int maxId = 0;

    // Iterate over the newsgroups to find the highest ID
    for (const auto& group : newsgroups) {
        maxId = std::max(maxId, group.first);  // group.first is the ID of the newsgroup
    }
    // Return the next ID by adding 1 to the maximum ID found
    return maxId + 1;
}

bool InMemoryDatabase::createNewsgroup(const std::string& name) { // title of the group is sent as a parameter
    for(const auto& group : newsgroups) {
        if(group.second.name == name) {
            return false;
        }
    }

    // Create a new newsgroup with a unique ID
    int newId = getNextNewsgroupId();
    Newsgroup new_group = {newId, name};
    newsgroups.insert({newId, new_group});
    return true;
}

bool InMemoryDatabase::deleteNewsgroup(int id) {
    auto found = newsgroups.find(id);
    if (found != newsgroups.end()) {
        newsgroups.erase(found);
        return true;
    }
    return false;
}
 
std::vector<ArticleSummary> InMemoryDatabase::listArticles(int ng_id) const {
    std::vector<Article> list;
    for(const auto& group : newsgroups) {
        list.push_back(group.second);
    }
    return list;
}

bool InMemoryDatabase::createArticle(int ng_id, const std::string& title, const std::string& author, const std::string& text) {

}

bool InMemoryDatabase::deleteArticle(int ng_id, int art_id) {

}

Article InMemoryDatabase::getArticle(int ng_id, int art_id) const {

}