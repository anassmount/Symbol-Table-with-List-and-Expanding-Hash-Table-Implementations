/* symtablehash.c. */
/* Anass Mountasser */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "symtable.h"

/* Each key and its value is stored in a binding, as well as a pointer 
to the next binding.  Bindingss are linked to form a list.  */
struct Binding {
   /* string that identifies the value */
   char *key;
   /* pointer to the value */
   const void *value;
   /* pointer to next linked Binding */
   struct Binding *next;};

/* A SymTable structure is a hashmap structure that stores buckets, 
   with each bucket containing a linked list of Bindings. */ 
struct SymTable {
   /* number of current buckets */
   size_t bucketSize;
   /* pointer to array of Bindings */
   struct Binding **collection;
   /* number of Bindings */
   size_t size;}; 

/* Stores the bucket counts when expansion is needed */
static size_t bucketCounts[] = {509, 1021, 2039, 4093, 8191, 16381,
                                32749, 65521};

/* Return a hash code for pcKey that is between 0 and uBucketCount-1,
   inclusive. */
static size_t SymTable_hash(const char *pcKey, size_t uBucketCount)
{
   const size_t HASH_MULTIPLIER = 65599;
   size_t u;
   size_t uHash = 0;

   assert(pcKey != NULL);

   for (u = 0; pcKey[u] != '\0'; u++)
      uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

   return uHash % uBucketCount;
}

/* frees the memory occupied by a pointer to an array of Binding 
   pointers size of size, which is named collection */
static void SymTable_freeCollection(struct Binding **collection,
                                    size_t size) {
   struct Binding *temp;
   struct Binding *after;
   size_t i;
   assert(collection != NULL);
   for(i = 0; i < size; i++) {
      if(collection[i] == NULL)
         continue;
      else {
        temp = collection[i];
        after = collection[i];
        while(after != NULL) {
           temp = after;
           after = temp->next;
           free(temp->key);
           free(temp);
        }
      }
   }
   free(collection);
}

/* Takes a Symbol Table oSymTable and grows the number 
of buckets it has */
static void SymTable_grow(SymTable_T oSymTable) {
   size_t oldBucketSize;
   size_t newBucketSize;
   size_t hash;
   struct Binding **newCollection;
   struct Binding **oldCollection;
   struct Binding *newBinding;
   struct Binding *temp;
   size_t i;
   /* This section finds new bucket count */
   oldBucketSize = oSymTable->bucketSize;
   oldCollection = oSymTable->collection;
   for(i = 0; bucketCounts[i] <= oldBucketSize; i++)
      ;
   newBucketSize = bucketCounts[i];
   newCollection = (struct Binding**)calloc(newBucketSize,
                                        sizeof(struct Binding*));
   /* Adds every key in old collection to the new one */
   if(newCollection == NULL)
      return;
   for(i = 0; i < oldBucketSize; i++) {
      if(oSymTable->collection[i] == NULL)
         continue;
      for(temp = oSymTable->collection[i]; temp != NULL;
              temp = temp->next) {
         hash = SymTable_hash(temp->key, newBucketSize);
         newBinding = (struct Binding*)malloc(sizeof(struct
                                                         Binding));
         if(newBinding == NULL) {
            SymTable_freeCollection(newCollection,
                                        newBucketSize);
            return; }
         newBinding->key = malloc(sizeof(char) *
                                      (strlen(temp->key) + 1));
         if(newBinding->key == NULL) {
            free(newBinding);
            SymTable_freeCollection(newCollection,
                                        newBucketSize);
            return; }
         strcpy(newBinding->key, temp->key);
         newBinding->value = temp->value;
         newBinding->next = newCollection[hash];
         newCollection[hash] = newBinding;
      }
   }
   oSymTable->bucketSize = newBucketSize;
   oSymTable->collection = newCollection;
   SymTable_freeCollection(oldCollection, oldBucketSize);
   return;
}
  
SymTable_T SymTable_new(void) {
   SymTable_T oSymTable;

   oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
   if (oSymTable == NULL)
      return NULL;
   oSymTable->collection = (struct Binding**)calloc(bucketCounts[0],
                                        sizeof(struct Binding*));
   if(oSymTable->collection == NULL) {
      free(oSymTable);
      return NULL; }
   oSymTable->size = 0;
   oSymTable->bucketSize = bucketCounts[0];
   return oSymTable;
}

