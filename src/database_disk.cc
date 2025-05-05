#include <filesystem>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <sstream>

#include "database_disk.h"
#include "entities.h"

namespace fs = std::filesystem;

DiskDatabase::DiskDatabase(const std::string& rootPath) : root(rootPath) {
    if (!fs::exists(root)) {
        fs::create_directory(root);
    }
}

int DiskDatabase::getNextNewsgroupId() const {
    int maxId = 0;
    for (const auto& entry : fs::directory_iterator(root)) {
        auto name = entry.path().filename().string();
        auto id = std::stoi(name.substr(0, name.find('_')));
        maxId = std::max(maxId, id);
    }
    return maxId + 1;
}

std::string DiskDatabase::idTitleToDirName(int id, const std::string& title) const {
    std::string safeTitle = title;
    std::replace(safeTitle.begin(), safeTitle.end(), ' ', '_');
    return std::to_string(id) + "_" + safeTitle;
}

std::vector<Database::Newsgroup> DiskDatabase::listNewsgroups() const {
    std::vector<Database::Newsgroup> groups;
    for (const auto& entry : fs::directory_iterator(root)) {
        if (entry.is_directory()) {
            auto dir = entry.path().filename().string();
            auto pos = dir.find('_');
            if (pos != std::string::npos) {
                int id = std::stoi(dir.substr(0, pos));
                std::string title = dir.substr(pos + 1);
                std::replace(title.begin(), title.end(), '_', ' ');
                groups.emplace_back(id, title);
            }
        }
    }
    return groups;
}


void DiskDatabase::createNewsgroup(const std::string& title) {
    int id = getNextNewsgroupId();
    fs::create_directory(fs::path(root) / idTitleToDirName(id, title));
}

void DiskDatabase::deleteNewsgroup(int id) {
    for (const auto& entry : fs::directory_iterator(root)) {
        auto dir = entry.path().filename().string();
        if (dir.rfind(std::to_string(id) + "_", 0) == 0) {
            fs::remove_all(entry.path());
            return;
        }
    }
    throw std::runtime_error("Newsgroup ID not found");
}

std::string DiskDatabase::getNewsgroupPath(int id) const {
    for (const auto& entry : fs::directory_iterator(root)) {
        auto dir = entry.path().filename().string();
        if (dir.rfind(std::to_string(id) + "_", 0) == 0) {
            return entry.path().string();
        }
    }
    throw std::runtime_error("Newsgroup ID not found");
}

int DiskDatabase::getNextArticleId(int newsgroupId) const {
    std::string path = getNewsgroupPath(newsgroupId);
    int maxId = 0;
    for (const auto& file : fs::directory_iterator(path)) {
        auto filename = file.path().filename().string();
        if (filename.rfind("article_", 0) == 0) {
            int id = std::stoi(filename.substr(8, filename.find('.') - 8));
            maxId = std::max(maxId, id);
        }
    }
    return maxId + 1;
}

void DiskDatabase::createArticle(int newsgroupId, const std::string& title,
                                 const std::string& author, const std::string& text) {
    int id = getNextArticleId(newsgroupId);
    std::string path = getNewsgroupPath(newsgroupId) + "/article_" + std::to_string(id) + ".txt";
    std::ofstream out(path);
    out << title << "\n" << author << "\n" << text;
}

std::vector<Database::ArticleSummary> DiskDatabase::listArticles(int newsgroupId) const {
    std::vector<Database::ArticleSummary> summaries;
    std::string dir = getNewsgroupPath(newsgroupId);
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.path().extension() == ".txt") {
            std::ifstream in(entry.path());
            std::string title;
            std::getline(in, title);
            std::string filename = entry.path().filename().string();
            int id = std::stoi(filename.substr(8, filename.find('.') - 8));
            summaries.emplace_back(id, title);
        }
    }
    return summaries;
}

void DiskDatabase::deleteArticle(int newsgroupId, int articleId) {
    std::string file = getNewsgroupPath(newsgroupId) + "/article_" + std::to_string(articleId) + ".txt";
    if (!fs::remove(file)) {
        throw std::runtime_error("Article not found");
    }
}

Database::Article DiskDatabase::getArticle(int newsgroupId, int articleId) const {
    std::string file = getNewsgroupPath(newsgroupId) + "/article_" + std::to_string(articleId) + ".txt";
    std::ifstream in(file);
    if (!in) throw std::runtime_error("Article not found");

    std::string title, author, text;
    std::getline(in, title);
    std::getline(in, author);
    std::getline(in, text, '\0');
    return {title, author, text};
}

