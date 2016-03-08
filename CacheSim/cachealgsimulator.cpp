/* CacheReplAlgSimulator.CPP */
// Include files
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#define TRUE 1
#define FALSE 0
#define INVALID -1
// User Data Types
// Node structure in T1/T2/B1/B2
struct Node
{
  long MemAddress; // Address of block in main memory
  long CacheAddress; // Address of block in cache memory
  bool Reference; // Reference bit in T1/T2
  Node *nextNode;
  Node *prevNode;
};
// Linked list for T1/T2
struct List_T
{
  Node *Head;
  Node *Tail;
  long Size; // |T1| or |T2|
};
// Linked list for B1/B2
struct List_B
{
  Node *LRU;
  Node *MRU;
  long Size; // |B1| or |B2|
};
// Cache directory containing T1/T2/B1/B2 and p
typedef struct t_CAR_Directory
{
  List_T T1_List;
  List_T T2_List;
  List_B B1_List;
  List_B B2_List;
  long p; // Target size of T1
}t_CAR_Directory;
typedef struct t_LRU_Directory
{
  Node *LRU;
  Node *MRU;
  long Size;
}t_LRU_Directory;
// Cache memory sructure
typedef struct t_CacheMemory
{
  long MemAddress; // Address of block in main memory
  long Valid; // Check if data in cache is valid
  long Dirty; // Data in cache modified (or not)
}t_CacheMemory;
// Pointer to the cache replacement funcion
typedef bool (* t_pReplAlgFunction)(void *, long, long *);
typedef void (* t_pDisplayFunction)(void *);
// Global variables
/* CAR Directory */
t_CAR_Directory CAR_Dir;
t_LRU_Directory LRU_Dir;
/* Cache Variables */
long CACHE_SIZE; //Size of cache from user input
t_CacheMemory *CacheMemory; //Actual data representation in cache
// Function prototypes
/* General functions */
/* Cache memory Functions */
void CacheMemInit(void); //Cache initialization function
long CacheMemGetFreeLocation(void); //Function to find a free location in the
//cache
void CacheMemUpdate(long CacheAddress, long MemAddress); //Function to update the Cache
void CacheDisplay(void); //Function to display the cache details
/* CAR Functions */
void CAR_Init(t_CAR_Directory *Dir); //Function to initialize the
// CAR_Directory structure
bool CAR_AlgorithmExec(void *pDirectory, long MemAddress, long *CacheAddress); //Main
//function
// implementing the CAR algorithm
long CAR_Replace(t_CAR_Directory *pDir); //Replace function in the CAR
//implementation
Node *CAR_InsertToTail_T(List_T *pList, long MemAddress); //Function to insert a node into
//T1/T2
Node *CAR_InsertToMRU_B(List_B *pList, long MemAddress); //Function to insert a node into
//B1/B2
long CAR_DiscardHead_T(List_T *pList); //Function to discard a node
//from
//T1/T2
long CAR_DiscardLRU_B(List_B *pList); //Function to discard a node
//from
//B1/B2
void CAR_DiscardNode_B(List_B *pList, long MemAddress); //Function to discard a node
//from
//B1/B2
Node *CAR_Search_T(t_CAR_Directory *pDir, long MemAddress); //Function to search for a
//node in T1/T2
Node *CAR_Search_B(t_CAR_Directory *pDir, long MemAddress, int * Index); //Function to
//search for
//a node in B1/B2
void CAR_Delete(t_CAR_Directory *pDir); //Function to
// delete node from T1/T2/B1/B2
void CAR_Display(void *pDirectory); //Function to display the CAR
//Directory structure

/* LRU Functions */
void LRU_Init(t_LRU_Directory *pDir);
bool LRU_AlgorithmExec(void *pDirectory, long MemAddress, long *CacheAddress);
Node *LRU_InsertToMRU(t_LRU_Directory *pDir, long MemAddress);
void LRU_MoveToMRU(t_LRU_Directory *pDir, Node *pNode);
long LRU_DiscardLRU(t_LRU_Directory *pDir);
Node *LRU_Search(t_LRU_Directory *pDir, long MemAddress);
void LRU_Delete(t_LRU_Directory *pDir);
void LRU_Display(void *pDirectory);


