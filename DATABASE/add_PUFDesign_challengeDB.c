// ========================================================================================================
// ========================================================================================================
// ************************************** add_PUFDesign_challengeDB.c *************************************
// ========================================================================================================
// ========================================================================================================
//
//--------------------------------------------------------------------------------
// Company: IC-Safety, LLC and University of New Mexico
// Engineer: Professor Jim Plusquellic
// Exclusive License: IC-Safety, LLC
// Copyright: Univ. of New Mexico
// 
// Create Date: 8/15/2015
// Functions covered by License and Copyright: All 
//--------------------------------------------------------------------------------

#include "commonDB.h"

// ========================================================================================================
// ========================================================================================================
// Add master vectors to the Vectors table, and then vector pairs to the VecPairs table, and then the timing 
// data to the TimingVals table. It is common for this routine to issue constraint violations because vectors 
// or vector pairs already exist in the tables. Timing values are deleted and then re-added if they already 
// exist b/c the user must first decide if the PUFInstance is to be preserved or replaced. This is 
// accomplished because a foreign key and ON DELETE CASCADE is set on the TimingVals database to the PUFInstance 
// table.

void ProcessMasterVecs(int max_string_len, sqlite3 *db, int design_index, int instance_index,
   int master_num_vec_pairs, int master_num_rise_vec_pairs, unsigned char **master_first_vecs_b, 
   unsigned char **master_second_vecs_b, int num_PIs, int num_POs)
   {
   int vecpair_index;
   char rise_fall_str[200];
   int vec_num, vec_len_bytes;

   int first_vec_index, second_vec_index;

   int tot_TVs;

   vec_len_bytes = num_PIs/8;

#ifdef DEBUG
struct timeval t1, t2;
long elapsed; 
#endif

printf("ENROLL:\tPUFDesign %d: PUFInstance %d\tFor Num Vectors %d\n", design_index, instance_index, master_num_vec_pairs); fflush(stdout);
#ifdef DEBUG
#endif

// ---------------------------------------------------------
   tot_TVs = 0;
   for ( vec_num = 0; vec_num < master_num_vec_pairs; vec_num++ )
      {
#ifdef DEBUG
gettimeofday(&t2, 0);
#endif

#ifdef DEBUG
printf("\n\nPROCESSING vector %d of %d\n", vec_num, master_num_vec_pairs); fflush(stdout);
#endif

#ifdef DEBUG
printf("SQL Query '%s'\n", SQL_Vectors_insert_into_cmd);
#endif

#ifdef DEBUG
printf("\tWriting 1st vector number %d\n", vec_num);
#endif

// Insert first vector of vector pair into Vectors table if it does already exist. It is common for vectors to already exist
// as enrollment data for new chips is added.
      InsertIntoTable(max_string_len, db, "Vectors", SQL_Vectors_insert_into_cmd, master_first_vecs_b[vec_num], vec_len_bytes, 
         NULL, NULL, NULL, NULL, NULL, -1, -1, -1, -1, -1, -1.0, -1.0);

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
#endif

#ifdef DEBUG
printf("\tWriting 2nd vector number %d\n", vec_num);
#endif

// Insert second vector of vector pair.
      InsertIntoTable(max_string_len, db, "Vectors", SQL_Vectors_insert_into_cmd, master_second_vecs_b[vec_num], vec_len_bytes, 
         NULL, NULL, NULL, NULL, NULL, -1, -1, -1, -1, -1, -1.0, -1.0);

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
#endif

// ------------------------------------------------------------------------------------------------------------
// For each pair of vectors that have been added, add a row to the VecPairs table. First find the vectors in the
// Vectors table (that were either just added or already existed). Since only unique vectors are added, these number 
// can jump around because the calls above may not add a vector (if it is not unique).

#ifdef DEBUG
printf("\nProcessMasterVecsAndTimingData(): Find first vector!\n"); fflush(stdout);
#endif

// Get index for first vector of the vector pair from the Vectors table.
      if ( (first_vec_index = GetIndexFromTable(max_string_len, db, "Vectors", SQL_Vectors_get_index_cmd, master_first_vecs_b[vec_num], 
         vec_len_bytes, NULL, NULL, NULL, NULL, -1, -1, -1)) == -1 )
         { 
         printf("ERROR: ProcessMasterVecsAndTimingData(): Failed to find first_vec_index for vec_num %d in Vectors table!\n", vec_num); 
         exit(EXIT_FAILURE); 
         }

#ifdef DEBUG
printf("Vec %d, First vector index %d\n", vec_num, first_vec_index); fflush(stdout);
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
#endif

#ifdef DEBUG
// Sanity check. Read back the first vector and check it against array values.
      int byte_num;
      unsigned char temp_vec[vec_len_bytes];
      ReadBinaryBlob(db, SQL_Vectors_read_vector_cmd, first_vec_index, temp_vec, vec_len_bytes, 0, NULL);

// printf("Stored and retrieved vector %d => size is %d:\n\t\t", vec_num, vec_len_bytes); fflush(stdout);
      for ( byte_num = 0; byte_num < vec_len_bytes; byte_num++ ) 
         {
         printf("%02X <=> %02X  ", master_first_vecs_b[vec_num][byte_num], temp_vec[byte_num]); 
         if ( (byte_num + 1) % 16 == 0 )
            printf("\n\t\t");
         if ( master_first_vecs_b[vec_num][byte_num] != temp_vec[byte_num] )
            {
            printf("ERROR: ProcessMasterVecsAndTimingData(): Master Vector byte %02X does NOT equal stored vector byte %02X!\n", 
               master_first_vecs_b[vec_num][byte_num], temp_vec[byte_num]); 
            exit(EXIT_FAILURE);
            }
         }
      printf("\n"); fflush(stdout);
#endif

#ifdef DEBUG
printf("\nProcessMasterVecsAndTimingData(): Find second vector!\n"); fflush(stdout);
#endif

// Get index for second vector of the vector pair from the Vectors table.
      if ( (second_vec_index = GetIndexFromTable(max_string_len, db, "Vectors", SQL_Vectors_get_index_cmd, master_second_vecs_b[vec_num], 
         vec_len_bytes, NULL, NULL, NULL, NULL, -1, -1, -1)) == -1 )
         { 
         printf("ERROR: ProcessMasterVecsAndTimingData(): Failed to find second_vec_index for vec_num %d in Vectors table!\n", vec_num); 
         exit(EXIT_FAILURE); 
         }

#ifdef DEBUG
printf("Vec %d, Second vector index %d\n", vec_num, second_vec_index); fflush(stdout);
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
#endif

#ifdef DEBUG
printf("\tWrote binary vectors into Vector Table for %d with 1st vector index %d and 2nd vector index %d\n", vec_num, first_vec_index, second_vec_index); fflush(stdout);
#endif

#ifdef DEBUG
// Sanity check. Read back the second vector and check it against array values.
      ReadBinaryBlob(db, SQL_Vectors_read_vector_cmd, second_vec_index, temp_vec, vec_len_bytes, 0, NULL);

// printf("Stored and retrieved vector %d => size is %d:\n\t\t", vec_num, vec_len_bytes); fflush(stdout);
      for ( byte_num = 0; byte_num < vec_len_bytes; byte_num++ ) 
         {
         printf("%02X <=> %02X  ", master_second_vecs_b[vec_num][byte_num], temp_vec[byte_num]); 
         if ( (byte_num + 1) % 16 == 0 )
            printf("\n\t\t");
         if ( master_second_vecs_b[vec_num][byte_num] != temp_vec[byte_num] )
            {
            printf("ERROR: ProcessMasterVecsAndTimingData(): Master Vector byte %02X does NOT equal stored vector byte %02X!\n", 
               master_second_vecs_b[vec_num][byte_num], temp_vec[byte_num]); 
            exit(EXIT_FAILURE);
            }
         }
      printf("\n"); fflush(stdout);
#endif

// We automatically detected when reading vectors and mask files which vectors are rising and counted them (and also made sure they
// were listed as the first set of vectors in the file). This number is passed in here as 'master_num_rise_vec_pairs'. We record in the
// VecPairs table whether the vector is rising or falling.
      if ( vec_num < master_num_rise_vec_pairs )
         strcpy(rise_fall_str, "R");
      else
         strcpy(rise_fall_str, "F");

#ifdef DEBUG
printf("Creating/Adding VecPair for vec_num %d, R/F %s, 1st vec index %d, 2nd vec index %d and design index %d!\n",
   vec_num, rise_fall_str, first_vec_index, second_vec_index, design_index); fflush(stdout);
#endif

// Add a vector pair to VecPair table. NOTE: a 'unique' index is setup on VecPair that uses 'first_vec_index', 'second_vec_index' and
// 'design_index', so if the VecPair already exists, it is NOT added again. ALSO NOTE: We will fill in NumPNs after we add and count the 
// timing values below.
      InsertIntoTable(max_string_len, db, "VecPairs", SQL_VecPairs_insert_into_cmd, NULL, 0, rise_fall_str, NULL, NULL, NULL, NULL, first_vec_index, 
         second_vec_index, -1, design_index, -1, -1.0, -1.0);

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
printf("Getting index from VecPair Table\n"); fflush(stdout);
#endif

// Get the index for this newly added VecPair.
      if ( (vecpair_index = GetIndexFromTable(max_string_len, db, "VecPairs", SQL_VecPairs_get_index_cmd, NULL, 0, NULL, NULL, NULL, NULL, 
         first_vec_index, second_vec_index, design_index)) == -1 )
         {
         printf("ERROR: ProcessMasterVecsAndTimingData(): Failed to find vecpair_index for vec_num %d in VecPairs table!\n", vec_num); 
         exit(EXIT_FAILURE); 
         }

#ifdef DEBUG
printf("\tWrote VecPair with index %d\n", vecpair_index); fflush(stdout);
#endif

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed);
gettimeofday(&t2, 0);
printf("Saving timing data to TimingVals table\n"); fflush(stdout);
#endif

// Add the timing data for this PUFInstance and VecPair. Unlike above, this should never find that the TimingVal exists. 
//      num_PNs_per_vecpair = AddTimingDataToDB(max_string_len, db, SQL_TimingVals_insert_into_cmd, vec_num, instance_index, vecpair_index, 
//         PNX, PNX_Tsig, rise_fall, vec_pairs, POs, num_PNR, num_PNX, master_num_rise_vec_pairs, num_POs);

// Update NumPNs field in the VecPairs record with 'num_PNs_per_vecpair'.
      UpdateVecPairsNumPNsField(max_string_len, db, 32, vecpair_index);

      tot_TVs += 32;

#ifdef DEBUG
gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed);
fflush(stdout);
#endif
      }

