#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ncurses.h>
#include <signal.h>
#include <unistd.h>

#define MAX_PROGRESS 50
#define MAX_FILES 8

typedef struct
{
    char *url;
    char *fileName;
    char *fileSize;
} Format;

typedef struct
{
    Format formats[8];
    int lineCount;
} FormatArray;

typedef struct
{
    pid_t pid;
    char *fileName;
    int progress;
    // enum으로 하는게 나을려나?
    int state; // 0: Running, 1: Stopped, 2: Terminated, 3: Completed
} Download;

FILE *openFile(const char *fileName);
void checkFileExist(FormatArray formatArray);
FormatArray getformatArray(const char *fileName);
void freeFormatArray(FormatArray *formatArray);
void startDownloading(FormatArray formatArray, Download downloads[]);
void drawProgressBars(Download downloads[], int fileCount, int selected);
void updateProgress(FormatArray formatArray, Download downloads[], int *allCompleted);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Missing file name as an argument\n");
        return 1;
    }

    const char *fileName = argv[1];
    FormatArray formatArray = getformatArray(fileName);
    checkFileExist(formatArray);

    Download downloads[formatArray.lineCount];
    startDownloading(formatArray, downloads);

    // Initialize ncurses
    initscr();
    noecho();
    curs_set(0);
    cbreak();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); // make getch() nonblocking

    int selected = 0;
    int quit = 0;

    while (!quit)
    {
        

        int allCompleted = 1;
        updateProgress(formatArray, downloads, &allCompleted);
        

        if (allCompleted)
        {
            drawProgressBars(downloads, formatArray.lineCount, selected);
            mvprintw(formatArray.lineCount + 4, 0, "File download completed. Press any key to exit.");
            refresh();
            getch();
            break;
        }

        clear();
        drawProgressBars(downloads, formatArray.lineCount, selected);
        refresh();


        int ch = getch();
        switch (ch)
        {
        case KEY_UP:
            if (selected > 0)
                selected--;
            break;
        case KEY_DOWN:
            if (selected < formatArray.lineCount - 1)
                selected++;
            break;
        case 's': // Stop downloading
            if (downloads[selected].state == 0)
            {
                kill(downloads[selected].pid, SIGSTOP);
                downloads[selected].state = 1; // Stop
            }
            break;
        case 'c': // Continue downloading
            if (downloads[selected].state == 1)
            {
                kill(downloads[selected].pid, SIGCONT);
                downloads[selected].state = 0; // Running
            }
            break;
        case 'x': // Terminate downloading
            if (downloads[selected].state == 0 || downloads[selected].state == 1)
            {
                kill(downloads[selected].pid, SIGKILL);
                downloads[selected].state = 2; // Terminated
            }
            break;
        case 'q': // Quit program
            quit = 1;
            for (int i = 0; i < formatArray.lineCount; i++)
            {
                if (downloads[i].state == 0 || downloads[i].state == 1)
                {
                    kill(downloads[i].pid, SIGKILL);
                }
            }
            break;
        }

        usleep(50000);
    }

    nodelay(stdscr, FALSE);
    mvprintw(formatArray.lineCount + 4, 0, quit ? "Quit program. Press any key to exit." : "File download completed. Press any key to exit.");
    refresh();
    getch();

    // Close ncurses
    endwin();
    freeFormatArray(&formatArray);

    return 0;
}

// ========================= end of main ==========================

FILE *openFile(const char *fileName)
{
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        perror("Error, there is no file\n");
        exit(1);
    }
    return fp;
}

void checkFileExist(FormatArray formatArray)
{
    for (int i = 0; i < formatArray.lineCount; i++)
    {
        char *fileName = formatArray.formats[i].fileName;
        FILE *fp = fopen(fileName, "r");
        if (fp != NULL)
        {
            fclose(fp);

            if (remove(fileName) == 0)
            {
            }
            else
            {
                printf("Error: Unable to delete file '%s'.\n", fileName);
                exit(1);
            }
        }
    }
}

