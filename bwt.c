#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_TREE_HT 100

const int N = 2e5+1;
int f[96]; // f[i]: the frequency of character i
int before = 0; // size before compression
int after = 0;  // size after compression
char hf[100][30]; // hf[i][]: huffman code of character i

/* STEP 1: BURROWS-WHEELER TRANSFORM  */
// Structure to store data of a rotation
struct rotation {
	int index;
	char* suffix;
};

// Compares the rotations and sorts the rotations alphabetically
int cmpfunc(const void* x, const void* y){
	struct rotation* rx = (struct rotation*)x;
	struct rotation* ry = (struct rotation*)y;
	return strcmp(rx->suffix, ry->suffix);
}

// The returned result is stored in the argument array
int* computeSuffixArray(char* text,int* sa ,int len_text, struct rotation* suff){
	// Structure is needed to maintain old indexes of
	// rotations after sorting them
	int i;
	for (i = 0; i != len_text; i++){
		suff[i].index = i;
		suff[i].suffix = (text + i);
	}

	// Sorts rotations using comparison function defined above
	qsort(suff, len_text, sizeof(struct rotation), cmpfunc);

	// Stores the indexes of sorted rotations
	for (i = 0; i != len_text; i++)
		sa[i] = suff[i].index;

	// Returns the computed suffix array
	return sa;
}

// Takes a array to store result, suffix array and its size
// as arguments and returns the
// Burrows - Wheeler Transform of given text
char* findLastChar(char* text, char* bwt_arr, int* sa, int n) {
	int i, j;
	for (i = 0; i != n; i++) {
		// Computes the last char which is given by
		// text[(sa[i] + n - 1) % n]
		j = sa[i] - 1;
		if(j < 0) j = j + n;

		bwt_arr[i] = text[j];
	}
	bwt_arr[i] = '\0';

	// Returns the computed Burrows - Wheeler Transform
	return bwt_arr;
}

/* STEP 2: MOVE TO FRONT */
// Returns index at which character of the input text exists in the list
int search(char input_char, char* list){
	int i;
	int len = strlen(list);
	for(i = 0; i != len; i++)
		if(list[i] == input_char) return i;

}

// Takes curr_index of input_char as argument
// to bring that character to the front of the list
void moveToFront(int curr_index, char* list){
    char c = list[curr_index];
    int i;
    // update characters list
    for(i = curr_index; i != 0; i--)
        list[i] = list[i-1];

    // Character at curr_index stored at 0th position
    list[0] = c;
}

// Move to Front Encoding
void mtfEncode(FILE* fptr, char* input_text, int len_text, char* list){
	int i, k;
	for(i = 0; i != len_text; i++){
		// Linear Searches the characters of input_text in list
		k = search(input_text[i], list);
		before++;
        f[k] += 1;
        fprintf(fptr ,"%d ", k);
		// Moves the searched character to the front of the list
		moveToFront(k, list);
	}
}

/* STEP 3: HUFFMAN ENCODING */

// A Huffman tree node
struct MinHeapNode {
	int data; // One of the input characters
    unsigned freq; // Frequency of the character
	struct MinHeapNode *left, *right; // Left and right child of this node
};

// A Min Heap: Collection of
// min-heap (or Huffman tree) nodes
struct MinHeap {
    unsigned size; // Current size of min heap
    unsigned capacity; // capacity of min heap
    struct MinHeapNode** array;// Array of min-heap node pointers
};

// A utility function allocate a new min heap node with given character
// and frequency of the character
struct MinHeapNode* newNode(int data, unsigned freq){
	struct MinHeapNode* temp = (struct MinHeapNode*)malloc(sizeof(struct MinHeapNode));
	temp->left = temp->right = NULL;
	temp->data = data;
	temp->freq = freq;

	return temp;
}

// A utility function to create a min heap of given capacity
struct MinHeap* createMinHeap(unsigned capacity){
	struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap));
	// current size is 0
	minHeap->size = 0;
	minHeap->capacity = capacity;
	minHeap->array = (struct MinHeapNode**)malloc(minHeap->capacity*sizeof(struct MinHeapNode*));

	return minHeap;
}

// A utility function to swap two min heap nodes
void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b){
	struct MinHeapNode* t = *a;
	*a = *b;
	*b = t;
}

