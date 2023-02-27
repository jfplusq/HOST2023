// ========================================================================================================
// ========================================================================================================
// ********************************************** enrollDB.c **********************************************
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
// ========================================================================================================

unsigned char **master_first_vecs_b;
unsigned char **master_second_vecs_b;
char **master_masks;


char Netlist_name[MAX_STRING_LEN];
char Chip_name[MAX_STRING_LEN];
char Device_name[MAX_STRING_LEN];
char Synthesis_name[MAX_STRING_LEN];
char Placement_name[MAX_STRING_LEN];
char ChipEnrollDatafile[MAX_STRING_LEN];

char MasterDBname[MAX_STRING_LEN];

char MasterVecPrefix[MAX_STRING_LEN];
char MasterVecFile[MAX_STRING_LEN];
char MasterMaskFile[MAX_STRING_LEN];


int main(int argc, char **argv)
   {
   int master_num_vec_pairs, master_num_rise_vec_pairs;

   int *rise_fall, *vec_pairs, *POs;
   float *PNX, *PNX_Tsig;
   int num_PNX, num_PNR;
   int has_masks;

   sqlite3 *db;
   int rc;

   int design_index;
   int instance_index;

   int num_PIs, num_POs;
   int read_db_into_memory;

   int rise_fall_bit_pos;

   int DEBUG_FLAG;

// ===============================================================================
   if ( argc != 12 )
      { 
      printf("ERROR: %s: Master Database (NAT_Master.db) -- Netlist name (SR_RFM_V4_MMCM) -- Synthesis name (SRFSyn1) -- Device name (ZYBO) -- Placement name (P1) -- Chip name (C50) -- \
num inputs (784) -- num outputs (32) -- MasterVecFilePrefix (challenges/SR_RFM_V4_MMCM_Random_Rise_1000Vs_Fall_1000Vs_NumSeeds_10_vecs) -- has_masks (0/1) -- \
Enroll path file (/borg_data/FPGAs/ZYBO/SR_RFM/ANALYSIS/data/7_19_2021/C50_SR_RFM_V4_MMCM_P1_25C_1.00V_NCs_2000_E_PUFNums.txt)\n", argv[0]); 
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
   strcpy(ChipEnrollDatafile, argv[11]);

   printf("Master DB '%s'\tNetlist name '%s'\tSynthesis name '%s'\tDevice name '%s'\tPlacement name '%s'\tChip name '%s'\n\tMasterVecPrefix '%s'\n\tChip enroll datafile '%s'\n\n", 
      MasterDBname, Netlist_name, Synthesis_name, Device_name, Placement_name, Chip_name, MasterVecPrefix, ChipEnrollDatafile); fflush(stdout);

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

   DEBUG_FLAG = 0;
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

// Read timing data from Chip's enrollment data file. Timing data has a blank line that separates the rising and falling PN.
   num_PNR = ReadChipEnrollPNs(MAX_STRING_LEN, ChipEnrollDatafile, &PNX, &PNX_Tsig, &rise_fall, &vec_pairs, &POs, &num_PNX, 
      num_POs, has_masks, master_num_vec_pairs, master_masks, DEBUG_FLAG);
   printf("\n\tNumber of PNR read %d\tNumber of PNF read %d\n", num_PNR, num_PNX - num_PNR);


#ifdef DEBUG
int i;
for ( i = 0; i < 10; i++ )
   printf("PNR %d\tM: %f\tTSig: %f\n", i, PNX[i], PNX_Tsig[i]);
for ( i = num_PNR; i < num_PNR + 10; i++ )
   printf("PNF %d\tM: %f\tTSig: %f\n", i, PNX[i], PNX_Tsig[i]);
fflush(stdout);
#endif

// ----------------------------------
// Get/create the PUFDesign index and PUFInstance index.
   GetCreatePUFDesignAndInstance(MAX_STRING_LEN, db, Netlist_name, Synthesis_name, Chip_name, Device_name, Placement_name, &design_index,
      &instance_index, num_PIs, num_POs);

// ----------------------------------
// Add master vectors to the Vectors table in the database, and then vector pairs to the VecPairs table, and then the timing data to the TimingVals
// table. It is common for this routine to issue constraint violations because vectors or vector pairs already exist in the tables. Timing values
// are deleted and then re-added if they already exist b/c the user must first decide if the PUFInstance is to be preserved or replaced. This is
// accomplished because a foreign key and ON DELETE CASCADE is set on the TimingVals database to the PUFInstance table.
   ProcessMasterVecsAndTimingData(MAX_STRING_LEN, db, design_index, instance_index, master_num_vec_pairs, master_num_rise_vec_pairs, master_first_vecs_b, 
      master_second_vecs_b, PNX, PNX_Tsig, rise_fall, vec_pairs, POs, num_PNR, num_PNX, num_PIs, num_POs);

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