FormatArray getformatArray(const char *fileName)
{
    FormatArray formatArray = {0};
    FILE *fp = openFile(fileName);

    int lineCount = 0;
    char line[256];
    while (fgets(line, sizeof(line), fp))
    {
        line[strcspn(line, "\n")] = '\0';

        // 토큰화
        char *url = strtok(line, ",");
        char *fileName = strtok(NULL, ",");
        char *fileSize = strtok(NULL, ",");

        if (url && fileName && fileSize)
        {
            formatArray.formats[lineCount].url = strdup(url);
            formatArray.formats[lineCount].fileName = strdup(fileName);
            formatArray.formats[lineCount].fileSize = strdup(fileSize);
        }
        else
        {
            printf("Invalid format in line %d\n", lineCount + 1);
            continue;
        }

        lineCount++;
        if (lineCount >= 8)
        {
            break;
        }
    }

    formatArray.lineCount = lineCount;
    fclose(fp);
    return formatArray;
}

void startDownloading(FormatArray formatArray, Download downloads[])
{
    for (int i = 0; i < formatArray.lineCount; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            execlp("wget", "wget", "-q", "-O", formatArray.formats[i].fileName, formatArray.formats[i].url, NULL);
            exit(1);
        }
        else if (pid > 0)
        {
            downloads[i].pid = pid;
            downloads[i].fileName = formatArray.formats[i].fileName;
            downloads[i].progress = 0;
            downloads[i].state = 0; // Running
        }
        else
        {
            perror("fork failed");
            exit(1);
        }
    }
}

void drawProgressBars(Download downloads[], int fileCount, int selected)
{
    for (int i = 0; i < fileCount; i++)
    {
        mvprintw(i, 0, "%sPID=%d: %-20s [%3d%%] [", (i == selected) ? ">> " : "   ",
                 downloads[i].pid, downloads[i].fileName, downloads[i].progress);

        int barLength = downloads[i].progress * MAX_PROGRESS / 100;
        for (int j = 0; j < barLength; j++)
        {
            printw("=");
        }
        for (int j = barLength; j < MAX_PROGRESS; j++)
        {
            printw(" ");
        }
        printw("] %s", downloads[i].state == 3 ? "Completed" : downloads[i].state == 2 ? "Terminated" : downloads[i].state == 1   ? "Stopped" : "Running");
    }
    mvprintw(fileCount + 2, 0, "s:stop, c:continue, x:terminated, q:quit(program)");
    refresh();
}

void updateProgress(FormatArray formatArray, Download downloads[], int *allCompleted)
{
    *allCompleted = 1;

    for (int i = 0; i < formatArray.lineCount; i++)
    {
        if (downloads[i].state == 0) // Running
        {
            struct stat st;
            if (stat(downloads[i].fileName, &st) == 0)
            {
                long currentSize = st.st_size; // 현재까지 다운한 사이즈
                long totalSize = atol(formatArray.formats[i].fileSize); // 파일의 전체 사이즈

                if (totalSize > 0)
                {
                    downloads[i].progress = (currentSize * 100) / totalSize;

                    // 다운한 사이즈가 파일 크기와 같아지면 100%
                    if (currentSize == totalSize)
                    {
                        downloads[i].progress = 100;
                    }
                }
            }

            // 프로세스가 종료되었는지 확인
            int status;
            pid_t result = waitpid(downloads[i].pid, &status, WNOHANG);
            if (result > 0 && WIFEXITED(status))
            {
                downloads[i].state = 3; // Completed
                downloads[i].progress = 100;  
            }
        }

        if (downloads[i].state != 3 && downloads[i].state != 2) 
        {
            *allCompleted = 0;
        }
    }
}


void freeFormatArray(FormatArray *formatArray)
{
    for (int i = 0; i < formatArray->lineCount; i++)
    {
        free(formatArray->formats[i].url);
        free(formatArray->formats[i].fileName);
        free(formatArray->formats[i].fileSize);
    }
}