// message_handler.cc
#include "message_handler.h"
#include "connection.h"
#include "connectionclosedexception.h"

#include <stdexcept>
#include <string>

// Helper macro to ensure full reads
#define ENSURE_READ(x) if (!(x)) throw ConnectionClosedException();

MessageHandler::MessageHandler(std::shared_ptr<Connection> conn)
  : conn(conn) {
  if (!conn || !conn->isConnected()) {
    throw std::invalid_argument("Invalid connection");
  }
}

void MessageHandler::sendCode(Protocol code) {
  // Cast enum to underlying byte value
  conn->write(static_cast<unsigned char>(code));
}

void MessageHandler::sendCode(unsigned char byte) {
  conn->write(byte);
}

Protocol MessageHandler::receiveCode() {
  return static_cast<Protocol>(readCode());
}

unsigned char MessageHandler::readCode() {
  unsigned char byte = conn->read();
  return byte;
}

void MessageHandler::sendNumber(int value) {
  sendCode(Protocol::PAR_NUM);
  conn->write((value >> 24) & 0xFF);
  conn->write((value >> 16) & 0xFF);
  conn->write((value >> 8) & 0xFF);
  conn->write(value & 0xFF);
}

int MessageHandler::readNumber() {

  if(receiveCode() != Protocol::PAR_NUM) {
    throw ConnectionClosedException();
  }

  unsigned char b1 = conn->read();
  unsigned char b2 = conn->read();
  unsigned char b3 = conn->read();
  unsigned char b4 = conn->read();
  return (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
}

void MessageHandler::sendString(const std::string& str) {
  sendCode(Protocol::PAR_STRING);
  sendNumber(str.size());
  for (char c : str) {
    conn->write(c);
  }
}

std::string MessageHandler::readString() {
  if(receiveCode() != Protocol::PAR_STRING) {
    throw ConnectionClosedException();
  }
  int length = readNumber();
  std::string result;
  for (int i = 0; i < length; ++i) {
    result += conn->read();
  }
  return result;
}


