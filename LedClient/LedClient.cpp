#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <thread>

#include "Client.h"

struct LedCmd
{
  std::string cmd;
  std::string hint;
  std::string args;
};

LedCmd commands[] = { {"set-led-state", "set state", "on/off"},
                      {"get-led-state", "get state"},
                      {"set-led-color", "set color", "red/green/blue"},
                      {"get-led-color", "get color"},
                      {"set-led-rate", "set rate", "0-5"},
                      {"get-led-rate", "get rate"},
                      };
const int cmd_count = sizeof(commands)/sizeof(LedCmd);

void clrline()
{
  printf("%c[2K\r", 27);
}

void printCommands()
{
  std::cout << "Commands:" << std::endl;
  for(int i = 0; i != cmd_count; i++)
    std::cout << i + 1 << " - " << commands[i].hint << std::endl;
  std::cout << "q - quit" << std::endl;
  std::cout << "c - print commands" << std::endl;
}

int main(int argc, char ** argv)
{
  int port = 2341;
  std::string ip_addr = "127.0.0.1";

  --argc;
  argc /= 2;

  for(int io = 0; io < argc; ++io)
  {
    char* opt = argv[io*2 + 1];
    char* val = argv[io*2 + 2];

    if(!strcmp(opt, "--port"))
      port = atoi(val);
    else if(!strcmp(opt, "--ip"))
      ip_addr = val;
  }

  std::cout << "Client started. Server ip: " << ip_addr << ", port: " << port << std::endl;

  Client client(port, ip_addr);

  printCommands();

  std::cout << std::endl << "Enter command: ";

  while(true)
  {
    std::string cmd;
    std::cin >> cmd;

    if(cmd == "q")
      break;
    else if(cmd == "c")
    {
      printCommands();
    }
    else
    {
      int cmd_num = 0;
      int count = sscanf(cmd.c_str(), "%d", &cmd_num);
      if(count == 1 && cmd_num >= 1 && cmd_num <= cmd_count)
      {
        --cmd_num;

        std::cout << "  " << commands[cmd_num].hint << ":" << std::endl;

        std::string args;

        if(!commands[cmd_num].args.empty())
        {
          std::cout << "Enter value (" << commands[cmd_num].args << "): ";
          std::cin >> args;
        }

        std::cout << "Waiting result...";

        if(!client.ExecuteCmd(commands[cmd_num].cmd, args))
        {
          clrline();
          std::cout << "Error: " << client.GetResult() << std::endl;
        }
        else
        {
          clrline();
          std::cout << "Result: " << client.GetResult() << std::endl;
        }
      }
      else
      {
        clrline();
        std::cout << "Unknown command" << std::endl;
      }
    }

    std::cout << std::endl << "Enter command: ";
  }

  return 0;
}
