#include "Client.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

Client::Client(int port, const std::string& ip_addr)
  : m_port(port)
  , m_ip_addr(ip_addr)
{
  // TODO Auto-generated constructor stub

}

Client::~Client()
{
  // TODO Auto-generated destructor stub
}

bool Client::ExecuteCmd(const std::string& cmd, const std::string& args)
{
  m_result.clear();

  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if(sck < 0)
  {
    ResultError("Can't create socket");
    return false;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(m_port);
  addr.sin_addr.s_addr = inet_addr(m_ip_addr.c_str());;

  if(connect(sck, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    close(sck);
    ResultError("Can't connect to server");
    return false;
  }

  timeval timeout = {0};
  timeout.tv_sec = 3;
  setsockopt(sck, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  std::string message = cmd;
  if(!args.empty())
  {
    message += " ";
    message += args;
  }
  message += "\n";

  if(send(sck, message.c_str(), message.size(), 0) <= 0)
  {
    close(sck);
    ResultError("Cant't send message");
    return false;
  }

  char buf[1025];
  int readed = recv(sck, buf, 1024, 0);
  close(sck);

  if(readed <= 0)
  {
    ResultError("Cant't recv message");
    return false;
  }

  buf[readed] = 0;
  char* end = strchr(buf, '\n');
  if(!end)
  {
    ResultError("End of message not found");
    return false;
  }

  end[0] = 0;

  ParseAnswer(buf);
  return true;
}

const char* Client::GetResult()
{
  return m_result.c_str();
}

void Client::ParseAnswer(char* buf)
{
  char* val = strchr(buf, ' ');
  if(val)
  {
    val[0] = 0;
    ++val;
  }

  if(!strcmp(buf, "FAILED"))
  {
    m_result = "Failed";
  }
  else if(!strcmp(buf, "OK"))
  {
    m_result = "OK";
    if(val && val[0])
    {
      m_result += ". Value: ";
      m_result += val;
    }
  }
  else
  {
    m_result = "Unknown server's answer";
  }
}

void Client::ResultError(const char* msg)
{
  m_result = msg;
  m_result += ": ";
  m_result += strerror(*__errno_location ());;
}
