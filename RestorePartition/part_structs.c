//////////////////////////////////////////////////////
//////////////////////// Structs /////////////////////
//////////////////////////////////////////////////////

// For const data that should be present at the given offset (magic numbers etc.)
typedef struct DataPattern {
   unsigned int offset;

   char* bytes;
   unsigned int bytes_len;
} DataPattern;

#define DATA_PATTERN(offset, data) {offset, data, sizeof(data)}


// Fot variable data
typedef struct DataVariable {
   unsigned int offset;
   unsigned int bytes_len;
} DataVariable;

#define DATA_VARIABLE(name, offset, size) const DataVariable name = {offset, size}


//////////////////////////////////////////////////////
//////////////////////// DATA ////////////////////////
//////////////////////////////////////////////////////
#define NTFS   0
#define EXT    1

const char* TYPE_STR[] = {
    "NTFS",
    "EXT"
};

//////////////////////// NTFS ////////////////////////
#define NTFS_JMP     (char[]){0xEB, 0x52, 0x90}
#define NTFS_MAGIC   (char[]){0x4e, 0x54, 0x46, 0x53, 0x20, 0x20, 0x20, 0x20}

const DataPattern NTFS_PATTERNS[] = {
   DATA_PATTERN(0x00, NTFS_JMP),
   DATA_PATTERN(0x03, NTFS_MAGIC)
};

DATA_VARIABLE(NTFS_BYTES_PER_SECTOR, 0x0B, 0x02);
DATA_VARIABLE(NTFS_SECTORS_COUNT, 0x28, 0x08);


//////////////////////// Ext  ////////////////////////
#define EXT_MAGIC    (char[]){0x00}

const DataPattern EXT_PATTERNS[] = {
   DATA_PATTERN(0x00, EXT_MAGIC)
};


//////////////////////////////////////////////////////
//////////////////////// ARRAY ///////////////////////
//////////////////////////////////////////////////////
typedef struct DataPatternArray {
   DataPattern*   arr;
   int            len;
} DataPatternArray;

const DataPatternArray PATTERNS[] = {
   { NTFS_PATTERNS, sizeof(NTFS_PATTERNS)/sizeof(DataPattern) }
};

#define PATTERNS_LEN sizeof(PATTERNS)/sizeof(DataPatternArray)