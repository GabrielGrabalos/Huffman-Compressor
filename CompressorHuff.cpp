#include <iostream>
#include <queue>
#include <bitset>
#include <fstream>
#include <vector>
#include <map>
#include <string>

constexpr auto BYTE_SIZE = 8;

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
map<char, unsigned> get_char_frequences(const vector<char>& buffer) {
	map<char, unsigned> frequences;

	// Count the frequency of each character:
	for (char c : buffer) {
		frequences[c]++;  // If the character is not in the map,
		// it will be added with the value 0
		// and then incremented.
	}

	return frequences;
}

priority_queue<Node*, vector<Node*>, CompareNodes> construct_queue(const vector<char>& buffer) {
	priority_queue<Node*, vector<Node*>, CompareNodes> queue;

	const auto char_frequences = get_char_frequences(buffer);

	for (const auto& pair : char_frequences) {
		queue.push(new Node(pair.first, pair.second));
	}

	return queue;
}

Node* construct_tree(const vector<char>& buffer) {
	priority_queue<Node*, vector<Node*>, CompareNodes> queue = construct_queue(buffer);

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

Node* construct_tree(const map<char, unsigned>& frequences) {
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

void print_huffman_codes(const map<char, string>& codes) {
	for (const auto& pair : codes) {
		cout << pair.first << ": " << pair.second << '\n'; // Ex: a: 01001
	}
}

void print_tree_structure(Node* root, int indent = 0) {
	if (root == nullptr) {
		return;
	}

	const int spaces_per_level = 4;

	if (root->right != nullptr) {
		print_tree_structure(root->right, indent + spaces_per_level);
	}

	cout << string(indent, ' ');
	if (root->data == '#') {
		cout << "| " << root->freq << " |" << '\n';
	}
	else {
		cout << "| " << root->data << ": " << root->freq << " |" << '\n';
	}

	if (root->left != nullptr) {
		print_tree_structure(root->left, indent + spaces_per_level);
	}
}

// Recursive function to generate the Huffman codes:
void generate_huffman_codes(const Node* root, string& code, map<char, string>& codes) {
	if (!root->left && !root->right) {
		codes[root->data] = code == "" ? "0" : code; // If the code is empty, the code of the root is "0
		return;
	}

	string prev_code = code;

	if (root->left) {
		code += "0"; // "Stream" goes to the left, so adds a 0 to the code. Ex: 01001 -> 010010 .
		generate_huffman_codes(root->left, code, codes);
		code = prev_code; // Remove the last character from the code.
	}

	if (root->right) {
		code += "1"; // "Stream" goes to the right, so adds a 1 to the code. Ex: 01001 -> 010011 .
		generate_huffman_codes(root->right, code, codes);
		code = prev_code; // Remove the last character from the code.
	}
}

// Returns a map with the Huffman codes of each character:
map<char, string> generate_huffman_codes(const Node* root) {
	string code;
	map<char, string> codes;

	if (root)
		generate_huffman_codes(root, code, codes); // Recursive function

	return codes;
}

// Turns the text into it's corresponding Huffman codes:
string get_bits_string(const vector<char>& buffer, const map<char, string>& codes) {
	string bits_string;

	for (char c : buffer) {
		bits_string += codes.at(c);
	}

	return bits_string;
}

// Waits for the bits to form a whole byte and then writes it to the file:
void append_bit_to_file(ofstream& file, char bit, char& current_byte, int& bit_count) {
	if (bit != '0' && bit != '1') {
		cerr << "Invalid bit: " << bit << '\n';
		return;
	}

	current_byte = (current_byte << 1) | (bit - '0'); // Shift left by 1 bit to make
	// room for the new bit and add
	// the new bit to the buffer.

	bit_count++; // Increment the number of bits in the buffer.

	if (bit_count == BYTE_SIZE) {
		file.write(&current_byte, 1); // Write the byte to the file.
		current_byte = 0;
		bit_count = 0;
	}
}

void write_string_to_file(const string& str, ofstream& file) {
	int str_length = str.length();

	file.write(reinterpret_cast<const char*>(&str_length), sizeof(str_length));

	file.write(str.c_str(), str_length);
}

void write_extension_to_file(ofstream& file, string extension) {
	int integer = 14;

	// This will be replaced by the amount of redundant bits:
	file.write(reinterpret_cast<const char*>(&integer), sizeof(integer));

	write_string_to_file(extension, file);
}

// Writes the bits to the file by calling append_bit_to_file() for each bit:
bool write_bits_to_file(ofstream& file, const string& bits) {
	if (!file.is_open()) {
		cerr << "Error opening file" << '\n';
		return false;
	}

	char current_byte = 0; // Current byte being filled with bits.
	int  bit_count = 0; // Number of bits in the current byte.

	for (char bit : bits) {
		append_bit_to_file(file, bit, current_byte, bit_count);
	}

	// If there are remaining bits in the buffer, write them to the file:
	if (bit_count > 0) {
		current_byte <<= (BYTE_SIZE - bit_count);
		file.write(&current_byte, 1);
	}

	int redundant_bits = BYTE_SIZE - bit_count; // Keep track of the amount of redundant bits.

	// Write the amount of redundant bits to the first line of the file
	file.seekp(0, ios::beg);
	file.write(reinterpret_cast<const char*>(&redundant_bits), sizeof(redundant_bits));

	return true;
}

int read_integer_from_file(ifstream& file) {
	int integer;

	file.read(reinterpret_cast<char*>(&integer), sizeof(integer));

	return integer;
}

unsigned read_unsigned_from_file(ifstream& file) {
	unsigned integer;

	file.read(reinterpret_cast<char*>(&integer), sizeof(integer));

	return integer;
}

char read_char_from_file(ifstream& file) {
	char c;

	file.read(&c, 1);

	return c;
}

string read_string_from_file(int str_length, ifstream& file) {
	char* buffer = new char[str_length + 1]; // +1 for null terminator

	file.read(buffer, str_length);

	buffer[str_length] = '\0'; // Null terminate the string

	string str(buffer);

	delete[] buffer;

	return str;
}

map<char, unsigned> read_frequences_from_file(ifstream& file, unsigned freqCount)
{
	map<char, unsigned> frequences;
	string line;

	if (!file.is_open())
	{
		cerr << "Error opening file" << '\n';
		return {};
	}

	// Reads as many pairs of char and
	// unsigned as the freqCount variable:
	for (unsigned i = 0; i < freqCount; i++)
	{
		char     c = read_char_from_file(file);
		unsigned freq = read_unsigned_from_file(file);

		frequences[c] = freq; // Add the pair to the map.
	}

	return frequences;
}

bool write_frequences_to_file(ofstream& file, const map<char, unsigned>& frequences) {
	if (!file.is_open()) {
		cerr << "Error opening file" << '\n';
		return false;
	}

	int amout_of_frequence_pairs = frequences.size();

	file.write(reinterpret_cast<const char*>(&amout_of_frequence_pairs), sizeof(amout_of_frequence_pairs));

	for (const auto& pair : frequences) {
		file.write(&pair.first, 1); // Write the char to the file.

		// Write the unsigned to the file:
		file.write(reinterpret_cast
			<const char*>(&pair.second),
			sizeof(pair.second));
	}

	return true;
}

string read_bits_string_from_file(ifstream& file) {
	if (!file.is_open()) {
		cerr << "Error opening file" << '\n';
		return "";
	}

	string bits_string;
	char byte;

	// Read the file byte by byte and
	// convert each byte to a string of bits:
	while (file.get(byte)) {
		bits_string += bitset<BYTE_SIZE>(byte).to_string();
	}

	return bits_string;
}

// Converts the bits to text using the Huffman tree:
string convert_bits_string_to_text(const string& bits_string, Node* root) {
	string text;

	Node* current_node = root; // Places the current node at the root.

	if (!current_node) {
		cerr << "Error: empty tree" << '\n';
		return "";
	}

	// Goes through the string of bits and
	// traverses the tree accordingly:
	for (size_t i = 0; bits_string[i] != '\0'; i++)
	{
		if (bits_string[i] == '0') // If the bit is 0, go left.
		{
			if (current_node->left)
				current_node = current_node->left;
		}
		else if (bits_string[i] == '1') // If the bit is 1, go right.
		{
			if (current_node->right)
				current_node = current_node->right;
		}

		if (!current_node->left && !current_node->right) // If the current node is a leaf, add the character to the text.
		{
			text += current_node->data;
			current_node = root;
		}
	}

	return text;
}

void delete_tree(Node* root) {
	if (root == nullptr) {
		return;
	}

	delete_tree(root->left);
	delete_tree(root->right);

	delete root;
}

ifstream open_input_file(const string& input_filename) {
	ifstream input_file(input_filename, ios::binary);

	if (!input_file.is_open())
	{
		cerr << "Error opening file: " << input_filename << '\n';
		return ifstream();
	}

	return input_file;
}

ofstream open_output_file(const string& output_filename) {
	ofstream output_file(output_filename, ios::binary);

	if (!output_file.is_open())
	{
		cerr << "Error opening file: " << output_filename << '\n';
		return ofstream();
	}

	return output_file;
}

int compress_file(const string& input_filename, const string& output_filename)
{
	ifstream input_file = open_input_file(input_filename);

	vector<char> buffer;

	char c;

	while (input_file.get(c)) {
		buffer.push_back(c);
	}

	input_file.close();


	// Write compressed file:

	ofstream output_file(output_filename, ios::binary);

	if (!output_file.is_open())
	{
		cerr << "Error opening file: " << output_filename << '\n';
		return 1;
	}

	Node* tree = construct_tree(buffer);

	print_huffman_codes(generate_huffman_codes(tree));

	map<char, string> codes = generate_huffman_codes(tree);

	string bits_string = get_bits_string(buffer, codes);

	size_t last_dot_pos = input_filename.find_last_of('.');
	string extension = input_filename.substr(last_dot_pos + 1);

	write_extension_to_file(output_file, extension);

	if (!write_frequences_to_file(output_file, get_char_frequences(buffer)))
	{
		cerr << "Error writing frequences to file" << '\n';
		return 1;
	}

	if (!write_bits_to_file(output_file, bits_string)) {
		cerr << "Error writing bits to file" << '\n';
		return 1;
	}

	output_file.close();

	delete_tree(tree);

	return 0;
}

int decompress_file(const string& input_filename, const string& output_filename)
{
	ifstream input_file(input_filename, ios::binary);

	if (!input_file.is_open())
	{
		cerr << "Error opening file: " << input_filename << '\n';
		return 1;
	}

	int amount_of_redundant_bits = read_integer_from_file(input_file);

	int amount_of_chars_in_extension = read_integer_from_file(input_file);

	string extension = read_string_from_file(amount_of_chars_in_extension, input_file);

	int amount_of_frequece_pairs = read_integer_from_file(input_file);

	map<char, unsigned> frequences = read_frequences_from_file(input_file, amount_of_frequece_pairs);

	Node* tree = construct_tree(frequences);

	print_huffman_codes(generate_huffman_codes(tree));

	string bits_string = read_bits_string_from_file(input_file);

	// Remove the redundant bits from the end of the string:
	bits_string = bits_string.substr(0, bits_string.size() - amount_of_redundant_bits);

	string text = convert_bits_string_to_text(bits_string, tree);

	ofstream output_file(output_filename + extension, ios::binary);

	if (!output_file.is_open())
	{
		cerr << "Error opening file: " << output_filename << '\n';
		return 1;
	}

	output_file << text;

	output_file.close();

	delete_tree(tree);

	return 0;
}

int compress_file(const string& input_filename) {
	size_t last_dot_pos = input_filename.find_last_of('.');
	string base_name = input_filename.substr(0, last_dot_pos);

	string output_filename = base_name + ".graba";

	return compress_file(input_filename, output_filename);
}

int decompress_file(const string& input_filename) {
	size_t last_dot_pos = input_filename.find_last_of('.');
	string base_name = input_filename.substr(0, last_dot_pos);

	string output_filename = base_name + "_decompressed.";

	return decompress_file(input_filename, output_filename);
}

int main(int argc, char* argv[])
{
	if (argc == 2) {
		string file_path = argv[1];
		size_t last_slash = file_path.find_last_of('\\');

		// Check if the last_slash was found:
		if (last_slash == string::npos) {
			last_slash = file_path.find_last_of('/');

			// Check if the last_slash was found:
			if (last_slash == string::npos) {
				cerr << "Invalid file path" << '\n';
				return 1;
			}
		}

		string file_name = file_path.substr(last_slash + 1);

		size_t last_dot_pos = file_name.find_last_of('.');

		// Check if the last_dot_pos was found:
		if (last_dot_pos == string::npos) {
			cerr << "Invalid file name" << '\n';
			return 1;
		}

		string extension = file_name.substr(last_dot_pos + 1);

		if (extension != "graba") {
			return compress_file(file_path);
		}

		return decompress_file(file_path);

		cerr << "Invalid file extension: " << extension << '\n';
		return 1;
	}
	else {
		cerr << "Invalid number of arguments" << '\n';
		return 1;
	}
}