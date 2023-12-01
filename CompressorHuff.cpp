#include <iostream>
#include <queue>
#include <chrono> // time
#include <bitset>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>

using namespace std;


// Node of the Huffman tree:
struct Node {
	char     data;
	unsigned freq;

	Node* left, * right;

	Node(char data, unsigned freq) : data(data), freq(freq),
		left(nullptr), right(nullptr) {}
};

// Used to compare two nodes in the priority queue:
struct CompareNodes {
	bool operator()(Node* const& node1, Node* const& node2) {
		return node1->freq > node2->freq;
	}
};

// Returns a map with the frequency of each character in the string:
unordered_map<char, unsigned> getCharFrequences(const string& text) {
	unordered_map<char, unsigned> frequences;

	// Count the frequency of each character:
	for (char c : text) {
		frequences[c]++;  // If the character is not in the map,
		// it will be added with the value 0
		// and then incremented.
	}

	return frequences;
}

priority_queue<Node*, vector<Node*>, CompareNodes> constructQueue(const string& text) {
	priority_queue<Node*, vector<Node*>, CompareNodes> queue;

	const auto charFrequences = getCharFrequences(text);

	for (const auto& pair : charFrequences) {
		queue.push(new Node(pair.first, pair.second));
	}

	return queue;
}

Node* constructTree(const string& text) {
	priority_queue<Node*, vector<Node*>, CompareNodes> queue = constructQueue(text);

	while (queue.size() > 1) {
		// Get the two nodes with
		// the lowest frequency:
		Node* left = queue.top();
		queue.pop();

		Node* right = queue.top();
		queue.pop();

		// Create a new internal node that
		// will be the parent of the two nodes:
		Node* internal = new Node('#', left->freq + right->freq);
		internal->left = left;
		internal->right = right;

		queue.push(internal); // Add the new node to the queue.
	}

	return queue.top(); // The tree is the remaining node in the queue.
}

Node* constructTree(const unordered_map<char, unsigned>& frequences) {
	priority_queue<Node*, vector<Node*>, CompareNodes> queue;

	// Add all the nodes to the queue:
	for (const auto& pair : frequences) {
		queue.push(new Node(pair.first, pair.second));
	}

	while (queue.size() > 1) {
		// Get the two nodes with
		// the lowest frequency:
		Node* left = queue.top();
		queue.pop();

		Node* right = queue.top();
		queue.pop();

		// Create a new internal node that
		// will be the parent of the two nodes:
		Node* internal = new Node('#', left->freq + right->freq);
		internal->left = left;
		internal->right = right;

		queue.push(internal); // Add the new node to the queue.
	}

	return queue.top(); // The tree is the remaining node in the queue.
}

void printHuffmanCodes(const unordered_map<char, string>& codes) {
	for (const auto& pair : codes) {
		cout << pair.first << ": " << pair.second << endl; // Ex: a: 01001
	}
}

void printTreeStructure(Node* root, int indent = 0) {
	if (root == nullptr) {
		return;
	}

	const int spacesPerLevel = 4;

	if (root->right != nullptr) {
		printTreeStructure(root->right, indent + spacesPerLevel);
	}

	cout << string(indent, ' ');
	if (root->data == '#') {
		cout << "| " << root->freq << " |" << endl;
	}
	else {
		cout << "| " << root->data << ": " << root->freq << " |" << endl;
	}

	if (root->left != nullptr) {
		printTreeStructure(root->left, indent + spacesPerLevel);
	}
}

// Recursive function to generate the Huffman codes:
void generateHuffmanCodes(Node* root, string& code, unordered_map<char, string>& codes) {
	if (!root->left && !root->right) {
		codes[root->data] = code;
		return;
	}

	string prev_code = code;

	if (root->left) {
		code += "0"; // "Stream" goes to the left, so adds a 0 to the code. Ex: 01001 -> 010010 .
		generateHuffmanCodes(root->left, code, codes);
		code = prev_code; // Remove the last character from the code.
	}

	if (root->right) {
		code += "1"; // "Stream" goes to the right, so adds a 1 to the code. Ex: 01001 -> 010011 .
		generateHuffmanCodes(root->right, code, codes);
		code = prev_code; // Remove the last character from the code.
	}
}

