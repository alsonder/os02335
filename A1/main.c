/* You are not allowed to use <stdio.h> */
#include "io.h"
#include <stdlib.h>

typedef struct Node {
    int value;
    struct Node* next;
    struct Node* prev; 
} Node;

void add_to_collection(Node** head, int value);
void remove_last(Node** head);
void print_collection(Node* head);
void free_collection(Node* head);

/**
 * @name  main
 * @brief This function is the entry point to your program
 * @return 0 for success, anything else for failure
 */
int main(void)
{
    int counter = 0;
    Node* head = NULL;
    int command;

    while (1) {
        command = read_char();
        
        
        if (command == EOF || command == 'q' || (command != 'a' && command != 'b' && command != 'c')) {
            break;
        }

        switch (command) {
            case 'a':
                add_to_collection(&head, counter);
                counter++;
                break;
            case 'b':
                counter++;
                break;
            case 'c':
                remove_last(&head);
                counter++;
                break;
        }
    }

    
    print_collection(head);

    free_collection(head);

    return 0;
}

void add_to_collection(Node** head, int value) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        write_string("Memory allocation failed\n");
        exit(1);
    }
    new_node->value = value;
    new_node->next = NULL;
    new_node->prev = NULL;  

    if (*head == NULL) {
        *head = new_node;
    } else {
        Node* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
        new_node->prev = current;  
    }
}

void remove_last(Node** head) {
    if (*head == NULL) return;

    if ((*head)->next == NULL) {
        free(*head);
        *head = NULL;
        return;
    }

    Node* current = *head;
    while (current->next != NULL) {
        current = current->next;
    }

    if (current->prev != NULL) {
        current->prev->next = NULL;  
    }
    free(current);
}

void print_collection(Node* head) {
    Node* current = head;
    int first = 1;

    while (current != NULL) {
        if (!first) {
            write_char(',');  
        }
        write_int(current->value);
        first = 0;
        current = current->next;
    }
    write_char(';');  
    write_char('\n');  
}

void free_collection(Node* head) {
    Node* current = head;
    while (current != NULL) {
        Node* next = current->next;
        free(current);
        current = next;
    }
}
