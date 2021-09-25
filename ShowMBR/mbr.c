#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ptypes.h"

// Print n characters from array as Hex
void printhex(char* ch, int n){
   for(int i = 0; i < n; i++)
      printf("%02X ", (unsigned char)ch[i]);
}

// Stores info about partitions
struct PartInfo{
   char status;
   char type;

   // CHS first
   unsigned int chs_first_c;
   unsigned int chs_first_h;
   unsigned int chs_first_s;
   
   // CHS last
   unsigned int chs_last_c;
   unsigned int chs_last_h;
   unsigned int chs_last_s;
   
   int lba;
   int sectors;
};

int main(int argc, char **argv)
{
   ////////////////// OPEN FILE ////////////////////
   char* file_name = (char *)malloc(256 * sizeof(char));
   if(argc < 2){
      printf("You must pass a file as argument.\n");
      return 0;
   }

   strcpy(file_name, argv[1]);
   printf("IN: %s\n", file_name);

   FILE *fptr;
   if ((fptr = fopen(file_name, "rb")) == NULL){
       printf("Error! opening file\n");
       exit(1);
   }

   ////////////////// READ DATA ////////////////////
   char* bootstrap = (char*)malloc(446 * sizeof(char));
   fread(bootstrap, sizeof(char), 446, fptr); 
   
   struct PartInfo part_infos[4];
   char tmp_chs[3];
   for(int i = 0; i < 4; i++){
      // Status
      fread(&part_infos[i].status, sizeof(char), 1, fptr); 

      // CHS first
      fread(&tmp_chs, sizeof(char), 3, fptr); 

      part_infos[i].chs_first_h = (unsigned char)tmp_chs[0];
      part_infos[i].chs_first_s = ((unsigned char)(tmp_chs[1] & ~(0b11 << 6)));
      part_infos[i].chs_first_c = (unsigned char)tmp_chs[2] + ((unsigned int)(tmp_chs[1] & (0b11 << 6)) << 2);

      // Type
      fread(&part_infos[i].type, sizeof(char), 1, fptr);

      // CHS last
      fread(&tmp_chs, sizeof(char), 3, fptr); 

      part_infos[i].chs_last_h = (unsigned char)tmp_chs[0];
      part_infos[i].chs_last_s = ((unsigned char)(tmp_chs[1] & ~(0b11 << 6)));
      part_infos[i].chs_last_c = (unsigned char)tmp_chs[2] + ((unsigned int)(tmp_chs[1] & (0b11 << 6)) << 2);

      // LBA & Number of sectors
      fread(&part_infos[i].lba, sizeof(int), 1, fptr);       
      fread(&part_infos[i].sectors, sizeof(int), 1, fptr); 
   }

   char* boot_sign = (char*)malloc(2 * sizeof(char));
   fread(boot_sign, sizeof(char), 2, fptr);

   ////////////////////// SHOW DATA //////////////////////
   for(int i = 0; i < 4; i++){
      printf("============================\n");
      printf("PARTITION %d\n", i);
      printf("----------------------------\n");

      printf("Status: %02X ", (unsigned char)part_infos[i].status);
      printf("(boot flag: %s)\n", (part_infos[i].status & 0x80) ? "YES" : "NO");

      printf("Type: %02X (%s)\n", (unsigned char)part_infos[i].type, get_part_info(part_infos[i].type));

      printf("Start-C/H/S: ");
      printf("%u/",  part_infos[i].chs_first_c);
      printf("%u/",  part_infos[i].chs_first_h);
      printf("%u\n", part_infos[i].chs_first_s);

      printf("End-C/H/S: ");
      printf("%u/",  part_infos[i].chs_last_c);
      printf("%u/",  part_infos[i].chs_last_h);
      printf("%u\n", part_infos[i].chs_last_s);

      printf("First LBA: %d\n", part_infos[i].lba);
      printf("Sectors: %d\n", part_infos[i].sectors);

      printf("============================\n");
   }

   printf("Boot signature:");
   printhex(boot_sign, 2);
   printf("\n");
   
   ////////////////// DESTROY DATA ////////////////
   free(bootstrap);
   free(boot_sign);
   
   fclose(fptr); 
   return 0;
}
