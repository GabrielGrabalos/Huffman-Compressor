#include <iostream>
#include <queue>
#include <bitset>
#include <fstream>
#include <vector>
#include <map>
#include <string>

#define BYTE_SIZE 8

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
map<char, unsigned> getCharFrequences(const vector<char>& buffer) {
	map<char, unsigned> frequences;

	// Count the frequency of each character:
	for (char c : buffer) {
		frequences[c]++;  // If the character is not in the map,
		// it will be added with the value 0
		// and then incremented.
	}

	return frequences;
}

priority_queue<Node*, vector<Node*>, CompareNodes> constructQueue(const vector<char>& buffer) {
	priority_queue<Node*, vector<Node*>, CompareNodes> queue;

	const auto charFrequences = getCharFrequences(buffer);

	for (const auto& pair : charFrequences) {
		queue.push(new Node(pair.first, pair.second));
	}

	return queue;
}

Node* constructTree(const vector<char>& buffer) {
	priority_queue<Node*, vector<Node*>, CompareNodes> queue = constructQueue(buffer);

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

Node* constructTree(const map<char, unsigned>& frequences) {
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

void printHuffmanCodes(const map<char, string>& codes) {
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
void generateHuffmanCodes(Node* root, string& code, map<char, string>& codes) {
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
map<char, string> generateHuffmanCodes(Node* root) {
	string code;
	map<char, string> codes;

	if (root)
		generateHuffmanCodes(root, code, codes); // Recursive function

	return codes;
}

// Turns the text into it's corresponding Huffman codes:
string getBitsString(const vector<char>& buffer, const map<char, string>& codes) {
	string bitsString;

	for (char c : buffer) {
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

	if (bitCount == BYTE_SIZE) {
		file.write(&currentByte, 1); // Write the byte to the file.
		currentByte = 0;
		bitCount = 0;
	}
}

void writeStringToFile(const string& str, ofstream& file) {
	int strLength = str.length();

	file.write(reinterpret_cast<const char*>(&strLength), sizeof(strLength));

	file.write(str.c_str(), strLength);
}

void writeExtensionToFile(ofstream& file, string extension) {

	int integer = 14;

	// This will be replaced by the amount of redundant bits:
	file.write(reinterpret_cast<const char*>(&integer), sizeof(integer));

	writeStringToFile(extension, file);
}

// Writes the bits to the file by calling writeBitToFile() for each bit:
bool writeBitsToFile(ofstream& file, const string& bits) {
	if (!file.is_open()) {
		cerr << "Error opening file" << endl;
		return false;
	}

	char currentByte = 0; // Current byte being filled with bits.
	int  bitCount = 0; // Number of bits in the current byte.

	for (char bit : bits) {
		writeBitToFile(file, bit, currentByte, bitCount);
	}

	// If there are remaining bits in the buffer, write them to the file:
	if (bitCount > 0) {
		currentByte <<= (BYTE_SIZE - bitCount);
		file.write(&currentByte, 1);
	}

	int redundantBits = BYTE_SIZE - bitCount; // Keep track of the amount of redundant bits.

	// Write the amount of redundant bits to the first line of the file
	file.seekp(0, ios::beg);
	file.write(reinterpret_cast<const char*>(&redundantBits), sizeof(redundantBits));

	return true;
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

string readStringFromFile(int strLength, ifstream& file) {
	char* buffer = new char[strLength + 1]; // +1 for null terminator

	file.read(buffer, strLength);

	buffer[strLength] = '\0'; // Null terminate the string

	string str(buffer);

	delete[] buffer;

	return str;
}

map<char, unsigned> readFrequencesFromFile(ifstream& file, unsigned freqCount)
{
	map<char, unsigned> frequences;
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
		char     c = readCharFromFile(file);
		unsigned freq = readUnsignedFromFile(file);

		frequences[c] = freq; // Add the pair to the map.
	}

	return frequences;
}

bool writeFrequencesToFile(ofstream& file, const map<char, unsigned>& frequences) {
	if (!file.is_open()) {
		cerr << "Error opening file" << endl;
		return false;
	}

	int amoutOfFrequencePairs = frequences.size();

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

string readBinaryStringFromFile(ifstream& file) {
	if (!file.is_open()) {
		cerr << "Error opening file" << endl;
		return "";
	}

	string bitsString;
	char byte;

	// Read the file byte by byte and
	// convert each byte to a string of bits:
	while (file.get(byte)) {
		bitsString += bitset<BYTE_SIZE>(byte).to_string();
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
			if (!currentNode->left->right && !currentNode->left->left)
			{
				text += currentNode->left->data;
				currentNode = root;
			}
			else
				currentNode = currentNode->left;
		}
		else if (bitsString[i] == '1') // If the bit is 1, go right.
		{
			if (!currentNode->right->right && !currentNode->right->left)
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

int compressFile(const string& inputFilename, const string& outputFilename)
{
	ifstream inputFile(inputFilename);

	if (!inputFile.is_open())
	{
		cerr << "Error opening file: " << inputFilename << endl;
		return 1;
	}

	// Seek to the end of the file to determine its size
	inputFile.seekg(0, ios::end);
	streampos fileSize = inputFile.tellg();
	inputFile.seekg(0, ios::beg);

	// Read the file into a vector
	vector<char> buffer(fileSize);
	inputFile.read(buffer.data(), fileSize);
	inputFile.close();

	// Write compressed file:

	ofstream outputFile(outputFilename, ios::binary);

	if (!outputFile.is_open())
	{
		cerr << "Error opening file: " << outputFilename << endl;
		return 1;
	}

	Node* tree = constructTree(buffer);

	printHuffmanCodes(generateHuffmanCodes(tree));

	map<char, string> codes = generateHuffmanCodes(tree);

	string bitsString = getBitsString(buffer, codes);

	size_t lastDotPos = inputFilename.find_last_of('.');
	string extension = inputFilename.substr(lastDotPos + 1);

	writeExtensionToFile(outputFile, extension);

	if (!writeFrequencesToFile(outputFile, getCharFrequences(buffer)))
	{
		cerr << "Error writing frequences to file" << endl;
		return 1;
	}

	if (!writeBitsToFile(outputFile, bitsString)) {
		cerr << "Error writing bits to file" << endl;
		return 1;
	}

	outputFile.close();

	free(tree);

	return 0;
}

int decompressFile(const string& inputFilename, const string& outputFilename)
{
	ifstream inputFile(inputFilename, ios::binary);

	if (!inputFile.is_open())
	{
		cerr << "Error opening file: " << inputFilename << endl;
		return 1;
	}

	int amountOfRedundantBits = readIntegerFromFile(inputFile);

	int amountOfCharsInExtension = readIntegerFromFile(inputFile);

	string extension = readStringFromFile(amountOfCharsInExtension, inputFile);

	int amountOfFrequecePairs = readIntegerFromFile(inputFile);

	map<char, unsigned> frequences = readFrequencesFromFile(inputFile, amountOfFrequecePairs);

	Node* tree = constructTree(frequences);

	printHuffmanCodes(generateHuffmanCodes(tree));

	string bitsString = readBinaryStringFromFile(inputFile);

	// Remove the redundant bits from the end of the string:
	bitsString = bitsString.substr(0, bitsString.size() - amountOfRedundantBits);

	string text = getTextFromBits(bitsString, tree);

	ofstream outputFile(outputFilename + extension);

	if (!outputFile.is_open())
	{
		cerr << "Error opening file: " << outputFilename << endl;
		return 1;
	}

	outputFile << text;

	outputFile.close();

	free(tree);

	return 0;
}

int compressFile(const string& inputFilename) {
	size_t lastDotPos = inputFilename.find_last_of('.');
	string baseName = inputFilename.substr(0, lastDotPos);

	string outputFilename = baseName + ".graba";

	return compressFile(inputFilename, outputFilename);
}

int decompressFile(const string& inputFilename) {
	size_t lastDotPos = inputFilename.find_last_of('.');
	string baseName = inputFilename.substr(0, lastDotPos);

	string outputFilename = baseName + "_decompressed.";

	return decompressFile(inputFilename, outputFilename);
}

int main(int argc, char* argv[])
{
	if (argc == 2) {
		string filePath = argv[1];
		size_t lastSlash = filePath.find_last_of('\\');

		// Check if the lastSlash was found:
		if (lastSlash == string::npos) {
			lastSlash = filePath.find_last_of('/');

			// Check if the lastSlash was found:
			if (lastSlash == string::npos) {
				cerr << "Invalid file path" << endl;
				return 1;
			}
		}

		string fileName = filePath.substr(lastSlash + 1);

		size_t lastDotPos = fileName.find_last_of('.');

		// Check if the lastDotPos was found:
		if (lastDotPos == string::npos) {
			cerr << "Invalid file name" << endl;
			return 1;
		}

		string extension = fileName.substr(lastDotPos + 1);

		if (extension != "graba") {
			return compressFile(filePath);
		}

		return decompressFile(filePath);


		cerr << "Invalid file extension: " << extension << endl;
		return 1;
	}
	else {
		cerr << "Invalid number of arguments" << endl;
		return 1;
	}
}