#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <map>

struct Newsgroup {
  int id;
  std::string name;
};

struct ArticleSummary{
  int id;
  std::string title;
};

struct Article {
  int id;
  std::string title;
  std::string author;
  std::string text;
};

class Database {
public:
  virtual ~Database() = default;

  // Newsgroups
  virtual std::vector<Newsgroup> listNewsgroups() const = 0;
  virtual bool createNewsgroup(const std::string& name) = 0;
  virtual bool deleteNewsgroup(int id) = 0;

  // Articles
  virtual std::vector<ArticleSummary> listArticles(int ng_id) const = 0;
  virtual bool createArticle(int ng_id, const std::string& title, const std::string& author, const std::string& text) = 0;
  virtual bool deleteArticle(int ng_id, int art_id) = 0;
  virtual Article getArticle(int ng_id, int art_id) const = 0;
};

#endif // DATABASE_H

