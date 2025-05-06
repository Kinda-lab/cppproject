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
        if (argc != 2) {
                cerr << "Usage: myserver port-number" << endl;
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

void process_request(std::shared_ptr<Connection>& conn) 
{
        //int nbr = readNumber(conn);
        MessageHandler hand{conn};
        try {
                auto task = hand.read_byte();
                switch (task){
                        //DETTA KOMMER NOG INTE FUNKA DÅ DET ÄR LITE OLKART HUR DE OLIKA METODERNA TAR IN PARAMETRAR!!
                        case Protocol::COM_LIST_NG   : listNewsgroups (); break;
                        case Protocol::COM_CREATE_NG : createNewsgroup(hand); break;
                        case Protocol::COM_DELETE_NG : deleteNewsgroup(hand); break;
                        case Protocol::COM_LIST_ART  : listArticles   (hand); break;
                        case Protocol::COM_CREATE_ART: createArticle  (hand); break;
                        case Protocol::COM_DELETE_ART: deleteArticle  (hand); break;
                        case Protocol::COM_GET_ART   : getArticle     (hand); break;
                        default: throw ConnectionClosedException();
                }
                hand.send_byte(Protocol::ANS_END);
                if (Protocol::COM_END != m.recvCode()) {
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

        while (true) {
            serve_one(server); //så länge man inte stänger av servern kör den 
        }
        return 0;
}
