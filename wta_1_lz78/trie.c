//
// Contains implementation of Trie ADT in the context of compression
//

#include "trie.h"

//
// Constructor for a TrieNode.
//
// code: Code of the constructed TrieNode.
// returns: Pointer to a TrieNode that has been allocated memory.
//
TrieNode *trie_node_create(uint16_t code) {
  TrieNode *new = (TrieNode *)malloc(sizeof(TrieNode));
  if (new) {
    new->code = code;
    for (uint32_t i = 0; i < ALPHABET; i++) {
      new->children[i] = NULL;
    }
    return new;
  }
  return (void *)0;
}

//
// Destructor for a TrieNode.
//
// n: TrieNode to free allocated memory for.
// returns: Void.
//
void trie_node_delete(TrieNode *n) {
  free(n);
  n = NULL;
  return;
}

//
// Initializes a Trie: a root TrieNode with the code EMPTY_CODE.
//
// returns: Pointer to the root of a Trie.
//
TrieNode *trie_create(void) {
  return trie_node_create(EMPTY_CODE);
}

//
// Resets a Trie to just the root TrieNode.
//
// root: Root of the Trie to reset.
// returns: Void.
//
void trie_reset(TrieNode *root) {
  if (root != NULL) {
    for (int i = 0; i < ALPHABET; i++) {
      trie_delete(root->children[i]);
      root->children[i] = NULL;
    }
  }
  return;
}

//
// Deletes a sub - Trie starting from the sub - Trie’s root.
//
// n: Root of the sub - Trie to delete.
// returns: Void.
//
void trie_delete(TrieNode *n) {
  if (n != NULL) {
    for (int i = 0; i < ALPHABET; i++) {
      trie_delete(n->children[i]);
      n->children[i] = NULL;
    }
    trie_node_delete(n);
  }
  return;
}

//
// Returns a pointer to the child TrieNode reprsenting the symbol sym.
// If the symbol doesn’t exist, NULL is returned.
//
// n: TrieNode to step from.
// sym: Symbol to check for.
// returns: Pointer to the TrieNode representing the symbol.
//
TrieNode *trie_step(TrieNode *n, uint8_t sym) {
  return n->children[sym];
}

//
// Prints a Trie
//
// root: TrieNode representing root
// returns: Void.
//
void trie_print(TrieNode *root) {
  static uint32_t level = 0;
  for (uint32_t i = 0; i < ALPHABET; i++) {
    if (root->children[i] != NULL) {
      for (uint32_t j = 0; j < level; j++) {
        printf(" ");
      }
      printf("%c: %u\n", i, root->children[i]->code);
      level = level + 1;
      trie_print(root->children[i]);
      level = level - 1;
    }
  }
  return;
}
