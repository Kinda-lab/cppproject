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
    try {
        MessageHandler mh(conn);
        mh.sendCode(Protocol::COM_LIST_NG);
        mh.sendCode(Protocol::COM_END);

        if(mh.receiveCode() != Protocol::ANS_LIST_NG) {
            cerr << "Unexpected response from server\n";
            return 1;
        }

        int groupCount = mh.readNumber();
        cout << "Number of newsgroups: " << groupCount << endl;
        if(groupCount == 0) {
            cout << "Currently no newsgorups." << endl;
        }

        for (int i = 0; i < groupCount; ++i) {
            int id = mh.readNumber();
            std::string name = mh.readString();
            cout << "ID: [" << id << "], Group name: " << name << endl;
        }

        if (mh.receiveCode() != Protocol::ANS_END) {
            cerr << "Missing ANS_END\n";
            return 1;
        }
    } catch (ConnectionClosedException&) {
        cerr << "Server closed the connection unexpectedly.\n";
        return 1;
    }
    return 0;
};

int createNewsgroup(std::shared_ptr<Connection> conn) {
    try {
        MessageHandler mh(conn);
        std::string group_title;
        cout << "Enter a name for the new newsgroup: ";
        std::getline(std::cin >> std::ws, group_title); // ws discards leading whitespace from an input stream

        mh.sendCode(Protocol::COM_CREATE_NG);
        mh.sendString(group_title);
        mh.sendCode(Protocol::COM_END);

        if(mh.receiveCode() != Protocol::ANS_CREATE_NG) {
            cerr << "Unexpected response, expected ANS_CREATE_NG\n";
            return 1;
        }

        Protocol ack = mh.receiveCode();
        if(ack == Protocol::ANS_ACK) { // acknowledge
            cout << "Created newsgroup " << group_title << "\n";
        } else { // negative acknowledge
            Protocol error_code = mh.receiveCode();
            if (error_code == Protocol::ERR_NG_ALREADY_EXISTS) { 
                cerr << "The newsgroup already exists.\n"; // character error stream
            } else {
                cerr << "Unexpected error.\n";
            }
        }
        if (mh.receiveCode() != Protocol::ANS_END) {
            cerr << "Unexpected response, expected ANS_END\n";
            return 1;
        }
    } catch (ConnectionClosedException&) {
        cerr << "Server closed the connection unexpectedly.\n";
        return 1;
    }
    return 0;
};

int deleteNewsgroup(std::shared_ptr<Connection> conn) {
    try {
        MessageHandler mh(conn);
        int id;
        cout << "Enter the ID of the newsgroup you want to delete: ";
        std::cin >> id;

        mh.sendCode(Protocol::COM_DELETE_NG);
        mh.sendNumber(id);
        mh.sendCode(Protocol::COM_END);

        if(mh.receiveCode() != Protocol::ANS_DELETE_NG) {
            cerr << "Unexpected response, expected ANS_DELETE_NG\n";
            return 1;
        }

        Protocol ack = mh.receiveCode(); // acknowledge
        if(ack == Protocol::ANS_ACK) {
            cout << "Deleted newsgroup " << id << "\n";
        } else { // negative acknowledge
            Protocol error_code = mh.receiveCode();
            if (error_code == Protocol::ERR_NG_DOES_NOT_EXIST) {
                cerr << "The newsgroup does not exists.\n"; //character error stream
            } else {
                cerr << "Unexpected error.\n";
            }
        }
        if (mh.receiveCode() != Protocol::ANS_END) {
            cerr << "Unexpected response, expected ANS_END\n";
            return 1;
        }
    } catch (ConnectionClosedException&) {
        cerr << "Server closed the connection unexpectedly.\n";
        return 1;
    }
    return 0;
};

int listArticles(std::shared_ptr<Connection> conn) { //list articles IN A NEWSGROUP
    try {
        MessageHandler mh(conn);
        std::string id;
        std::getline(std::cin >> std::ws, id); // ws discards leading whitespace from an input stream
        mh.sendCode(Protocol::COM_LIST_ART);
        mh.sendString(id); // newsgroup id is sent as parameter
        mh.sendCode(Protocol::COM_END);

        if(mh.receiveCode() != Protocol::ANS_LIST_ART) {
            cerr << "Unexpected response from server\n";
            return 1;
        }

        Protocol ack = mh.receiveCode();
        if(ack == Protocol::ANS_ACK) { // acknowledge
            int articleCount = mh.readNumber();
            cout << "Number of articles: " << articleCount << endl;
            if(articleCount == 0) {
                cout << "Currently no articles in this newsgroup." << endl;
            }
    
            for (int i = 0; i < articleCount; ++i) {
                int id = mh.readNumber();
                std::string title = mh.readString();
                cout << "ID: [" << id << "], Title: " << title << endl;
            }
        } else { // negative acknowledge
            Protocol error_code = mh.receiveCode();
            if (error_code == Protocol::ERR_NG_DOES_NOT_EXIST) { 
                cerr << "The newsgroup does not exists.\n"; // character error stream
            } else {
                cerr << "Unexpected error.\n";
            }
        }

        if (mh.receiveCode() != Protocol::ANS_END) {
            cerr << "Missing ANS_END\n";
            return 1;
        }
    } catch (ConnectionClosedException&) {
        cerr << "Server closed the connection unexpectedly.\n";
        return 1;
    }
    return 0;
};