#ifdef DEBUG
printf("\tWrote Total TimingVals %d\n", tot_TVs); fflush(stdout);
#endif

   return;
   }


// ========================================================================================================
// ========================================================================================================
// ========================================================================================================
// ========================================================================================================

unsigned char **master_first_vecs_b;
unsigned char **master_second_vecs_b;
char **master_masks;

char Netlist_name[MAX_STRING_LEN];
char Chip_name[MAX_STRING_LEN];
char Device_name[MAX_STRING_LEN];
char Synthesis_name[MAX_STRING_LEN];
char Placement_name[MAX_STRING_LEN];

char MasterDBname[MAX_STRING_LEN];

char MasterVecPrefix[MAX_STRING_LEN];
char MasterVecFile[MAX_STRING_LEN];
char MasterMaskFile[MAX_STRING_LEN];

int main(int argc, char **argv)
   {
   int master_num_vec_pairs, master_num_rise_vec_pairs;
   int has_masks;

   sqlite3 *db;
   int rc;

   int design_index;
   int instance_index;

   int num_PIs, num_POs;
   int read_db_into_memory;

   int rise_fall_bit_pos;

//   int DEBUG_FLAG;

// ===============================================================================
   if ( argc != 11 )
      { 
      printf("ERROR: %s: Master Database (Customer.db) -- Netlist name (SR_RFM_V4_TDC) -- Synthesis name (SRFSyn1) -- Device name (ZYBO) -- Placement name (P1) -- Chip name (C50) -- \
num inputs (784) -- num outputs (32) -- MasterVecFilePrefix (challenges/SR_RFM_V4_MMCM_Random_Rise_1000Vs_Fall_1000Vs_NumSeeds_10_vecs) -- has_masks (0/1)\n", argv[0]); 
      exit(EXIT_FAILURE); 
      }

   strcpy(MasterDBname, argv[1]);
   strcpy(Netlist_name, argv[2]);
   strcpy(Synthesis_name, argv[3]);
   strcpy(Device_name, argv[4]);
   strcpy(Placement_name, argv[5]);
   strcpy(Chip_name, argv[6]);
   sscanf(argv[7], "%d", &num_PIs);
   sscanf(argv[8], "%d", &num_POs);
   strcpy(MasterVecPrefix, argv[9]);
   sscanf(argv[10], "%d", &has_masks);

   printf("Master DB '%s'\tNetlist name '%s'\tSynthesis name '%s'\tDevice name '%s'\tPlacement name '%s'\tChip name '%s'\n\tMasterVecPrefix '%s'\n\n", 
      MasterDBname, Netlist_name, Synthesis_name, Device_name, Placement_name, Chip_name, MasterVecPrefix); fflush(stdout);

// ====================================================== PARAMETERS ====================================================
// This is currently the bit position in the configuration vector that indicates rise and fall, with '1' indicating rise
// and 0 indicating fall.
   rise_fall_bit_pos = 15;

// Set this to 1 to create an in-memory version of the database. Memory limited operation but it at least 100 times faster.
   read_db_into_memory = 1;

// Names of the files that contain the complete list of vectors and masks used to generate the MasterDB. Names of the form 
// 'challenges/AES_MIXCOL_12_2016_AMSyn1_7010_AMS_Random_ATPG_Rise_702Vs_Fall_701Vs_NumSeeds_15_vecs'
   sprintf(MasterVecFile, "%s.txt", MasterVecPrefix);

// If 'has_masks' is true, then we typically don't want to read ALL the enrollment data into the database. Construct the mask file
// name. THIS IS A HACK -- NEEDS TO CHANGE. This allows a subset of the enrollment data to be enrolled but is specific to one
// chip. In the meantime, JUST enrollment EVERYTHING, e.g., set 'has_masks' in the command line to 0.
   strcpy(MasterMaskFile, MasterVecPrefix);
   if ( strlen(MasterMaskFile) >= 5 && strcmp(&(MasterMaskFile[strlen(MasterMaskFile) - 5]), "_vecs") == 0 )
      MasterMaskFile[strlen(MasterMaskFile) - 5] = '\0';
   strcat(MasterMaskFile, "_TVChar_optimalKEK_TVN_0.60_WID_1.10_qualifing_path_masks.txt");

printf("MasterVecFile\n\t'%s'\nMasterMaskFile USED ONLY if 'has_masks' %d is 1\n\t'%s'\n\n", MasterVecFile, has_masks, MasterMaskFile); fflush(stdout);
#ifdef DEBUG
#endif

//   DEBUG_FLAG = 0;
// ======================================================================================================================
// ======================================================================================================================
// Database is too big to read into memory? The only reason you would want to use this operation.
   if ( read_db_into_memory == 0 )
      {
      rc = sqlite3_open(MasterDBname, &db);
      if ( rc )
         { printf("ERROR: CANNOT open Master Database '%s': %s\n", MasterDBname, sqlite3_errmsg(db)); sqlite3_close(db); exit(EXIT_FAILURE); }
      }

// Using an in-memory version speeds this whole process up by about a factor of 100.
   else
      {

// Open an in-memory database.
      rc = sqlite3_open(":memory:", &db);

// Copy the disk version to memory for very fast access.
      printf("Reading filesystem database '%s' into memory!\n", MasterDBname); fflush(stdout);
      if ( LoadOrSaveDb(db, MasterDBname, 0) != 0 )
         { printf("Failed to open and copy into memory the Master Database: %s\n", sqlite3_errmsg(db)); sqlite3_close(db); exit(EXIT_FAILURE); }
      }

// Read the MasterVecFile 
   master_num_vec_pairs = ReadVectorAndASCIIMaskFiles(MAX_STRING_LEN, MasterVecFile, &master_num_rise_vec_pairs, &master_first_vecs_b, 
      &master_second_vecs_b, has_masks, MasterMaskFile, &master_masks, num_PIs, num_POs, rise_fall_bit_pos);
   printf("\n\tNumber of MASTER vectors read %d\tNumber of rising vectors %d\n", master_num_vec_pairs, master_num_rise_vec_pairs);

// ----------------------------------
// Get/create the PUFDesign index and PUFInstance index.
   GetCreatePUFDesignAndInstance(MAX_STRING_LEN, db, Netlist_name, Synthesis_name, Chip_name, Device_name, Placement_name, &design_index,
      &instance_index, num_PIs, num_POs);

// ----------------------------------
// Add master vectors to the Vectors table in the database, and then vector pairs to the VecPairs table, and then the timing data to the TimingVals
// table. It is common for this routine to issue constraint violations because vectors or vector pairs already exist in the tables. Timing values
// are deleted and then re-added if they already exist b/c the user must first decide if the PUFInstance is to be preserved or replaced. This is
// accomplished because a foreign key and ON DELETE CASCADE is set on the TimingVals database to the PUFInstance table.
   ProcessMasterVecs(MAX_STRING_LEN, db, design_index, instance_index, master_num_vec_pairs, master_num_rise_vec_pairs, master_first_vecs_b, 
      master_second_vecs_b, num_PIs, num_POs);

// If we read the database into memory, and updated it with data then we need to store it back.
   if ( read_db_into_memory == 1 )
      {
      printf("Saving 'in memory' database back to filesystem '%s'\n", MasterDBname); fflush(stdout);
      if ( LoadOrSaveDb(db, MasterDBname, 1) != 0 )
         { printf("Failed to store 'in memory' database to %s: %s\n", MasterDBname, sqlite3_errmsg(db)); sqlite3_close(db); exit(EXIT_FAILURE); }
      }

   sqlite3_close(db);

   return 0;
   }
