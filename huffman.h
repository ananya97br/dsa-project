#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>

#define MAX 16

// Huffman tree node
typedef struct Node {
    char character;
    int freq;
    struct Node *l, *r;
} Node;

// Min heap for building the Huffman tree
typedef struct Min_Heap {
    int size;
    int capacity;
    Node** array;
} Min_Heap;

// Huffman code storage
typedef struct code {
    char k;
    int l;
    int *code_arr;
    struct code* p;
} code;

// Tree node for decompression
typedef struct Tree {
    char g;
    int len;
    int dec;
    struct Tree* f;
    struct Tree* r;
} Tree;

// External declaration for decompression tree
extern Tree* tree;

// Function declarations
Node* newNode(char character, int freq);
Tree* newTreeNode();
void swapNode(Node** a, Node** b);
void Heapify(Min_Heap* heap, int i);
int isSizeOne(Min_Heap* heap);
Node* extractMinFromMin_Heap(Min_Heap* heap);
void insertIntoMin_Heap(Min_Heap* heap, Node* node);
int isLeaf(Node* node);
int convertBinaryToDecimal(int arr[], int len);
void convertDecimalToBinary(int *bin, int dec, int len);
Min_Heap* createAndBuildMin_Heap(char arr[], int freq[], int size);
Node* buildHuffmanTree(char arr[], int freq[], int size);
void printCodesIntoFile(FILE* fp2, Node* root, int t[], int top, code** front, code** rear);
void compressFile(FILE* fp1, FILE* fp2, code* front);
void freeCodeList(code* front);
void freeHuffmanTree(Node* root);
void rebuildTree(FILE* fp, int unique_size);
int isLeafNode(Tree* node);
void decompress(FILE* fp_in, FILE* fp_out, int total_chars);
void freeDecompressionTree(Tree* root);

#endif
