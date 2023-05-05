// COMP3511 Spring 2023
// PA3: Page Replacement Algorithms
//
// Your name: ZHAO Yu Xuan
// Your ITSC email: yxzhao@connect.ust.hk
//
// Declaration:
//
// I declare that I am not involved in plagiarism
// I understand that both parties (i.e., students providing the codes and students copying the codes) will receive 0 marks.

// ===
// Region: Header files
// Note: Necessary header files are included, do not include extra header files
// ===
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ===
// Region: Constants
// ===

#define UNFILLED_FRAME -1
#define MAX_QUEUE_SIZE 10
#define MAX_FRAMES_AVAILABLE 10
#define MAX_REFERENCE_STRING 30

#define ALGORITHM_FIFO "FIFO"
#define ALGORITHM_OPT "OPT"
#define ALGORITHM_LRU "LRU"
#define ALGORITHM_CLOCK "CLOCK"

// Keywords (to be used when parsing the input)
#define KEYWORD_ALGORITHM "algorithm"
#define KEYWORD_FRAMES_AVAILABLE "frames_available"
#define KEYWORD_REFERENCE_STRING_LENGTH "reference_string_length"
#define KEYWORD_REFERENCE_STRING "reference_string"



// Assume that we only need to support 2 types of space characters:
// " " (space), "\t" (tab)
#define SPACE_CHARS " \t"

// ===
// Region: Global variables:
// For simplicity, let's make everything static without any dyanmic memory allocation
// In other words, we don't need to use malloc()/free()
// It will save you lots of time to debug if everything is static
// ===
char algorithm[10];
int reference_string[MAX_REFERENCE_STRING];
int reference_string_length;
int frames_available;
int frames[MAX_FRAMES_AVAILABLE];

// Helper function: Check whether the line is a blank line (for input parsing)
int is_blank(char *line)
{
    char *ch = line;
    while (*ch != '\0')
    {
        if (!isspace(*ch))
            return 0;
        ch++;
    }
    return 1;
}
// Helper function: Check whether the input line should be skipped
int is_skip(char *line)
{
    if (is_blank(line))
        return 1;
    char *ch = line;
    while (*ch != '\0')
    {
        if (!isspace(*ch) && *ch == '#')
            return 1;
        ch++;
    }
    return 0;
}
// Helper: parse_tokens function
void parse_tokens(char **argv, char *line, int *numTokens, char *delimiter)
{
    int argc = 0;
    char *token = strtok(line, delimiter);
    while (token != NULL)
    {
        argv[argc++] = token;
        token = strtok(NULL, delimiter);
    }
    *numTokens = argc;
}

// Helper: parse the input file
void parse_input()
{
    FILE *fp = stdin;
    char *line = NULL;
    ssize_t nread;
    size_t len = 0;

    char *two_tokens[2];                                 // buffer for 2 tokens
    char *reference_string_tokens[MAX_REFERENCE_STRING]; // buffer for the reference string
    int numTokens = 0, n = 0, i = 0;
    char equal_plus_spaces_delimiters[5] = "";

    strcpy(equal_plus_spaces_delimiters, "=");
    strcat(equal_plus_spaces_delimiters, SPACE_CHARS);

    while ((nread = getline(&line, &len, fp)) != -1)
    {
        if (is_skip(line) == 0)
        {
            line = strtok(line, "\n");
            if (strstr(line, KEYWORD_ALGORITHM))
            {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2)
                {
                    strcpy(algorithm, two_tokens[1]);
                }
            }
            else if (strstr(line, KEYWORD_FRAMES_AVAILABLE))
            {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2)
                {
                    sscanf(two_tokens[1], "%d", &frames_available);
                }
            }
            else if (strstr(line, KEYWORD_REFERENCE_STRING_LENGTH))
            {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2)
                {
                    sscanf(two_tokens[1], "%d", &reference_string_length);
                }
            }
            else if (strstr(line, KEYWORD_REFERENCE_STRING))
            {
                parse_tokens(two_tokens, line, &numTokens, "=");
                if (numTokens == 2)
                {
                    parse_tokens(reference_string_tokens, two_tokens[1], &n, SPACE_CHARS);
                    for (i = 0; i < n; i++)
                    {
                        sscanf(reference_string_tokens[i], "%d", &reference_string[i]);
                    }
                }
            }
        }
    }
}
// Helper: Display the parsed values
void print_parsed_values()
{
    int i;
    printf("%s = %s\n", KEYWORD_ALGORITHM, algorithm);
    printf("%s = %d\n", KEYWORD_FRAMES_AVAILABLE, frames_available);
    printf("%s = %d\n", KEYWORD_REFERENCE_STRING_LENGTH, reference_string_length);
    printf("%s = ", KEYWORD_REFERENCE_STRING);
    for (i = 0; i < reference_string_length; i++)
        printf("%d ", reference_string[i]);
    printf("\n");
}

