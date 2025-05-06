#include "connection.h"
#include "connectionclosedexception.h"
#include "message_handler.h"
#include "protocol.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

using std::string;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;

/* Creates a client for the given args, if possible.
 * Otherwise exits with error code.
 */
std::shared_ptr<Connection> init(int argc, char* argv[])
{
    if (argc != 3) {
            cerr << "Usage: myclient host-name port-number" << endl;
            exit(1);
    }

    int port = -1;
    try {
            port = std::stoi(argv[2]);
    } catch (std::exception& e) {
            cerr << "Wrong port number. " << e.what() << endl;
            exit(2);
    }
    auto conn = std::make_shared<Connection>(argv[1], port);
    if (!conn->isConnected()) {
            cerr << "Connection attempt failed" << endl;
            exit(3);
    }

    return conn;
}

int listNewsgroups(std::shared_ptr<Connection> conn) {
    MessageHandler mh(conn);
    mh.sendCode(Protocol::COM_LIST_NG);
    mh.sendCode(Protocol::COM_END);

    if(mh.readCode() != Protocol::ANS_LIST_NG) {
        
    }
};

int createNewsgroup(std::shared_ptr<Connection> conn) {

};

int deleteNewsgroup(std::shared_ptr<Connection> conn) {

};

int listArticles(std::shared_ptr<Connection> conn) {

};

bool createArticle(std::shared_ptr<Connection> conn) {

};

bool deleteArticle(std::shared_ptr<Connection> conn) {

};

int getArticle(std::shared_ptr<Connection> conn) {

};

int app(std::shared_ptr<Connection> conn) {
    std::string input;
    int commandCode = 0;
    while (true) {
        cout << "Welcome to our news system! Here you can: \n";
        cout << "1. List a newsgroup\n";
        cout << "2. Create a newsgroup\n";
        cout << "3. Delete a newsgroup\n";
        cout << "4. List an articles\n";
        cout << "5. Create an article\n";
        cout << "6. Delete an article\n";
        cout << "7. Get an article\n";
        cout << "8. Exit system\n"; // comand end
        cout << "Choose an action by number: ";
    }

    std::getline(std::cin, input);

    try {
        int commandCode = std::stoi(input);
    } catch (ConnectionClosedException&) {
            cout << " no reply from server. Exiting." << endl;
            return 1;
    }

    switch (commandCode)
    {
    case 1:
        return listNewsgroups(conn);
    case 2:
        return createNewsgroup(conn);
    case 3:
        return deleteNewsgroup(conn);
    case 4:
        return listArticles(conn);
    case 5:
        return createArticle(conn);
    case 6:
    return deleteArticle(conn);
    case 7:
    return getArticle(conn);
    case 8:
        return 0;
    }
}

int main(int argc, char* argv[]) {
        auto conn = init(argc, argv);
        return app(conn);
}
