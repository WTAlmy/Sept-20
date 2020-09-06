//
// Contains implementation of Word and WordTable data types
//

#include "word.h"

//
// Constructor for a word.
//
// syms: Array of symbols a Word represents.
// len: Length of the array of symbols.
// returns: Pointer to a Word that has been allocated memory.
//
Word *word_create(uint8_t *syms, uint64_t len) {
  Word *new = (Word *)malloc(sizeof(Word));
  if (new != NULL) {
    new->len = len;
    new->syms = (uint8_t *)calloc(len, sizeof(uint8_t));
    if (new->syms != NULL) {
      for (uint64_t i = 0; i < len; i++) {
        new->syms[i] = syms[i];
      }
      return new;
    }
    printf("Failed to allocate word symbols.\n");
  }
  free(new);
  printf("Failed to allocate word memory.\n");
  return (void *)0;
}

//
// Constructs a new Word from the specified Word appended with a symbol.
// The Word specified to append to may be empty.
// If the above is the case, the new Word should contain only the symbol.
//
// w: Word to append to.
// sym: Symbol to append.
// returns: New Word which represents the result of appending.
//
Word *word_append_sym(Word *w, uint8_t sym) {
  Word *new = (Word *)malloc(sizeof(Word));
  if (new != NULL) {
    new->len = w->len + 1;
    new->syms = (uint8_t *)calloc(new->len, sizeof(uint8_t));
    if (new->syms != NULL) {
      for (uint64_t i = 0; i < w->len; i++) {
        new->syms[i] = w->syms[i];
      }
      new->syms[w->len] = sym;
      return new;
    }
    printf("Failed to allocate appended word symbols.\n");
  }
  free(new);
  new = NULL;
  printf("Failed to allocate appended word memory.\n");
  return (void *)0;
}

//
// Destructor for a Word.
//
// w: Word to free memory for.
// returns: Void.
//
void word_delete(Word *w) {
  free(w->syms);
  w->syms = NULL;
  free(w);
  w = NULL;
  return;
}

//
// Creates a new WordTable, which is an array of Words.
// A WordTable has a pre - defined size of MAX_CODE (UINT16_MAX - 1).
// This is because codes are 16 - bit integers.
// A WordTable is initialized with a single Word at index EMPTY_CODE.
// This Word represents the empty word, a string of length of zero.
//
// returns: Initialized WordTable.
//
WordTable *wt_create(void) {
  WordTable *new = (WordTable *)malloc(MAX_CODE * sizeof(WordTable));
  if (new != NULL) {
    for (uint32_t i = 0; i < MAX_CODE; i++) {
      new[i] = NULL;
    }
    //
    // Causes error on timeshare infer, but not my Mac's newer infer
    // The error is not detected in scan-build or valgrind either
    new[EMPTY_CODE] = word_create(0, 0);
    //
    //
    return new;
  }
  printf("Failed to allocate for word table.\n");
  return (void *)0;
}

//
// Resets a WordTable to having just the empty Word.
//
// wt: WordTable to reset.
// returns: Void.
//
void wt_reset(WordTable *wt) {
  for (uint32_t i = 2; i < MAX_CODE; i++) {
    if (wt[i] != NULL) {
      word_delete(wt[i]);
      wt[i] = NULL;
    }
  }
  return;
}

//
// Deletes an entire WordTable.
// All Words in the WordTable must be deleted as well.
//
// wt: WordTable to free memory for.
// returns: Void.
//
void wt_delete(WordTable *wt) {
  for (uint32_t i = 1; i < MAX_CODE; i++) {
    if (wt[i] != NULL) {
      word_delete(wt[i]);
      wt[i] = NULL;
    }
  }
  free(wt);
  return;
}

//
// Prints a Word
//
// w: Word to print
//
void word_print(Word *w) {
  for (uint64_t i = 0; i < w->len; i++) {
    if (w->syms[i]) {
      printf("%c(%" PRIu8 ") --> ", w->syms[i], w->syms[i]);
    }
  }
  printf("\n");
  return;
}

//
// Prints a WordTable
//
// wt: WordTable to print
//
void wt_print(WordTable *wt) {
  uint32_t i = 1;
  Word *w = NULL;
  while ((w = wt[i]) != NULL) {
    word_print(wt[i]);
    i++;
  }
  return;
}
