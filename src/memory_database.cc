#include "memory_database.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

std::vector<Newsgroup> InMemoryDatabase::listNewsgroups() const {
    return newsgroups;
}

bool InMemoryDatabase::createNewsgroup(const std::string& name) { // title of the group is sent as a parameter
    
}

bool InMemoryDatabase::deleteNewsgroup(int id) {

}
 
std::vector<ArticleSummary> InMemoryDatabase::listArticles(int ng_id) const {

}

bool InMemoryDatabase::createArticle(int ng_id, const std::string& title, const std::string& author, const std::string& text) {

}

bool InMemoryDatabase::deleteArticle(int ng_id, int art_id) {

}

Article InMemoryDatabase::getArticle(int ng_id, int art_id) const {

}