// message_handler.h
#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <memory>
#include <string>

class Connection; // forward declaration

class MessageHandler {
public:
  explicit MessageHandler(std::shared_ptr<Connection> conn);

  void sendByte(unsigned char code);
  unsigned char readByte();

  void sendNumber(int value);
  int readNumber();

  void sendString(const std::string& str);
  std::string readString();

private:
  std::shared_ptr<Connection> conn;
};

#endif // MESSAGE_HANDLER_H
