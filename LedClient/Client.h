#ifndef CLIENT_H_
#define CLIENT_H_

#include <string>

class Client
{
public:
  Client(int port, const std::string& ip_addr);
  ~Client();

  bool ExecuteCmd(const std::string& cmd, const std::string& args);
  const char* GetResult();

private:
  int         m_port;
  std::string m_ip_addr;
  std::string m_result;

  void ParseAnswer(char* buf);
  void ResultError(const char* msg);
};

#endif /* CLIENT_H_ */
