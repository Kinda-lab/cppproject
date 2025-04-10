// message_handler.h
#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include "protocol.h"
#include "connection.h"
#include <memory>
#include <string>

class MessageHandler {
public:
  explicit MessageHandler(std::shared_ptr<Connection> conn);

  void sendCode(Protocol code); // for the protocal commands
  void sendCode(unsigned char code); // for raw bytes (if ever needed)
  unsigned char readCode();

  void sendNumber(int value);
  int readNumber();

  void sendString(const std::string& str);
  std::string readString();

private:
  std::shared_ptr<Connection> conn;
};

#endif // MESSAGE_HANDLER_H