bool createArticle(std::shared_ptr<Connection> conn) {
    try {
        MessageHandler mh(conn);
        int id;
        std::string title;
        std::string author;
        std::string text;
        cout << "Enter the newsgroup ID: ";
        std::cin >> id;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear leftover newline
        
        std::cout << "Enter the title of the article: ";
        std::getline(std::cin, title);

        std::cout << "Enter the author fo the article: ";
        std::getline(std::cin, author);

        std::cout << "Enter the article text: ";
        std::getline(std::cin, text);

        mh.sendCode(Protocol::COM_CREATE_ART);
        mh.sendNumber(id);
        mh.sendString(title);
        mh.sendString(author);
        mh.sendString(text);
        mh.sendCode(Protocol::COM_END);

        if(mh.receiveCode() != Protocol::ANS_CREATE_NG) {
            cerr << "Unexpected response, expected ANS_CREATE_NG\n";
            return 1;
        }

        Protocol ack = mh.receiveCode();
        if(ack == Protocol::ANS_ACK) { // acknowledge
            cout << "Created article" << title << "\n";
        } else { // negative acknowledge
            Protocol error_code = mh.receiveCode();
            if (error_code == Protocol::ERR_NG_DOES_NOT_EXIST) { 
                cerr << "The newsgroup does not exist.\n"; // character error stream
            } else {
                cerr << "Unexpected error.\n";
            }
        }
        if (mh.receiveCode() != Protocol::ANS_END) {
            cerr << "Unexpected response, expected ANS_END\n";
            return 1;
        }
    } catch (ConnectionClosedException&) {
        cerr << "Server closed the connection unexpectedly.\n";
        return 1;
    }
    return 0;
};

bool deleteArticle(std::shared_ptr<Connection> conn) {
    try {
        MessageHandler mh(conn);
        int group_id;
        int article_id;

        cout << "Enter the newsgroup ID: ";
        std::cin >> group_id;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear leftover newline
        
        std::cout << "Enter the article ID: ";
        std::cin >> article_id;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear leftover newline

        mh.sendCode(Protocol::COM_DELETE_ART);
        mh.sendNumber(group_id);
        mh.sendNumber(article_id);
        mh.sendCode(Protocol::COM_END);

        if(mh.receiveCode() != Protocol::ANS_DELETE_ART) {
            cerr << "Unexpected response, expected ANS_CREATE_NG\n";
            return 1;
        }

        Protocol ack = mh.receiveCode();
        if(ack == Protocol::ANS_ACK) { // acknowledge
            cout << "Deleted article" << article_id << "\n";
        } else { // negative acknowledge
            Protocol error_code = mh.receiveCode();
            if (error_code == Protocol::ERR_NG_DOES_NOT_EXIST) { 
                cerr << "The newsgroup does not exist.\n"; // character error stream
            } else if (error_code == Protocol::ERR_ART_DOES_NOT_EXIST) {
                cerr << "The article does not exist.\n"; // character error stream
            } else {
                cerr << "Unexpected error.\n";
            }
        }
        if (mh.receiveCode() != Protocol::ANS_END) {
            cerr << "Unexpected response, expected ANS_END\n";
            return 1;
        }
    } catch (ConnectionClosedException&) {
        cerr << "Server closed the connection unexpectedly.\n";
        return 1;
    }
    return 0;
};

int getArticle(std::shared_ptr<Connection> conn) {
    try {
        MessageHandler mh(conn);
        int group_id;
        int article_id;

        cout << "Enter the newsgroup ID: ";
        std::cin >> group_id;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear leftover newline
        
        std::cout << "Enter the article ID: ";
        std::cin >> article_id;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear leftover newline

        mh.sendCode(Protocol::COM_GET_ART);
        mh.sendNumber(group_id);
        mh.sendNumber(article_id);
        mh.sendCode(Protocol::COM_END);

        if(mh.receiveCode() != Protocol::ANS_GET_ART) {
            cerr << "Unexpected response, expected ANS_CREATE_NG\n";
            return 1;
        }

        Protocol ack = mh.receiveCode();
        if(ack == Protocol::ANS_ACK) { // acknowledge
            string title = mh.readString();
            string author = mh.readString();
            string text = mh.readString();

            cout << "Title" << title << "\n";
            cout << "Author" << author << "\n";
            cout << "Text" << text << "\n";

        } else { // negative acknowledge
            Protocol error_code = mh.receiveCode();
            if (error_code == Protocol::ERR_NG_DOES_NOT_EXIST) { 
                cerr << "The newsgroup does not exist.\n"; // character error stream
            } else if (error_code == Protocol::ERR_ART_DOES_NOT_EXIST) {
                cerr << "The article does not exist.\n"; // character error stream
            } else {
                cerr << "Unexpected error.\n";
            }
        }
        if (mh.receiveCode() != Protocol::ANS_END) {
            cerr << "Unexpected response, expected ANS_END\n";
            return 1;
        }
    } catch (ConnectionClosedException&) {
        cerr << "Server closed the connection unexpectedly.\n";
        return 1;
    }
    return 0;
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
    
        std::getline(std::cin, input);

        try {
            int commandCode = std::stoi(input);
            if(commandCode < 1 || commandCode > 7) {
                cout << "Not a valid number.\n";
                continue;
            }
            break;
        } catch (ConnectionClosedException&) {
                cout << "No reply from server. Exiting." << endl;
                return 1;
        }
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
