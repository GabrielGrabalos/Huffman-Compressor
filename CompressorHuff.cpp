#include <iostream>
#include <queue>
#include <chrono> // time
#include <bitset>

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

#include <unordered_map>

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
		internal->left  = left;
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
		cout << root->data << ": " << code << std::endl;
		return;
	}

	printHuffmanCodes(root->left,  code + "0");
	printHuffmanCodes(root->right, code + "1");
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

unordered_map<char, bitset<8>> generateHuffmanCodes(Node* root) {
	unordered_map<char, bitset<8>> codes;
	bitset<8> code;

	generateHuffmanCodes(root, code, codes);

	return codes;
}

void generateHuffmanCodes(Node* root, bitset<8>& code, unordered_map<char, bitset<8>>& codes) {
	if (root) {
		if (!root->left && !root->right) {
			codes[root->data] = code;
		}

		if (root->left) {
			code <<= 1;
			generateHuffmanCodes(root->left, code, codes);
			code >>= 1;
		}

		if (root->right) {
			code <<= 1;
			code.set(0, 1); // "Stream" goes to the right, so adds a 1 to the code. Ex: 01001 -> 010011 .
			generateHuffmanCodes(root->right, code, codes);
			code >>= 1;
		}
	}
}

int main()
{
	auto start = chrono::high_resolution_clock::now();

	// Your function call
	printHuffmanCodes(constructTree("Era uma vez um elfo encantado que morava num pe de caqui"));

	auto end = chrono::high_resolution_clock::now();

	auto duration = chrono::duration_cast<chrono::duration<double>>(end - start);

	cout << "Time taken by function: " << duration.count() << " seconds" << endl;

	return 0;
}