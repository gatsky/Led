#ifndef SERVER_H_
#define SERVER_H_

#include <thread>
#include <mutex>
#include <memory>
#include <map>
#include <vector>

class Server
{
public:
  Server();
  ~Server();

  bool Run(int port);
  void Stop();

  void GetLedState(std::string& state);

private:
  std::shared_ptr<std::thread> m_work_thread;
  int                          m_listener;
  bool                         m_stop;

  typedef bool(Server::*tCmdFn)(const std::string& args, std::string& res);
  std::map<std::string, tCmdFn> m_cmd_funcs;

  struct LedState
  {
    bool        on;
    int         rate;
    std::string color;
  };
  LedState   m_led_state;
  std::mutex m_led_state_lock;

  static std::vector<std::string> m_led_colors;

  void WorkThread();
  void ExecuteCmd(int sck, std::string& cmd, std::string& args);

  bool CmdSetLedState(const std::string& args, std::string& res);
  bool CmdGetLedState(const std::string& args, std::string& res);
  bool CmdSetLedColor(const std::string& args, std::string& res);
  bool CmdGetLedColor(const std::string& args, std::string& res);
  bool CmdSetLedRate(const std::string& args, std::string& res);
  bool CmdGetLedRate(const std::string& args, std::string& res);
};

#endif /* SERVER_H_ */
