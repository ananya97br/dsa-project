#include "huffman.h"
#include <stdlib.h>

// Define external variable
Tree* tree = NULL;

//creating a node for the huffman tree
//character and frequency
Node* newNode(char character, int freq) {
    Node* temp = malloc(sizeof(Node));
    temp->character = character;
    temp->freq = freq;
    temp->l = temp->r = NULL;
    return temp;
}

//creating a decompression tree node
Tree* newTreeNode() {
    Tree* node = malloc(sizeof(Tree));
    node->g = '\0';//character
    node->len = 0;//code length
    node->dec = 0;//decimal rep of binary code
    node->f = NULL;//left pointer
    node->r = NULL;//right pointer
    return node;
}
//swapping two node used in  heap operations
void swapNode(Node** a, Node** b) {
    Node* t = *a;
    *a = *b;
    *b = t;
}
//the new node after combining is inserted here
void Heapify(Min_Heap* heap, int i) {
    int smallest = i;
    int left = 2*i + 1;
    int right = 2*i + 2;

    if (left < heap->size && heap->array[left]->freq < heap->array[smallest]->freq)
        smallest = left;
    if (right < heap->size && heap->array[right]->freq < heap->array[smallest]->freq)
        smallest = right;

    if (smallest != i) {
        swapNode(&heap->array[i], &heap->array[smallest]);
        Heapify(heap, smallest);
    }
}
//all the nodes combine and finally the root will be left
int isSizeOne(Min_Heap* heap) {
    return heap->size == 1;
}
//extract the minimum value
Node* extractMinFromMin_Heap(Min_Heap* heap) {
    Node* temp = heap->array[0];
    heap->array[0] = heap->array[--heap->size];
    Heapify(heap, 0);
    return temp;
}
//this is to put in the newly combined nodes
void insertIntoMin_Heap(Min_Heap* heap, Node* node) {
    ++heap->size;
    int i = heap->size - 1;
    while (i && node->freq < heap->array[(i - 1) / 2]->freq) {
        heap->array[i] = heap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    heap->array[i] = node;
}
//represent the actual characters while internal nodes and just adding them up
int isLeaf(Node* node)
{
    return !(node->l) && !(node->r);
}

int convertBinaryToDecimal(int arr[], int len) {
    int dec = 0;
    for (int i = 0; i < len; i++) {
        dec = dec * 2 + arr[i];
    }
    return dec;
}

void convertDecimalToBinary(int *bin, int dec, int len) {
    for (int i = len - 1; i >= 0; i--) {
        bin[i] = dec % 2;
        dec /= 2;
    }
}
//creating the min heap
Min_Heap* createAndBuildMin_Heap(char arr[], int freq[], int size) {
    Min_Heap* heap = (Min_Heap*)malloc(sizeof(Min_Heap));
    heap->size = size;//no. of characters
    heap->capacity = size;//max number of nodes
    heap->array = (Node**)malloc(size * sizeof(Node*));

    for (int i = 0; i < size; i++) {
        heap->array[i] = newNode(arr[i], freq[i]);
    }

    for (int i = (size - 1) / 2; i >= 0; i--) {
        Heapify(heap, i);
    }

    return heap;
}

Node* buildHuffmanTree(char arr[], int freq[], int size) {
    Min_Heap* heap = createAndBuildMin_Heap(arr, freq, size);
    while (!isSizeOne(heap))//keep looping until one node remains 
    {
        Node* l = extractMinFromMin_Heap(heap);
        Node* r = extractMinFromMin_Heap(heap);
        Node* top = newNode('$', l->freq + r->freq);
        top->l = l;
        top->r = r;
        insertIntoMin_Heap(heap, top);
    }
    Node* root = extractMinFromMin_Heap(heap);
    free(heap->array);
    free(heap);
    return root;
}

void printCodesIntoFile(FILE* fp2, Node* root, int t[], int top, code** front, code** rear) {
    if (root->l) {
        t[top] = 0;
        printCodesIntoFile(fp2, root->l, t, top + 1, front, rear);
    }
    if (root->r) {
        t[top] = 1;
        printCodesIntoFile(fp2, root->r, t, top + 1, front, rear);
    }
    if (isLeaf(root)) {
        code* data = (code*)malloc(sizeof(code));
        data->p = NULL;
        data->k = root->character;
        
        // Dynamically allocate code array
        data->code_arr = (int*)malloc(top * sizeof(int));
        
        // Write character
        fwrite(&root->character, sizeof(char), 1, fp2);
        
        for (int i = 0; i < top; i++) {
            data->code_arr[i] = t[i];
        }
        
        // Write length
        fwrite(&top, sizeof(int), 1, fp2);
        
        // Calculate and write decimal representation
        int dec = convertBinaryToDecimal(t, top);
        fwrite(&dec, sizeof(int), 1, fp2);
        
        data->l = top;
//linked list for the codes so tht for compression we can just look for the codes
        if (*front == NULL) {
            *front = *rear = data;
        } else {
            (*rear)->p = data;
            *rear = data;
        }
    }
}

void compressFile(FILE* fp1, FILE* fp2, code* front) {
    char n;
    int h = 0;
    unsigned char a = 0;

    while (fread(&n, sizeof(char), 1, fp1) != 0) {
        code* temp = front;
        while (temp && temp->k != n) {
            temp = temp->p;
        }
        if (temp) {
            for (int i = 0; i < temp->l; i++) {
                a = (a << 1) | temp->code_arr[i];
                h++;
                if (h == 8) {
                    fwrite(&a, sizeof(char), 1, fp2);
                    a = 0;
                    h = 0;
                }
            }
        }
    }

    if (h > 0) {
        a <<= (8 - h);
        fwrite(&a, sizeof(char), 1, fp2);
    }
}

void freeCodeList(code* front) {
    while (front) {
        code* temp = front;
        front = front->p;
        free(temp->code_arr);
        free(temp);
    }
}

void freeHuffmanTree(Node* root) {
    if (root) {
        freeHuffmanTree(root->l);
        freeHuffmanTree(root->r);
        free(root);
    }
}

void rebuildTree(FILE* fp, int unique_size) {
    tree = newTreeNode();

    for (int i = 0; i < unique_size; i++) {
        char g;
        int len, dec;
        
        fread(&g, sizeof(char), 1, fp);
        fread(&len, sizeof(int), 1, fp);
        fread(&dec, sizeof(int), 1, fp);

        // Dynamically allocate binary array
        int *bin = (int*)calloc(len, sizeof(int));
        convertDecimalToBinary(bin, dec, len);

        Tree* temp = tree;
        for (int j = 0; j < len; j++) {
            if (bin[j] == 0) {
                if (!temp->f) {
                    temp->f = newTreeNode();
                }
                temp = temp->f;
            } else {
                if (!temp->r) {
                    temp->r = newTreeNode();
                }
                temp = temp->r;
            }
        }

        temp->g = g;
        free(bin);
    }
}

int isLeafNode(Tree* node) {
    return node && node->f == NULL && node->r == NULL;
}

void decompress(FILE* fp_in, FILE* fp_out, int total_chars) {
    unsigned char byte;
    int count = 0;
    Tree* temp = tree;

    while (fread(&byte, sizeof(char), 1, fp_in) > 0 && count < total_chars) {
        for (int i = 7; i >= 0 && count < total_chars; i--) {
            int bit = (byte >> i) & 1;
            
            if (bit == 0) {
                temp = temp->f;
            } else {
                temp = temp->r;
            }

            if (isLeafNode(temp)) {
                fwrite(&temp->g, sizeof(char), 1, fp_out);
                temp = tree;
                count++;
            }
        }
    }
}

void freeDecompressionTree(Tree* root) {
    if (root) {
        freeDecompressionTree(root->f);
        freeDecompressionTree(root->r);
        free(root);
    }
}

