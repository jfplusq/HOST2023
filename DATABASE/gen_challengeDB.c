// ===========================================================================================================
// ===========================================================================================================
// ****************************************** gen_challengeDB.c **********************************************
// ===========================================================================================================
// ===========================================================================================================
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

// ===========================================================================================================
// ===========================================================================================================
// ===========================================================================================================
// This code generates additional, randomly selected challenge sets from special challenges added by add_challengeDB
// that use '0', '1', 'u' and 'q' designators. '0' means output has no transition, '1' means we MUST use that path,
// 'u' (unqualifying) means the path has a transition but is not 'compatible' with the elements that are marked 'q' 
// (for qualifying).

// In this routine, we assume the challenge set specified on the command line exists already in the database (added
// by add_challengeDB.c). It MUST be a challenge set that has 'u' and 'q' designators otherwise it is fully 
// specified by add_challengeDB.c and there is nothing we can do here to create additional challenges.

char Netlist_name[MAX_STRING_LEN];
char Synthesis_name[MAX_STRING_LEN];

char MasterDBname[MAX_STRING_LEN];
char ChallengeSetName[MAX_STRING_LEN];

char outfile_dir[MAX_STRING_LEN];
char outfile_vecs[MAX_STRING_LEN];
char outfile_masks[MAX_STRING_LEN];

// ------------------------------------------
int main(int argc, char *argv[])
   {
   sqlite3 *db;
   int rc;

   unsigned int Seed;
   int save_vecs_masks;

   unsigned char **vecs1_bin, **vecs2_bin, **masks_bin;
   VecPairPOStruct *challenge_vecpair_id_PO;
   int num_challenge_vecpair_id_PO;

   int num_vecs_masks;
   int num_rise_vecs_masks;

   int design_index;
   int num_PIs, num_POs; 

   static pthread_mutex_t GenChallenge_mutex = PTHREAD_MUTEX_INITIALIZER;

// ===============================================================================
   if( argc != 7 )
      { 
      printf("ERROR: %s: Master Database (Master.db) -- Netlist name (AES_MIXCOL_12_2016) -- Synthesis name (AMSyn1) -- challenge set name (Master1_OptKEK_TVN_0.60_WID_1.10) -- Save vecs/masks (0/1) -- Seed (n)\n", argv[0]); 
      exit(EXIT_FAILURE); 
      }

   strcpy(MasterDBname, argv[1]);
   strcpy(Netlist_name, argv[2]);
   strcpy(Synthesis_name, argv[3]);
   strcpy(ChallengeSetName, argv[4]);
   sscanf(argv[5], "%u", &save_vecs_masks);
   sscanf(argv[6], "%u", &Seed);

   printf("\nMain(): \tMaster DB '%s'\tNetlist name '%s'\tSynthesis name '%s'\tChallenge set name '%s'\tSeed %d\n\n", 
      MasterDBname, Netlist_name, Synthesis_name, ChallengeSetName, Seed); fflush(stdout);

// ===========================================================================================================
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! PARAMETERS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ===========================================================================================================

// Directory that the generated challenge are written to when 'save_vecs_masks' is 1.
   strcpy(outfile_dir, "challenges/generated/");

// ===========================================================================================================
// Open up the master database. 
   rc = sqlite3_open(MasterDBname, &db);
   if ( rc )
      { printf("ERROR: CANNOT open Master Database '%s': %s\n", MasterDBname, sqlite3_errmsg(db)); sqlite3_close(db); exit(EXIT_FAILURE); }

   outfile_vecs[0] = '\0';
   outfile_masks[0] = '\0';
   if ( save_vecs_masks == 1 )
      {
      sprintf(outfile_vecs, "%s%s_%s_%s_Seed_%d_vecs.txt", outfile_dir, Netlist_name, Synthesis_name, ChallengeSetName, Seed);
      sprintf(outfile_masks, "%s%s_%s_%s_Seed_%d_masks.txt", outfile_dir, Netlist_name, Synthesis_name, ChallengeSetName, Seed);
      }

// Get the PUF design characteristics. Right now, this is the design_index, num_PIs and num_POs.
   if ( GetPUFDesignParams(MAX_STRING_LEN, db, Netlist_name, Synthesis_name, &design_index, &num_PIs, &num_POs) != 0 )
      { printf("ERROR: PUFDesign index NOT found for '%s', '%s'!\n", Netlist_name, Synthesis_name); exit(EXIT_FAILURE); }

   printf("\nPUFDesign index %d\tNum PIs %d\tNum POs %d\n", design_index, num_PIs, num_POs); fflush(stdout);

// Generate the challenge and write it to a file (if you don't in this routine, then nothing is done except error checks).
// Note we don't need the last three parameters in this code. THESE MUST BE FREED WHEN DONE.
   GenChallengeDB(MAX_STRING_LEN, db, design_index, ChallengeSetName, Seed, save_vecs_masks, outfile_vecs, outfile_masks, &vecs1_bin, 
      &vecs2_bin, &masks_bin, &num_vecs_masks, &num_rise_vecs_masks, &GenChallenge_mutex, &num_challenge_vecpair_id_PO, 
      &challenge_vecpair_id_PO);

// DEBUG
if ( 0 )
{
int PO_num, PI_num, i;
FILE *OUTFILE;
char vec1[num_PIs + 1];
char vec2[num_PIs + 1];
char mask[num_POs + 1];

if ( (OUTFILE = fopen("vecs.txt", "w")) == NULL )
   { printf("ERROR: Data file vecs.txt open failed for writing!\n"); exit(EXIT_FAILURE); }
for ( i = 0; i < num_vecs_masks; i++ )
   {
   for ( PI_num = 0; PI_num < num_PIs; PI_num++ )
      {
      vec1[num_PIs - PI_num - 1] = (GetBitFromByte((vecs1_bin)[i][PI_num/8], PI_num % 8) == 1 ) ? '1' : '0';
      vec2[num_PIs - PI_num - 1] = (GetBitFromByte((vecs2_bin)[i][PI_num/8], PI_num % 8) == 1 ) ? '1' : '0';
      }
   vec1[num_PIs] = '\0';
   vec2[num_PIs] = '\0';
   fprintf(OUTFILE, "%s\n%s\n\n", vec1, vec2);
   }
fclose(OUTFILE);

if ( (OUTFILE = fopen("masks.txt", "w")) == NULL )
   { printf("ERROR: Data file masks.txt open failed for writing!\n"); exit(EXIT_FAILURE); }
for ( i = 0; i < num_vecs_masks; i++ )
   {
   for ( PO_num = 0; PO_num < num_POs; PO_num++ )
      mask[num_PIs - PO_num - 1] = (GetBitFromByte((masks_bin)[i][PO_num/8], PO_num % 8) == 1 ) ? '1' : '0';
   mask[num_POs] = '\0';
   fprintf(OUTFILE, "%s\n", mask);
   }
fclose(OUTFILE);
}

// DEBUG
printf("\n\nVecPair and PO informaton associated with newly created challenge:\n\t");
int j;
for ( j = 0; j < NUM_REQUIRED_PNS; j++ )
   {
   if ( j != 0 && (j % 8) == 0 )
      printf("\n\t");
   printf("(VP: %5d, PO %2d)  ", challenge_vecpair_id_PO[j].vecpair_id, challenge_vecpair_id_PO[j].PO_num);
   }
printf("\n");

   return 0;
   }
