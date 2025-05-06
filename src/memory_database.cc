#include "memory_database.h"
#include <string>
#include <vector>
#include <map>
#include <stdexcept>

using namespace std;

std::vector<Newsgroup> InMemoryDatabase::listNewsgroups() const {
    std::vector<Newsgroup> list;
    for(const auto& group : newsgroups) {
        list.push_back(group.second);
    }
    return list;
}

// Creates a new newsgroup and allocates a new ID number (using a counter).
// First iterates through already existing newsgroups to recogize if the name is already used, if so returns false.
bool InMemoryDatabase::createNewsgroup(const std::string& name) { // title of the group is sent as a parameter
    for(const auto& group : newsgroups) {
        if(group.second.name == name) {
            return false; // Name alrady exists
        }
    }
    newsgroups[next_group_id] = {next_group_id, name};
    next_group_id++;
    return true;
}

// Erases the newsgroup and its articles
bool InMemoryDatabase::deleteNewsgroup(int id) {
    auto found = newsgroups.find(id);
    if (found == newsgroups.end()) return false; //changed != to ==
    newsgroups.erase(found);
    articles_by_ng.erase(id); // Also erase its articles
    return true;

}

// Returns list of all articles in the given newsgroup
//  Every element in the list contains article ID and title of the article
std::vector<ArticleSummary> InMemoryDatabase::listArticles(int ng_id) const {
    std::vector<ArticleSummary> summaries;
    auto it = articles_by_ng.find(ng_id);
    if (it == articles_by_ng.end()) return summaries;

    for (const Article& article : it->second) {
        summaries.push_back({article.id, article.title});  // Only store ID and title
    }
    return summaries;
}

// Assigns/publishes an article to a newsgroup
bool InMemoryDatabase::createArticle(int ng_id, const std::string& title, const std::string& author, const std::string& text) {
    if (newsgroups.find(ng_id) == newsgroups.end()) return false;
    Article a{next_article_id++, title, author, text};
    articles_by_ng[ng_id].push_back(a);
    return true;
}

// Deletes article from newsgroup
bool InMemoryDatabase::deleteArticle(int ng_id, int art_id) {
    auto it = articles_by_ng.find(ng_id);
    if (it == articles_by_ng.end()) return false;

    auto& list = it->second;
    for (auto iter = list.begin(); iter != list.end(); ++iter) {
        if (iter->id == art_id) {
            list.erase(iter);
            return true;
        }
    }
    return false;
}

// Returns a certain article object based on newsgroup and article ID
// Throws error if article or newsgroup does not exist
Article InMemoryDatabase::getArticle(int ng_id, int art_id) const {
    auto ng_it = articles_by_ng.find(ng_id);
    if (ng_it == articles_by_ng.end()) throw std::runtime_error("Newsgroup does not exist");

    for (const auto& art : ng_it->second) {
        if (art.id == art_id) return art;
    }
    throw std::runtime_error("Article does not exist");
}