long Min(long a, long b);
long Max(long a, long b);
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/* Cache initialization */
void CacheMemInit(void)
{
  for(int i=0;i<CACHE_SIZE;i++)
    {
      CacheMemory[i].Valid = FALSE;
      CacheMemory[i].Dirty = FALSE;
      CacheMemory[i].MemAddress = INVALID;
    }
}
/* Finding a free location in cache */
long CacheMemGetFreeLocation(void)
{
  for(int i=0;i<CACHE_SIZE;i++)
    {
      if(CacheMemory[i].Valid == FALSE)
        return i;
    }
  return -1;
}
/* Updating the cache */
void CacheMemUpdate(long CacheAddress, long MemAddress)
{
  if(CacheAddress < 0 || CacheAddress >= CACHE_SIZE)
    {
      printf("ERROR: Address out or range in Cache Memory !!!\n");
      return;
    }
  CacheMemory[CacheAddress].MemAddress = MemAddress;
  CacheMemory[CacheAddress].Valid = TRUE;
}
/* Displaying the Cache directory */
void CacheDisplay(void)
{
  printf("Cache : ");
  for(int i=0;i<CACHE_SIZE;i++)
    {
      if(CacheMemory[i].Valid == TRUE)
        printf("%4d ", CacheMemory[i].MemAddress);
    }
  printf("\n");
}
//////////////////////////////////////////////////////////////////////////
// CAR
//////////////////////////////////////////////////////////////////////////
/* Initialize CAR Directory */
void CAR_Init(t_CAR_Directory *pDir)
{
  pDir->T1_List.Head = NULL;
  pDir->T1_List.Tail = NULL;
  pDir->T1_List.Size = 0;
  pDir->T2_List.Head = NULL;
  pDir->T2_List.Tail = NULL;
  pDir->T2_List.Size = 0;
  pDir->B1_List.LRU = NULL;
  pDir->B1_List.MRU = NULL;
  pDir->B1_List.Size = 0;
  pDir->B2_List.LRU = NULL;
  pDir->B2_List.MRU = NULL;
  pDir->B2_List.Size = 0;
  pDir->p = 0;
}
/* CAR Replacement Algorithm Implementation */
bool CAR_AlgorithmExec(void *pDirectory, long MemAddress, long * CacheAddress)
{
  Node *pNode;
  int B_ListIndex;
  t_CAR_Directory *pDir = (t_CAR_Directory *)pDirectory;
  *CacheAddress = INVALID;
  if( (pNode = CAR_Search_T(pDir, MemAddress)) != NULL) /* Cache Hit if in T1 or T2 */
    {
      pNode->Reference = TRUE;
      return TRUE;
    }
  else /* Cache Miss not in T1 or T2*/
    {
      if( (pDir->T1_List.Size + pDir->T2_List.Size) == CACHE_SIZE) /* Cache Full */
        {
          *CacheAddress = CAR_Replace(pDir);
          /* Cache Directory Replacement */
          if( CAR_Search_B(pDir, MemAddress, &B_ListIndex) == NULL)
            {
              if( (pDir->T1_List.Size + pDir->B1_List.Size) == CACHE_SIZE )
                {
                  /* Discarding a node from B1 */
                  CAR_DiscardLRU_B(&pDir->B1_List);
                }
              else if( (pDir->T1_List.Size + pDir->T2_List.Size + pDir->B1_List.Size +
                        pDir->B2_List.Size) == 2*CACHE_SIZE )
                {
                  /* Discarding a node from B2 */
                  CAR_DiscardLRU_B(&pDir->B2_List);
                }
            }
        }
      /* Check Cache Directory */
      if( (pNode = CAR_Search_B(pDir, MemAddress, &B_ListIndex)) == NULL)
        {
          /* Cache Directory Miss */
          pNode = CAR_InsertToTail_T(&pDir->T1_List, MemAddress);
        }
      else
        {
          /* Cache Directory Hit */
          if (B_ListIndex == 1)
            {
              /* Found in B1 List */
              pDir->p = Min( pDir->p + Max(1,pDir->B2_List.Size/pDir->B1_List.Size),
                             CACHE_SIZE);
              CAR_DiscardNode_B(&pDir->B1_List, MemAddress);
            }
          else
            {
              /* Found in B2 List */
              pDir->p = Max( pDir->p - Max(1, pDir->B1_List.Size/pDir->B2_List.Size), 0);
              CAR_DiscardNode_B(&pDir->B2_List, MemAddress);
            }
          pNode = CAR_InsertToTail_T(&pDir->T2_List, MemAddress);
        }
      pNode->Reference = FALSE;
      /* Updating the cache */
      if(*CacheAddress == INVALID)
        {
          pNode->CacheAddress = CacheMemGetFreeLocation();
          *CacheAddress = pNode->CacheAddress;
        }
      else
        pNode->CacheAddress = *CacheAddress;
    }
  return FALSE;
}
/* Replace node in cache – T1/T2 */
long CAR_Replace(t_CAR_Directory *pDir)
{
  bool Found = FALSE;
  Node *pNode;
  long MemAddress;
  long CacheAddress;
  do
    {
      /* T1 size is more than max of target size and 1 */
      if(pDir->T1_List.Size >= (pDir->p > 1 ? pDir->p : 1) )
        {
          /* Looking for a node in T1 whose reference bit is 0 starting from LRU position */
          if(pDir->T1_List.Head->Reference == FALSE)
            {
              /* Found a node in T1 whose referenece bit is 0 */
              Found = TRUE;
              MemAddress = pDir->T1_List.Head->MemAddress;
              CacheAddress = pDir->T1_List.Head->CacheAddress;
              CAR_DiscardHead_T(&pDir->T1_List);
              /* Moving the node found from T1 to B1 */
              pNode = CAR_InsertToMRU_B(&pDir->B1_List, MemAddress);
              /* Removing the node from Cache */
              pNode->CacheAddress = CacheAddress;
              pNode->Reference = FALSE;
            }
          else
            {

              MemAddress = pDir->T1_List.Head->MemAddress;
              CacheAddress = pDir->T1_List.Head->CacheAddress;
              /* Reference bit of node in LRU position is T1 is not 0
   Move it to T2 and reset the reference bit */
              CAR_DiscardHead_T(&pDir->T1_List);
              pNode = CAR_InsertToTail_T(&pDir->T2_List,MemAddress);
              pNode->CacheAddress = CacheAddress;
              pNode->Reference = FALSE;
            }
        }
      else
        {
          /* T1 is less than target size, look for replacing nodes in T2 */
          if(pDir->T2_List.Head->Reference == FALSE)
            {
              /* Found a node in T2 whose reference bit is 0 */
              Found = TRUE;
              MemAddress = pDir->T2_List.Head->MemAddress;
              CacheAddress = pDir->T2_List.Head->CacheAddress;
              /* Moving the node from T2 to B2 and resetting the reference bit */
              CAR_DiscardHead_T(&pDir->T2_List);
              pNode = CAR_InsertToMRU_B(&pDir->B2_List, MemAddress);
              pNode->CacheAddress = CacheAddress;
              pNode->Reference = FALSE;
            }
          else
            {
              /* Could not find a node in T2 whose reference bit is 0 */
              MemAddress = pDir->T2_List.Head->MemAddress;
              CacheAddress = pDir->T2_List.Head->CacheAddress;
              /* Move it to the tail of T2 and reset its reference bit */
              CAR_DiscardHead_T(&pDir->T2_List);
              pNode = CAR_InsertToTail_T(&pDir->T2_List, MemAddress);
              pNode->CacheAddress = CacheAddress;
              pNode->Reference = FALSE;
            }
        }
    }while(Found == FALSE);
  return CacheAddress;
}
/* Discarding the node from T1/T2 */
long CAR_DiscardHead_T(List_T *pList)
{
  long MemAddress;
  Node *pNode;
  if(pList->Head != NULL)
    {
      /* Discarding the node at the head position in T1/T2 */
      MemAddress = pList->Head->MemAddress;
      pNode = pList->Head->nextNode;
      delete pList->Head;
      /* If there are more nodes in T1/T2, change the Head position */
      if(pNode != NULL)
        {
          pList->Head = pNode;
          pList->Head->prevNode = NULL;
        }
      else /* No more elements are present in T1/T2 */
        {
          pList->Head = NULL;
          pList->Tail = NULL;
        }
      pList->Size--;
      return MemAddress;
    }
  return INVALID;
}
/* Discard node in LRU position of B1/B2 */
long CAR_DiscardLRU_B(List_B *pList)
{
  long MemAddress;
  Node *pNode;
  if(pList->LRU != NULL)
    {
      /* Discarding the node at the LRU position in B1/B2 */
      MemAddress = pList->LRU->MemAddress;
      pNode = pList->LRU->nextNode;
      delete pList->LRU;
      /* If there are more nodes in B1/B2, change the LRU position */
      if(pNode != NULL)
        {
          pList->LRU = pNode;
          pList->LRU->prevNode = NULL;
        }
      else /* No more nodes are present in B1/B2 */
        {
          pList->LRU = NULL;
          pList->MRU = NULL;
        }
      pList->Size--;
      return MemAddress;
    }
  return INVALID;
}
/* Function to discard a particular node in B1/B2 */
void CAR_DiscardNode_B(List_B *pList, long MemAddress)
{
  Node *pNode = pList->LRU;
  /* The node to be deleted is the only node in the B list */
  if( (pList->LRU == pList->MRU) && (pNode->MemAddress == MemAddress) )
    {
      delete pList->LRU;
      pList->LRU = NULL;
      pList->MRU = NULL;
      pList->Size = 0;
      return;
    }
  while(pNode != NULL)
    {
      Node *tNode;
      /* Found the node to be deleted */
      if(pNode->MemAddress == MemAddress)
        {
          tNode = pNode->prevNode;
          /* Node to be deleted is in LRU position */
          if(tNode == NULL)
            {
              pList->LRU = pNode->nextNode;
              pList->LRU->prevNode = NULL;
            }
          else /* Node to be deleted is NOT in LRU position */
            {
              if(pNode->nextNode == NULL) /* Node to be deleted is in MRU position */
                {
                  pList->MRU = tNode;
                  pList->MRU->nextNode = NULL;
                }
              else /* Node to be deleted is NOT in MRU position */
                {
                  (pNode->nextNode)->prevNode = tNode;
                  tNode->nextNode = pNode->nextNode;
                }
            }
          delete pNode;
          pList->Size--;
          break;
        }
      pNode = pNode->nextNode;
    }
}
/* Insert a node to tail of T1/T2 */
Node *CAR_InsertToTail_T(List_T *pList, long MemAddress)
{
  Node *newNode = new Node;
  newNode->MemAddress = MemAddress;
  newNode->nextNode = NULL;
  if(pList->Head == NULL)
    {
      /* Inserting into an empty list */
      pList->Head = newNode;
      pList->Tail = newNode;
      newNode->prevNode = NULL;
    }
  else
    {
      /* Inserting into a list containing only one element */
      if(pList->Head == pList->Tail)
        {
          pList->Head->nextNode = newNode;
        }
      pList->Tail->nextNode = newNode;
      newNode->prevNode = pList->Tail;
      pList->Tail = newNode;
    }
  pList->Size++;
  return newNode;
}
/* Insert a node to MRU position of B1/B2 */
Node *CAR_InsertToMRU_B(List_B *pList, long MemAddress)
{
  Node *newNode = new Node;
  newNode->MemAddress = MemAddress;
  newNode->nextNode = NULL;
  if(pList->LRU == NULL)
    {
      /* Inserting into a list containing only one element */
      pList->LRU = newNode;
      pList->MRU = newNode;
      newNode->prevNode = NULL;
    }
  else
    {
      /* Inserting at the MRU position */
      if(pList->LRU == pList->MRU)
        {
          pList->LRU->nextNode = newNode;
        }
      pList->MRU->nextNode = newNode;
      newNode->prevNode = pList->MRU;
      pList->MRU = newNode;
    }
  pList->Size++;
  return newNode;
}
/* Search for memory reference in CAR directory (T1/T2) */
Node *CAR_Search_T(t_CAR_Directory *pDir, long MemAddress)
{
  /* Searching for the node in T1 */
  Node *pNode = pDir->T1_List.Head;
  while(pNode != NULL)
    {
      /* Found the node */
      if(pNode->MemAddress == MemAddress)
        return pNode;
      pNode = pNode->nextNode;
    }
  /* Searching for the node in T2 */
  pNode = pDir->T2_List.Head;
  while(pNode != NULL)
    {
      /* Found the node */
      if(pNode->MemAddress == MemAddress)
        return pNode;
      pNode = pNode->nextNode;
    }
  return NULL;
}
/* Search for memory reference in B1/B2 */
Node *CAR_Search_B(t_CAR_Directory *pDir, long MemAddress, int * Index)
{
  /* Searching for node in B1 */
  Node *pNode = pDir->B1_List.LRU;
  *Index = 1;
  while(pNode != NULL)
    {
      if(pNode->MemAddress == MemAddress)
        return pNode;
      pNode = pNode->nextNode;
    }
  /* Searching for node in B2 */
  pNode = pDir->B2_List.LRU;
  *Index = 2;
  while(pNode != NULL)
    {
      if(pNode->MemAddress == MemAddress)
        return pNode;
      pNode = pNode->nextNode;
    }
  /* Node not found in B1 or B2 */
  *Index = 0;
  return NULL;
}
/* Remove all memory allocation for LRU directory */
void CAR_Delete(t_CAR_Directory *pDir)
{
  Node *pNode;
  Node *pDelNode;
  pNode = pDir->T1_List.Head;
  while(pNode != NULL)
    {
      pDelNode = pNode;
      pNode = pNode->nextNode;
      delete pDelNode;
    }
  pNode = pDir->T2_List.Head;
  while(pNode != NULL)
    {
      pDelNode = pNode;
      pNode = pNode->nextNode;
      delete pDelNode;
    }
  pNode = pDir->B1_List.LRU;
  while(pNode != NULL)
    {
      pDelNode = pNode;
      pNode = pNode->nextNode;
      delete pDelNode;
    }
  pNode = pDir->B2_List.LRU;
  while(pNode != NULL)
    {
      pDelNode = pNode;
      pNode = pNode->nextNode;
      delete pDelNode;
    }
  CAR_Init(pDir);
}
/* Display CAR directory content */
void CAR_Display(void *pDirectory)
{
  t_CAR_Directory *pDir = (t_CAR_Directory *)pDirectory;
  /* Displaying the T1 list */
  Node *pNode = pDir->T1_List.Head;
  printf(" T1 : ");
  while(pNode != NULL)
    {
      printf("%4d|%d ", pNode->MemAddress, pNode->Reference);
      pNode = pNode->nextNode;
    }
  printf("\n");
  /* Displaying the B1 list */
  pNode = pDir->B1_List.LRU;
  printf(" B1 : ");
  while(pNode != NULL)
    {
      printf("%4d ", pNode->MemAddress);
      pNode = pNode->nextNode;
    }
  printf("\n\n");
  /* Displaying the T2 list */
  pNode = pDir->T2_List.Head;
  printf(" T2 : ");
  while(pNode != NULL)
    {
      printf("%4d|%d ", pNode->MemAddress, pNode->Reference);
      pNode = pNode->nextNode;
    }
  printf("\n");
  /* Displaying the B2 list */
  pNode = pDir->B2_List.LRU;
  printf(" B2 : ");
  while(pNode != NULL)
    {
      printf("%4d ", pNode->MemAddress);
      pNode = pNode->nextNode;
    }
  printf("\n\n");
}
/* Find minimum of two integers */
long Min(long a, long b)
{
  if(a < b) return a;
  return b;
}
/* Find maximum of two integers */
long Max(long a, long b)
{
  if(a > b) return a;
  return b;
}
//////////////////////////////////////////////////////////////////////////
// LRU
//////////////////////////////////////////////////////////////////////////
/* Initialize LRU directory */
void LRU_Init(t_LRU_Directory *pDir)
{
  pDir->LRU = NULL;
  pDir->MRU = NULL;
  pDir->Size = 0;
}
/* LRU Replacement Algorithm Implementation */
bool LRU_AlgorithmExec(void *pDirectory, long MemAddress, long *CacheAddress)
{
  Node *pNode;
  t_LRU_Directory *pDir = (t_LRU_Directory *)pDirectory;
  if( (pNode = LRU_Search(pDir, MemAddress)) != NULL)
    {
      /* HIT */
      LRU_MoveToMRU(pDir, pNode);
      return TRUE;
    }
  else
    {
      /* MISS */
      if(pDir->Size == CACHE_SIZE)
        {
          /* Cache full */
          *CacheAddress = LRU_DiscardLRU(pDir);
          pNode = LRU_InsertToMRU(pDir,MemAddress);
          pNode->CacheAddress = *CacheAddress;
        }
      else
        {
          /* cache not yet full */
          pNode = LRU_InsertToMRU(pDir,MemAddress);
          pNode->CacheAddress = CacheMemGetFreeLocation();
          *CacheAddress = pNode->CacheAddress;
        }
    }
  return FALSE;
}
/* Search for reference in Directory */
Node *LRU_Search(t_LRU_Directory *pDir, long MemAddress)
{
  Node *pNode = pDir->LRU;
  while(pNode != NULL)
    {
      if(pNode->MemAddress == MemAddress)
        return pNode;
      pNode = pNode->nextNode;
    }
  return NULL;
}
/* Move node to MRU position */
void LRU_MoveToMRU(t_LRU_Directory *pDir, Node *pNode)
{
  Node *pTempNode;
  if(pNode == NULL) return;
  if(pNode == pDir->MRU)
    {
      /* pNode already in MRU position. */
      return;
    }
  /* Remove pNode from current position */
  pTempNode = pNode->prevNode;
  if(pTempNode == NULL)
    {
      pDir->LRU = pNode->nextNode;
      pDir->LRU->prevNode = NULL;
    }
  else
    {
      (pNode->nextNode)->prevNode = pTempNode;
      pTempNode->nextNode = pNode->nextNode;
    }
  /* Add pNode to MRU position */
  pDir->MRU->nextNode = pNode;
  pNode->prevNode = pDir->MRU;
  pDir->MRU = pNode;
  pDir->MRU->nextNode = NULL;
}
/* Insert node to LRU position */
Node *LRU_InsertToMRU(t_LRU_Directory *pDir, long MemAddress)
{
  Node *newNode = new Node;
  newNode->MemAddress = MemAddress;
  newNode->CacheAddress = INVALID;
  newNode->nextNode = NULL;
  if(pDir->LRU == NULL)
    {
      /* List empty */
      pDir->LRU = newNode;
      pDir->MRU = newNode;
      newNode->prevNode = NULL;
    }
  else
    {
      if(pDir->LRU == pDir->MRU)
        {
          pDir->LRU->nextNode = newNode;
        }
      pDir->MRU->nextNode = newNode;
      newNode->prevNode = pDir->MRU;
      pDir->MRU = newNode;
    }
  pDir->Size++;
  return newNode;
}
/* Discard node at LRU position */
long LRU_DiscardLRU(t_LRU_Directory *pDir)
{
  long CacheAddress;
  Node *pNode;
  if(pDir->LRU != NULL)
    {
      CacheAddress = pDir->LRU->CacheAddress;
      pNode = pDir->LRU->nextNode;
      delete pDir->LRU;
      if(pNode != NULL)
        {
          pDir->LRU = pNode;
          pDir->LRU->prevNode = NULL;
        }
      else
        {
          pDir->LRU = NULL;
          pDir->MRU = NULL;
        }
      pDir->Size--;
      return CacheAddress;
    }
  return INVALID;
}
/* Display LRU directory content */
void LRU_Display(void *pDirectory)
{
  t_LRU_Directory *pDir = (t_LRU_Directory *)pDirectory;
  Node *pNode = pDir->LRU;
  printf("LRU-MRU: ");
  while(pNode != NULL)
    {
      printf("%4d ", pNode->MemAddress);
      pNode = pNode->nextNode;
    }
  printf("\n");
}
/* Remove all memory allocation for LRU directory */
void LRU_Delete(t_LRU_Directory *pDir)
{
  Node *pNode = pDir->LRU;
  Node *pDelNode;
  while(pNode != NULL)
    {
      pDelNode = pNode;
      pNode = pNode->nextNode;
      delete pDelNode;
    }
  pDir->LRU = NULL;
  pDir->MRU = NULL;
  pDir->Size = 0;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////End of file ////////////////////////////////
//////////////////////////////////////////////////////////////////////////