// The standard min-Heapify function.
void minHeapify(struct MinHeap* minHeap, int idx){
	int smallest = idx;
	int left = 2 * idx + 1;
	int right = 2 * idx + 2;
	if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;

	if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;

	if (smallest != idx){
		swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
	}
}

// A utility function to check if size of heap is 1 or not
int isSizeOne(struct MinHeap* minHeap){

	return (minHeap->size == 1);
}

// A standard function to extract minimum value node from heap
struct MinHeapNode* extractMin(struct MinHeap* minHeap){
	struct MinHeapNode* temp = minHeap->array[0];
	minHeap->array[0] = minHeap->array[minHeap->size - 1];

	--minHeap->size;
	minHeapify(minHeap, 0);

	return temp;
}

// A utility function to insert a new node to Min Heap
void insertMinHeap(struct MinHeap* minHeap, struct MinHeapNode* minHeapNode){
	++minHeap->size;
	int i = minHeap->size - 1;

	while (i && minHeapNode->freq < minHeap->array[(i - 1)/2]->freq){
		minHeap->array[i] = minHeap->array[(i - 1) / 2];
		i = (i - 1) / 2;
	}

	minHeap->array[i] = minHeapNode;
}

// A standard function to build min heap
void buildMinHeap(struct MinHeap* minHeap){
	int n = minHeap->size - 1;
	int i;
	for (i = (n - 1) / 2; i >= 0; i--)
		minHeapify(minHeap, i);
}

// A utility function to compute huffman code of the character data
void computeHFCode(int data,int arr[], int n){
	int i;
	for (i = 0; i != n; i++)
        hf[data][i] = arr[i]+48;
}

// The utility function to check if this node is leaf
int isLeaf(struct MinHeapNode* root){

	return !(root->left) && !(root->right);
}

// Creates a min heap of capacity equal to size and inserts all character of data[] in min heap.
// Initially size of min heap is equal to capacity
struct MinHeap* createAndBuildMinHeap(int data[], int freq[], int size){
    struct MinHeap* minHeap = createMinHeap(size);
    int i;
    int j = 0;
	for (i = 0; i != size; ++i){
		 if(freq[i] != 0) {
            minHeap->array[j] = newNode(data[i], freq[i]);
            j++;
		 }
	}

	minHeap->size = j;
	buildMinHeap(minHeap);

	return minHeap;
}

// The main function that builds Huffman tree
struct MinHeapNode* buildHuffmanTree(int data[], int freq[], int size){
	struct MinHeapNode *left, *right, *top;

	// Step 1: Create a min heap of capacity equal to size. Initially, there are modes equal to size.
	struct MinHeap* minHeap = createAndBuildMinHeap(data, freq, size);

	// Iterate while size of heap doesn't become 1
	while (!isSizeOne(minHeap)) {
		// Step 2: Extract the two minimum freq items from min heap
		left = extractMin(minHeap);
		right = extractMin(minHeap);

		// Step 3: Create a new internal node with frequency equal to the sum of the two nodes frequencies.
		// Make the two extracted node as left and right children of this new node.
        // Add this node to the min heap
		// '$' is a special value for internal nodes, not used
		top = newNode('$', left->freq + right->freq);

		top->left = left;
		top->right = right;

		insertMinHeap(minHeap, top);
	}

	// Step 4: The remaining node is the root node and the tree is complete.
	return extractMin(minHeap);
}

// Compute huffman codes from the root of Huffman Tree.
// It uses arr[] to store codes
void computeData(struct MinHeapNode* root, int arr[], int top){
	// Assign 0 to left edge and recur
	if (root->left) {
		arr[top] = 0;
		computeData(root->left, arr, top + 1);
	}

	// Assign 1 to right edge and recur
	if (root->right) {
		arr[top] = 1;
		computeData(root->right, arr, top + 1);
	}

	// If this is a leaf node, then it contains one of the input characters,
	// compute the character and its code from arr[]
	if (isLeaf(root))
		computeHFCode(root->data ,arr, top);
}

// The main function that builds a
// Huffman Tree and print codes by traversing the built Huffman Tree
void HuffmanCodes(int data[], int freq[], int size){
	// Construct Huffman Tree
	struct MinHeapNode* root = buildHuffmanTree(data, freq, size);

	// Compute Huffman codes using the Huffman tree built above
	int arr[MAX_TREE_HT], top = 0;

	computeData(root, arr, top);
}

