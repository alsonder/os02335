#include "io.h"
#include <unistd.h>  // For write and read system calls
#include <string.h>  // For strlen

/* Function to convert an integer to a string */
void int_to_string(int n, char* buffer) {
    int i = 0;
    int is_negative = 0;

    // Handle 0 explicitly
    if (n == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }

    // Handle negative numbers
    if (n < 0) {
        is_negative = 1;
        n = -n;
    }

    // Convert integer to string
    while (n != 0) {
        buffer[i++] = (n % 10) + '0'; // Get last digit and convert to char
        n /= 10; // Remove last digit
    }

    if (is_negative) {
        buffer[i++] = '-'; // Add negative sign
    }

    buffer[i] = '\0'; // Null-terminate the string

    // Reverse the string
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}

/* Reads next char from stdin. If no more characters, it returns EOF */
int read_char(void) {
    char c;
    ssize_t result = read(STDIN_FILENO, &c, 1); // Read one byte from stdin

    if (result == 1) {
        return (unsigned char)c; // Return the character read
    } else {
        return EOF; // Return EOF if no more characters
    }
}

/* Writes a character to stdout. If no errors occur, it returns 0, otherwise EOF */
int write_char(char c) {
    ssize_t result = write(STDOUT_FILENO, &c, 1); // Write one byte to stdout

    if (result == 1) {
        return 0; // Success
    } else {
        return EOF; // Error occurred
    }
}

/* Writes a null-terminated string to stdout. If no errors occur, it returns 0, otherwise EOF */
int write_string(const char* s) {
    if (s == NULL) {
        return EOF; // Handle null string
    }

    ssize_t length = strlen(s);
    ssize_t result = write(STDOUT_FILENO, s, length); // Write the string to stdout

    if (result == length) {
        return 0; // Success
    } else {
        return EOF; // Error occurred
    }
}

/* Writes an integer to stdout (without any formatting).   
 * If no errors occur, it returns 0, otherwise EOF
 */
int write_int(int n) {
    char buffer[12]; // Buffer to hold the string representation of the integer
    int_to_string(n, buffer); // Convert integer to string

    ssize_t result = write(STDOUT_FILENO, buffer, strlen(buffer)); // Write the string to stdout

    if (result == (ssize_t)strlen(buffer)) {
        return 0; // Success
    } else {
        return EOF; // Error occurred
    }
}
