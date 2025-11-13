#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include "huffman.h"

int main() {
    FILE* fp1 = fopen("sample.txt", "rb");
    if (fp1 == NULL) {
        printf("Open Failed For Input File\n");
        exit(1);
    }

    FILE* fp2 = fopen("compressed.txt", "wb");
    if (fp2 == NULL) {
        printf("Open Failed For Output File\n");
        fclose(fp1);
        exit(1);
    }

    clock_t start = clock();

    // Step 1: Count character frequencies
    int *freq = (int*)calloc(256, sizeof(int));
    char ch;
    int total_chars = 0;
    while (fread(&ch, sizeof(char), 1, fp1) > 0) {
        freq[(unsigned char)ch]++;
        total_chars++;
    }

    // Step 2: Build character and frequency arrays
    char *arr = (char*)malloc(256 * sizeof(char));
    int *freq_arr = (int*)malloc(256 * sizeof(int));
    int unique_size = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            arr[unique_size] = (char)i;
            freq_arr[unique_size] = freq[i];
            unique_size++;
        }
    }
    free(freq);

    // Step 3: Build Huffman Tree
    Node* root = buildHuffmanTree(arr, freq_arr, unique_size);

    // Step 4: Write metadata (Huffman codes) to output file
    int *t = (int*)malloc(MAX * sizeof(int));
    code *front = NULL, *rear = NULL;
    printCodesIntoFile(fp2, root, t, 0, &front, &rear);
    free(t);

    // IMPORTANT: Flush the metadata to disk
    fflush(fp2);

    // Step 5: Reset input file pointer to beginning
    fseek(fp1, 0, SEEK_SET);

    // Step 6: Compress the file (writes compressed data after metadata)
    compressFile(fp1, fp2, front);

    // IMPORTANT: Flush compressed data to disk
    fflush(fp2);

    // Close files before checking size
    fclose(fp1);
    fclose(fp2);

    // End timing
    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    // Compression ratio
    struct stat input_stat, output_stat;
    stat("sample.txt", &input_stat);
    stat("compressed.txt", &output_stat);

    double ratio = (double)output_stat.st_size / input_stat.st_size;

    // Display results
    printf("\nCompression complete.\n");
    printf("Original size     : %ld bytes\n", input_stat.st_size);
    printf("Compressed size   : %ld bytes\n", output_stat.st_size);
    printf("Compression ratio : %.2f%%\n", ratio * 100);
    printf("Space saved       : %.2f%%\n", (1 - ratio) * 100);
    printf("Time taken        : %.4f seconds\n", time_taken);

    // Step 7: Decompression
    fp1 = fopen("compressed.txt", "rb");
    if (fp1 == NULL) {
        printf("Open Failed For Compressed File\n");
        freeCodeList(front);
        freeHuffmanTree(root);
        free(arr);
        free(freq_arr);
        exit(1);
    }

    FILE* fp3 = fopen("decompressed.txt", "wb");
    if (fp3 == NULL) {
        printf("Open Failed For Decompressed File\n");
        fclose(fp1);
        freeCodeList(front);
        freeHuffmanTree(root);
        free(arr);
        free(freq_arr);
        exit(1);
    }

    // Step 8: Rebuild Huffman Tree from compressed file metadata
    rebuildTree(fp1, unique_size);

    // Step 9: Move file pointer past metadata to start of compressed data
    fseek(fp1, unique_size * (sizeof(char) + sizeof(int) + sizeof(int)), SEEK_SET);

    // Step 10: Decompress
    decompress(fp1, fp3, total_chars);

    printf("Decompression complete. Output written to decompressed.txt\n");

    // Cleanup
    freeCodeList(front);
    freeHuffmanTree(root);
    free(arr);
    free(freq_arr);
    freeDecompressionTree(tree);

    fclose(fp1);
    fclose(fp3);

    return 0;
}
