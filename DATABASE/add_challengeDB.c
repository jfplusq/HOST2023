// ========================================================================================================
// ========================================================================================================
// **************************************** add_challengeDB.c *********************************************
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
// Add masks to the PathSelectMasks table, and then the Challenges. As challenges are added, the ChallengeVecPairs 
// table is updated with a pointer to the VecPair element (which MUST exist) and the PSM just created. It 
// is common for this routine to issue constraint violations because masks already exist. 

void ProcessChallenges(int max_string_len, sqlite3 *db, int design_index, char *ChallengeSetName, 
   int num_PIs, int num_POs, int challenge_num_vecpairs, int challenge_num_rise_vecpairs, 
   unsigned char **challenge_first_vecs_b, unsigned char **challenge_second_vecs_b, char **challenge_masks,
   int do_timing_val_check)
   {
   int first_vec_index, second_vec_index; 
   int vecpair_index, PSM_index;
   int challenge_vecpair_index;
   int challenge_index;
   int vecpair_num;

   int num_PNs_per_vecpair;
   int tot_PNs, tot_rise_PNs;
   int tot_vecs, tot_rise_vecs;
   int vec_len_bytes = num_PIs/8; 

   int num_no_trans, num_hard_selected, num_unqual_selected, num_qual_selected;

printf("\n\nPUFDesign %d: Adding Challenge Set '%s'\n", design_index, ChallengeSetName); fflush(stdout);
#ifdef DEBUG
#endif

// First thing to do is to create the challenge (if it does not already exist). Note, we will update NumVecs, NumRiseVecs, NumPNs and 
// NumRisePNs once we processed the vector pairs below.
   InsertIntoTable(max_string_len, db, "Challenges", SQL_Challenges_insert_into_cmd, NULL, 0, ChallengeSetName, NULL, NULL, NULL, NULL, 
      -1, -1, -1, -1, design_index, -1.0, -1.0);

// Get the index for this challenge. 
   if ( (challenge_index = GetIndexFromTable(max_string_len, db, "Challenges", SQL_Challenges_get_index_cmd, NULL, 0, ChallengeSetName, 
      NULL, NULL, NULL, -1, -1, -1)) == -1 )
      {
      printf("ERROR: ProcessChallenge(): Failed to find challenge index for ChallengeSetName '%s' in Challenge table!\n", ChallengeSetName); 
      exit(EXIT_FAILURE); 
      }

#ifdef DEBUG
printf("\nCreated/found challenge_index %d!\n", challenge_index); fflush(stdout);
#endif

// -----------------------------------
// For each vector pair, add an entry to the ChallengeVecPairs table.
   tot_PNs = 0;
   tot_rise_PNs = 0;
   tot_vecs = 0;
   tot_rise_vecs = 0;
   for ( vecpair_num = 0; vecpair_num < challenge_num_vecpairs; vecpair_num++ )
      {
printf("\nVecpair %d\n", vecpair_num); fflush(stdout);
#ifdef DEBUG
#endif

// Find the vector pair in the Vector's table. If not present, error. WE MUST HAVE run enrollDB.c that recorded these vectors AND 
// timing data BEFORE this routine will succeed. And then get the vecpair_index from the VecPairs table keying on the combination 
// of two Vector id's (VecPairs does NOT allow duplicates for the combination (VA, VB)).
      GetVectorAndVecPairIndexesForBinaryVectors(max_string_len, db, design_index, vec_len_bytes, challenge_first_vecs_b[vecpair_num], 
         challenge_second_vecs_b[vecpair_num], &first_vec_index, &second_vec_index, &vecpair_index, vecpair_num);

printf("\tFirst Vector Index %d\tSecond Vector Index %d and corresponding VecPair index %d\n", first_vec_index, second_vec_index, 
   vecpair_index); fflush(stdout);
#ifdef DEBUG
#endif

// Get the total number of PNs tested by this VecPair WITHOUT considering a mask.
      num_PNs_per_vecpair = GetVecPairsNumPNsField(max_string_len, db, vecpair_index);

// Analyze the mask to determine the number of '0', '1', 'u' and 'q's for adding to the total tested PNs.
      DetermineNumSelectedPOsInMask(challenge_masks[vecpair_num], num_POs, &num_no_trans, &num_hard_selected, &num_unqual_selected, 
         &num_qual_selected);

// Sanity check. A more elaborate check is done below. 9/30/2018: Creating smaller databases now with just qualified paths.
//      if ( num_hard_selected + num_unqual_selected + num_qual_selected > num_PNs_per_vecpair )
      if ( num_hard_selected + num_qual_selected > num_PNs_per_vecpair )
         {
         printf("ERROR: ProcessChallenges(): Mask '%s' identifies more tested PNs %d than available %d by vecpair %d!\n", 
            challenge_masks[vecpair_num], num_hard_selected + num_unqual_selected + num_qual_selected, num_PNs_per_vecpair, vecpair_num);
         exit(EXIT_FAILURE); 
         }

// Right now, I don't allow hard selected paths '1's (fixed vector/masks) with 'u's and 'q's (compatibility vector/masks) but this may change.
      if ( num_hard_selected != 0 && num_unqual_selected + num_qual_selected != 0 )
         {
         printf("ERROR: ProcessChallenges(): Currently encoding mask '%s' with BOTH '1's %d and 'u's/'q's %d NOT supported!\n", 
            challenge_masks[vecpair_num], num_hard_selected, num_unqual_selected + num_qual_selected); 
         exit(EXIT_FAILURE); 
         }

// Sanity check. Right now, if the number of 'u's and 'q's that appear in the mask is not equal to the number of PNs tested by the vecpair, then error.
// 9/30/2018: Allow qualified == num_PNs (don't require 'unqualified' PNs to be in the database).
//      if ( num_unqual_selected + num_qual_selected > 0 && num_unqual_selected + num_qual_selected != num_PNs_per_vecpair )
      if ( num_unqual_selected + num_qual_selected > 0 && num_qual_selected > num_PNs_per_vecpair )
         {
         printf("ERROR: ProcessChallenges(): Mask '%s' identifies more tested PNs %d than available %d by vecpair %d!\n", 
            challenge_masks[vecpair_num], num_hard_selected + num_unqual_selected + num_qual_selected, num_PNs_per_vecpair, vecpair_num);
         exit(EXIT_FAILURE); 
         }

// For compatibility masks, if the number of 'q's is zero, skip this vector -- do NOT add it to the Challenge record.
      if ( num_unqual_selected + num_qual_selected > 0 && num_qual_selected == 0 )
         {
         printf("INFO: ProcessChallenges(): Mask '%s' does NOT identify any qualified PNs. Vecpair %d will NOT be added to Challenge!\n", 
            challenge_masks[vecpair_num], vecpair_num);
         fflush(stdout); 
         continue;
         }

// Returns 0 if 'R' or 1 if 'F'.
      if ( GetVecPairsRiseFallStrField(max_string_len, db, vecpair_index) == 0 )
         {
         tot_rise_vecs++;
         tot_rise_PNs += num_hard_selected + num_qual_selected;
         }
      tot_vecs++; 
      tot_PNs += num_hard_selected + num_qual_selected;

printf("\tRising Vector Pair NumPNs %d\tRunning TotRisePNs %d\tTotPNs %d\n", num_PNs_per_vecpair, tot_rise_PNs, tot_PNs); fflush(stdout);
#ifdef DEBUG
#endif

// Check that this PathSelectMask is consistent with the TimingVals data. For every '1', 'u' or 'q' in the mask, we MUST find a timing value
// in the set associated with the VecPair that has a PO number that corresponds to this position in the mask. 
// 10_25_2021: Added flag to disable this. Trying to create a Customer Database that has ONLY challenge vectors -- NO TIMING VALUES.
      if ( do_timing_val_check == 1 )
         {
         CheckMaskIsConsistentWithVecPairTimingVals(max_string_len, db, vecpair_index, challenge_masks[vecpair_num], num_POs);

printf("\tConsistency check of PathSelectMask with stored enrollment TimingVals passed\n"); fflush(stdout);
#ifdef DEBUG
#endif
         }
      else
         {
         printf("\tConsistency check of PathSelectMask NOT PERFORMED! You should ENABLE THIS check if you are building a database WITH TIMING VALUES!\n"); 
         fflush(stdout);
         }

// Write the mask binary vector into the PathSelectMask table.
      InsertIntoTable(max_string_len, db, "PathSelectMasks", SQL_PathSelectMasks_insert_into_cmd, NULL, 0, challenge_masks[vecpair_num], NULL, NULL, 
         NULL, NULL, -1, -1, -1, -1, -1, -1.0, -1.0);

// Get the PSM_index.
      if ( (PSM_index = GetIndexFromTable(max_string_len, db, "PathSelectMasks", SQL_PathSelectMasks_get_index_cmd, NULL, 0, challenge_masks[vecpair_num], 
         NULL, NULL, NULL, -1, -1, -1)) == -1 )
         {
         printf("ERROR: ProcessChallenges(): Failed to find vecpair_index for vecpair_num %d in VecPairs table!\n", vecpair_num); 
         exit(EXIT_FAILURE); 
         }

printf("\tWrote/Found mask '%s' at index %d\n", challenge_masks[vecpair_num], PSM_index); fflush(stdout);
#ifdef DEBUG
#endif

      InsertIntoTable(max_string_len, db, "ChallengeVecPairs", SQL_ChallengeVecPairs_insert_into_cmd, NULL, 0, NULL, NULL, NULL, NULL, NULL, challenge_index, 
         vecpair_index, PSM_index, -1, -1, -1.0, -1.0);

// Just for fun, read it back.
      if ( (challenge_vecpair_index = GetIndexFromTable(max_string_len, db, "ChallengeVecPairs", SQL_ChallengeVecPairs_get_index_cmd, NULL, 0, NULL, NULL, 
         NULL, NULL, challenge_index, vecpair_index, PSM_index)) == -1 )
         {
         printf("ERROR: ProcessChallenge(): Failed to find challenge_vecpair index for vecpair, PSM, challenge_index %d, %d, %d in ChallengeVecPairs table!\n", 
            vecpair_index, PSM_index, challenge_index); 
         exit(EXIT_FAILURE); 
         }

printf("\tWrote/Found ChallengeVecPair %d\n", challenge_vecpair_index); fflush(stdout);
#ifdef DEBUG
#endif
      }

   UpdateChallengesNumVecFields(max_string_len, db, challenge_index, tot_vecs, tot_rise_vecs); 
   UpdateChallengesNumPNFields(max_string_len, db, challenge_index, tot_PNs, tot_rise_PNs); 

printf("\n\tUpdated Challenge Set '%s' NumVecs %d\tNumRiseVecs %d\tNumPNs %d\tNumRisePNs %d\n", ChallengeSetName, tot_vecs, tot_rise_vecs, tot_PNs, tot_rise_PNs); fflush(stdout);
printf("DONE\n\n"); fflush(stdout);
#ifdef DEBUG
#endif

   return;
   }


