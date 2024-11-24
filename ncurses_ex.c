#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ncurses.h>
#include <string.h>
#include <signal.h>

#define NUM_DATA 6
#define MAX_PROGRESS 50

typedef struct {
    int id; 
    int state; 
    char name[32];
} data_t; 

void draw_progress_bars(data_t data[], int selected);


int main(int argc, char *argv[]) 
{
    data_t data[NUM_DATA] = {
        {1, 0, "Alice"},
        {2, 0, "Bob"},
        {3, 0, "Charlie"},
        {4, 0, "Diana"},
        {5, 0, "Eve"},
        {6, 0, "Frank"}
    };

    // Initialize ncurses 
    initscr();
    noecho();
    curs_set(0);
    cbreak();               
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);  // make getch() nonblocking

    int selected = 0;
    int quit = 0;

    while (quit == 0) {
        clear();
        draw_progress_bars(data, selected);

        int ch = getch();
        switch (ch) {
            case KEY_UP:
                if (selected > 0) 
                    selected--;
                break;
            case KEY_DOWN:
                if (selected < NUM_DATA - 1) 
                    selected++;
                break;
            case 's': // switch
                if (data[selected].state == 0) {
                    data[selected].state = 1;
                } else {
                    data[selected].state = 0;
                }
                break;
            case 'q': // Quit program 
                quit = 1;
                break;
        }

        usleep(50000); // 50ms 
    }

    // Final message 
    mvprintw(NUM_DATA + 4, 0, "Press any key to exit.");
    refresh();
    nodelay(stdscr, FALSE); // make getch() blocking mode
    getch();

    // Close ncurses
    endwin();

    return 0;
}

// Draw progress bar 
void draw_progress_bars(data_t data[], int selected) 
{
    int progress = 100; 
    for (int i = 0; i < NUM_DATA; i++) {
        mvprintw(i + 1, 0, "%sID=%d: %-20s [%3d%%] [", (i == selected) ? ">> " : "   ", 
                data[i].id, data[i].name, progress);

        for (int j = 0; j < MAX_PROGRESS; j++) {
            printw("=");
        }
        printw("] %s", data[i].state ? "OFF" : "ON");  // Print state 
    }

    mvprintw(NUM_DATA+2, 0, "   s:switch, q:quit");
    refresh();
}
