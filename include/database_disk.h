#include "database.h"
#include "entities.h"
#include <string>
#include <vector>

class DiskDatabase : public Database {
public:
    DiskDatabase(const std::string& rootPath = "news_database");

    std::vector<ArticleSummary> listNewsgroups() const override;
    void createNewsgroup(const std::string& title) override;
    void deleteNewsgroup(int id) override;

    std::vector<ArticleSummary> listArticles(int newsgroupId) const override;
    void createArticle(int newsgroupId, const std::string& title,
                       const std::string& author, const std::string& text) override;
    void deleteArticle(int newsgroupId, int articleId) override;
    Article getArticle(int newsgroupId, int articleId) const override;

private:
    std::string root;
    int getNextNewsgroupId() const;
    int getNextArticleId(int newsgroupId) const;
    std::string getNewsgroupPath(int id) const;
    std::string getNewsgroupTitle(int id) const;
    std::string idTitleToDirName(int id, const std::string& title) const;
};