void SymTable_free(SymTable_T oSymTable) {
   struct Binding *temp;
   struct Binding *after;
   size_t i;
   assert(oSymTable != NULL);
   for(i = 0; i < oSymTable->bucketSize; i++) {
      if(oSymTable->collection[i] == NULL)
         continue;
      else {
        temp = oSymTable->collection[i];
        after = oSymTable->collection[i];
        while(after != NULL) {
           temp = after;
           after = temp->next;
           free(temp->key);
           free(temp);
        }
      }
   }
   free(oSymTable->collection);
   free(oSymTable);
}
   

size_t SymTable_getLength(SymTable_T oSymTable) {
   assert(oSymTable != NULL);
   return oSymTable->size;
}

int SymTable_put(SymTable_T oSymTable,
                 const char *pcKey, const void *pvValue) {
   struct Binding *newBinding;
   size_t hash;
   assert(oSymTable != NULL);
   assert(pcKey != NULL);
   if(SymTable_contains(oSymTable, pcKey) == 1)
      return 0;
   hash = SymTable_hash(pcKey, oSymTable->bucketSize);
   newBinding = (struct Binding*)malloc(sizeof(struct Binding));
   if (newBinding == NULL)
      return 0;
   newBinding->key = malloc(sizeof(char) * (strlen(pcKey) + 1));
   if (newBinding->key == NULL) {
      free(newBinding);
      return 0; }
   strcpy(newBinding->key, pcKey);
   newBinding->value = pvValue;

   newBinding->next = oSymTable->collection[hash];
   oSymTable->collection[hash] = newBinding;
   oSymTable->size++;
   if(oSymTable->size > oSymTable->bucketSize && oSymTable->bucketSize
      < bucketCounts[sizeof(bucketCounts)/sizeof(bucketCounts[0])])
      SymTable_grow(oSymTable);
   return 1;
}

void *SymTable_replace(SymTable_T oSymTable,
                       const char *pcKey, const void *pvValue) {
   struct Binding *temp;
   size_t hash;
   void *oldValue;
   assert(oSymTable != NULL);
   assert(pcKey != NULL);
   hash = SymTable_hash(pcKey, oSymTable->bucketSize);
   if(oSymTable->collection[hash] == NULL)
      return NULL;
   for(temp = oSymTable->collection[hash]; temp != NULL;
       temp = temp->next) {
      if(strcmp(temp->key, pcKey) == 0) {
         oldValue = (void*)temp->value;
         temp->value = pvValue;
         return oldValue;
      }
   }
   return NULL;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
   struct Binding *temp;
   size_t hash;
   assert(oSymTable != NULL);
   assert(pcKey != NULL);
   hash = SymTable_hash(pcKey, oSymTable->bucketSize);
   if(oSymTable->collection[hash] == NULL)
      return 0;
   for(temp = oSymTable->collection[hash]; temp != NULL;
       temp = temp->next) {
      if(strcmp(temp->key, pcKey) == 0)
         return 1;
   }
   return 0;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
   struct Binding *temp;
   size_t hash;
   assert(oSymTable != NULL);
   assert(pcKey != NULL);
   hash = SymTable_hash(pcKey, oSymTable->bucketSize);
   if(oSymTable->collection[hash] == NULL)
      return NULL;
   for(temp = oSymTable->collection[hash]; temp != NULL;
       temp = temp->next) {
      if(strcmp(temp->key, pcKey) == 0)
         return (void*)temp->value;
   }
   return NULL;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
   void *oldValue;
   struct Binding *cur;
   struct Binding *prev;
   size_t hash;
   assert(oSymTable != NULL);
   assert(pcKey != NULL);
   hash = SymTable_hash(pcKey, oSymTable->bucketSize);
   if(oSymTable->collection[hash] == NULL)
      return NULL;
   for(cur = oSymTable->collection[hash], prev = NULL; (cur != NULL) &&
          (strcmp(cur->key, pcKey) != 0);
       prev = cur, cur = cur->next)
      ;
   if(cur == NULL)
      return NULL;
   oldValue = (void*)cur->value;
   if(prev == NULL)
      oSymTable->collection[hash] = oSymTable->collection[hash]->next;
   else
      prev->next = cur->next;
   oSymTable->size--;
   free(cur->key);
   free(cur);
   return oldValue;
}

void SymTable_map(SymTable_T oSymTable,
     void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
                  const void *pvExtra) {
   struct Binding *temp;
   size_t i;
   assert(oSymTable != NULL);
   assert(pfApply != NULL);
   for(i = 0; i < oSymTable->bucketSize; i++) {
      if(oSymTable->collection[i] == NULL)
         continue;
      else {
          for(temp = oSymTable->collection[i]; temp != NULL;
              temp = temp->next) {
             (*pfApply)(temp->key, (void*)temp->value, (void*)pvExtra);
          }
      }
   }
}

