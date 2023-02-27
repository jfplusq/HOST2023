// ========================================================================================================
// ========================================================================================================
// ************************************ gen_random_enroll_data.c ******************************************
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

// This routine is used to populate the HELP PUF data with PUFInstances that have randomly generated data.
// It is used to increase the size of the data to test performance issues. The random data is generated
// by first computing the Median values from the existing data in the database and then randomly adding
// offsets to the data to make it look like a different chip.

#include "commonDB.h"


// ===========================================================================================================
// ===========================================================================================================
// Compute the Median values for each PNs across all chips. This routine assumes the user has enabled the 
// TimingVals cache and the relevant PNs are in an array.

void ComputeEnrollmentMedianPUFNumsTVC(int num_chips, int num_qualified_PNs, TimingValCacheStruct *TVC_arr, 
   float **MedianPNR_ptr, float **MedianPNF_ptr, int *num_qualified_rise_PNs_ptr, int *num_qualified_fall_PNs_ptr)
   {
   int PN_num, num_rise_PNs, num_fall_PNs;
   float float_val; 

// Force realloc to behave like malloc on the first call.
   *MedianPNR_ptr = NULL;
   *MedianPNF_ptr = NULL;
   num_rise_PNs = 0;
   num_fall_PNs = 0;
   for ( PN_num = 0; PN_num < num_qualified_PNs; PN_num++ )
      {

// Compute the Median value of the PN using data from all chips.
      float_val = ComputeMedian(num_chips, TVC_arr[PN_num].PNs);

// Rising PNs
      if ( TVC_arr[PN_num].rise_or_fall == 0 )
         {
         if ( (*MedianPNR_ptr = (float *)realloc(*MedianPNR_ptr, sizeof(float) * (num_rise_PNs + 1))) == NULL )
            { printf("ERROR: ComputeEnrollmentMedianPUFNumsTVC(): Failed to allocate storagefor *MedianPNR_ptr!\n"); exit(EXIT_FAILURE); }
         (*MedianPNR_ptr)[num_rise_PNs] = float_val;
         num_rise_PNs++;
         }

// Falling PNs
      else
         {
         if ( (*MedianPNF_ptr = (float *)realloc(*MedianPNF_ptr, sizeof(float) * (num_fall_PNs + 1))) == NULL )
            { printf("ERROR: ComputeEnrollmentMedianPUFNumsTVC(): Failed to allocate storagefor *MedianPNF_ptr!\n"); exit(EXIT_FAILURE); }
         (*MedianPNF_ptr)[num_fall_PNs] = float_val;
         num_fall_PNs++;
         }
      }

   *num_qualified_rise_PNs_ptr = num_rise_PNs;
   *num_qualified_fall_PNs_ptr = num_fall_PNs;
   return;
   }


// ============================================================================
// ============================================================================

unsigned char **master_first_vecs_b;
unsigned char **master_second_vecs_b;
char **master_masks;

char MasterDB_name[MAX_STRING_LEN];
char Netlist_name[MAX_STRING_LEN];
char Synthesis_name[MAX_STRING_LEN];
char ChallengeSetName[MAX_STRING_LEN];

char Chip_name[MAX_STRING_LEN];
char Device_name[MAX_STRING_LEN];
char Placement_name[MAX_STRING_LEN];
char SampleChipEnrollDatafile[MAX_STRING_LEN];
char MasterVecFile[MAX_STRING_LEN];
char MasterMaskFile[MAX_STRING_LEN];

