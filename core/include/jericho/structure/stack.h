#ifndef STRUCTURES_STACK_H_
#define STRUCTURES_STACK_H_

#include "error.h"

#ifdef CPP_VERSION

struct StackNode {
    int data;
    struct StackNode* next;
};
 
class Stack {
  public:
    Stack(int data) {
        _root = (struct StackNode*)malloc(sizeof(struct StackNode));
        _root->data = data;
        _root->next = NULL;
    }
    
    ~Stack() { printf("Destroying Stack...\n"); }

    struct StackNode* newNode(int data) {
        struct StackNode* stackNode = (struct StackNode*)malloc(sizeof(struct StackNode));
        stackNode->data = data;
        stackNode->next = NULL;
        return stackNode;
    }
    
    int isEmpty() {
        return !_root;
    }
    
    void push(int data) {
        struct StackNode* stackNode = newNode(data);
        stackNode->next = _root;
        _root = stackNode;
    }
    
    int pop() {
        if (isEmpty()) return -1;
        struct StackNode* temp = _root;
        _root = _root->next;
        int popped = temp->data;
        free(temp);
    
        return popped;
    }
    
    int peek() {
        if (isEmpty()) return -1;
        return _root->data;
    }

    void print(StackNode* node) {
        printf("data is: %i\n", node->data);
    }

    void dump() {
        printf("###################################\n");
        printf("Stack Track\n");
        printf("===================================\n");
        StackNode* node = _root;
        do {
            print(node);
            node = node->next;
        } while (node != NULL);
        printf("###################################\n");
    }

  private:
    StackNode* _root;
};

#else

#include <stdlib.h>

typedef struct StackNode {
    int data;
    struct StackNode* next;
} StackNode;

typedef struct Stack {
    StackNode* root;
} Stack;

int stack_construct(Stack* stack, int data) {
    stack->root = (StackNode*)malloc(sizeof(StackNode));
    if (stack->root == NULL) {
        PERR(ENEM, "Failed to construct stack!\n");
    }
    stack->root->data = data;
    stack->root->next = NULL;
}
    
void stack_destroy(Stack* stack) {
    free(stack);
}

// ~Stack() { printf("Destroying Stack...\n"); }

struct StackNode* newNode(int data) {
    struct StackNode* stackNode = (struct StackNode*)malloc(sizeof(struct StackNode));
    stackNode->data = data;
    stackNode->next = NULL;
    return stackNode;
}

#endif

#endif