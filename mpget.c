#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char *url;
    char *fileName;
    char *fileSize;
} Format;

typedef struct
{
    Format formts[8];
    int lineCount;
} FormatArray;

FILE *openFile(const char *fileName);
void checkFileExist(FormatArray formatArray);
FormatArray getformatArray(const char *fileName);
void freeFormatArray(FormatArray *formatArray);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Missing file name as an argument\n");
        return 1;
    }

    const char *fileName = argv[1];

    FormatArray formatArray = getformatArray(fileName);

    // Print the extracted data for DEBUG
    printf("Lines read: %d\n", formatArray.lineCount);
    for (int i = 0; i < formatArray.lineCount; i++)
    {
        printf("URL: %s, File Name: %s, File Size: %s\n",
               formatArray.formts[i].url,
               formatArray.formts[i].fileName,
               formatArray.formts[i].fileSize);
    }

    freeFormatArray(&formatArray);

    return 0;
}

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
        char *fileName = formatArray.formts[i].fileName;
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
            formatArray.formts[lineCount].url = strdup(url);
            formatArray.formts[lineCount].fileName = strdup(fileName);
            formatArray.formts[lineCount].fileSize = strdup(fileSize);
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

void freeFormatArray(FormatArray *formatArray)
{
    for (int i = 0; i < formatArray->lineCount; i++)
    {
        free(formatArray->formts[i].url);
        free(formatArray->formts[i].fileName);
        free(formatArray->formts[i].fileSize);
    }
}