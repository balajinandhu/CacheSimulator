#include <iostream>
#include "cachealgsimulator.cpp"
using namespace std;
int main(int argc, char * argv[])
{
  t_pReplAlgFunction ExecReplacementAlg; //Variable for the function to implement a
  //replacement algorithm
  t_pDisplayFunction DisplayDirectory; //Cache content display function pointer
  void * Directory; //Pointer for directory structure
  FILE * pInputFile; //Input file
  int AlgOption; // Alforithm selection option
  char Selection[1024];
  long MemAddress, CacheAddress; //Addresses of memory locations in Main memory
  //and cache
  long NoOfReferences = 0;
  long HitCount = 0;
  bool EnableDirDisp = FALSE;
  printf("=======================================================\n");
  printf("SIMULATION AND ANALYSIS OF CACHE REPLACEMENT ALGORITHMS\n");
  printf("=======================================================\n\n");
  printf(" COEN 283 Winter 2016\n\n");
  printf("Team:\n");
  printf(" Balaji Narayanaswami\n");
  printf(" Manish Kaushik\n");
  printf(" Tanmay Peter Kuruvilla\n\n");
  printf("=======================================================\n\n");
  if(argc > 1)
    {
      EnableDirDisp = (strcmp(argv[1], "D") == 0) ? TRUE : FALSE;
    }
  else
    {
      printf("**Add D in command line to display directory contents\n\n");
    }
  while(TRUE)
    {
      NoOfReferences = 0;
      HitCount = 0;
      /* Select Algorithm */
      printf("CAR:1 LRU:2 Quit:0 \n");
      printf("Select Replacement Algorithm :");
      scanf("%s", Selection);
      if(strcmp(Selection, "0") == 0) exit(0);
      AlgOption = atoi(Selection);
      switch(AlgOption)
        {
        case 1: // CAR
          ExecReplacementAlg = CAR_AlgorithmExec;
          Directory = &CAR_Dir;
          DisplayDirectory = CAR_Display;
          CAR_Init(&CAR_Dir);
          break;
        case 2: // LRU
          ExecReplacementAlg = LRU_AlgorithmExec;
          Directory = &LRU_Dir;
          DisplayDirectory = LRU_Display;
          LRU_Init(&LRU_Dir);
          break;
        default:
          printf("ERROR: Invalid Selection. Try again...\n\n");
          continue;
        }
      printf("Cache Size:");
      scanf("%d", &CACHE_SIZE);
      /* Allocate cache memory array */
      CacheMemory = new t_CacheMemory [CACHE_SIZE];
      /* Initialize Cache memory */
      CacheMemInit();
      /* Error handling for the input file */
      // if( (pInputFile = fopen("C:\\Users\\Balaji\\Documents\\OS_Project\\CacheSim\\input.txt", "r")) == NULL)
      // if( (pInputFile = fopen("/Users/tanmaykuruvilla/Projects/CacheSimulator/CacheSim/input.txt", "r")) == NULL)
      if( (pInputFile = fopen("./input.txt", "r")) == NULL)
        {
          printf("Unable to open input file\n");
          exit(0);
        }
      /* Parsing through the input file */
      while(!feof(pInputFile))
        {
          fscanf(pInputFile, "%d\n", &MemAddress);
          if(EnableDirDisp == TRUE)
            {
              printf("Mem Ref: %4d \n\n",MemAddress);
            }
          NoOfReferences++;
          /* Calling the replacement algorithm */
          if(ExecReplacementAlg(Directory, MemAddress, &CacheAddress) == FALSE)
            {
              /* Updating the cache memory */
              CacheMemUpdate(CacheAddress, MemAddress);
            }
          else HitCount++;
          if(EnableDirDisp == TRUE)
            {
              DisplayDirectory(Directory);
              CacheDisplay();
              printf("-----------------------------------------------------\n");
            }
        }
      /* Printing out statistics */
      printf("=====================================================\n");
      printf("\n");
      printf("No. of References : %d\n", NoOfReferences);
      printf("Total Hit Count : %d\n", HitCount);
      printf("Percentage Hit : %3.2f\n", ((double)
                                          HitCount/(double)NoOfReferences)*100.0);
      printf("\n");
      printf("=====================================================\n");
      printf("\n");
      /* Delete all memory allocations */
      CAR_Delete(&CAR_Dir);
      LRU_Delete(&LRU_Dir);
      delete CacheMemory;
      fclose(pInputFile);
    }
  return 0;
}