// Compress data by huffman codes
void Encoding(FILE* input, FILE* output){
    int num; // store the value is read in file MTF
    unsigned char* encode = (unsigned char*) malloc(16*sizeof(unsigned char));
    int cur = 0;
    unsigned char c = 0;
    int i, j;
    while(fscanf(input,"%d", &num) != EOF){
        i = 0;
        while(hf[num][i] != '\0') {
            encode[cur] = hf[num][i];
            i++; cur++;
        }
        // if enough 8 bits then print it in file output
        if(cur >= 8){
            for(j = 0; j != 8; j++){
                if(encode[j] - 48 != 0)
                    c += 1 << 7-j;
                if(j+8 < cur) encode[j] = encode[j+8];
            }
            cur -= 8;
            fprintf(output, "%c", c);
            after++;
            c = 0;
        }
    }
    // print the rest bits in file output
    if(cur > 0){
        for(j = 0; j < cur; j++){
            c += (encode[j]-48) << 7-j;
        }
        fprintf(output, "%c", c);
        after++;
    }
    free(encode);
}

int main(){
    //  file input
    char* source = "1MB.txt";
    //  file to store the result of step 2: move to front
    char* mtfFile = "tmp.txt";
    //  file output
    char* destination = "output.txt";
    FILE* fi = fopen(source, "r");
    FILE* f_tmp = fopen(mtfFile, "w");
    FILE* fo = fopen(destination, "w");

    if(fi != NULL){
        /* Prepare data structure */

        // Store each block of the input file
        // maximum is ~200Kb
        char* text = (char*) malloc(N*sizeof(char));

        // Store the computed Burrows - Wheeler Transform
        char* bwt_arr = (char*) malloc((N+1)*sizeof(char));

        // Store the computed suffix array
        int* sa = (int*) malloc(N*sizeof(int));

        // Store suffix string and it index
        struct rotation* suff = (struct rotation*) malloc(N*sizeof(struct rotation));

        // Length of input block-text
        int len;

        // Maintains an ordered list of legal symbols include 95 characters can print on screen
        char* list = (char*) malloc(96*sizeof(char));
        int i;
        for(i = 0; i != 95; i++){
            list[i] = 32+i;
            f[i] = 0;
        }
        list[i] = '\0';

        /* STEP 1+2: BWT and MTF */
        printf(" ******************************************************\n");
        printf(" **                                                  **\n");
        printf(" ** USING BURROWS-WHEELER TRANSFORM TO COMPRESS DATA **\n");
        printf(" **                                                  **\n");
        printf(" ******************************************************\n\n");
        printf(" Step 1 and 2 are processing...\n------------------------------------------\n");

        // Get the N-length text from input file
        // then compute BWT and store it in output file
        // Repeat until there are no more characters left in input file
        while(fgets(text, N, fi) != NULL){
            len = strlen(text);
            sa = computeSuffixArray(text, sa, len, suff);
            bwt_arr = findLastChar(text, bwt_arr, sa, len);
            mtfEncode(f_tmp, bwt_arr, len, list);
        }
        fclose(f_tmp);
        printf(" Step 1 and 2 are done!\n------------------------------------------\n");
        printf(" Step 3 is processing...\n------------------------------------------\n");

        /* STEP 3: HUFFMAN ENCODING */
        FILE *fi_tmp = fopen(mtfFile, "r");
        int arr[95];
        for(i = 0; i < 95; i++) arr[i] = i;
        int j;
        // init array to store huffman codes
        for(i = 0; i < 100; i++){
            for(j = 0; j < 30; j++){
            hf[i][j] = '\0';
            }
        }

        HuffmanCodes(arr, f, 95);

        Encoding(fi_tmp, fo);
        fclose(fi_tmp);
        printf(" Step 3 is done!\n------------------------------------------\n");
        printf(" Size before compression: %d\n Size after compression: %d\n", before, after);

        // compute the compression ratio
        double cr = (1 - 1.0*after/before)*100;
        printf(" Compression ratio = %.2lf%%\n", cr);

        // Release memory
        free(suff);
        free(sa);
        free(bwt_arr);
        free(text);
        free(list);
    }
    else printf(" FILE CAN NOT OPEN!");

    // Close the input and output files
    fclose(fi);
    fclose(fo);

	return 0;
}


