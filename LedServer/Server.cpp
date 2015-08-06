#include "Server.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <string.h>
#include <list>
#include <sstream>

#include <iostream>

std::vector<std::string> Server::m_led_colors{"red", "green", "blue"};

Server::Server()
  : m_listener(-1)
  , m_stop(false)
{
  m_led_state.on = false;
  m_led_state.color = "red";
  m_led_state.rate = 0;

  m_cmd_funcs["set-led-state"] = &Server::CmdSetLedState;
  m_cmd_funcs["get-led-state"] = &Server::CmdGetLedState;
  m_cmd_funcs["set-led-color"] = &Server::CmdSetLedColor;
  m_cmd_funcs["get-led-color"] = &Server::CmdGetLedColor;
  m_cmd_funcs["set-led-rate"] = &Server::CmdSetLedRate;
  m_cmd_funcs["get-led-rate"] = &Server::CmdGetLedRate;

}

Server::~Server()
{
  Stop();
}

bool Server::Run(int port)
{
  Stop();
  m_stop = false;

  struct sockaddr_in addr;

  m_listener = socket(AF_INET, SOCK_STREAM, 0);
  if(m_listener < 0)
  {
    perror("Can't create socket");
    return false;
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(m_listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    close(m_listener);
    m_listener = -1;

    perror("Can't bind socket");
    return false;
  }

  listen(m_listener, 1);

  m_work_thread.reset(new std::thread(&Server::WorkThread, this));

  return true;
}

void Server::Stop()
{
  m_stop = true;

  if(m_work_thread)
  {
    m_work_thread->join();
    m_work_thread.reset();
  }

  if(m_listener > -1)
  {
    close(m_listener);
    m_listener = -1;
  }
}

void Server::GetLedState(std::string& state)
{
  std::lock_guard<decltype(m_led_state_lock)> locker(m_led_state_lock);

  std::ostringstream ss;
  ss << "State: " << (m_led_state.on ? "on" : "off") << ", color: " << m_led_state.color << ", rate: " << m_led_state.rate;
  state = ss.str();
}

void Server::WorkThread()
{
  char buf[1025];

  std::list<int> clients;

  while(true)
  {
    if(m_stop)
      break;

    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(m_listener, &readset);
    int max_sck = m_listener;

    for(auto it = clients.begin(); it != clients.end(); ++it)
    {
      FD_SET(*it, &readset);

      if(*it > max_sck)
        max_sck = *it;
    }

    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if(select(max_sck + 1, &readset, NULL, NULL, &timeout) <= 0)
      continue;

    if(FD_ISSET(m_listener, &readset))
    {
      int sck = accept(m_listener, NULL, NULL);
      if(sck < 0)
        break;

      fcntl(sck, F_SETFL, O_NONBLOCK);
      clients.push_back(sck);
    }

    for(auto it = clients.begin(), itn = it; it != clients.end(); it = itn)
    {
      itn++;

      if(FD_ISSET(*it, &readset))
      {
        int readed = recv(*it, buf, 1024, 0);

        if(readed <= 0)
        {
          close(*it);
          clients.erase(it);
          continue;
        }

        buf[readed] = 0;

        char* end = strchr(buf, '\n');
        if(end)
        {
          end[0] = 0;

          std::string msg = buf;
          std::string args;

          size_t pos = msg.find (' ');
          if(pos != std::string::npos)
          {
            args = msg.substr(pos + 1);
            msg.erase(pos);
          }

          ExecuteCmd(*it, msg, args);
        }
      }
    }
  }

  for(auto it = clients.begin(); it != clients.end(); ++it)
    close(*it);
}

void Server::ExecuteCmd(int sck, std::string& cmd, std::string& args)
{
  std::string message = "FAILED";

  auto it = m_cmd_funcs.find(cmd);
  if(it != m_cmd_funcs.end())
  {
    std::string res_args;

    m_led_state_lock.lock();
    bool res = (this->*(it->second))(args, res_args);
    m_led_state_lock.unlock();

    if(res)
    {
      message = "OK";
      if(!res_args.empty())
      {
        message += " ";
        message += res_args;
      }
    }
  }

  message += "\n";
  send(sck, message.c_str(), message.size(), 0);
}

bool Server::CmdSetLedState(const std::string& args, std::string& res)
{
  if(args == "on")
    m_led_state.on = true;
  else if(args == "off")
    m_led_state.on = false;
  else
    return false;

  return true;
}

bool Server::CmdGetLedState(const std::string& args, std::string& res)
{
  res = m_led_state.on ? "on" : "off";
  return true;
}

bool Server::CmdSetLedColor(const std::string& args, std::string& res)
{
  auto it = std::find(m_led_colors.begin(), m_led_colors.end(), args);
  if(it == m_led_colors.end())
    return false;

  m_led_state.color = args;
  return true;
}

bool Server::CmdGetLedColor(const std::string& args, std::string& res)
{
  res = m_led_state.color;
  return true;
}

bool Server::CmdSetLedRate(const std::string& args, std::string& res)
{
  int rate;
  int count = sscanf(args.c_str(), "%d", &rate);
  if(count != 1)
    return false;
  if(rate < 0 || rate > 5)
    return false;

  m_led_state.rate = rate;
  return true;
}

bool Server::CmdGetLedRate(const std::string& args, std::string& res)
{
  char rate[20];
  sprintf(rate, "%d", m_led_state.rate);

  res = rate;
  return true;
}
