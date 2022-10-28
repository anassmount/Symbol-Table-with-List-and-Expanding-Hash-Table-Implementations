/* symtablelist.c. */
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

/* A SymTable structure is a manager structure that points to the 
first Binding, and stores the number of bindings. */
struct SymTable {
   /* pointer to the first Binding */
   struct Binding *first;
   /* number of Bindings */
   size_t size;};

SymTable_T SymTable_new(void) {
   SymTable_T oSymTable;

   oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
   if (oSymTable == NULL)
      return NULL;

   oSymTable->first = NULL;
   oSymTable->size = 0;
   return oSymTable;
}

void SymTable_free(SymTable_T oSymTable) {
   struct Binding *temp;
   struct Binding *after;
   assert(oSymTable != NULL);
   temp = oSymTable->first;
   after = oSymTable->first;
   while(after != NULL) {
      temp = after;
      after = temp->next;
      free(temp->key);
      free(temp);
   }
   free(oSymTable);
}
   
size_t SymTable_getLength(SymTable_T oSymTable) {
   assert(oSymTable != NULL);
   return oSymTable->size;
}

int SymTable_put(SymTable_T oSymTable,
                 const char *pcKey, const void *pvValue) {
   struct Binding *newBinding;
   assert(oSymTable != NULL);
   assert(pcKey != NULL);
   if(SymTable_contains(oSymTable, pcKey) == 1)
      return 0;
   newBinding = (struct Binding*)malloc(sizeof(struct Binding));
   if (newBinding == NULL)
      return 0;
   newBinding->key = malloc(sizeof(char) * (strlen(pcKey) + 1));
   if (newBinding->key == NULL) {
      free(newBinding);
      return 0; }
   strcpy(newBinding->key, pcKey);
   newBinding->value = pvValue;

   newBinding->next = oSymTable->first;
   oSymTable->first = newBinding;
   oSymTable->size++;
   return 1;
}

void *SymTable_replace(SymTable_T oSymTable,
                       const char *pcKey, const void *pvValue) {
   struct Binding *temp;
   void *oldValue;
   assert(oSymTable != NULL);
   assert(pcKey != NULL);
   for(temp = oSymTable->first; temp != NULL; temp = temp->next) {
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
   assert(oSymTable != NULL);
   assert(pcKey != NULL);
   for(temp = oSymTable->first; temp != NULL; temp = temp->next) {
      if(strcmp(temp->key, pcKey) == 0)
         return 1;
   }
   return 0;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
   struct Binding *temp;
   assert(oSymTable != NULL);
   assert(pcKey != NULL);
   for(temp = oSymTable->first; temp != NULL; temp = temp->next) {
      if(strcmp(temp->key, pcKey) == 0)
         return (void*)temp->value;
   }
   return NULL;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
   void *oldValue;
   struct Binding *cur;
   struct Binding *prev;
   assert(oSymTable != NULL);
   assert(pcKey != NULL);
   for(cur = oSymTable->first, prev = NULL; (cur != NULL) &&
          (strcmp(cur->key, pcKey) != 0);
       prev = cur, cur = cur->next)
      ;
   if(cur == NULL)
      return NULL;
   oldValue = (void*)cur->value;
   if(prev == NULL)
      oSymTable->first = oSymTable->first->next;
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
   assert(oSymTable != NULL);
   assert(pfApply != NULL);
   for(temp = oSymTable->first; temp != NULL; temp = temp->next) {
      (*pfApply)(temp->key, (void*)temp->value, (void*)pvExtra); }
}