int main(int argc, char *argv[])
   {
   sqlite3 *db = NULL;
   int rc;

   int design_index1, design_index2;
   int num_PIs, num_POs;
   int instance_index;

   float *MedianPNR = NULL;
   float *MedianPNF = NULL;
   int num_qualified_rise_PNs;
   int num_qualified_fall_PNs;

   int num_chips;

   TimingValCacheStruct *TVC_arr = NULL;
   int num_TVC_arr;

   unsigned int within_die_variation_estimate;
   unsigned int Seed;

   int chip_num, PN_num;

   int master_num_vec_pairs, master_num_rise_vec_pairs;
   int *rise_fall, *vec_pairs, *POs;
   float *PNX, *PNX_Tsig;
   int num_PNX, num_PNR;
   int has_masks;

   int PUFInstance, PUFInstance_start;
   int num_PUFInstances_to_add;

   int read_db_into_memory;

   int rise_fall_bit_pos;

   int DEBUG;

// ===============================================================================
   if ( argc != 5 )
      { 
      printf("ERROR: gen_random_enroll_data: Master Database (MasterWithRandom.db) -- Seed (n) -- num_PUFInstances_to_add (n) -- Start PUFInstance index (0/100/200)\n"); 
      exit(EXIT_FAILURE); 
      }

   strcpy(MasterDB_name, argv[1]);
   sscanf(argv[2], "%u", &Seed);
   sscanf(argv[3], "%d", &num_PUFInstances_to_add);
   sscanf(argv[4], "%d", &PUFInstance_start); 

// ======================================================================================================================
// ======================================================================================================================
// This is currently the bit position in the configuration vector that indicates rise and fall, with '1' indicating rise
// and 0 indicating fall.
   rise_fall_bit_pos = 15;

// Set this to 1 to create an in-memory version of the database. Memory limited operation but it at least 100 times faster.
   read_db_into_memory = 1;

// Set this to an estimate of the within-die variations that exist in the chip population. For example, for the 500 chip 
// ZED experiment, it is about 24 (plus and minus 12). Random value around the mean values computed below will be generated
// and added or subtracted from the mean to generate the data for a new PUF instance. ALWAYS MAKE THIS AN ODD NUMBER.
   within_die_variation_estimate = 25;

// For reading out and computing the median for existing data that exists in the database. Need to parameterize this to make it generic.
   strcpy(Netlist_name, "SR_RFM_V4_MMCM");
   strcpy(Synthesis_name, "SRFSyn1");

// For reading dummy information so we can call routines enrollDB.c calls.
   strcpy(Device_name, "7020");
   strcpy(Placement_name, "V1");
// *** VEC_CHLNG ***
   num_PIs = 392;
   num_POs = 32;

// FULL BLOWN 50K database
//   strcpy(ChallengeSetName, "ChngALL");
//   has_masks = 0;
//   strcpy(MasterVecFile, "challenges/SR_RFM_V4_MMCM_Random_Rise_1000Vs_Fall_1000Vs_NumSeeds_10_vecs.txt");
//   strcpy(MasterMaskFile, "");
//   strcpy(SampleChipEnrollDatafile, "/borg_data/FPGAs/ZED/SR_RFM/ANALYSIS/data/6_20_2021/C2_SR_RFM_V4_MMCM_P1_25C_1.00V_NCs_2000_E_PUFNums.txt");

// REDUCED SIZE 6639 database
   strcpy(ChallengeSetName, "Master1_OptKEK_TVN_0.70_WID_1.20");
   has_masks = 1;
   strcpy(MasterVecFile, "challenges/SR_RFM_V4_MMCM_Random_Rise_1000Vs_Fall_1000Vs_NumSeeds_10_vecs.txt");
   strcpy(MasterMaskFile, "challenges/optKEK_qualifing_path_TVN_0.70_WID_1.20_SetSize_64000_masks.txt");
   strcpy(SampleChipEnrollDatafile, "/borg_data/FPGAs/ZED/SR_RFM/ANALYSIS/data/6_20_2021/C2_SR_RFM_V4_MMCM_P1_25C_1.00V_NCs_2000_E_PUFNums.txt");

   DEBUG = 0;
// ======================================================================================================================
// Database is too big to read into memory? The only reason you would want to use this operation.
   if ( read_db_into_memory == 0 )
      {
   
// Open up the master database. 
      rc = sqlite3_open(MasterDB_name, &db);
      if ( rc != 0 )
         { printf("Failed to open Master Database: %s\n", sqlite3_errmsg(db)); sqlite3_close(db); exit(EXIT_FAILURE); }
      }

// Using an in-memory version speeds this whole process up by about a factor of 100.
   else
      {

// Open an in-memory database.
      rc = sqlite3_open(":memory:", &db);

// Copy the disk version to memory for very fast access.
      printf("Reading filesystem database '%s' into memory!\n", MasterDB_name); fflush(stdout);
      if ( LoadOrSaveDb(db, MasterDB_name, 0) != 0 )
         { printf("Failed to open and copy into memory the Master Database: %s\n", sqlite3_errmsg(db)); sqlite3_close(db); exit(EXIT_FAILURE); }
      }

// --------------------------------------------------------------------------------------------
// Get the PUFDesign parameters.
   if ( GetPUFDesignParams(MAX_STRING_LEN, db, Netlist_name, Synthesis_name, &design_index1, &num_PIs, &num_POs) != 0 )
      { printf("ERROR: PUFDesign index NOT found for '%s', '%s'!\n", Netlist_name, Synthesis_name); exit(EXIT_FAILURE); }

// --------------------------------------------------------------------------------------------
// Create a cache and then compute the Median values for the PNs in the existing database. Not needed when read_db_into_memory is 1 but it's
// easier this way. ONLY USING THE Cx chip names in the existing database to compute the Median values. Use '%' like '*' and '_' like '?'
   num_chips = CreateTimingValsCacheFromChallengeSet(MAX_STRING_LEN, db, design_index1, ChallengeSetName, "C%", &TVC_arr, &num_TVC_arr);

// Compute the Median values across all chips. These will serve as the average values to randomize. 
   printf("\n\nComputing Median values using %d chips\n", num_chips); fflush(stdout);
   ComputeEnrollmentMedianPUFNumsTVC(num_chips, num_TVC_arr, TVC_arr, &MedianPNR, &MedianPNF, &num_qualified_rise_PNs, &num_qualified_fall_PNs);

   for ( PN_num = 0; PN_num < num_qualified_rise_PNs; PN_num++ )
      printf("Qualified RISE PN %d\tMedian PN %f\n", PN_num, MedianPNR[PN_num]);
   for ( PN_num = 0; PN_num < num_qualified_fall_PNs; PN_num++ )
      printf("Qualified FALL PN %d\tMedian PN %f\n", PN_num, MedianPNF[PN_num]);

   printf("Num Qualified RISE PNs %d\tNum Qualified FALL PNs %d\n\n", num_qualified_rise_PNs, num_qualified_fall_PNs); fflush(stdout);

// Free up the cache -- we are done with it after computing the Median values.
   if ( TVC_arr != NULL )
      {
      for ( chip_num = 0; chip_num < num_chips; chip_num++ )
         if ( TVC_arr[chip_num].PNs != NULL ) 
            free(TVC_arr[chip_num].PNs);
      free(TVC_arr);
      }

// --------------------------------------------------------------------------------------------
// We need to add instances to the database. Easiest thing to do is follow what's done in enrollDB.c -- we'll read in an existing
// file as a dummy to get the ancillery information, and then call the enrollment routine.

// Read the MasterVecFile 
   master_num_vec_pairs = ReadVectorAndASCIIMaskFiles(MAX_STRING_LEN, MasterVecFile, &master_num_rise_vec_pairs, &master_first_vecs_b, 
      &master_second_vecs_b, has_masks, MasterMaskFile, &master_masks, num_PIs, num_POs, rise_fall_bit_pos);
   printf("\n\tNumber of MASTER vectors read %d\tNumber of rising vectors %d\n", master_num_vec_pairs, master_num_rise_vec_pairs);

// Read timing data from Chip's enrollment data file. Timing data has a blank line that separates the rising and falling PN.
   num_PNR = ReadChipEnrollPNs(MAX_STRING_LEN, SampleChipEnrollDatafile, &PNX, &PNX_Tsig, &rise_fall, &vec_pairs, &POs, &num_PNX, 
      num_POs, has_masks, master_num_vec_pairs, master_masks, DEBUG);
   printf("\n\tNumber of PNR read %d\tNumber of PNF read %d\n", num_PNR, num_PNX - num_PNR);

// Sanity check. Number of values read for the challenge set must equal number that's in the dummy file.
   if ( num_PNX != num_qualified_rise_PNs + num_qualified_fall_PNs )
      { 
      printf("ERROR: Number of values that exist in the database for the design index %d MUST equal number in the dummy enrollment file %d\n", 
         num_PNX, num_qualified_rise_PNs + num_qualified_fall_PNs);
      exit(EXIT_FAILURE);
      }

// --------------------------------------------------------------------------------------------
   srand(Seed);

   for ( PUFInstance = PUFInstance_start; PUFInstance < num_PUFInstances_to_add + PUFInstance_start; PUFInstance++ )
      {

// Generate a random integer in the +/- the 1/2 the range process variations and add it to the Median values.
      int temp_rand, PN_tracker;
      PN_tracker = 0;
      for ( PN_num = 0; PN_num < num_qualified_rise_PNs; PN_num++ )
         {
         temp_rand = (int)(rand() % within_die_variation_estimate) - (int)(within_die_variation_estimate - 1)/2;

// Overwrite PNX values read above for the dummy file. 
         PNX[PN_tracker] = MedianPNR[PN_num] + (float)temp_rand;

#ifdef DEBUG
printf("%d) Median val %f\tRandom val add to RISE Median %2d\tFinal val %f\n", PN_num, MedianPNR[PN_num], temp_rand, PNX[PN_tracker]);
#endif
         PN_tracker++;
         }
      for ( PN_num = 0; PN_num < num_qualified_fall_PNs; PN_num++ )
         {
         temp_rand = (int)(rand() % within_die_variation_estimate) - (int)(within_die_variation_estimate - 1)/2;

// Overwrite PNX values read above for the dummy file. 
         PNX[PN_tracker] = MedianPNF[PN_num] + temp_rand;

#ifdef DEBUG
printf("%d) Median val %f\tRandom val add to FALL Median %2d\tFinal val %f\n", PN_num, MedianPNF[PN_num], temp_rand, PNX[PN_tracker]);
#endif

         PN_tracker++;
         }

      sprintf(Chip_name, "R%d", PUFInstance);

// Add the PUFInstance, checking for duplicate names. Give user ability to overwrite data.
      GetCreatePUFDesignAndInstance(MAX_STRING_LEN, db, Netlist_name, Synthesis_name, Chip_name, Device_name, Placement_name, &design_index2,
         &instance_index, num_PIs, num_POs);

// Sanity check
      if ( design_index1 != design_index2 )
         { printf("ERROR: design index 1 %d MUST match design index 2 %d\n", design_index1, design_index2); exit(EXIT_FAILURE); }

      printf("New instance index %d\n", instance_index); fflush(stdout);

      ProcessMasterVecsAndTimingData(MAX_STRING_LEN, db, design_index2, instance_index, master_num_vec_pairs, master_num_rise_vec_pairs, master_first_vecs_b, 
         master_second_vecs_b, PNX, PNX_Tsig, rise_fall, vec_pairs, POs, num_PNR, num_PNX, num_PIs, num_POs);
      }

// If we read the database into memory, and updated it with data then we need to store it back.
   if ( read_db_into_memory == 1 )
      {
      printf("Saving 'in memory' database back to filesystem '%s'\n", MasterDB_name); fflush(stdout);
      if ( LoadOrSaveDb(db, MasterDB_name, 1) != 0 )
         { printf("Failed to store 'in memory' database to %s: %s\n", MasterDB_name, sqlite3_errmsg(db)); sqlite3_close(db); exit(EXIT_FAILURE); }
      }

   sqlite3_close(db);

   return 0;
   }