// ========================================================================================================
// ========================================================================================================
// ========================================================================================================

unsigned char **challenge_first_vecs_b;
unsigned char **challenge_second_vecs_b;
char **challenge_masks;

char Netlist_name[MAX_STRING_LEN];
char Synthesis_name[MAX_STRING_LEN];

char MasterDBname[MAX_STRING_LEN];

char ChallengeSetName[MAX_STRING_LEN];
char ChallengeVecFilePrefix[MAX_STRING_LEN];
char ChallengeMaskFilePrefix[MAX_STRING_LEN];

char ChallengeVecFile[MAX_STRING_LEN];
char ChallengeMaskFile[MAX_STRING_LEN];


int main(int argc, char **argv)
   {
   int challenge_num_vecpairs, challenge_num_rise_vecpairs;

   sqlite3 *db;
   int rc;

   int design_index;

   int num_PIs, num_POs;
   int rise_fall_bit_pos;

// 10_25_2021
   int do_timing_val_check;

//   int DEBUG;

// ===============================================================================
   if( argc != 8 )
      { 
      printf("ERROR: %s: Master Database (Master.db) -- Netlist name (SR_RFM_V4_MMCM) -- Synthesis name (SRFSyn1) -- challenge set name (ChngSet1) -- Challenge vec prefix (challenges/SR_RFM_V4_MMCM_Random_Rise_1000Vs_Fall_1000Vs_NumSeeds_10) -- Challenge mask prefix (challenges/optKEK_qualifing_path_TVN_0.70_WID_1.20_SetSize_64000) -- do_timing_val_check (0/1)\n", argv[0]); 
      exit(EXIT_FAILURE); 
      }

   strcpy(MasterDBname, argv[1]);
   strcpy(Netlist_name, argv[2]);
   strcpy(Synthesis_name, argv[3]);
   strcpy(ChallengeSetName, argv[4]);
   strcpy(ChallengeVecFilePrefix, argv[5]);
   strcpy(ChallengeMaskFilePrefix, argv[6]);
   sscanf(argv[7], "%d", &do_timing_val_check);

   printf("\n\tMaster DB '%s'\tNetlist name '%s'\tSynthesis name '%s'\tChallenge set name '%s'\n\t\tChallenge vector prefix '%s'\n\t\tChallenge mask prefix '%s'\tDo TimingVal Consistency check %d\n\n", 
      MasterDBname, Netlist_name, Synthesis_name, ChallengeSetName, ChallengeVecFilePrefix, ChallengeMaskFilePrefix, do_timing_val_check); fflush(stdout);

// ====================================================== PARAMETERS ====================================================
// This is currently the bit position in the configuration vector that indicates rise and fall, with '1' indicating rise
// and 0 indicating fall.
   rise_fall_bit_pos = 15;

// Names of the files that contain the challenge
   sprintf(ChallengeVecFile, "%s_vecs.txt", ChallengeVecFilePrefix);
   sprintf(ChallengeMaskFile, "%s_masks.txt", ChallengeMaskFilePrefix);

//   DEBUG = 1;

// ======================================================================================================================
// ======================================================================================================================
// Open up the master database. 
   rc = sqlite3_open(MasterDBname, &db);
   if ( rc )
      { printf("ERROR: CANNOT open Master Database '%s': %s\n", MasterDBname, sqlite3_errmsg(db)); sqlite3_close(db); exit(EXIT_FAILURE); }

// Get the PUF design characteristics. Right now, this is the design_index, num_PIs and num_POs.
   if ( GetPUFDesignParams(MAX_STRING_LEN, db, Netlist_name, Synthesis_name, &design_index, &num_PIs, &num_POs) != 0 )
      { printf("ERROR: PUFDesign index NOT found for '%s', '%s'!\n", Netlist_name, Synthesis_name); exit(EXIT_FAILURE); }

   printf("\nPUFDesign index %d\tRead num_PIs %d and num_POs %d\n", design_index, num_PIs, num_POs); fflush(stdout);

// Read the Challenge vectors and masks. Mask are ALWAYS present in add_challengesDB.c, in contrast to enrollDB.c, where the are (NEVER?) present. 
// When we process 'compatibility' challenges (those with 'u's and 'q's, TVCharacterization.c ALWAYS writes out ALL vectors and masks used during
// enrollment, even if TVCharacterization decided that some vectors do NOT have any compatible PNs. The new Challenges record will record ONLY those
// that test non-zero compatible paths. 
   challenge_first_vecs_b = challenge_second_vecs_b = NULL; 
   challenge_masks = NULL;
   challenge_num_vecpairs = ReadVectorAndASCIIMaskFiles(MAX_STRING_LEN, ChallengeVecFile, &challenge_num_rise_vecpairs, &challenge_first_vecs_b, 
      &challenge_second_vecs_b, 1, ChallengeMaskFile, &challenge_masks, num_PIs, num_POs, rise_fall_bit_pos);

   printf("\n\tNumber of Challenge vectors read %d\tNumber of rising vectors %d\tHas masks? %d\n", challenge_num_vecpairs, challenge_num_rise_vecpairs, 1);

// ----------------------------------
// Add masks to the PathSelectMasks table, and then the Challenges. As challenges are added, the ChallengeVecPairs table is updated with a pointer
// to the VecPair element (which MUST exist) and the PSM just created. It is common for this routine to issue constraint violations because masks
// already exist. 
   ProcessChallenges(MAX_STRING_LEN, db, design_index, ChallengeSetName, num_PIs, num_POs, challenge_num_vecpairs, challenge_num_rise_vecpairs, 
      challenge_first_vecs_b, challenge_second_vecs_b, challenge_masks, do_timing_val_check);

   sqlite3_close(db);

   return 0;
   }
