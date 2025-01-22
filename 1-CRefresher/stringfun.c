#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *), print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);

//add additional prototypes here
int reverse_string(char *buff, int len, int str_len), print_words(char *buff, int len, int str_len);

int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions
    char *toPtr = buff;
    char *fromPtr = user_str;
    int count = 0, consecutiveSpaceBool = 0;

    while (*fromPtr != '\0') {
        if (*fromPtr == ' ' || *fromPtr == '\t') {
            if (!consecutiveSpaceBool) {
                if (count  >= len) {
                    return -1; // String too large
                }
                *toPtr = ' ';
                toPtr++;
                count++;
                consecutiveSpaceBool = 1;
            }
        } else {
            if (count >= len) {
                return -1; // String too large
            }
            *toPtr = *fromPtr;
            toPtr++;
            count++;
            consecutiveSpaceBool = 0;
        }
        fromPtr++;
    }

    while (count < len) {
        *toPtr++ = '.';

        count++;
    }

    int validCharacters = 0;
    char *checkPtr = buff;
    for (int i = 0; i < len; i++) {
        if (*checkPtr == '.') {
            break;
        }
        validCharacters++;
        checkPtr++;
    }

    return validCharacters;
}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
    printf("-h  prints help about the program\n");
    printf("-c  counts number of words in the string\n");
    printf("-r  reverses the string\n");
    printf("-w  prints the individual words and their lenghts\n");
    printf("-x  replaces one word with another.\n");
}

int count_words(char *buff, int len, int str_len){
    int count = 0, inWord = 0; // count is a counter, inWord is a boolean.
    char *pCurrentChar = buff;

    for (int i = 0; i < str_len; i++) {
        if (*pCurrentChar != ' ' && inWord == 0) {
            count++;
            inWord = 1;
            }
        if (*pCurrentChar == ' ' && inWord == 1){
                inWord = 0;
            }

        pCurrentChar++;

    }

    return count;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

int reversed_string(char *buff, int len, int str_len) {
    if (str_len < 1) {
        return -1; // invalid input or string doesn't exist
    }

    char *startChar = buff, *endChar = buff + (str_len - 1);

    while (startChar < endChar) {
        char swappedChar = *startChar;
        *startChar = *endChar;
        *endChar = swappedChar;

        // Close the gap from both directions
        startChar++;
        endChar--;

    }
    return 0;
}


int print_words(char *buff, int len, int str_len) {
    printf("Word Print\n");
    printf("----------\n");

    char *pCurrentChar = buff;
    int i = 0, wordCount = 0;

    while (i < str_len) {
        while ((i < str_len) && (*pCurrentChar == ' ')) {
            pCurrentChar++;
            i++;
        }
        if (i >= str_len) {
            break;
        }
       wordCount++;
        printf("%d. ", wordCount);

       int wordLength = 0;
        while ((i < str_len) && (*pCurrentChar != ' ')) {
            printf("%c", *pCurrentChar);
            pCurrentChar++;
            i++;
            wordLength++;
        }
        printf("(%d)\n", wordLength);
    }
    printf("\nNumber of words returned: %d\n", wordCount);

    return 0;
}

int main(int argc, char *argv[]){

    char opt, *buff = "\0", *input_string = "\0";
    int  rc, user_str_len;
    /*
     * buff - placehoder for the internal buffer
     * input_string - holds the string provided by the user on cmd line
     * opt - used to capture user option from cmd line
     * rc - used for return codes
     * user_str_len - length of user supplied string
     */

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
/*
 * If there is less than the required amount of arguments the code will exit with exit code 1,
 * informing us of the fault. Exit code 1 means we have a formatting errors or
 * missing arguments. This is significantly more informative than letting it run till it crashes and
 * having a random error pop up.
 */

    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    /*
     * This code checks that we have provided a string argument. If the check fails,
     * it will exit code 1. Once again indicating a command-line error.
     */
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a
    //          return code of 99
    // CODE GOES HERE FOR #3
    buff = (char *)malloc(BUFFER_SZ);
    if (!buff) {
        fprintf(stderr, "Memory got cooked\n");
        exit(99);
    }

    // setup_buff
    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        free(buff);
        exit(3);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);
            if (rc < 0) {
                printf("Error counting words, rc = %d", rc);
                free(buff);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;
        case 'r':
            rc = reversed_string(buff, BUFFER_SZ, user_str_len);
        if (rc < 0) {
            printf("Error reversing string, rc = %d", rc);
            free(buff);
            exit(3);
        }
//        printf("Reversed String:");
//        for (int i = 0; i < user_str_len; i++) {
//            putchar(*(buff+i));
//        }
//        putchar('\n');
        break;

        case 'w':
            rc = print_words(buff, BUFFER_SZ, user_str_len);

        /*
         * Everything that would be checked in print_words is already checked, so
         * print_words does not do any error checking. The code below it redundant
         * but consistent.
         */
        if (rc < 0) {
                printf("Error printing words, rc = %d", rc);
                free(buff);
                exit(3);
            }	
            break;

        case 'x':
            // not implemented 
            printf("Not Implemented!");
        free(buff);
	exit(0);
        break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        default:
            usage(argv[0]);
            free(buff);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that
//          the buff variable will have exactly 50 bytes?
//
//          PLACE YOUR ANSWER HERE
//          It is easier to read for the person that is maintaining the code.
//          It is also good practice because it can be helpful to catch errors.

