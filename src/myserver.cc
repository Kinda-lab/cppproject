#include "connection.h"
#include "connectionclosedexception.h"
#include "server.h"
#include "database.h"
#include "message_handler.h"
#include "protocol.h"
#include "memory_database.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

using std::cerr;
using std::cout;
using std::endl;
using std::string;

Server init(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: myserver port-number mem/disk" << endl;
        exit(1);
    }

    int port = -1;
    try
    {
        port = std::stoi(argv[1]);
    }
    catch (std::exception &e)
    {
        cerr << "Wrong format for port number. " << e.what() << endl;
        exit(2);
    }

    Server server(port);
    if (!server.isReady())
    {
        cerr << "Server initialization error." << endl;
        exit(3);
    }

    return server;
}
void list_ng(Database &db, MessageHandler hand)
{
    auto g = db.listNewsgroups();
    hand.sendCode(Protocol::ANS_LIST_NG);
    hand.sendNumber(g.size()); // antal grupper

    for (Newsgroup ng : g)
    {
        hand.sendNumber(ng.id);
        hand.sendString(ng.name);
    }

    hand.sendCode(Protocol::ANS_END);
}
void create_ng(Database &db, MessageHandler hand)
{
    try
    {
        string ng_title = hand.readString();
        hand.sendCode(Protocol::ANS_CREATE_NG);
        if (db.createNewsgroup(ng_title))
        {
            hand.sendCode(Protocol::ANS_ACK);
            std::cout << "Created newsgroup: " << ng_title << std::endl;
        }
        else
        {
            hand.sendCode(Protocol::ANS_NAK);
            hand.sendCode(Protocol::ERR_NG_ALREADY_EXISTS);
            std::cerr << "Failed to create newsgroup – already exists\n";
        }
        hand.sendCode(Protocol::ANS_END);
    } catch (const std::exception& e) {
        cerr << "Error in create_ng: " << e.what() << endl;
        throw ConnectionClosedException();
    }
}

void delete_ng(Database &db, MessageHandler hand)
{
    int ng_id = hand.readNumber();
    hand.sendCode(Protocol::ANS_DELETE_NG);
    try
    {
        db.deleteNewsgroup(ng_id);
    }
    catch (const std::runtime_error &e)
    {
        hand.sendCode(Protocol::ANS_NAK);
        hand.sendCode(Protocol::ERR_NG_DOES_NOT_EXIST);
        std::cerr << "Failed to delete newsgroup – doesn't exist\n";
    }
    hand.sendCode(Protocol::ANS_END);
}

void list_a(Database &db, MessageHandler hand)
{
    int ng_id = hand.readNumber();
    hand.sendCode(Protocol::ANS_LIST_ART);
    try
    {
        std::vector<ArticleSummary> summaries = db.listArticles(ng_id);
        hand.sendCode(Protocol::ANS_ACK);
        hand.sendNumber(summaries.size());

        for (const ArticleSummary &summary : summaries)
        {
            hand.sendNumber(summary.id);
            hand.sendString(summary.title);
        }
    }
    catch (...)
    {
        hand.sendCode(Protocol::ANS_NAK);
        hand.sendCode(Protocol::ERR_NG_DOES_NOT_EXIST);
    }
    hand.sendCode(Protocol::ANS_END);
}

void create_a(Database &db, MessageHandler hand)
{
    int group_id = hand.readNumber();
    string title = hand.readString();
    string author = hand.readString();
    string text = hand.readString();

    hand.sendCode(Protocol::ANS_CREATE_ART);
    try
    {
        db.createArticle(group_id, title, author, text);
        hand.sendCode(Protocol::ANS_ACK);
        std::cout << "Article added to group " << group_id << ": " << title << std::endl;
    }
    catch (...)
    {
        hand.sendCode(Protocol::ANS_NAK);
        hand.sendCode(Protocol::ERR_NG_DOES_NOT_EXIST);
        std::cerr << "Failed to add article – newsgroup does not exist\n";
    }
    hand.sendCode(Protocol::ANS_END);
}

