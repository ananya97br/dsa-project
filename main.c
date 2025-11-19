#include "huffman.h"

int main() {
    char infile[256], outfile[256], decfile[256];
    char choice;
    do {
        printf("Enter input filename: ");
        scanf("%s", infile);

        printf("Enter compressed output filename: ");
        scanf("%s", outfile);

        printf("Enter decompressed output filename: ");
        scanf("%s", decfile);

        FILE* fp1 = fopen(infile, "rb");
        if (fp1 == NULL) {
            printf("Open Failed For Input File\n");
            exit(1);
        }

        FILE* fp2 = fopen(outfile, "wb");
        if (fp2 == NULL) {
            printf("Open Failed For Output File\n");
            fclose(fp1);
            exit(1);
        }

        clock_t start = clock();

        //Count character frequencies
        int *freq = (int*)calloc(256, sizeof(int));
        char ch;
        int total_chars = 0;
        while (fread(&ch, sizeof(char), 1, fp1) > 0) {
            freq[(unsigned char)ch]++;
            total_chars++;
        }

        //Build character and frequency arrays
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

        //Build Huffman Tree
        Node* root = buildHuffmanTree(arr, freq_arr, unique_size);

        int *t = (int*)malloc(MAX * sizeof(int));
        code *front = NULL, *rear = NULL;
        printCodesIntoFile(fp2, root, t, 0, &front, &rear);
        free(t);

        fflush(fp2);

        fseek(fp1, 0, SEEK_SET);

        compressFile(fp1, fp2, front);

        fflush(fp2);

        fclose(fp1);
        fclose(fp2);

        // End timing
        clock_t end = clock();
        double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

        // Compression ratio
        struct stat input_stat, output_stat;
        stat(infile, &input_stat);
        stat(outfile, &output_stat);

        double ratio = (double)output_stat.st_size / input_stat.st_size;

        // Display results
        printf("\nCompression complete.\n");
        printf("Original size     : %ld bytes\n", input_stat.st_size);
        printf("Compressed size   : %ld bytes\n", output_stat.st_size);
        printf("Compression ratio : %.2f%%\n", ratio * 100);
        printf("Space saved       : %.2f%%\n", (1 - ratio) * 100);
        printf("Time taken        : %.4f seconds\n", time_taken);

        fp1 = fopen(outfile, "rb");
        if (fp1 == NULL) {
            printf("Open Failed For Compressed File\n");
            freeCodeList(front);
            freeHuffmanTree(root);
            free(arr);
            free(freq_arr);
            exit(1);
        }
        FILE* fp3 = fopen(decfile, "wb");
        if (fp3 == NULL) {
            printf("Open Failed For Decompressed File\n");
            fclose(fp1);
            freeCodeList(front);
            freeHuffmanTree(root);
            free(arr);
            free(freq_arr);
            exit(1);
        }

        rebuildTree(fp1, unique_size);

        fseek(fp1, unique_size * (sizeof(char) + sizeof(int) + sizeof(int)), SEEK_SET);

        decompress(fp1, fp3, total_chars);

        printf("Decompression complete. Output written to %s\n", decfile);

        // Cleanup
        freeCodeList(front);
        freeHuffmanTree(root);
        free(arr);
        free(freq_arr);
        freeDecompressionTree(tree);

        fclose(fp1);
        fclose(fp3);

        printf("\nDo you want to process another file? (Y/N): ");
        scanf(" %c", &choice);
    } while (choice == 'Y' || choice == 'y');

    return 0;
}
