#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

// compile cmd: gcc -g -o textEdit.exe ./textEdit.c
// run cmd: ./textEdit.exe

HANDLE hStdin;
DWORD fdwSaveOldMode;
int allocSize = 1024;
int bufferSize = 0;
int cursorPosition = 0;

void disable_raw_mode()
{
  SetConsoleMode(hStdin, fdwSaveOldMode);
}

void enable_raw_mode()
{
  hStdin = GetStdHandle(STD_INPUT_HANDLE);
  if (hStdin == INVALID_HANDLE_VALUE)
  {
    fprintf(stderr, "Error getting handle to stdin\n");
    exit(EXIT_FAILURE);
  }

  if (!GetConsoleMode(hStdin, &fdwSaveOldMode))
  {
    fprintf(stderr, "Error getting console mode\n");
    exit(EXIT_FAILURE);
  }

  atexit(disable_raw_mode);

  DWORD fdwMode = fdwSaveOldMode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
  if (!SetConsoleMode(hStdin, fdwMode))
  {
    fprintf(stderr, "Error setting console mode\n");
    exit(EXIT_FAILURE);
  }
}

void editor_refresh_screen()
{
  DWORD n;
  COORD coord = {0, 0};
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), ' ', csbi.dwSize.X * csbi.dwSize.Y, coord, &n);
  SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void editor_process_keypress(char *buffer)
{
  INPUT_RECORD irInBuf[128];
  DWORD cNumRead;
  while (1)
  {

    if (!ReadConsoleInput(hStdin, irInBuf, 128, &cNumRead))
    {
      perror("ReadConsoleInput");
      exit(1);
    }

    for (DWORD i = 0; i < cNumRead; i++)
    {
      if (irInBuf[i].EventType == KEY_EVENT && irInBuf[i].Event.KeyEvent.bKeyDown)
      {

        char c = irInBuf[i].Event.KeyEvent.uChar.AsciiChar;
        char key = irInBuf[i].Event.KeyEvent.wVirtualKeyCode;
        if (c == 8 && cursorPosition > 0)
        {
          cursorPosition--;
          printf("\b \b");
          bufferSize--;
        }
        else if (key == 37 || key == 39)
        {
          if (key == 37)
          {
            if (cursorPosition > 0)
            {
              cursorPosition--;
              printf("\b");
            }
          }
          else if (key == 39)
          {
            if (cursorPosition < bufferSize)
              cursorPosition++;
          }
          COORD coord = {cursorPosition, 0};
          SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
        }
        else if (c == 'q')
        {
          return;
        }
        else
        {
          if (bufferSize < allocSize - 1)
          {
            buffer[cursorPosition++] = c;
            bufferSize++;
            printf("%c", c);
          }
          else
          {
            allocSize *= 2;
            buffer = realloc(buffer, allocSize * sizeof(char));
            if (buffer == NULL)
            {
              printf("Failed to realloc memory for buffer.");
              exit(1);
            }
            buffer[cursorPosition++] = c;
            bufferSize++;
            printf("%c", c);
          }
          break;
        }
      }
    }
  }
}

int main()
{
  char *buffer = malloc(allocSize * sizeof(char));
  enable_raw_mode();
  editor_refresh_screen();
  editor_process_keypress(buffer);
  free(buffer);
  return 0;
}