// Returns a map with the Huffman codes of each character:
unordered_map<char, string> generateHuffmanCodes(Node* root) {
	string code;
	unordered_map<char, string> codes;

	if (root)
		generateHuffmanCodes(root, code, codes); // Recursive function

	return codes;
}

// Turns the text into it's corresponding Huffman codes:
string getBitsString(const string& text, const unordered_map<char, string>& codes) {
	string bitsString;

	for (char c : text) {
		bitsString += codes.at(c);
	}

	return bitsString;
}

// Waits for the bits to form a whole byte and then writes it to the file:
void writeBitToFile(ofstream& file, char bit, char& currentByte, int& bitCount) {
	if (bit != '0' && bit != '1') {
		cerr << "Invalid bit: " << bit << endl;
		return;
	}

	currentByte = (currentByte << 1) | (bit - '0'); // Shift left by 1 bit to make
													// room for the new bit and add
													// the new bit to the buffer.

	bitCount++; // Increment the number of bits in the buffer.

	if (bitCount == 8) {
		file.write(&currentByte, 1); // Write the byte to the file.
		currentByte = 0;
		bitCount = 0;
	}
}

// Writes the bits to the file by calling writeBitToFile() for each bit:
void writeBitsToFile(ofstream& file, const string& bits) {
	if (!file.is_open()) {
		cerr << "Error opening file" << endl;
		return;
	}

	char currentByte = 0; // Current byte being filled with bits.
	int  bitCount    = 0; // Number of bits in the current byte.

	for (char bit : bits) {
		writeBitToFile(file, bit, currentByte, bitCount);
	}

	// If there are remaining bits in the buffer, write them to the file:
	if (bitCount > 0) {
		currentByte <<= (8 - bitCount);
		file.write(&currentByte, 1);
	}

	int redundantBits = 8 - bitCount; // Keep track of the amount of redundant bits.

	// Write the amount of redundant bits to the first line of the file
	file.seekp(0, ios::beg);
	file.write(reinterpret_cast<const char*>(&redundantBits), sizeof(redundantBits));
}

int readIntegerFromFile(ifstream& file) {
	int integer;

	file.read(reinterpret_cast<char*>(&integer), sizeof(integer));

	return integer;
}

unsigned readUnsignedFromFile(ifstream& file) {
	unsigned integer;

	file.read(reinterpret_cast<char*>(&integer), sizeof(integer));

	return integer;
}

char readCharFromFile(ifstream& file) {
	char c;

	file.read(&c, 1);

	return c;
}

unordered_map<char, unsigned> readFrequencesFromFile(ifstream& file, unsigned freqCount)
{
	unordered_map<char, unsigned> frequences;
	string line;

	if (!file.is_open())
	{
		cerr << "Error opening file" << endl;
		return {};
	}

	// Reads as many pairs of char and
	// unsigned as the freqCount variable:
	for (unsigned i = 0; i < freqCount; i++)
	{
		char     c    = readCharFromFile(file);
		unsigned freq = readUnsignedFromFile(file);

		frequences[c] = freq; // Add the pair to the map.
	}

	return frequences;
}

bool writeFrequencesToFile(ofstream& file, const unordered_map<char, unsigned>& frequences) {
	if (!file.is_open()) {
		cerr << "Error opening file" << endl;
		return false;
	}

	int amoutOfFrequencePairs = frequences.size();

	// The first write will be replaced by the amount of redundant bits:
	file.write(reinterpret_cast<const char*>(&amoutOfFrequencePairs), sizeof(amoutOfFrequencePairs));
	file.write(reinterpret_cast<const char*>(&amoutOfFrequencePairs), sizeof(amoutOfFrequencePairs));

	for (const auto& pair : frequences) {
		file.write(&pair.first, 1); // Write the char to the file.

		// Write the unsigned to the file:
		file.write(reinterpret_cast
				   <const char*>(&pair.second),
				   sizeof(pair.second));
	}

	return true;
}

string readBitsFromFile(ifstream& file) {
	if (!file.is_open()) {
		cerr << "Error opening file" << endl;
		return "";
	}

	string bitsString;
	char byte;

	// Read the file byte by byte and
	// convert each byte to a string of bits:
	while (file.get(byte)) {
		bitsString += bitset<8>(byte).to_string();
	}

	return bitsString;
}

