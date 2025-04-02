#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <tuple>

class Database {
public:
  using Newsgroup = std::pair<int, std::string>; // id, name
  using ArticleSummary = std::pair<int, std::string>; // id, title
  using Article = std::tuple<std::string, std::string, std::string>; // title, author, text

  virtual ~Database() {}

  virtual std::vector<Newsgroup> listNewsgroups() const = 0;
  virtual void createNewsgroup(const std::string& name) = 0;
  virtual void deleteNewsgroup(int id) = 0;

  virtual std::vector<ArticleSummary> listArticles(int ng_id) const = 0;
  virtual void createArticle(int ng_id, const std::string& title, const std::string& author, const std::string& text) = 0;
  virtual void deleteArticle(int ng_id, int art_id) = 0;
  virtual Article getArticle(int ng_id, int art_id) const = 0;
};

#endif // DATABASE_H

