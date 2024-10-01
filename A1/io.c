    #include "io.h"
    #include <unistd.h>  // For write and read system calls
    #include <string.h>  // For strlen

    /*Github Copilot and perplexity.ai were used to correct the code and get insights on how to implement 
    the functions in the io.c file.*/

    static void int_to_string(int n, char* buffer);

    /* Reads next char from stdin. If no more characters, it returns EOF */
    int read_char(void) {
        char c;
        ssize_t result = read(STDIN_FILENO, &c, 1);

        if (result == 1) {
            return (unsigned char)c; 
        } else {
            return EOF; 
        }
    }

    /* Writes a character to stdout. If no errors occur, it returns 0, otherwise EOF */
    int write_char(char c) {
        ssize_t result = write(STDOUT_FILENO, &c, 1);

        if (result == 1) {
            return 0; 
        } else {
            return EOF; 
        }
    }
    /* Writes a null-terminated string to stdout. If no errors occur, it returns 0, otherwise EOF */
    int write_string(char* s) {
        if (s == NULL) {
            return EOF; 
        }

        ssize_t length = strlen(s);
        ssize_t result = write(STDOUT_FILENO, s, length); 

        if (result == length) {
            return 0; 
        } else {
            return EOF; 
        }
    }


    /* Writes an integer to stdout (without any formatting).   
    * If no errors occur, it returns 0, otherwise EOF
    */
    int write_int(int n) {
        char buffer[12]; 
        int_to_string(n, buffer); 

        ssize_t result = write(STDOUT_FILENO, buffer, strlen(buffer)); 

        if (result == (ssize_t)strlen(buffer)) {
            return 0; 
        } else {
            return EOF; 
        }
    }

    void int_to_string(int n, char* buffer) {
        int i = 0;
        int isNegative = 0;
        
        if (n == 0) {
            buffer[i++] = '0';
            buffer[i] = '\0';
            return;
        }
        
        if (n < 0) {
            isNegative = 1;
            n = -n;
        }
        
        while (n != 0) {
            buffer[i++] = n % 10 + '0';
            n = n / 10;
        }
        
        if (isNegative)
            buffer[i++] = '-';
        
        buffer[i] = '\0';
        
        // Reverse the string
        int start = 0;
        int end = i - 1;
        while (start < end) {
            char temp = buffer[start];
            buffer[start] = buffer[end];
            buffer[end] = temp;
            start++;
            end--;
        }
    }