// Useful string template used in printf()
// We will use diff program to auto-grade the PA2 submissions
// Please use the following templates in printf to avoid formatting errors
//
// Example:
//
//   printf(template_total_page_fault, 0)    # Total Page Fault: 0 is printed on the screen
//   printf(template_no_page_fault, 0)       # 0: No Page Fault is printed on the screen

const char template_total_page_fault[] = "Total Page Fault: %d\n";
const char template_no_page_fault[] = "%d: No Page Fault\n";

// Helper function:
// This function is useful for printing the fault frames in this format:
// current_frame: f0 f1 ...
//
// For example: the following 4 lines can use this helper function to print
//
// 7: 7
// 0: 7 0
// 1: 7 0 1
// 2: 2 0 1
//
// For the non-fault frames, you should use template_no_page_fault, referne (see above)
//
void display_fault_frame(int current_frame)
{
    int j;
    printf("%d: ", current_frame);
    for (j = 0; j < frames_available; j++)
    {
        if (frames[j] != UNFILLED_FRAME)
            printf("%d ", frames[j]);
        else
            printf("  ");
    }
    printf("\n");
}

// Helper function: initialize the frames
void frames_init()
{
    int i;
    for (i = 0; i < frames_available; i++)
        frames[i] = UNFILLED_FRAME;
}

// Helper function to check whether there is a page fault
// return: hit->0; page fault->1
int is_page_fault(int current_frame)
{
    int i;
    for (i = 0; i < frames_available; i++)
    {
        if (current_frame == frames[i])
            return 0;
    }
    return 1;
}

int empty_frame_spot()
{
    int i;
    for (i = 0; i < frames_available; i++)
    {
        if (frames[i] == UNFILLED_FRAME)
            return i;
    }
    return -1;
}
void FIFO_replacement()
{
    // TODO: Implement FIFO replacement here
    int i;
    int pointer = 0, total_page_faults = 0;
    for (i = 0; i < reference_string_length; i++)
    {
        int empty_index = empty_frame_spot;
        if (is_page_fault(reference_string[i]))
        {
            total_page_faults++;
            display_fault_frame(reference_string[i]);
            if (empty_frame_spot != -1)
                frames[empty_index] = reference_string[i];
            else
            {
                frames[pointer++] = reference_string[i];
                pointer %= frames_available;
            }
        }else
            printf(template_no_page_fault, reference_string[i]);
    }
    printf(template_total_page_fault, total_page_faults);
}

// return: the smallest victim frame, -1 indicates there is no victim frame
int victim(int current_reference_index)
{
    int i,j;
    int isVictim = 1, numVictims = 0;
    int victimFrames[MAX_REFERENCE_STRING] = {-1};
    int smallestVictim = 100000;

    for(i = 0; i < frames_available; i++)
    {
         for (j = current_reference_index + 1; i < reference_string_length; j++)
        {
            if (reference_string[j] == frames[i])
            {
                isVictim = 0;
                break;
            }
        }
        if (isVictim)
            victimFrames[numVictims++] = reference_string[j];
        isVictim = 1;
    }

    if (!numVictims)
        return -1;
    else
    {
        for (i = 0; i < numVictims; i++)
            smallestVictim = (victimFrames[i] < smallestVictim) ? victimFrames[i] : smallestVictim;
        return smallestVictim;
    }
}

