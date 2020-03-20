#include "command.h"
#include "20150514.h"
#include "BasicCommand.h"
#define CTOI(x) ((int)x-(int)'a')

Trie* getNewTrieNode() {
	Trie* node = NULL;
	node = (Trie*)malloc(sizeof(Trie));
	if (node) {
		node->terminal = false;
		for (int i = 0; i < MAX_ALPHA; i++) {
			node->child[i] = NULL;
		}
	}
	return node;
}
void deleteTrie(struct Trie* root) {
	if (root) {
		for (int i = 0; i < MAX_ALPHA; i++) {
			if (root->child[i]) {
				deleteTrie(root->child[i]);
			}
		}
		free(root);
	}
}

void insertTrie(Trie* root, const char* key) {
	int length = strlen(key);
	Trie* temp = root;
	for (int i = 0; i < length; i++) {
		int next = CTOI(key[i]);
		if (!temp->child[next]) {
			temp->child[next] = getNewTrieNode();
		}
		temp = temp->child[next];
	}
}

bool searchTrie(Trie* root, const char* key) {
	int length = strlen(key);
	Trie* temp = root;

	for (int i = 0; i < length; i++) {
		int next = CTOI(key[i]);
		if (!temp->child[next]) {
			return false;
		}
		temp = temp->child[next];
	}
	return temp != NULL && temp->terminal;
}