void delete_a(Database &db, MessageHandler hand)
{
    int ng_id = hand.readNumber();
    int art_nbr = hand.readNumber();

    hand.sendCode(Protocol::ANS_DELETE_ART);
    try
    {
        db.deleteArticle(ng_id, art_nbr);
        hand.sendCode(Protocol::ANS_ACK);
        std::cout << "Article deleted " << ng_id << ": " << art_nbr << std::endl;
    }
    catch (...)
    {
        hand.sendCode(Protocol::ANS_NAK);
        hand.sendCode(Protocol::ERR_ART_DOES_NOT_EXIST);
        std::cerr << "Failed to add article – newsgroup does not exist\n";
    }
    hand.sendCode(Protocol::ANS_END);
}

void get_a(Database &db, MessageHandler hand)
{
    int ng_id = hand.readNumber();
    int art_id = hand.readNumber();

    hand.sendCode(Protocol::ANS_GET_ART);
    try
    {
        // db.getArticle(ng_id, art_nbr);
        // hand.sendCode(Protocol::ANS_ACK);
        Article article = db.getArticle(ng_id, art_id); // Fetch FULL article
        hand.sendCode(Protocol::ANS_ACK);
        hand.sendString(article.title);
        hand.sendString(article.author);
        hand.sendString(article.text);
        std::cout << "Article found " << ng_id << ": " << art_id << std::endl;
    }
    catch (...)
    {
        hand.sendCode(Protocol::ANS_NAK);
        hand.sendCode(Protocol::ERR_ART_DOES_NOT_EXIST);
        std::cerr << "Failed to get article – article does not exist\n";
    }
    hand.sendCode(Protocol::ANS_END);
}

void process_request(Server &server, std::shared_ptr<Connection> &conn, Database &db)
{
    MessageHandler hand{conn};
    try
    {
        auto task = hand.receiveCode();
        switch (task)
        {
        case Protocol::COM_LIST_NG:
            list_ng(db, hand);
            break;
        case Protocol::COM_CREATE_NG:
            create_ng(db, hand);
            break;
        case Protocol::COM_DELETE_NG:
            delete_ng(db, hand);
            break;
        case Protocol::COM_LIST_ART:
            list_a(db, hand);
            break;
        case Protocol::COM_CREATE_ART:
            create_a(db, hand);
            break;
        case Protocol::COM_DELETE_ART:
            delete_a(db, hand);
            break;
        case Protocol::COM_GET_ART:
            get_a(db, hand);
            break;
        default:
            throw ConnectionClosedException();
        }
        if (Protocol::COM_END != hand.receiveCode())
        {
            throw ConnectionClosedException();
        }
    }
    catch (ConnectionClosedException &)
    {
        server.deregisterConnection(conn);
        cout << "Client closed connection" << endl;
    } /* catch (ProtocolViolationException&) {
            server.deregisterConnection(conn);
            cout << "Client violated protocol" << endl;
    } */
}

void serve_one(Server &server, Database &db)
{
    auto conn = server.waitForActivity();
    if (conn != nullptr)
    {
        try
        {
            process_request(server, conn, db);
        }
        catch (ConnectionClosedException &)
        {
            server.deregisterConnection(conn);
            cout << "Client closed connection" << endl;
        }
    }
    else
    {
        conn = std::make_shared<Connection>();
        server.registerConnection(conn);
        cout << "New client connects" << endl;
    }
}

int main(int argc, char *argv[])
{
    auto server = init(argc, argv); // skapar en server
    string type = argv[2];
    std::unique_ptr<Database> db;
    if (type == "mem")
    {
        db = std::make_unique<InMemoryDatabase>();
    }
    else if (type == "disk")
    {
        // DiskDatabase db("/tmp/c++discdb"); //AVKOMEENTERA NÄR DISK ÄR INLAGD
    }
    else
    {
        cerr << "Invalid database type: " << type << endl;
        return 1;
    }

    while (true)
    {
        serve_one(server, *db); // så länge man inte stänger av servern kör den
    }
    return 0;
}