#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <thread>

#include "Server.h"

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}

void clrline()
{
  printf("%c[2K\r", 27);
}

int main(int argc, char ** argv)
{
  int port = 2341;

  --argc;
  argc /= 2;

  for(int io = 0; io < argc; ++io)
  {
    char* opt = argv[io*2 + 1];
    char* val = argv[io*2 + 2];

    if(!strcmp(opt, "--port"))
      port = atoi(val);
  }

  std::cout << "Starting server at port " << port << ". Press 'q' for exit." << std::endl;

  Server srv;
  if(!srv.Run(port))
  {
    exit(1);
  }

  std::cout << "Server started." << std::endl << std::endl;

  std::string led_state;

  while(true)
  {
    std::string new_state;
    srv.GetLedState(new_state);
    if(new_state != led_state)
    {
      led_state = new_state;
      clrline();
      std::cout << led_state;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if(!kbhit())
      continue;

    char c = getchar();

    if(c == 'q')
      break;
  }

  srv.Stop();

	return 0;
}
