#include <iostream>
#include <queue>
#include <chrono> // time
#include <bitset>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>

using namespace std;

struct Node {
	char data;
	unsigned freq;
	Node* left, * right;

	Node(char data, unsigned freq) : data(data), freq(freq), left(nullptr), right(nullptr) {}
};

struct CompareNodes {
	bool operator()(Node* const& node1, Node* const& node2) {
		return node1->freq > node2->freq;
	}
};

unordered_map<char, unsigned> getCharFreq(const string& str) {
	unordered_map<char, unsigned> freq;

	for (char c : str) {
		freq[c]++;
	}

	return freq;
}

priority_queue<Node*, vector<Node*>, CompareNodes> constructQueue(const string& text) {
	priority_queue<Node*, vector<Node*>, CompareNodes> queue;

	unordered_map<char, unsigned> charFreq = getCharFreq(text);

	for (const auto& pair : charFreq) {
		queue.push(new Node(pair.first, pair.second));
	}

	return queue;
}

Node* constructTree(const string& text) {
	priority_queue<Node*, vector<Node*>, CompareNodes> queue = constructQueue(text);

	while (queue.size() > 1) {
		Node* left = queue.top();
		queue.pop();

		Node* right = queue.top();
		queue.pop();

		Node* internal = new Node('#', left->freq + right->freq);
		internal->left = left;
		internal->right = right;

		queue.push(internal);
	}

	return queue.top();
}

Node* constructTree(const unordered_map<char, unsigned>& frequences) {
	priority_queue<Node*, vector<Node*>, CompareNodes> queue;

	for (const auto& pair : frequences) {
		queue.push(new Node(pair.first, pair.second));
	}

	while (queue.size() > 1) {
		Node* left = queue.top();
		queue.pop();

		Node* right = queue.top();
		queue.pop();

		Node* internal = new Node('#', left->freq + right->freq);
		internal->left = left;
		internal->right = right;

		queue.push(internal);
	}

	return queue.top();
}

void printHuffmanCodes(Node* root, string code = "") {
	if (root == nullptr) {
		return;
	}

	// Leaf:
	if (!root->left && !root->right) {
		cout << root->data << ": " << code << endl;
		return;
	}

	printHuffmanCodes(root->left, code + "0");
	printHuffmanCodes(root->right, code + "1");
}

