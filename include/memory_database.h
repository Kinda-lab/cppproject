#ifndef INMEMORYDATABASE_H
#define INMEMORYDATABASE_H

#include "database.h"
#include <string>
#include <vector>
#include <map>

class InMemoryDatabase : public Database {
private:
   int next_group_id = 1; // Counter Newsgroup ID
   int next_article_id = 1; // Counter Article ID

   std::map<int, Newsgroup> newsgroups; // maps group ID to newsgroup in the database
   std::map<int, std::vector<Article>> articles_by_ng; // maps group ID to articles in newsgroup with a certin newsgroup id
   

public:
   // Newsgroups
   std::vector<Newsgroup> listNewsgroups() const override;
   bool createNewsgroup(const std::string& name) override;
   bool deleteNewsgroup(int id) override;
 
   // Articles
   std::vector<ArticleSummary> listArticles(int ng_id) const override;
   bool createArticle(int ng_id, const std::string& title, const std::string& author, const std::string& text) override;
   bool deleteArticle(int ng_id, int art_id) override;
   Article getArticle(int ng_id, int art_id) const override;
};

#endif // INMEMORYDATABASE_H