// Converts the bits to text using the Huffman tree:
string getTextFromBits(const string& bitsString, Node* root) {
	string text;

	Node* currentNode = root; // Places the current node at the root.

	// Goes through the string of bits and
	// traverses the tree accordingly:
	for (size_t i = 0; bitsString[i] != '\0'; i++)
	{
		if (bitsString[i] == '0') // If the bit is 0, go left.
		{
			if (!(currentNode->left->right && currentNode->left->left))
			{
				text += currentNode->left->data;
				currentNode = root;
			}
			else
				currentNode = currentNode->left;
		}
		else if (bitsString[i] == '1') // If the bit is 1, go right.
		{
			if (!(currentNode->right->right && currentNode->right->left))
			{
				text += currentNode->right->data;
				currentNode = root;
			}
			else
				currentNode = currentNode->right;
		}
	}

	return text;
}

void compressFile(const string& inputFilename, const string& outputFilename)
{
	ifstream inputFile(inputFilename);

	if (!inputFile.is_open())
	{
		cerr << "Error opening file: " << inputFilename << endl;
		return;
	}

	string text;

	// Read the file:
	char c;
	while (inputFile.get(c))
	{
		text += c;
	}

	inputFile.close();

	// Write compressed file:

	ofstream outputFile(outputFilename, ios::binary);

	if (!outputFile.is_open())
	{
		cerr << "Error opening file: " << outputFilename << endl;
		return;
	}

	Node* tree = constructTree(text);

	unordered_map<char, string> codes = generateHuffmanCodes(tree);

	string bitsString = getBitsString(text, codes);

	if (!writeFrequencesToFile(outputFile, getCharFrequences(text)))
	{
		cerr << "Error writing frequences to file" << endl;
		return;
	}

	writeBitsToFile(outputFile, bitsString);

	outputFile.close();
}

void decompressFile(const string& inputFilename, const string& outputFilename)
{
	ifstream inputFile(inputFilename, ios::binary);

	if (!inputFile.is_open())
	{
		cerr << "Error opening file: " << inputFilename << endl;
		return;
	}

	int amountOfRedundantBits = readIntegerFromFile(inputFile);

	int amountOfFrequecePairs = readIntegerFromFile(inputFile);

	unordered_map<char, unsigned> frequences = readFrequencesFromFile(inputFile, amountOfFrequecePairs);

	Node* tree = constructTree(frequences);

	string bitsString = readBitsFromFile(inputFile);

	// Remove the redundant bits from the end of the string:
	bitsString = bitsString.substr(0, bitsString.size() - amountOfRedundantBits);

	string text = getTextFromBits(bitsString, tree);

	ofstream outputFile(outputFilename);

	if (!outputFile.is_open())
	{
		cerr << "Error opening file: " << outputFilename << endl;
		return;
	}

	outputFile << text;

	outputFile.close();
}

void compressFile(const string& inputFilename) {
	size_t lastDotPos = inputFilename.find_last_of('.');
	string baseName   = inputFilename.substr(0, lastDotPos);

	string currentExtension = inputFilename.substr(lastDotPos + 1);

	if (currentExtension != "txt") {
		cerr << "Invalid file extension: " << currentExtension << endl;
		return;
	}

	string outputFilename = baseName + ".graba";

	compressFile(inputFilename, outputFilename);
}

void decompressFile(const string& inputFilename) {
	size_t lastDotPos = inputFilename.find_last_of('.');
	string baseName   = inputFilename.substr(0, lastDotPos);

	string currentExtension = inputFilename.substr(lastDotPos + 1);

	if (currentExtension != "graba") {
		cerr << "Invalid file extension: " << currentExtension << endl;
		return;
	}

	string outputFilename = baseName + "_decompressed.txt";

	decompressFile(inputFilename, outputFilename);
}

int main()
{
	auto start = chrono::high_resolution_clock::now();

	//compressFile("C:\\Users\\grarb\\Documentos\\GitHub\\Huffman-Compressor\\Testes\\input.txt");
	decompressFile("C:\\Users\\grarb\\Documentos\\GitHub\\Huffman-Compressor\\Testes\\input.graba");

	auto end = chrono::high_resolution_clock::now();

	auto duration = chrono::duration_cast<chrono::duration<double>>(end - start);

	cout << "Time taken by function: " << duration.count() << " seconds" << endl;

	return 0;
}