void printHuffmanCodes(const unordered_map<char, string>& codes) {
	for (const auto& pair : codes) {
		cout << pair.first << ": " << pair.second << endl;
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

void generateHuffmanCodes(Node* root, string& code, unordered_map<char, string>& codes) {
	if (root) {
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
}

unordered_map<char, string> generateHuffmanCodes(Node* root) {
	string code;
	unordered_map<char, string> codes;

	generateHuffmanCodes(root, code, codes);

	return codes;
}

string getBitsString(const string& text, const unordered_map<char, string>& codes) {
	string bitsString;

	for (char c : text) {
		bitsString += codes.at(c);
	}

	return bitsString;
}

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
		file.put(currentByte); // Write the buffer to the file.
		currentByte = 0;
		bitCount = 0;
	}
}

int writeBitsToFile(ofstream& file, const string& bits) {
	if (!file.is_open()) {
		cerr << "Error opening file" << endl;
		return -1;
	}

	char currentByte = 0; // Current byte being filled with bits
	int bitCount = 0;     // Number of bits in the current byte

	for (char bit : bits) {
		writeBitToFile(file, bit, currentByte, bitCount);
	}

	// If there are remaining bits in the buffer, write them to the file
	if (bitCount > 0) {
		currentByte <<= (8 - bitCount);
		file.put(currentByte);
	}

	file << '\n';

	return (8 - bitCount);
}

bool getline(ifstream& file, string& line) {
	char c;

	while (file.get(c)) {
		if (c == '\n') {
			return true;
		}
		else {
			line += c;
		}
	}

	return false;
}

unordered_map<char, unsigned> readFrequencesFromFile(ifstream& file) {
	unordered_map<char, unsigned> frequences;
	string line;

	if (!file.is_open()) {
		cerr << "Error opening file" << endl;
		return {};
	}

	while (getline(file, line)) {
		if (line == "") {
			break;
		}

		// Format: <char><freq>

		if (line[0] == '\\' && line[1] == 'n') { // '\n' is read as two characters ('\' and 'n')
			line[0] = '\n'; // but is read as a single character ('\n')
			line.erase(1, 1); // so we need to replace the first character with '\n' and erase the second character.
		}

		char c = line[0];
		unsigned freq = stoi(line.substr(1));

		frequences[c] = freq;
	}

	return frequences;
}

bool writeFrequencesToFile(ofstream& file, const unordered_map<char, unsigned>& frequences) {
	if (!file.is_open()) {
		cerr << "Error opening file" << endl;
		return false;
	}

	for (const auto& pair : frequences) {
		if(pair.first != '\n')
			file << pair.first << pair.second << endl;
		else
			file << "\\n" << pair.second << endl;
	}

	file << '\n';

	return true;
}

string readBitsFromFile(const string& filename) {
	ifstream file(filename, ios::binary);

	if (!file.is_open()) {
		cerr << "Error opening file: " << filename << endl;
		return "";
	}

	string bitsString;
	char byte;

	while (file.get(byte)) {
		bitsString += bitset<8>(byte).to_string();
	}

	file.close();

	return bitsString;
}

string getStringFromBits(const string& bitsString, const unordered_map<char, string>& codes) {
	string text;

	string currentCode;

	for (char bit : bitsString) {
		currentCode += bit;

		for (const auto& pair : codes) {
			if (pair.second == currentCode) {
				text += pair.first;
				currentCode.clear();
				break;
			}
		}
	}

	return text;
}

void compressFile(const string& inputFilename, const string& outputFilename) {
	ifstream inputFile(inputFilename);

	if (!inputFile.is_open()) {
		cerr << "Error opening file: " << inputFilename << endl;
		return;
	}

	string text;

	while (inputFile) {
		string line;
		getline(inputFile, line);
		text += line + '\n';
	}

	inputFile.close();

	// Write compressed file:
	ofstream outputFile(outputFilename);

	if (!outputFile.is_open()) {
		cerr << "Error opening file: " << outputFilename << endl;
		return;
	}

	Node* root = constructTree(text);

	unordered_map<char, string> codes = generateHuffmanCodes(root);

	string bitsString = getBitsString(text, codes);

	if (!writeFrequencesToFile(outputFile, getCharFreq(text))) {
		cerr << "Error writing frequences to file" << endl;
		return;
	}

	int amountOfRedundantBits = writeBitsToFile(outputFile, bitsString);

	outputFile << amountOfRedundantBits << endl;

	outputFile.close();
}

void decompressFile(const string& inputFilename, const string& outputFilename) {
	ifstream inputFile(inputFilename);

	if (!inputFile.is_open()) {
		cerr << "Error opening file: " << inputFilename << endl;
		return;
	}

	unordered_map<char, unsigned> frequences = readFrequencesFromFile(inputFile);

	Node* root = constructTree(frequences);

	string bitsString = readBitsFromFile(inputFilename);

	int amountOfRedundantBits;

	inputFile >> amountOfRedundantBits; // Read the amount of redundant bits from the file.


	// Remove the redundant bits from the end of the string:
	bitsString.erase(bitsString.size() - amountOfRedundantBits - 1, amountOfRedundantBits);

	string text = getStringFromBits(bitsString, generateHuffmanCodes(root));

	ofstream outputFile(outputFilename);

	if (!outputFile.is_open()) {
		cerr << "Error opening file: " << outputFilename << endl;
		return;
	}

	outputFile << text;

	outputFile.close();
}

int main()
{
	auto start = chrono::high_resolution_clock::now();

	//compressFile("C:\\Users\\grarb\\Documentos\\GitHub\\Huffman-Compressor\\Testes\\input.txt", "C:\\Users\\grarb\\Documentos\\GitHub\\Huffman-Compressor\\Testes\\output.txt");
	decompressFile("C:\\Users\\grarb\\Documentos\\GitHub\\Huffman-Compressor\\Testes\\output.txt", "C:\\Users\\grarb\\Documentos\\GitHub\\Huffman-Compressor\\Testes\\decompressed.txt");

	auto end = chrono::high_resolution_clock::now();

	auto duration = chrono::duration_cast<chrono::duration<double>>(end - start);

	cout << "Time taken by function: " << duration.count() << " seconds" << endl;

	return 0;
}