// return furthest frame
int furthest_frame(int current_reference_index)
{
    int i,j, furthestFrame;
    int furthestDistance = -1000, furthestFrame = -1;
    for (i = 0; i < frames_available; i++)
    {
        for (j = current_reference_index + 1; j < reference_string_length; j++)
        {
            if (reference_string[j] == frames[i])
            {
                if (furthestDistance < (j - current_reference_index))
                {
                    furthestDistance = j - current_reference_index;
                    furthestFrame = frames[i];
                }
                break;
            }
        }
    }
    return furthestFrame;
}

// Assumption, the parameter frame must be one of the values inside frames
int getFrameIndex(int frame)
{
    int i;
    for (i = 0; i < frames_available; i++)
        if (frame == frames[i])
            return i;
}

void OPT_replacement()
{
    // TODO: Implement OPT replacement here
    int i;
    int total_page_faults = 0;
    for (i = 0; i < reference_string_length; i++)
    {
        int empty_index = empty_frame_spot();
        if (is_page_fault(reference_string[i]))
        {
            total_page_faults++;
            display_fault_frame(reference_string[i]);
            if (empty_index == -1)
            {
                int victim_frame = victim(i);
                if (victim_frame != -1)
                    frames[getFrameIndex(victim_frame)] = reference_string[i];
                else
                {
                    int frame = furthest_frame(i);
                    frames[getFrameIndex(frame)] = reference_string[i];
                }
            }else
                frames[empty_index] = reference_string[i];
        }else
            printf(template_no_page_fault, reference_string[i]);
    }
    printf(template_total_page_fault, total_page_faults);
}

void LRU_replacement()
{
    // TODO: Implement LRU replacement here
    int counters[MAX_QUEUE_SIZE] = {10000};
    int i, total_page_faults = 0;
    for (i = 0; i < reference_string_length; i++)
    {
        int emptyIndex = empty_frame_spot();
        if (is_page_fault(reference_string[i]))
        {   
            total_page_faults++;
            display_fault_frame(reference_string[i]);
            if (emptyIndex == -1)
            {
                int index, smallestIndex;
                int lru = 10000;
                for (index = 0; i < MAX_QUEUE_SIZE; index++)
                    if (counters[index] < lru)
                    {
                        lru = counters[index];
                        smallestIndex = index;
                    }

                frames[getFrameIndex(smallestIndex)] = reference_string[i];
                counters[smallestIndex] = i;
            }else
            {
                frames[emptyIndex] = reference_string[i];
                counters[reference_string[i]] = i;
            }
        }else
        {
            counters[reference_string[i]] = i;
            printf(template_no_page_fault, reference_string[i]);
        }
    }
    printf(template_total_page_fault, total_page_faults);
}

void CLOCK_replacement()
{
    // TODO: Implement CLOCK replacement here
    int second_chance[MAX_FRAMES_AVAILABLE] = {0};
    int pointer = 0, total_page_faults = 0, i;
    for (i = 0; i < reference_string_length; i++)
    {
        int emptyIndex = empty_frame_spot();
        if (is_page_fault(reference_string[i]))
        {
            if (emptyIndex == -1)
            {
                while (second_chance[pointer])
                {
                    second_chance[pointer] = 0;
                    pointer = (pointer + 1) % frames_available;
                }
                frames[pointer] = reference_string[i];
                pointer = (pointer + 1) % frames_available;
            }else
                frames[emptyIndex] = reference_string[i];
        }else
        {
            second_chance[getFrameIndex(reference_string[i])] = 1;
            printf(template_no_page_fault, reference_string[i]);
        }
    }
    printf(template_total_page_fault, total_page_faults);
}

int main()
{
    parse_input();              
    print_parsed_values();      
    frames_init();              

    if (strcmp(algorithm, ALGORITHM_FIFO) == 0)
    {
        FIFO_replacement();
    }
    else if (strcmp(algorithm, ALGORITHM_OPT) == 0)
    {
        OPT_replacement();
    }
    else if (strcmp(algorithm, ALGORITHM_LRU) == 0)
    {
        LRU_replacement();
    }
    else if (strcmp(algorithm, ALGORITHM_CLOCK) == 0)
    {
        CLOCK_replacement();
    }

    return 0;
}