/*
   Software for restoring lost disk partitions.

   Usage: ./restore ./disk_file.dd

   Based on characteristic data patterns. Probability points 
   out how much similarity to partition type processed data 
   has.

   Formula: (detected_patterns) / total_number_of_patterns
   
   Maciej Wydro, 2019
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "part_structs.c"

#define MIN_IND_VALUE 0.8f
#define MAX_RESULTS 20
#define BLOCK_SIZE 512

typedef struct ResultData {
   int offset;
   int part_type;
   float ind;
} ResultData;

typedef struct ResultDataArr {
   ResultData arr[MAX_RESULTS];
   int len;
} ResultDataArray;

// Open file
FILE* open_file(char* file_name){
   FILE *fptr;
   
   if ((fptr = fopen(file_name, "rb")) == NULL){
       printf("Error! opening file\n");
       exit(1);
   }

   return fptr;
}

// Add result helper 
void add_result(ResultDataArray* results, ResultData res){
   if( results->len >= MAX_RESULTS ){
      printf("Results array limit reached (%d). Result skipped.", MAX_RESULTS);
      return;
   }

   results->arr[results->len++] = res;
}

// Search for patterns
void search_patterns(FILE* fptr, ResultDataArray* results){
   char tmp_data[BLOCK_SIZE];

   // Search for patterns
   int read_size = BLOCK_SIZE;
   int count = 0;
   
   while(read_size == BLOCK_SIZE){
      read_size = fread(tmp_data, sizeof(char), BLOCK_SIZE, fptr);

      // Search through all partition types
      for(int type = 0; type < PATTERNS_LEN; ++type){
         DataPattern* patterns = PATTERNS[type].arr;
         int patterns_correct = 0;

         // Specific partition patterns
         for(int i = 0; i < PATTERNS[type].len; ++i){
            if( patterns[i].offset + patterns[i].bytes_len > read_size ){
               continue;
            }
            
            if( !memcmp(tmp_data + patterns[i].offset, patterns[i].bytes, patterns[i].bytes_len) ){
               patterns_correct++;
            }
         }

         float ind = (float)patterns_correct / (float)PATTERNS[type].len;

         if( ind >= MIN_IND_VALUE ){
            add_result(results, (ResultData){count * BLOCK_SIZE, type, ind});
         }
      }

      count++;
   }
}

// Show results
void show_results(ResultDataArray* results){
   printf("Results:\n=============\n");

   if( !results->len ){
      printf("No potential partitions found.\n");
   }

   for(int i = 0; i < results->len; ++i){
      printf("Partition [%d]\nOffset: %d\nType: %d (%s)\nProbability: %.2f \%\n",
         i,
         results->arr[i].offset, 
         results->arr[i].part_type,
         TYPE_STR[results->arr[i].part_type],
         results->arr[i].ind * 100
      );
      printf("-------------\n");
   }
}

typedef struct MBRRecord {
   char status;
   char chs_start[3];
   char type;
   char chs_end[3];
   int lba;
   int size;
} MBRRecord;

void disk_geometry(FILE* fptr, unsigned char* chs /* C, H, S */){
   fseek(fptr, 0, SEEK_END);
   int size = ftell(fptr);
   rewind(fptr);

   chs[0] = 255;
   chs[1] = 63;
   chs[2] = (size / BLOCK_SIZE) / (chs[0] * chs[1]);
}

void lba_to_chs(const unsigned char* chs, unsigned char* out_chs, int sector_start){
   out_chs[2] = sector_start / (chs[0] * chs[1]);  // C
   out_chs[0] = (sector_start / chs[1]) % chs[0];  // H
   out_chs[1] = (sector_start % chs[1]) + 1;       // S
}

int restore_ntfs(FILE* fptr, unsigned char* disk_chs, ResultData* result, int i){
   unsigned char chs_start[3], chs_end[3];
   char buffor[512];
   int read_size;

   // Search for total number of sectors
   fseek(fptr, result->offset, SEEK_SET);
   read_size = fread(buffor, sizeof(char), 512, fptr);

   unsigned long long int total;
   memcpy(&total, buffor + 0x28, sizeof(total));

   // Prepare CHS
   lba_to_chs(disk_chs, chs_start, result->offset / BLOCK_SIZE);
   lba_to_chs(disk_chs, chs_end, (result->offset / BLOCK_SIZE) + total);

   // Prepare record to save
   MBRRecord tmp_record = {
      0x00,
      {chs_start[0], chs_start[1], chs_start[2]},
      0x86, /* NTFS */
      {chs_end[0], chs_end[1], chs_end[2]},
      result->offset / BLOCK_SIZE,
      (int)total
   };

   fseek(fptr, 0x1be + (i * 0x10), SEEK_SET);
   fwrite(&tmp_record, sizeof(tmp_record), 1, fptr);

   return 1;
}

int main(int argc, char **argv)
{
   if(argc < 2){
      printf("You must pass a file as argument.\n");
      return 0;
   }

   // Open file
   char* file_name = argv[1];
   FILE *fptr = fopen(file_name, "r+b");
   printf("Input file: %s\n", file_name);
   
   // Traverse disk
   ResultDataArray results = { .len = 0 };
   search_patterns(fptr, &results);

   // Show results
   show_results(&results);

   // Try to restore
   printf("Trying to restore found partitions.\n");

   unsigned char disk_chs[3];   
   disk_geometry(fptr, disk_chs);
   
   for(int i = 0; i < 4 && i < results.len; ++i){
      int type = results.arr[i].part_type;

      switch(type){
         case NTFS:
            restore_ntfs(fptr, disk_chs, &results.arr[i], i);
            break;
         default:
            printf("Partition type not supported. (%s)", TYPE_STR[type]);
            break;
      }

      printf("Partition no.%d has been written in MBR.\n", i);
   }
}