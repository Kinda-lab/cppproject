#include "connection.h"
#include "connectionclosedexception.h"
#include "server.h"
#include "database.h"
#include "message_handler.h"
#include "protocol.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

Server init(int argc, char* argv[])
{
        if (argc != 3) {
                cerr << "Usage: myserver port-number mem/disk" << endl;
                exit(1);
        }

        int port = -1;
        try {
                port = std::stoi(argv[1]);
        } catch (std::exception& e) {
                cerr << "Wrong format for port number. " << e.what() << endl;
                exit(2);
        }

        Server server(port);
        if (!server.isReady()) {
                cerr << "Server initialization error." << endl;
                exit(3);
        }

        return server;
}
void list_ng(Database db, MessageHandler hand) {
        auto g = db.listNewsgroups();
        hand.sendCode(Protocol::ANS_LIST_NG);
        hand.sendIntParameter(g.size());  //antal grupper
    
        for (Newsgroup ng : g) {
            hand.sendIntParameter(ng.getId());
            hand.sendStringParameter(ng.getName());
        }
    
        hand.sendCode(Protocol::ANS_END);
}
void create_ng(Database db, MessageHandler hand) {
        string ng_title = hand.readString();
        hand.sendCode(Protocol::ANS_CREATE_NG);
        if (db.createNewsgroup(ng_title)) {
                hand.sendCode(Protocol::ANS_ACK);
                std::cout << "Created newsgroup: " << ng_title << std::endl;
        } else {
                hand.sendCode(Protocol::ANS_NAK);
                hand.sendCode(Protocol::ERR_NG_ALREADY_EXISTS);
                std::cerr << "Failed to create newsgroup – already exists\n";
        }
        hand.sendCode(Protocol::ANS_END);
}

void delete_ng(Database db, MessageHandler hand) {
        int ng_id = hand.readNumber();
        hand.sendCode(Protocol::ANS_DELETE_NG);
        try {
                db.deleteNewsgroup(ng_id);
        } catch (const std::runtime_error& e) {
                hand.sendCode(Protocol::ANS_NAK);
                hand.sendCode(Protocol::ERR_NG_DOES_NOT_EXIST);
                std::cerr << "Failed to delete newsgroup – doesn't exist\n";
        }
        hand.sendCode(Protocol::ANS_END);
}

void list_a(Database db, MessageHandler hand) {
        int ng_id = hand.readNumber();
        hand.sendCode(Protocol::ANS_LIST_ART);
        
        try {
                auto art = db.listArticles(ng_id);
                hand.sendCode(Protocol::ANS_NAK);
                hand.sendNumber(art.size());
        } catch (const std::runtime_error& e) {
                hand.sendCode(Protocol::ANS_NAK);
                hand.sendCode(Protocol::ERR_NG_DOES_NOT_EXIST);
                std::cerr << "Failed to list articles – newsgroup doesn't exist\n";
        }
        hand.sendCode(Protocol::ANS_END);
}

void create_a(Database db, MessageHandler hand) {
        int ng_id = hand.readNumber();
        string title = hand.readString();
        string author = hand.readString();
        string text = hand.readString();

        hand.sendCode(Protocol::ANS_CREATE_ART);
        try{ 
                db.createArticle(ng_id, title, author, text);
                hand.sendCode(Protocol::ANS_ACK);
                std::cout << "Article added to group " << ngId << ": " << title << std::endl;
        } else {
                hand.sendCode(Protocol::ANS_NAK);
                hand.sendCode(Protocol::ERR_NG_DOES_NOT_EXIST);
                std::cerr << "Failed to add article – newsgroup does not exist\n";
        }
        msg.sendCode(Protocol::ANS_END);
}

void delete_a(Database db, MessageHandler hand) {
        int ng_id = hand.readNumber();
        int art_nbr = hand.readNumber();

        hand.sendCode(Protocol::ANS_DELETE_ART);
        try{ 
                db.deleteArticle(ng_id, art_nbr);
                hand.sendCode(Protocol::ANS_ACK);
                std::cout << "Article deleted " << ng_id << ": " << art_nbr << std::endl;
        } else {
                hand.sendCode(Protocol::ANS_NAK);
                hand.sendCode(Protocol::ERR_ART_DOES_NOT_EXIST);
                std::cerr << "Failed to add article – newsgroup does not exist\n";
        }
        msg.sendCode(Protocol::ANS_END);

}

void get_a(Database db, MessageHandler hand) {
        int ng_id = hand.readNumber();
        int art_nbr = hand.readNumber();

        hand.sendCode(Protocol::ANS_GET_ART);
        try{ 
                db.getArticle(ng_id, art_nbr);
                hand.sendCode(Protocol::ANS_ACK);
                std::cout << "Article found " << ng_id << ": " << art_nbr << std::endl;
        } else {
                hand.sendCode(Protocol::ANS_NAK);
                hand.sendCode(Protocol::ERR_ART_DOES_NOT_EXIST);
                std::cerr << "Failed to get article – article does not exist\n";
        }
        msg.sendCode(Protocol::ANS_END);
}

void process_request(std::shared_ptr<Connection>& conn) 
{
        MessageHandler hand{conn};
        try {
                auto task = hand.receiveCode();
                switch (task){
                        case Protocol::COM_LIST_NG   : list_ng(db, hand); break;
                        case Protocol::COM_CREATE_NG : create_ng(db, hand); break;
                        case Protocol::COM_DELETE_NG : delete_ng(db, hand); break;
                        case Protocol::COM_LIST_ART  : list_a(db, hand); break;
                        case Protocol::COM_CREATE_ART: create_a(db, hand); break;
                        case Protocol::COM_DELETE_ART: delete_a(db, hand); break;
                        case Protocol::COM_GET_ART   : get_a(db, hand); break;
                        default: throw ConnectionClosedException();
                }
                if (Protocol::COM_END != hand.receiveCode()) {
                        throw ConnectionClosedException();
                }
        } catch (ConnectionClosedException&) {
                server.deregisterConnection(conn);
                cout << "Client closed connection" << endl;
        } catch (ProtocolViolationException&) {
                server.deregisterConnection(conn);
                cout << "Client violated protocol" << endl;
        }
}


void serve_one(Server& server)
{
        auto conn = server.waitForActivity();
        if (conn != nullptr) { 
                try {
                    process_request(conn);
                } catch (ConnectionClosedException&) {
                        server.deregisterConnection(conn);
                        cout << "Client closed connection" << endl;
                }
        } else {
                conn = std::make_shared<Connection>();
                server.registerConnection(conn);
                cout << "New client connects" << endl;
        }
}

int main(int argc, char* argv[])
{
        auto server = init(argc, argv); //skapar en server
        string type = argv[2];
        if (type == "mem") {
                InMemoryDatabase db;
        } else if (type =="disk") {
                DiskDatabase db("/tmp/c++discdb");
        }

        while (true) {
            serve_one(server); //så länge man inte stänger av servern kör den 
        }
        return 0;
}