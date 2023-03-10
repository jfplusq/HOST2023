// ========================================================================================================
// ========================================================================================================
// ************************************* verifier_regen_funcs.c *******************************************
// ========================================================================================================
// ========================================================================================================
//
//--------------------------------------------------------------------------------
// Company: IC-Safety, LLC and University of New Mexico
// Engineer: Professor Jim Plusquellic
// Exclusive License: IC-Safety, LLC
// Copyright: Univ. of New Mexico
//--------------------------------------------------------------------------------

#include "common.h"
#include "verifier_common.h"
#include "verifier_regen_funcs.h"
#include "commonDB_RT.h"
#include <math.h>  

typedef struct
   {
   int index; 
   int NSB; 
   float NMM;
   float NMBF;
   float NTBF;
   float CC;
   } AuthenDataStruct;

// Set to -1 to disable
#define DO_DUMP_PN_DATA_CHIP_NUM -1
char *DumpDir = "../DumpData/";
int Chlng_cnter = 0;


// ===========================================================================================================
// ===========================================================================================================
// Sort ADS in ascending order.

int ADS_CC_AscendCompareFunc(const void *a1, const void *a2)
   {
   if ( (int)(*(AuthenDataStruct *)a1).CC - (int)(*(AuthenDataStruct *)a2).CC > 0.0 )
      return 1;
   else if ( (int)(*(AuthenDataStruct *)a1).CC - (int)(*(AuthenDataStruct *)a2).CC < 0.0 )
      return -1;
   else
      return 0;
   }


// ===========================================================================================================
// ===========================================================================================================
// Sort ADS in descending order.

int ADS_CC_DescendCompareFunc(const void *a1, const void *a2)
   {
   if ( (int)(*(AuthenDataStruct *)a1).CC - (int)(*(AuthenDataStruct *)a2).CC < 0.0 )
      return 1;
   else if ( (int)(*(AuthenDataStruct *)a1).CC - (int)(*(AuthenDataStruct *)a2).CC > 0.0 )
      return -1;
   else
      return 0;
   }


// ========================================================================================================
// ========================================================================================================
// Free up the timing data arrays dynamically allocated.

void FreeAllTimingValsForChallenge(int *num_PUF_instances_ptr, float ***PNR_ptr, float ***PNF_ptr)
   {
   int chip_num;

   if ( PNR_ptr != NULL && *PNR_ptr != NULL )
      {
      for ( chip_num = 0; chip_num < *num_PUF_instances_ptr; chip_num++ )
         free((*PNR_ptr)[chip_num]);
      free(*PNR_ptr);
      }
   else
      printf("WARNING: FreeAllTimingValsForChallenge(): Called with NULL PNR_ptr!\n");
   *PNR_ptr = NULL;

   if ( PNF_ptr != NULL && *PNF_ptr != NULL )
      {
      for ( chip_num = 0; chip_num < *num_PUF_instances_ptr; chip_num++ )
         free((*PNF_ptr)[chip_num]);
      free(*PNF_ptr);
      }
   else
      printf("WARNING: FreeAllTimingValsForChallenge(): Called with NULL PNF_ptr!\n");
   *PNF_ptr = NULL;

   *num_PUF_instances_ptr = 0;

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Compute the PND from the PNR/PNF, using two 11-bit LFSR seeds. 

float ComputePNDiffsTwoSeeds(int num_PNDiffs, float *PNR, float *PNF, float *fPND, int LFSR_seed_low, 
   int LFSR_seed_high)
   {
   uint16_t lfsr_val_low, lfsr_val_high;
   float largest_neg_PND = 0.0;
   int PND_num; 

// Sanity check: Don't allow this because first call uses the LFSR seed directly.
   if ( LFSR_seed_low >= num_PNDiffs || LFSR_seed_high >= num_PNDiffs )
      { 
      printf("ERROR: ComputePNDiffsTwoSeeds(): SEED for LFSR low %d or high %d larger than max %d!\n", 
         LFSR_seed_low, LFSR_seed_high, num_PNDiffs); 
      exit(EXIT_FAILURE); 
      }

// Create differences from rising and falling edge delays. Confirmed that the lfsr cycles through all 2048 values. 
   for ( PND_num = 0; PND_num < num_PNDiffs; PND_num++ )
      {

// Load up the LFSR with the seed on the first iteration and get the first value. First parameter is 'load_seed'
      if ( PND_num == 0 )
         {
         LFSR_11_A_bits_low(1, (uint16_t)LFSR_seed_low, &lfsr_val_low);
         LFSR_11_A_bits_high(1, (uint16_t)LFSR_seed_high, &lfsr_val_high);
         }

// Sanity check
      if ( (int)lfsr_val_low >= num_PNDiffs || (int)lfsr_val_high >= num_PNDiffs )
         { 
         printf("ERROR: ComputePNDiffsTwoSeeds(): LFSR low %d or high %d larger than max %d!\n", 
            lfsr_val_low, lfsr_val_high, num_PNDiffs); exit(EXIT_FAILURE); 
         }

// Compute the PNDiff
      fPND[lfsr_val_low] = PNR[lfsr_val_low] - PNF[lfsr_val_high];

// Check for overflow that would happen in the hardware.
      if ( fPND[lfsr_val_low] > LARGEST_POS_VAL || fPND[lfsr_val_low] < LARGEST_NEG_VAL )
         { 
         printf("ERROR: ComputePNDiffsTwoSeeds(): fPND larger than largest or smaller than smallest allowable value %d/%d!\n", 
            LARGEST_POS_VAL, LARGEST_NEG_VAL); 
         exit(EXIT_FAILURE); 
         }

// Get largest neg PND so we can use it in ComputeBoundedRange.
      if ( PND_num == 0 || largest_neg_PND > fPND[lfsr_val_low] )
         largest_neg_PND = fPND[lfsr_val_low];

// Update to the next LFSR value from the current state.
      LFSR_11_A_bits_low(0, (uint16_t)0, &lfsr_val_low);
      LFSR_11_A_bits_high(0, (uint16_t)0, &lfsr_val_high);
      }

   return largest_neg_PND;
   }


// ========================================================================================================
// ========================================================================================================
// Get the range of values between the 6.25% and 93.75% distribution quantifiers. I find the largest negative 
// value in the distribution, subtract that from all values (shifting the distribution left for negative largest 
// values and right for positive). The binning of values therefore starts at bin 0 and goes up through the largest 
// positive value (minus the negative value). 

int ComputeBoundedRange(int num_PNDiffs, float *fPND, float range_low_limit, float range_high_limit, int DIST_range, 
   float largest_neg_PND)
   {
   int low_done, low_index = 0, high_index = 0;
   int PND_num, bin_num, PNDiff_int;
   int PN_bins[DIST_range];
   float temp_float;
   int i, range;
   float sum;

// Clear out the counts in the distribution bins. Note I double the FPA range to handle differences (negative values).
   for ( bin_num = 0; bin_num < DIST_range; bin_num++ )
      PN_bins[bin_num] = 0;

   for ( PND_num = 0; PND_num < num_PNDiffs; PND_num++ ) 
      {

// Adjust the PND according to the smallest value in the distribution. If largest is negative, this shifts distribution to
// right, if positive, to the left. 
      temp_float = fPND[PND_num] - largest_neg_PND;

// Sanity check.
      if ( temp_float > (float)DIST_range || temp_float < 0 )
         { printf("ERROR: ComputeBoundedRange(): Adjusted PNDIFF LESS THAN 0 or OUTSIDE of DIST_range => %f\n", temp_float); exit(EXIT_FAILURE); }

// Bin the PND. NOTE: The values are ALWAYS positive here. 'int' truncates. 
      PNDiff_int = (int)(temp_float + 0.5);
      PN_bins[PNDiff_int]++; 
      }

   sum = 0.0;
   low_done = 0;
   for ( i = 0; i < DIST_range; i++ ) 
      { 
      sum += PN_bins[i]; 
      if ( low_done == 0 && sum >= range_low_limit )
         {
         low_done = 1;
         low_index = i;
         }
      if ( sum <= range_high_limit )
         { high_index = i; }
      }

//   printf("ComputeBoundedRange():Low index %d\tHigh index %d\n", low_index, high_index);
   range = high_index - low_index;

#ifdef DEBUG
   printf("ComputeBoundedRange():NOTE: Low index %d\tHigh index %d\tRange %d\n", low_index, high_index, range);
#endif

   return range; 
   }


// ========================================================================================================
// ========================================================================================================
// GPEVCal compensates for global process variations and temperature-voltage variations. NOTE: PND and PNDc
// MAY BE THE SAME ARRAY -- make sure this works under these conditions.

void GPEVCal(int num_PNDiffs, float *PND, float *PNDc, float range_low_limit, float range_high_limit, int DIST_range, 
   unsigned int RangeConstant, float largest_neg_PND)
   {
   float cur_mean, cur_range, range_conv; 
   int val_num;

// DEBUG
#ifdef DEBUG
printf("GPEVCal called!\n"); fflush(stdout);
#endif

// Calibration involves standardizing the delays, which means subtracting the mean and dividing by the range.
// We also multiply the standardized values by a constant to scale them to integer values larger than +/- 1,
// which occurs after standardization.
   cur_mean = 0.0;
   for ( val_num = 0; val_num < num_PNDiffs; val_num++ )
      cur_mean += PND[val_num];
   cur_mean /= (float)num_PNDiffs;

// Compute the range
   cur_range = ComputeBoundedRange(num_PNDiffs, PND, range_low_limit, range_high_limit, DIST_range, largest_neg_PND); 

// This is what the hardware does.
   range_conv = (float)RangeConstant/cur_range;

#ifdef DEBUG
printf("\tGPEVCal(): Existing mean %9.4f and range %9.4f!\n", cur_mean, cur_range); fflush(stdout);
printf("\tGPEVCal(): New      mean 0.0 and range %d\tRange Conversion %9.4f\n", RangeConstant, range_conv); fflush(stdout);
#endif

// Standardize and then scale. 
   for ( val_num = 0; val_num < num_PNDiffs; val_num++ )
      {

// Eliminate global process and temperature-voltage variations
      PNDc[val_num] = (float)((int)(((PND[val_num] - cur_mean)*range_conv)*16.0))/16.0;

#ifdef DEBUG
printf("\t%4d) %9.4f\n", val_num, PNDc[val_num]);
#endif
      }

   return;
   }



// ========================================================================================================
// ========================================================================================================
// Add SpreadFactors

void AddSpreadFactors(int max_PNDiffs, float *PNDc, float *PNDco, float *fSpreadFactors, int TrimCodeConstant, 
   int chip_num)
   {
   int PND_num;

#ifdef DEBUG
int limit = 20;
#endif

// Don't need to fix the final value because the incoming PNDc are all trimmed to 4 bits of precision. 
   for ( PND_num = 0; PND_num < max_PNDiffs; PND_num++ )
      {

// Subtract the SF from the PNDc. The SF now move the PNDc to the nearest lower multiple of TrimCodeConstant. 
      PNDco[PND_num] = PNDc[PND_num] - fSpreadFactors[PND_num];

// Remove the path length bias by moving the PNDco (with SF subtracted and adjusted to surround the nearest lower
// multiple of TrimCodeConstant) toward zero mean. For example, keep adding 16 when the PNDco is less than -16 
// (with TrimCodeConstant set to 32).
      while (1)
         {
         if ( PNDco[PND_num] < (float)-TrimCodeConstant/2 )
            PNDco[PND_num] += (float)TrimCodeConstant;
         else if ( PNDco[PND_num] > (float)TrimCodeConstant/2 )
            PNDco[PND_num] -= (float)TrimCodeConstant;
         else
            break;
         }

#ifdef DEBUG
if ( chip_num == 0 && PND_num < limit )
printf("Chip %d\tfSF %9.4f\n", chip_num, fSpreadFactors[PND_num]);
#endif
      }

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Do SRF Engine operations in software 

void DoSRFComp(int max_string_len, SRFAlgoParamsStruct *SAP_ptr, int do_dump)
   {
   float largest_neg_PND;

   struct timeval t0, t1;
   long elapsed; 

   char outfilename[max_string_len];
   char temp_str[max_string_len];
   FILE *OUTFILE;
   int PN;

#ifdef DEBUG
printf("\tDoSRFComp(): LFSR seed low %u\tLFSR seed high %u\tRangeConstant %d\tSpreadConstant %u\tTrimCodeConstant %u\tThreshold %u\n\n", 
   SAP_ptr->param_LFSR_seed_low, SAP_ptr->param_LFSR_seed_high, SAP_ptr->param_RangeConstant, SAP_ptr->param_SpreadConstant, 
   SAP_ptr->param_TrimCodeConstant, SAP_ptr->param_Threshold); fflush(stdout);
#endif

// DEBUG. Only on first iteration of the SKE authentication loop.
   if ( DO_DUMP_PN_DATA_CHIP_NUM != -1 && DO_DUMP_PN_DATA_CHIP_NUM == SAP_ptr->chip_num && do_dump == 1 )
      {
      strcpy(outfilename, DumpDir);
      sprintf(temp_str, "PNR_Chip_%d_Chlng_%d.xy", SAP_ptr->chip_num, Chlng_cnter);
      strcat(outfilename, temp_str);
      if ( (OUTFILE = fopen(outfilename, "w")) == NULL )
         { printf("ERROR: DoSRFComp(): Data file '%s' open failed for writing!\n", outfilename); exit(EXIT_FAILURE); }

      for ( PN = 0; PN < SAP_ptr->num_required_PNDiffs; PN++ )
         fprintf(OUTFILE, "%d\t%.4f\n", PN, SAP_ptr->PNR[SAP_ptr->chip_num][PN]); 
      fclose(OUTFILE);

      strcpy(outfilename, DumpDir);
      sprintf(temp_str, "PNF_Chip_%d_Chlng_%d.xy", SAP_ptr->chip_num, Chlng_cnter);
      strcat(outfilename, temp_str);
      if ( (OUTFILE = fopen(outfilename, "w")) == NULL )
         { printf("ERROR: DoSRFComp(): Data file '%s' open failed for writing!\n", outfilename); exit(EXIT_FAILURE); }

      for ( PN = 0; PN < SAP_ptr->num_required_PNDiffs; PN++ )
         fprintf(OUTFILE, "%d\t%.4f\n", PN, SAP_ptr->PNF[SAP_ptr->chip_num][PN]); 
      fclose(OUTFILE);
      }

// ****************************************
// ***** Compute PND
   if ( SAP_ptr->DEBUG_FLAG == 1 )
      {
      printf("\tHE.1) Computing PND\n");
      gettimeofday(&t0, 0);
      }

   largest_neg_PND = ComputePNDiffsTwoSeeds(SAP_ptr->num_required_PNDiffs, SAP_ptr->PNR[SAP_ptr->chip_num], SAP_ptr->PNF[SAP_ptr->chip_num], 
      SAP_ptr->fPND, SAP_ptr->param_LFSR_seed_low, SAP_ptr->param_LFSR_seed_high);

// DEBUG
   if ( DO_DUMP_PN_DATA_CHIP_NUM != -1 && DO_DUMP_PN_DATA_CHIP_NUM == SAP_ptr->chip_num && do_dump == 1 )
      {
      strcpy(outfilename, DumpDir);
      sprintf(temp_str, "PND_Chip_%d_Chlng_%d.xy", SAP_ptr->chip_num, Chlng_cnter);
      strcat(outfilename, temp_str);
      if ( (OUTFILE = fopen(outfilename, "w")) == NULL )
         { printf("ERROR: DoSRFComp(): Data file '%s' open failed for writing!\n", outfilename); exit(EXIT_FAILURE); }

      for ( PN = 0; PN < SAP_ptr->num_required_PNDiffs; PN++ )
         fprintf(OUTFILE, "%d\t%.4f\n", PN, SAP_ptr->fPND[PN]); 
      fclose(OUTFILE);
      }

   if ( SAP_ptr->DEBUG_FLAG == 1 )
      { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed); }

// ****************************************
// ***** GPEVCal
   if ( SAP_ptr->DEBUG_FLAG == 1 )
      {
      printf("\tHE.2) Computing PNDc\n");
      gettimeofday(&t0, 0);
      }

   GPEVCal(SAP_ptr->num_required_PNDiffs, SAP_ptr->fPND, SAP_ptr->fPNDc, SAP_ptr->range_low_limit, SAP_ptr->range_high_limit, SAP_ptr->dist_range, 
      SAP_ptr->param_RangeConstant, largest_neg_PND);

// DEBUG
   if ( DO_DUMP_PN_DATA_CHIP_NUM != -1 && DO_DUMP_PN_DATA_CHIP_NUM == SAP_ptr->chip_num && do_dump == 1 )
      {
      strcpy(outfilename, DumpDir);
      sprintf(temp_str, "PNDc_Chip_%d_Chlng_%d.xy", SAP_ptr->chip_num, Chlng_cnter);
      strcat(outfilename, temp_str);
      if ( (OUTFILE = fopen(outfilename, "w")) == NULL )
         { printf("ERROR: DoSRFComp(): Data file '%s' open failed for writing!\n", outfilename); exit(EXIT_FAILURE); }

      for ( PN = 0; PN < SAP_ptr->num_required_PNDiffs; PN++ )
         fprintf(OUTFILE, "%d\t%.4f\n", PN, SAP_ptr->fPNDc[PN]); 
      fclose(OUTFILE);
      }

   if ( SAP_ptr->DEBUG_FLAG == 1 )
      { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed); }

// ****************************************
// ***** Add SpreadFactors. Only clear SAP_ptr->num_required_PNDiffs of these above.
   if ( SAP_ptr->DEBUG_FLAG == 1 )
      {
      printf("\tHE.3) Add SpreadFactors\n");
      gettimeofday(&t0, 0);
      }

   AddSpreadFactors(SAP_ptr->num_required_PNDiffs, SAP_ptr->fPNDc, SAP_ptr->fPNDco, SAP_ptr->fSpreadFactors, SAP_ptr->param_TrimCodeConstant,
      SAP_ptr->chip_num);

// DEBUG
   if ( DO_DUMP_PN_DATA_CHIP_NUM != -1 && DO_DUMP_PN_DATA_CHIP_NUM == SAP_ptr->chip_num && do_dump == 1 )
      {
      strcpy(outfilename, DumpDir);
      sprintf(temp_str, "PNDco_Chip_%d_Chlng_%d.xy", SAP_ptr->chip_num, Chlng_cnter);
      strcat(outfilename, temp_str);
      if ( (OUTFILE = fopen(outfilename, "w")) == NULL )
         { printf("ERROR: DoSRFComp(): Data file '%s' open failed for writing!\n", outfilename); exit(EXIT_FAILURE); }

      for ( PN = 0; PN < SAP_ptr->num_required_PNDiffs; PN++ )
         fprintf(OUTFILE, "%d\t%.4f\n", PN, SAP_ptr->fPNDco[PN]); 
      fclose(OUTFILE);

// Increment the Chlng_cnter.
      Chlng_cnter++;
      }

   if ( SAP_ptr->DEBUG_FLAG == 1 )
      { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed); }

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// We use 8-bit SpreadFactors now, so MUST reduce the median PNDc to a value < TrimCodeConstant, and make all 
// SF positive (they can be very small negative numbers via other modes). This was debugged originally in 
// /borg_data/FPGAs/ZYBO/SR_RFM/ANALYSIS/code/common.c, in ComputePCRSpreadFactors().

//#define DEBUG1 0

void ReduceRawSF(int max_string_len, float *fSpreadFactors, signed char *iSpreadFactors, int iSpreadFactorScaler,
   int num_SF, int TrimCodeConstant)
   {
   int SF_num, multiple;

#ifdef DEBUG1
int limit = 20;
#endif

   for ( SF_num = 0; SF_num < num_SF; SF_num++ )
      {

// Find the nearest multiple of TrimCodeConstant for the current fSpreadFactors. If positive, truncate to lowest integer. 
// If negative, we need to round down to the next smallest negative number, e.g., if -1.x, round to -2. Preserve the sign here. 
// NOTE: We have rounded fSpreadFactors to 4 binary digits of precision in the caller.
      multiple = (int)(fSpreadFactors[SF_num]*16)/(TrimCodeConstant*16);
      if ( fSpreadFactors[SF_num] < 0 )
         {

// If the remainder is non-zero, round down. NOTE: fSpreadFactors MUST be rounded to 4 precision places for this to work.
         if ( (int)fabs(fSpreadFactors[SF_num] * 16) % (TrimCodeConstant*16) != 0 )
            multiple -= 1;
         }

#ifdef DEBUG1
if ( SF_num < limit )
   { 
   printf("ReduceRawSF(): Orig SpreadFactor[%4d]: %+8.4f\tmultiple %2d\tAdd -1 ? %3d with %3d mod %3d =?= 0\t", SF_num, fSpreadFactors[SF_num], multiple, 
      (int)fabs(fSpreadFactors[SF_num] * 16) % (TrimCodeConstant*16), (int)fabs(fSpreadFactors[SF_num] * 16), (TrimCodeConstant*16)); fflush(stdout); 
   }
#endif

// THIS MUST be always positive and less than TrimCodeConstant.
      fSpreadFactors[SF_num] = fSpreadFactors[SF_num] - (float)multiple*TrimCodeConstant;

#ifdef DEBUG1
if ( SF_num < limit )
   { printf("ReduceRawSF(): New SpreadFactor[%4d]: %+8.4f\n", SF_num, fSpreadFactors[SF_num]); fflush(stdout); }
#endif

// Sanity check
      if ( fSpreadFactors[SF_num] < 0 || fSpreadFactors[SF_num] >= TrimCodeConstant )
         {
         printf("ERROR: ReduceRawSF(): fSpreadFactors[%d] is less than 0 or >= TrimCodeConstant %d => %f\n", 
            SF_num, TrimCodeConstant, fSpreadFactors[SF_num]);
         exit(EXIT_FAILURE);
         }
      }

// Convert floats to signed char for transmission to device. Round off appropriately. With iSpreadFactorScaler at 2, and 
// if I have 35.6875, then store 35.5000. But if I have 16.9375, store 17.000 (anything larger than 16.75 should be 17.0, 
// anything larger than 16.25 should be 16.5, etc). Similarly, with iSpreadFactorScaler at 1, than anything larger than 
// 16.5 should be 17, etc. NOTE: Use 0.5 here for all cases, i.e., when iSpreadFactorScaler is 2, we need to add 0.5 and 
// NOT 0.25!
   for ( SF_num = 0; SF_num < num_SF; SF_num++ )
      if ( fSpreadFactors[SF_num] >= 0 )
         iSpreadFactors[SF_num] = (signed char)(fSpreadFactors[SF_num] * (float)iSpreadFactorScaler + 0.5);
      else 
         iSpreadFactors[SF_num] = (signed char)(fSpreadFactors[SF_num] * (float)iSpreadFactorScaler - 0.5);

#ifdef DEBUG1
for ( SF_num = 0; SF_num < limit; SF_num++ )
   printf("ReduceRawSF(): TRIMMED PopOnly SpreadFactors[%4d], PART A, Median (fSpreadFactor) %8.4f\n", SF_num, fSpreadFactors[SF_num]);
fflush(stdout);
#endif

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// When the server needs to compute the SF COMPLETELY (device does NOT do it), the PopOnly SF MUST be updated
// to flip the distribution on the device. NOTE: NORMALLY, WE ONLY CALL THIS ROUTINE when we know which device
// we are communicating with. HOWEVER, FOR DA where we do NOT know, we will need to call this routine for each 
// device in the DB starting with fresh PopOnly SF. We assume the SAP_ptr->chip_num field is VALID (checked in 
// the caller). 
//
// NOTE: This routine is passed the PNDc from a specific chip, i.e., PopOnly SF have not been applied.
//
// NOTE: Although the PopOnly SF are < TrimCodeConstant after ReduceRawSF() is called, that MAY NOT be the case
// after we do distribution flips here. This routine is called when this server needs to compute the SF
// completely. ONLY IN THE CASE where the server sends pure PopOnly SF will the SF remain < TrimCodeConstant.

void FlipPO_SF(int max_string_len, float *PNDc, float *fSpreadFactors, signed char *iSpreadFactors, 
   int iSpreadFactorScaler, int num_SF, int TrimCodeConstant)
   {
   int SF_num, num_TCC_iters;
   float PNDco; 

#ifdef DEBUG
float orig_SF;
#endif

#ifdef DEBUG
int limit = 20;
float temp;
printf("FlipPO_SF(): CALLED -- MODIFYING PopOnly SF!\n"); fflush(stdout);
#endif

// Sanity check
   if ( TrimCodeConstant == 0 )
      { printf("ERROR: FlipPO_SF(): TrimCodeConstant MUST be > 0!\n"); exit(EXIT_FAILURE); }

   for ( SF_num = 0; SF_num < num_SF; SF_num++ )
      {

#ifdef DEBUG
orig_SF = fSpreadFactors[SF_num];
#endif

// Sanity check. ONLY POSITIVE SF are allowed now.
      if ( fSpreadFactors[SF_num] < 0 )
         { printf("ERROR: FlipPO_SF(): fSpreadFactor[%d] is NEGATIVE %9.4f!\n", SF_num, fSpreadFactors[SF_num]); exit(EXIT_FAILURE); }

// Apply the PopOnly SF to the chip's PNDc since we need this value below.
      PNDco = PNDc[SF_num] - fSpreadFactors[SF_num];

// 12_24_2021: I originally worked on this in 10_15-21_2021 and came up with two algorithms that reduce the SF to 8-bit values (but 
// NOT with 4 bits of binary precision, only 3 or 2). We need to iteratively reduce the fPNDco. The SF are ALWAYS positive either 
// reducing positive SF (when subtracted) to the nearest multiple of TrimCodeConstant toward 0 or further reducing negative SF more 
// negative, ie., from -50 to -64 when the TrimCodeConstant is 32. This reduction also done in AddSpreadFactors()
      num_TCC_iters = 0;
      while (1)
         {
         if ( PNDco < (float)-TrimCodeConstant/2 )
            {
            PNDco += (float)TrimCodeConstant;
            num_TCC_iters++;
            }
         else if ( PNDco > (float)TrimCodeConstant/2 )
            {
            PNDco -= (float)TrimCodeConstant;
            num_TCC_iters++;
            }
         else
            break;
         }

// If the number of iterations is odd, adjust the fSpreadFactor.
      if ( (num_TCC_iters % 2) == 1 )
//      if ( ((num_TCC_iters % 2) == 1 && (((int)(fSpreadFactors[SF_num]*16) % (TrimCodeConstant*16)) >= TrimCodeConstant/2*16)) ||
//           ((num_TCC_iters % 2) == 0 && (((int)(fSpreadFactors[SF_num]*16) % (TrimCodeConstant*16)) < TrimCodeConstant/2*16)) )
         {
         fSpreadFactors[SF_num] += 2.0*PNDco;


// Needed when the actual PNDco are passed in, as opposed to a temporary copy that happens in ComputePxxSpreadFactors. 
// DO THIS AFTER the modification to fSpreadFactors. Does NOTHING here.
         PNDco = -PNDco;
         }

#ifdef DEBUG
if ( SF_num < limit )
   printf("SF_num %4d\tSF %9.4f\tFinal PNDco %9.4f\n", SF_num, fSpreadFactors[SF_num], PNDco);
fflush(stdout);
#endif

#ifdef DEBUG
if ( fSpreadFactors[SF_num] < 0 || fSpreadFactors[SF_num] >= (float)TrimCodeConstant )
   printf("SF_num %4d\tOrig SF %9.4f\tNew SF <0 or >TrimCodeConstant %d => %9.4f\n", 
      SF_num, orig_SF, TrimCodeConstant, fSpreadFactors[SF_num]);
fflush(stdout);
#endif
      }

// Round off appropriately. With iSpreadFactorScaler at 2, and if I have 35.6875, then store 35.5000. But if I have 16.9375, 
// store 17.000 (anything larger than 16.75 should be 17.0, Anything larger than 16.25 should be 16.5, etc). Similarly, 
// with iSpreadFactorScaler at 1, than anything larger than 16.5 should be 17, etc. NOTE: Use 0.5 here for all cases, i.e., 
// when iSpreadFactorScaler is 2, we need to add 0.5 and NOT 0.25!
   for ( SF_num = 0; SF_num < num_SF; SF_num++ )
      if ( fSpreadFactors[SF_num] >= 0 )
         iSpreadFactors[SF_num] = (signed char)(fSpreadFactors[SF_num] * (float)iSpreadFactorScaler + 0.5);
      else 
         iSpreadFactors[SF_num] = (signed char)(fSpreadFactors[SF_num] * (float)iSpreadFactorScaler - 0.5);

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// Compute the PCR SpreadFactors. NOTE: YOU CAN NOT reseed srand with the same seed in this routine. In fact,
// you cannot use srand or rand because it is NOT re-entrant.

void ComputePxxSpreadFactors(int max_string_len, SRFAlgoParamsStruct *SAP_ptr, int do_part_A_or_B)
   {
   float strong_1_center, strong_0_center;
   int PND_num;

   float *fPNDco, *fSpreadFactors; 
   signed char *iSpreadFactors;
   int num_PNDiffs, num_chips; 
   int SpreadConstant;
   int TrimCodeConstant;

// Fields of the structure used within this routine.
   num_PNDiffs = SAP_ptr->num_required_PNDiffs; 
   num_chips = SAP_ptr->num_chips; 
   fPNDco = SAP_ptr->fPNDco;
   fSpreadFactors = SAP_ptr->fSpreadFactors; 
   iSpreadFactors = SAP_ptr->iSpreadFactors; 
   SpreadConstant = SAP_ptr->param_SpreadConstant;
   TrimCodeConstant = SAP_ptr->param_TrimCodeConstant;

// Sanity check. For rounding below.
   if ( SAP_ptr->iSpreadFactorScaler < 1 || SAP_ptr->iSpreadFactorScaler > 2 )
      { printf("ERROR: ComputePxxSpreadFactors(): iSpreadFactorScaler %d MUST be 1 or 2!\n", SAP_ptr->iSpreadFactorScaler); exit(EXIT_FAILURE); }

// ======================================
// ======================================
// Does PopOnly SF. Here we calculate the median values of each PNDc using enrollment data across all chips.
   if ( do_part_A_or_B == 0 )
      {
      int chip_num, PND_num;
      float largest_neg_PND;
      float **PO_PNDc; 
      float *vals;

#ifdef DEBUG
printf("Compute Pop SF\n"); fflush(stdout);
#endif

// Don't use all the chips if the number is very large b/c it takes too long -- only a subset is needed. 
// 6_27_2022: LATENT BUG: If you have do_PO_dist_flip set to 1, YOU MUST USE ALL CHIPS HERE. See below
// FlipPO_SF which uses the PO_PNDc[SAP_ptr->chip_num] computed here to determine when flips occur so 
// so the dataset of SAP_ptr->chip_num better be present else crash. Alternatively, you can just make
// sure SAP_ptr->chip_num data is included in the selected set.
//
// 10_22_2022: You can add this back later -- I'm debugging.
//      if ( num_chips >= 100 && SAP_ptr->do_PO_dist_flip == 0 )
//         num_chips = 100;
//      else
         num_chips = SAP_ptr->num_chips;

// Allocate space to compute the Median values.
      if ( (PO_PNDc = (float **)malloc(sizeof(float *) * num_chips)) == NULL )
         { printf("ERROR: ComputePxxSpreadFactors(): failed to allocate temporary storage for 'PO_PNDc'!\n"); exit(EXIT_FAILURE); }
      for ( chip_num = 0; chip_num < num_chips; chip_num++ )
         if ( (PO_PNDc[chip_num] = (float *)malloc(sizeof(float) * SAP_ptr->num_required_PNDiffs)) == NULL )
            { printf("ERROR: ComputePxxSpreadFactors(): failed to allocate temporary storage for 'PO_PNDc[chip_num]'!\n"); exit(EXIT_FAILURE); }

// ---------------------------------
// Compute differences and calibrate
      for ( chip_num = 0; chip_num < num_chips; chip_num++ )
         {
// Compute the PND and PNDc for each chip
         largest_neg_PND = ComputePNDiffsTwoSeeds(SAP_ptr->num_required_PNDiffs, SAP_ptr->PNR[chip_num], SAP_ptr->PNF[chip_num], PO_PNDc[chip_num], 
            SAP_ptr->param_LFSR_seed_low, SAP_ptr->param_LFSR_seed_high);

// GPEVCal the PNDc for this chip. NOTE: Source and destination arrays are identical.
         GPEVCal(SAP_ptr->num_required_PNDiffs, PO_PNDc[chip_num], PO_PNDc[chip_num], SAP_ptr->range_low_limit, SAP_ptr->range_high_limit, 
            SAP_ptr->dist_range, SAP_ptr->param_RangeConstant, largest_neg_PND);
         }

// Allocate storage.
      if ( (vals = (float *)malloc(sizeof(float) * num_chips)) == NULL )
         { printf("ERROR: ComputePxxSpreadFactors(): failed to allocate temporary storage for 'vals'!\n"); exit(EXIT_FAILURE); }

// For each PNDc, compute the median value.
      for ( PND_num = 0; PND_num < SAP_ptr->num_required_PNDiffs; PND_num++ )
         {
         for ( chip_num = 0; chip_num < num_chips; chip_num++ )
            vals[chip_num] = PO_PNDc[chip_num][PND_num];

// These MUST be rounded to 4 binary digits for ReduceRawSF to work properly.
         fSpreadFactors[PND_num] = (float)((int)(ComputeMedian(num_chips, vals)*16.0))/16.0;

// They will NOT fit until they are trimmed below by ReduceRawSF().
//         iSpreadFactors[PND_num] = (signed short)(fSpreadFactors[PND_num] * (float)SAP_ptr->iSpreadFactorScaler);

#ifdef DEBUG
if ( PND_num == 0 )
   int i;
   for ( i = 0; i < num_chips; i++ )
      printf("Chip %d\tPND %d\tPO_PNDc %10.4f\tMedian fSpreadFactor %9.4f\tPO_PNDc_pop %10.4f\n", i , PND_num, PO_PNDc[i][PND_num], 
         fSpreadFactors[PND_num], PO_PNDc[i][PND_num] - fSpreadFactors[PND_num]);
#endif
#ifdef DEBUG
if ( PND_num < 10 )
   {
   int i, num_pos_vals = 0, num_neg_vals = 0, num_zero_vals = 0;
   for ( i = 0; i < num_chips; i++ )
      {
      if ( PO_PNDc[i][PND_num] - fSpreadFactors[PND_num] < 0.0 )
         num_neg_vals++; 
      if ( PO_PNDc[i][PND_num] - fSpreadFactors[PND_num] > 0.0 )
         num_pos_vals++; 
      if ( PO_PNDc[i][PND_num] - fSpreadFactors[PND_num] == 0.0 )
         num_zero_vals++; 
      }
   printf("\tPND %d\tnum chips %d\tnum_pos %d\tnum neg %d\tnum_zeros %d\n", PND_num, num_chips, num_pos_vals, num_neg_vals, num_zero_vals); fflush(stdout);
   }
#endif
         }

// -----------------------------------------
// We use 8-bit SpreadFactors now, so this MUST ALWAYS BE CALLED. We MUST reduce the median PNDc to a value < the TrimCodeConstant, and 
// make them ALL positive. 
      ReduceRawSF(max_string_len, fSpreadFactors, iSpreadFactors, SAP_ptr->iSpreadFactorScaler, SAP_ptr->num_required_PNDiffs, 
         TrimCodeConstant);

#ifdef DEBUG
int j;
for ( j = 0; j < 10; j++ )
   printf("ComputePxxSpreadFactors(): HERE: fSpreadFactors[%d] %.4f\tiSpreadFactors[%d] %d\n", j, fSpreadFactors[j], j, iSpreadFactors[j]);
#endif

// -----------------------------------------
// When the server needs to compute the SF COMPLETELY (device does NOT do it), the PopOnly SF MUST be updated to flip the distribution on the device, 
// e.g., FUNC_VA, FUNC_SE. NOTE: For FUNC_DA, WE DO NOT KNOW WHICH DEVICE we are communicating with so we will need to call this routine for each device 
// in the DB starting with fresh PopOnly SF. NOTE: The SF are NOT bounded to be >= 0 and < TrimCodeConstant after calling this routine (they are also NOT
// bounded by the pure PopOnly range after PCR either).
      if ( SAP_ptr->do_PO_dist_flip == 1 )
         {

// Sanity check. SAP_ptr->chip_num MUST be set to a number between 0 and num_chips-1.
         if ( SAP_ptr->chip_num < 0 || SAP_ptr->chip_num >= num_chips )
            { 
            printf("WARNING: ComputePxxSpreadFactors(): PO component WITH do_PO_dist_flip a 1 MUST have chip_num %d >= 0 and < %d -- NOT DOING PO_dist_flip!\n", 
               SAP_ptr->chip_num, num_chips);
            }
         else
            FlipPO_SF(max_string_len, PO_PNDc[SAP_ptr->chip_num], fSpreadFactors, iSpreadFactors, SAP_ptr->iSpreadFactorScaler, 
               SAP_ptr->num_required_PNDiffs, TrimCodeConstant);
         }

// Free the space.
      for ( chip_num = 0; chip_num < num_chips; chip_num++ )
         if ( PO_PNDc[chip_num] != NULL )
            free(PO_PNDc[chip_num]);
      if ( PO_PNDc != NULL )
         free(PO_PNDc); 
      if ( vals != NULL )
         free(vals);
      return;
      }

// ======================================
// ======================================
// Do PCR SF

// Sanity check. SAP_ptr->chip_num MUST be set to a number between 0 and num_chips-1.
   if ( SAP_ptr->chip_num < 0 || SAP_ptr->chip_num >= SAP_ptr->num_chips )
      { 
      printf("ERROR: ComputePxxSpreadFactors(): PerChip+Random component MUST have chip_num %d >= 0 and < num_chips %d\n", 
         SAP_ptr->chip_num, SAP_ptr->num_chips); exit(EXIT_FAILURE); 
      }

// Sanity check. Do NOT run PCR is we specify PopOnly.
   if ( SAP_ptr->param_PCR_or_PBD_or_PO == SF_MODE_POPONLY )
      { printf("ERROR: ComputePxxSpreadFactors(): Can NOT run PCR code when SF mode is PopOnly!\n"); exit(EXIT_FAILURE); }

// We first call DoSRFComp() to compute PND, PNDc and fPNDco for the chip number set in SAP_ptr->chip_num, which ALSO APPLIES the 
// population-based SpreadFactors to the fPNDco (useful in case we are computing a response bitstring within the verifier). 
   DoSRFComp(max_string_len, SAP_ptr, 0);

// We just need to move these population-adjusted PNDco to the CENTER of the strong bit region and then randomly distribute across 
// the SpreadConstant/2 and we are done. However, we also need to update fSpreadFactors because that is what we store when the 
// enrollment process is completed. So we focus here on adjusting the fSpreadFactors and then appling them to the PNDco to 
// generate the final PNDco. 

// Make the center of the strong bit region the SpreadConstant, 20/4 = +/-5. NOTE: SpreadConstant MUST be similar to the maximum 
// amount of entropy present in the path delay differences. If larger, information may leak in the final SpreadFactors
   strong_0_center = (float)(-SpreadConstant)/4.0;
   strong_1_center = (float)SpreadConstant/4.0;

// We canNOT use rand any longer in multi-threaded routines when use_database_chlngs = 1. So on the verifier, we use /dev/urandom
// instead.
   FILE *fp = fopen("/dev/urandom", "r");
   char num_str[5];
   num_str[4] = '\0';

// ------------------------------------------------
// Apply the remaining components of the PCR method. 
   float PerChip_spread_factor = 0.0;
   int random_val; 
   for ( PND_num = 0; PND_num < num_PNDiffs; PND_num++ )
      {
      float init_fPNDco;

// NOTE: We use enrollment PNDco which have been updated with the PopOnly SF already BUT ONLY IF PCR_or_PBD_or_PO is 
// set to PCR or PopOnly mode (otherwise they have NOT been updated and fSpreadFactors are all 0).
      init_fPNDco = fPNDco[PND_num];

// Sanity check
      if ( SAP_ptr->param_PCR_or_PBD_or_PO == SF_MODE_PBD && fSpreadFactors[PND_num] != 0.0 )
         { printf("ERROR: ComputePxxSpreadFactors(): fSpreadFactors MUST BE zero in PBD mode!\n"); exit(EXIT_FAILURE); }

// Determine the closest strong bit line (at SpreadConstant/2 or -SpreadConstant/2) in PCR mode. If PBD mode, use the low 
// order integer bit of the PNDc (NOT THE PRECISION BITS) to determine the super-strong bit region. NOTE: Similar to the 
// hardware, we do NOT apply Pop SFs above when SAP_ptr->param_PCR_or_PBD_or_PO is set to 1 since we want to avoid using 
// SFs altogether on the device in PBD mode.
      if ( (fabs(strong_0_center - init_fPNDco) < fabs(strong_1_center - init_fPNDco) && 
         SAP_ptr->param_PCR_or_PBD_or_PO == SF_MODE_PCR) || 
         ( (((int)init_fPNDco % 2) == 0) && SAP_ptr->param_PCR_or_PBD_or_PO == SF_MODE_PBD) )
         PerChip_spread_factor = strong_0_center - init_fPNDco;
      else
         PerChip_spread_factor = strong_1_center - init_fPNDco;

// CHANGE HERE: 10_17_2021: Handle the fPNDco 0.0 case. Leave the PerChip+Random Spreadfactor at zero. The population 
// value (already added into the fPNDco) makes this fPNDco 0.0 so it will NOT be used during bitstring generation.
      if ( init_fPNDco == 0 )
         {
#ifdef DEBUG
printf("\tChip %3d\tfPNDco[%4d] is 0.0 %f\tLeaving fPNDco and fSpreadFactors at population value\n", SAP_ptr->chip_num, 
   PND_num, init_fPNDco); fflush(stdout);
#endif
         continue;
         }

// PopOnly SF are just the median values of the PND across all chips, but with TrimCodeConstant, they are moved to the 
// next lower multiple of TrimCode. And they are always positive. They have ALREADY been applied to the fPNDco and trimmed.

#ifdef DEBUG
if ( PND_num < 20 )
   printf("\tChip %3d\tINIT fPNDco[%4d] %9.4f\tINIT SF %9.4f\tPerChip_spread_factor %9.4f\n", SAP_ptr->chip_num, 
      PND_num, init_fPNDco, fSpreadFactors[PND_num], PerChip_spread_factor); 
fflush(stdout);
#endif

// So PCR SpreadFactor = Pop SpreadFactor - PerChip SpreadFactor + random_val, which would then be SUBTRACTED from the PNDc. 
// Note that SF are the PopOnly SF, which with TrimCodeConstant applied, are all positive. 
// 1_1_2022: NOTE: After PCR, SF can become slightly negative and more positive than the TrimCodeConstant. 
      fSpreadFactors[PND_num] -= PerChip_spread_factor;

// Now randomize the value over the 0 or 1 regions. If SpreadConstant is 20, then rand() returns a number between 0 and 8, 
// 20/2 - 1 = 9. Subtracting 20/4 - 1 = 4 makes the range -4 to +4. TRIM THIS TO 16-bit so we can send the rand values to the 
// hardware and have it compute the same random value here.
      unsigned int rand_uint;
      fread(num_str, sizeof(uint8_t), 4, fp);
      rand_uint = (num_str[3] << 24) | (num_str[2] << 16) | (num_str[1] << 8) | num_str[0]; 

//if ( PND_num < 10 )
//   { printf("rand_uint %u\n", rand_uint); fflush(stdout); }

      random_val = (rand_uint % (SpreadConstant/2 - 1)) - (SpreadConstant/4 - 1);

// Update fSpreadFactors with random value.
      fSpreadFactors[PND_num] += random_val;

// We need to invert the signs here, i.e., add in the PerChip SF and subtract the random_val from the fPNDco. 
      fPNDco[PND_num] = fPNDco[PND_num] + PerChip_spread_factor - (float)random_val;

// Sanity check. We MUST have values within the bounds of the SpreadConstant. I purposely stay one away from the max values.
      if ( fPNDco[PND_num] >= (float)SpreadConstant/2.0 || fPNDco[PND_num] <= (float)(-SpreadConstant)/2.0 )
         { 
         printf("ERROR: ComputePxxSpreadFactors(): fPNDco[%d] %+8.4f OUTSIDE bounds of SpreadConstant +/- %d\n", 
            PND_num, fPNDco[PND_num], SpreadConstant/2); exit(EXIT_FAILURE); 
         }

// Sanity check
      if ( fSpreadFactors[PND_num]*16.0 < -32767 || fSpreadFactors[PND_num]*16.0 > 32767 )
         { 
         printf("ERROR: ComputePxxSpreadFactors(): Scaled integer SpreadFactor %d exceeds max/min signed short!\n", 
            (int)(fSpreadFactors[PND_num]*16.0)); exit(EXIT_FAILURE); 
         }

// Save the integer-based SpreadFactors to a separate array. Can SF these become negative after PCR adjustments -- yes, negative 
// and more positive than the TrimCodeConstant. See example(s) above. So a TrimCodeConstant of 32 will require one extra positive 
// bit to represent numbers >= 32 and one extra bit for negative numbers. So 7 bits will be needed, so only one bit of precision 
// for TrimCodeConstant <= 32. For TrimCodeConstant > 32 and <= 64, we need all 8 bits (no precision bits are preserved). For the
// first case, iSpreadFactorScaler shifts the fSpreadFactors by 1 bit to the left and then trunscates it to a (signed char). Otherwise
// multiple by 1 which does nothing by truncate the floating point value to an integer (signed char).
      if ( (int)(fSpreadFactors[PND_num] * (float)SAP_ptr->iSpreadFactorScaler) < -127 || 
         (int)(fSpreadFactors[PND_num] * (float)SAP_ptr->iSpreadFactorScaler > 127) )
         { 
         printf("ERROR: ComputePxxSpreadFactors(): iSpreadFactors[%d] exceeds (signed char) limits of -127 to 127: %d\n", PND_num, 
            (int)(fSpreadFactors[PND_num] * (float)SAP_ptr->iSpreadFactorScaler)); 
         exit(EXIT_FAILURE); 
         }

// Round off appropriately. With iSpreadFactorScaler at 2, and if I have 35.6875, then store 35.5000. But if I have 16.9375, 
// store 17.000 (anything larger than 16.75 should be 17.0, Anything larger than 16.25 should be 16.5, etc). Similarly, with 
// iSpreadFactorScaler at 1, than anything larger than 16.5 should be 17, etc. NOTE: Use 0.5 here for all cases, i.e., when 
// iSpreadFactorScaler is 2, we need to add 0.5 and NOT 0.25!
      if ( fSpreadFactors[PND_num] >= 0 )
         iSpreadFactors[PND_num] = (signed char)(fSpreadFactors[PND_num] * (float)SAP_ptr->iSpreadFactorScaler + 0.5);
      else 
         iSpreadFactors[PND_num] = (signed char)(fSpreadFactors[PND_num] * (float)SAP_ptr->iSpreadFactorScaler - 0.5);

#ifdef DEBUG
if ( PND_num < 20 )
   printf("\tChip %3d\tFINAL fPNDco[%4d] %9.4f\tFINAL fSF %9.4f\tRandom val %2d\tiSF %4d\n", SAP_ptr->chip_num, PND_num, 
      fPNDco[PND_num], fSpreadFactors[PND_num], random_val, (int)iSpreadFactors[PND_num]); 
fflush(stdout);
#endif

      }
   fclose(fp);

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Compute the SBS and SHD from the timing data for a given Threshold Note, we can get the raw bitstring
// with all 2048 bits by setting the Threshold to 0.

int SingleHelpBitGen(int max_PNDiffs, float *fPNDco, unsigned char *SBS, unsigned char *SHD, int *HD_num_bytes_ptr, 
   unsigned short Threshold)
   {
   int SBS_num_bits, PND_num;
   unsigned char bit_val;
   unsigned char HD_val;
   float fThreshold;

   fThreshold = (float)Threshold;

// Basic idea here is to carry out what the device did in the server authentication, compute the strong bitstring and single
// helper bitstring using the fPNDco 
   SBS_num_bits = 0;
   for ( PND_num = 0; PND_num < max_PNDiffs; PND_num++ )
      {

// Compute the bit value
      if ( fPNDco[PND_num] < 0.0 )
         bit_val = 0;
      else 
         bit_val = 1;

// Thresholding: if the fPNDco is within a 'Threshold' of the bit-flip line, consider it invalid. This works when Threshold of 0, which makes 
// ALL bits strong (required for KEK regeneration). MUST USE '>' to handle the case where the threshold is 0.
      if ( fPNDco[PND_num] > -fThreshold && fPNDco[PND_num] < fThreshold )
         HD_val = 0;
      else
         HD_val = 1;

#ifdef DEBUG
if ( PND_num < 10 )
   printf("%4d\tPNDco %+9.4f\tBit val %d\tHD val %d\n", PND_num, fPNDco[PND_num], bit_val, HD_val); 
fflush(stdout);
#endif

// Initialize the byte
      if ( (PND_num % 8) == 0 )
         SHD[PND_num/8] = 0;

// Store the helper bit if it is a '1' (otherwise, leave this bit at 0)
      if ( HD_val == 1 )
         SHD[PND_num/8] = SHD[PND_num/8] | (1 << (PND_num % 8));

// Do NOT add a strong bit to the 'SBS' for bits classified as weak
      else
         continue;

// Initialize the byte
      if ( (SBS_num_bits % 8) == 0 ) 
         SBS[SBS_num_bits/8] = 0;

// NOTE: WE DO NOT use a shift register approach for the strong bitstring in the hardware to make the strong bitstring consistent with the helper 
// data. So the last strong bits of LAST byte (highest numbered) are in the low order positions of the byte.
      if ( bit_val == 1 )
         SBS[SBS_num_bits/8] = SBS[SBS_num_bits/8] | (1 << (SBS_num_bits % 8));

      SBS_num_bits++;

// Sanity check
      if ( SBS_num_bits > max_PNDiffs )
         { printf("ERROR: SingleHelpBitGen(): SBS_num_bits larger than MAX -- increase in program!\n"); exit(EXIT_FAILURE); }
      }

#ifdef DEBUG
printf("\t\tSingleHelpBitGen(): Strong bitstring size %d (bits)\tSingle Helper Data size %d (bits)\n", SBS_num_bits, max_PNDiffs); fflush(stdout);
#endif

// Return the HD size in bytes -- this should always be 2048/8 = 256 bytes. Not really needed
   *HD_num_bytes_ptr = max_PNDiffs/8;

// Sanity check
   if ( Threshold == 0 && SBS_num_bits != max_PNDiffs )
      { printf("ERROR: SingleHelpBitGen(): Threshold is 0 but number of strong bits %d is NOT equal to max_PNDiffs %d!\n", SBS_num_bits, max_PNDiffs); exit(EXIT_FAILURE); }

   return SBS_num_bits;
   }


// ========================================================================================================
// ========================================================================================================
// Generate verifier nonce n1, send to device and get XOR nonce from device.

void GenNonceExchange(int max_string_len, int device_socket_desc, int num_required_nonce_bytes, 
   unsigned char *verifier_n2, unsigned char *XOR_nonce, int RANDOM, int DUMP_BITSTRINGS, int debug_flag)
   {
   struct timeval t0, t1;
   long elapsed; 

// ****************************************
// ***** Generate nonce n2. 
   if ( debug_flag == 1 )
      {
      printf("GNExch.1) Generating nonce n2\n");
      gettimeofday(&t0, 0);
      }

// Calling /dev/urandom here to get this
   if ( read(RANDOM, verifier_n2, num_required_nonce_bytes) == -1 )
      { printf("ERROR: GenNonceExchange(): Read /dev/urandom failed!\n"); exit(EXIT_FAILURE); }
   if ( debug_flag == 1 )
      { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed); }

// Dump out the values as hex
   if ( DUMP_BITSTRINGS == 1 )
      {
      char header_str[max_string_len];
      sprintf(header_str, "\n\tVerifier nonce n2 (high to low)\n");
      PrintHeaderAndHexVals(header_str, num_required_nonce_bytes, verifier_n2, 8);
      }

// ****************************************
// ***** Send the nonce n2 to the device
   if ( debug_flag == 1 )
      {
      printf("GNExch.2: Sending verifier nonce n2\n");
      gettimeofday(&t0, 0);
      }
   if ( SockSendB(verifier_n2, num_required_nonce_bytes, device_socket_desc) < 0 )
      { printf("ERROR: GenNonceExchange(): Failed to send 'verifier_n2' to device!\n"); exit(EXIT_FAILURE); }
   if ( debug_flag == 1 )
      { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed); }


// ****************************************
// ***** Receive nonce (n1 XOR n2) from device
   if ( debug_flag == 1 )
      {
      printf("GNExch.3) Receiving device nonce (n1 XOR n2)\n");
      gettimeofday(&t0, 0);
      }
   if ( SockGetB(XOR_nonce, num_required_nonce_bytes, device_socket_desc) != num_required_nonce_bytes )
      { printf("ERROR: GenNonceExchange(): Failed to get 'XOR_nonce' from device!\n"); exit(EXIT_FAILURE); }
   if ( debug_flag == 1 )
      { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed); }

// Dump out the values as hex
   if ( DUMP_BITSTRINGS == 1 )
      {
      char header_str[max_string_len];
      sprintf(header_str, "\n\tReceived device XOR nonces (n1 XOR n2) (high to low):\n");
      PrintHeaderAndHexVals(header_str, num_required_nonce_bytes, XOR_nonce, 8);
      }
   fflush(stdout);

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Compute and transmit SpreadFactors

void ComputeSendSpreadFactors(int max_string_len, SRFAlgoParamsStruct *SAP_ptr, int device_socket_desc, 
   int current_function, int send_SpreadFactors, int compute_PCR_PBD_SF)
   {
   struct timeval t0, t1;
   long elapsed; 

#ifdef DEBUG
printf("ComputeSendSpreadFactors(): CALLED!\n"); fflush(stdout);
#endif

// Clear the SpreadFactors array. 
   int PND_num;
   for ( PND_num = 0; PND_num < SAP_ptr->num_required_PNDiffs; PND_num++ ) 
      {
      SAP_ptr->iSpreadFactors[PND_num] = 0;
      SAP_ptr->fSpreadFactors[PND_num] = 0.0;
      }

// ----------------------------------------------------
// In the first call, we compute population SpreadFactors. We analysis the data from ALL chips (actually, if the number of chips is > 100, then we 
// only analyze the first 100) and compute a set of fSpreadFactors that center each PND distribution over the bit-flip line. 
   if ( SAP_ptr->DEBUG_FLAG == 1 )
      {
      printf("CO.1) Computing SpreadFactors, Part 1\n");
      gettimeofday(&t0, 0);
      }

// This routine ONLY computes the PopSF and stores results in SAP_ptr->i/fSpreadFactors. The 2-D arrays of PNR and PNF are referenced in this call.
// Note, when param_PCR_or_PBD_or_PO == SF_MODE_PBD, we do NOT use Pop. SF since the low order bit of the PNDc is used to determine the bit value. 
   int do_part_A_or_B = 0;
   if ( SAP_ptr->param_PCR_or_PBD_or_PO == SF_MODE_PCR || SAP_ptr->param_PCR_or_PBD_or_PO == SF_MODE_POPONLY )
      ComputePxxSpreadFactors(max_string_len, SAP_ptr, do_part_A_or_B);

#ifdef DEBUG
PrintHeaderAndHexVals("ComputeSendSpreadFactors(): Pop. SpreadFactors:\n", SAP_ptr->num_SF_bytes, (unsigned char *)SAP_ptr->iSpreadFactors, 32);
#endif

   if ( SAP_ptr->DEBUG_FLAG == 1 )
      { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed); }

   if ( SAP_ptr->DEBUG_FLAG == 1 )
      {
      char header_str[max_string_len];
      sprintf(header_str, "\n\tPopulation SpreadFactors %d\n", SAP_ptr->num_SF_bytes);
      PrintHeaderAndHexVals(header_str, SAP_ptr->num_SF_bytes, (unsigned char *)SAP_ptr->iSpreadFactors, 32);
      }

// If PCR or PBD are requested, then call ComputePxxSpreadFactors again.
   if ( compute_PCR_PBD_SF == 1 )
      {

// ----------------------------------------------------
// If requested, compute the PCR (this is done on the device now for DA and LL_Enroll). DoSRFComp() is called to compute fPNDco for the SAP_ptr->chip_num set, 
// and these are used to determine the adjustments that are made to fSpreadFactors. So we MUST set the chip_num BEFORE calling ComputePxxSpreadFactors. 
// Also, this routine updates the fPNDco with the PCR SpreadFactors. Useful in case we are computing a response bitstring within the verifier. Carry out 
// the full computation so we have the properly generated PND and PNDc.

// Sanity check. SAP_ptr->chip_num MUST be set to a number between 0 and num_chips-1.
      if ( SAP_ptr->chip_num < 0 || SAP_ptr->chip_num >= SAP_ptr->num_chips )
         { 
         printf("ERROR: ComputeSendSpreadFactors(): PCR component MUST have chip_num %d >= 0 and < num_chips %d\n", 
            SAP_ptr->chip_num, SAP_ptr->num_chips); exit(EXIT_FAILURE); 
         }

#ifdef DEBUG
printf("ComputeSendSpreadFactors(): Chip set for calculation of PCR SpreadFactors %d\n", SAP_ptr->chip_num); fflush(stdout);
#endif

      if ( SAP_ptr->DEBUG_FLAG == 1 )
         {
         printf("CO.1) Computing SpreadFactors, Part 2\n");
         gettimeofday(&t0, 0);
         }

// Second call to ComputePxxSpreadFactors. Note, we do NOT do SF at ALL when current function is DA or LL_ENROLL (they are computed on the device). 
      do_part_A_or_B = 1;
      if ( current_function != FUNC_DA && current_function != FUNC_LL_ENROLL )
         ComputePxxSpreadFactors(max_string_len, SAP_ptr, do_part_A_or_B);

      if ( SAP_ptr->DEBUG_FLAG == 1 )
         {
         printf("CO.2) Sending PCR SpreadFactors to Device\n");
         gettimeofday(&t0, 0);
         }
      }

// The iSpreadFactors are signed char (5- or 6-bit integer with 1- or 0-bits of binary precision), We can also fit these into 12-bits if needed which 
// gives +/-256.xxx (9 bits of integer and 3 bits of precision), reducing this transfer from 4096 bytes to 3072 bytes, but we would need to 'pack' them. 
// I can get back to 8-bits by giving up the 3 or 4 bits of precision. 
   if ( send_SpreadFactors == 1 )
      if ( SockSendB((unsigned char *)SAP_ptr->iSpreadFactors, SAP_ptr->num_SF_bytes, device_socket_desc) < 0 )
         { printf("ERROR: ComputeSendSpreadFactors(): Send 'PCR SpreadFactors' failed\n"); exit(EXIT_FAILURE); }
   if ( SAP_ptr->DEBUG_FLAG == 1 )
      { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed); }

   if ( SAP_ptr->DEBUG_FLAG == 1 )
      {
      char header_str[max_string_len];
      sprintf(header_str, "\n\tPCR SpreadFactors %d\n", SAP_ptr->num_SF_bytes);
      PrintHeaderAndHexVals(header_str, SAP_ptr->num_SF_bytes, (unsigned char *)SAP_ptr->iSpreadFactors, 32);
      }

   return;
   }


// ========================================================================================================
// ========================================================================================================
// This routine is a convenience routine to generate a random DB_ChallengeGen_seed, select a set of vectors
// from the timing DB and fetch the timing data into PNR and PNF arrays.

void GenVecSeedChlngsTimingData(int max_string_len, SRFAlgoParamsStruct *SAP_ptr, sqlite3 *timing_DB,
   char *ChlngSetName, TimingValCacheStruct *TVC_arr, int num_TVC_arr, int RANDOM)
   { 

// If the user wants to randomize the challenge vectors by selecting a random seed (vs. what is stored in 
// the SAP_ptr->Seed already), then get one from RANDOM and assign it. Only applicable to the DATABASE VERSION.
   if ( SAP_ptr->gen_random_challenge == 1 )
      {
      unsigned char seed_char[4];
      if ( read(RANDOM, seed_char, 4) == -1 )
         { printf("ERROR: GenVecSeedChlngsTimingData(): Read /dev/urandom failed!\n"); exit(EXIT_FAILURE); }
      SAP_ptr->DB_ChallengeGen_seed = seed_char[3] << 24 | seed_char[2] << 16 | seed_char[1] << 8 | seed_char[0];
      }

// Compute a random set of challenges from the compatibility set given by 'ChlngSetName'. This routine returns binary 
// vectors and masks (both are always returned) and an array of (vecpair, PO_num) used to locate the TimingVals data 
// for any one, a subset or all PUFInstances (chips). The field vecpair is an index into VecPair table for the PUFDesign 
// identifed using Netlist_name and Synthesis_name. NOTE: vecpair refers to the single configuration vector used in 
// the SRF PUF, which is split into two pieces (first_vec and second_vec) to optimize storage in the database. PO_num is 
// a value between 0 and num_POs - 1. EXACTLY NUM_REQUIRED_PNs values are returned in this array half of which identify 
// rising PN and the other half identify fall PNs. Both of these constraints are checked in GenChallengeDB. NOTE: THESE 
// VECTORS and DATA STRUCTURES MUST BE FREED once we are done with them. ONLY applicable to the DATABASE VERSION.
   if ( timing_DB != NULL )
      {
      VecPairPOStruct *challenge_vecpair_id_PO_arr = NULL;
      int num_challenge_vecpair_id_PO = 0;

      GenChallengeDB(max_string_len, timing_DB, SAP_ptr->design_index, ChlngSetName, SAP_ptr->DB_ChallengeGen_seed, 0, 
         NULL, NULL, &(SAP_ptr->first_vecs_b), &(SAP_ptr->second_vecs_b), &(SAP_ptr->masks_b), &(SAP_ptr->num_vecs), 
         &(SAP_ptr->num_rise_vecs), SAP_ptr->GenChallenge_mutex_ptr, &num_challenge_vecpair_id_PO, 
         &challenge_vecpair_id_PO_arr);

printf("\tGenVecSeedChlngsTimingData(): Number of vectors read %d\tNumber of rising vectors %d\n", SAP_ptr->num_vecs, 
   SAP_ptr->num_rise_vecs); fflush(stdout);
#ifdef DEBUG
#endif

// Get a subset of 2048 PNR and 2048 PNF of the timing data for all (or a subset) of PUFInstances. The set of 4096 timing 
// values are identified by an array of challenge_vecpair_id_PO_arr structures with (vecpair, PO) elements. These are 
// constructed by GenChallengeDB as the random challenge is generated and are guaranteed to match the PN tested by these 
// challenge vectors/masks. Use '%' for * and '_' for ? in pattern match. The PNR and PNF are DYNAMICALLY allocated based 
// on the challenge and will need to be freed once we are done with them.
      GetAllPUFInstanceTimingValsForChallenge(max_string_len, timing_DB, challenge_vecpair_id_PO_arr, num_challenge_vecpair_id_PO, 
         "%", &(SAP_ptr->PNR), &(SAP_ptr->PNF), &(SAP_ptr->num_chips), TVC_arr, num_TVC_arr, SAP_ptr->use_TVC_cache);

// Free up the challenge_vecpair_id_PO_arr. We'll free the vectors and timing data in the caller if it isn't needed again 
// for something else.
      if ( challenge_vecpair_id_PO_arr != NULL )
         free(challenge_vecpair_id_PO_arr);
      }
   else
      { printf("ERROR: Timing DB is NULL!\n"); exit(EXIT_FAILURE); }

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Common operations carried out indendent of the function.

void CommonCore(int max_string_len, SRFAlgoParamsStruct *SAP_ptr, int device_socket_desc, int RANDOM, 
   int set_threshold_to_zero, int target_attempts, int do_part_A_part_B_both, int current_function, 
   int compute_SpreadFactors, int send_SpreadFactors, int compute_PCR_PBD_SF)
   {
   struct timeval t0, t1;
   long elapsed; 

// Both authentication modes, session key and KEK call this routine with do_part_A_part_B_both set to 0 (do this if stmt and return) and then 
// with 1 (skip this if stmt and do the rest). This is true because we may need to call the CommonCore multiple times but do NOT want to execute 
// this if stmt after the first call. The multiple calls simply change the SRF parameters to generate additional key bits until the required 
// number is reached. We generate the challenge vectors and masks and then send them to the device. We are done with them at this point and 
// free them. During vector/mask generation, we also retrieve the timing values tested by these challenges (PNR and PNF) for ALL devices and 
// keep pass these back to the caller. So the caller will free the PNR and PNF once we are done with them.
   if ( do_part_A_part_B_both == 0 || do_part_A_part_B_both == 2 )
      {

      GenVecSeedChlngsTimingData(max_string_len, SAP_ptr, SAP_ptr->database_NAT, SAP_ptr->ChallengeSetName_NAT, SAP_ptr->TVC_arr_NAT, 
         SAP_ptr->num_TVC_arr_NAT, RANDOM);

// Receive 'GO' and send vectors and masks
      int wait_for_GO = 1;
      GoSendVectors(max_string_len, SAP_ptr->num_POs, SAP_ptr->num_PIs, device_socket_desc, SAP_ptr->num_vecs, SAP_ptr->num_rise_vecs, 
         SAP_ptr->has_masks, SAP_ptr->first_vecs_b, SAP_ptr->second_vecs_b, SAP_ptr->masks_b, wait_for_GO, SAP_ptr->use_database_chlngs, 
         SAP_ptr->DB_ChallengeGen_seed, SAP_ptr->DEBUG_FLAG);

// Generate verifier nonce n1, send to device and get XOR nonce from device.
      GenNonceExchange(max_string_len, device_socket_desc, SAP_ptr->num_required_nonce_bytes, SAP_ptr->verifier_n2, SAP_ptr->XOR_nonce, RANDOM, 
         SAP_ptr->DUMP_BITSTRINGS, SAP_ptr->DEBUG_FLAG);
      }

// If only part A is requested (Session Key Gen and Long-Lived), return. These routines will call CommonCore again with do_part_A_part_B_both set to 1
// possibly multiple times. If part B (Session Key Gen and Long-Lived) or both (Device Authentication and Verifier Authentication), do the rest.
   if ( do_part_A_part_B_both == 0 )
      return;

// ============================================================================
// Select parameter values. MUST DO THIS BEFORE ComputeSendSpreadFactors since we need the parameters to compute population SpreadFactors.
   if ( SAP_ptr->DEBUG_FLAG == 1 )
      {
      printf("ComCore.1) Selecting parameters\n");
      gettimeofday(&t0, 0);
      }

// Select parameters based on the value of XOR_nonce. 
   if ( SAP_ptr->fix_params == 0 )
      SelectParams(SAP_ptr->num_required_nonce_bytes, SAP_ptr->XOR_nonce, SAP_ptr->nonce_base_address, &(SAP_ptr->param_LFSR_seed_low), 
         &(SAP_ptr->param_LFSR_seed_high), &(SAP_ptr->param_RangeConstant), &(SAP_ptr->param_SpreadConstant), &(SAP_ptr->param_Threshold),
         &(SAP_ptr->param_TrimCodeConstant));

// Use randomly selected parameters if user requests fixed parameters.
   else
      {
      SAP_ptr->param_LFSR_seed_low = FIXED_LFSR_SEED_LOW;
      SAP_ptr->param_LFSR_seed_high = FIXED_LFSR_SEED_HIGH;
      SAP_ptr->param_RangeConstant = FIXED_RANGE_CONSTANT;
      SAP_ptr->param_SpreadConstant = FIXED_SPREAD_CONSTANT;
      SAP_ptr->param_Threshold = FIXED_THRESHOLD;
      }

   if ( SAP_ptr->DEBUG_FLAG == 1 )
      { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed); }

// Force threshold to be zero for RawBitstring.
   if ( set_threshold_to_zero == 1 )
      SAP_ptr->param_Threshold = 0;

// For iterating in session encryption and KEK until we reach a 'target_num_key_bits'. Assume same 'XOR_nonce' bytes are used repeatedly. 
// Just increment the LFSR seed (do NOT increment BOTH otherwise you get the same sequence).
   SAP_ptr->param_LFSR_seed_high = (SAP_ptr->param_LFSR_seed_high + target_attempts) % SAP_ptr->num_required_PNDiffs;

// Don't enable this -- it is called many times from Device SKE Authentication functions
#ifdef DEBUG
printf("\tLFSR seed low %u\tLFSR seed high %u\tRangeConstant %d\tSpreadConstant %u\tThreshold %u\n\n", 
   SAP_ptr->param_LFSR_seed_low, SAP_ptr->param_LFSR_seed_high, SAP_ptr->param_RangeConstant, SAP_ptr->param_SpreadConstant, 
   SAP_ptr->param_Threshold); fflush(stdout);
#endif

   if ( compute_SpreadFactors == 1 )
      ComputeSendSpreadFactors(max_string_len, SAP_ptr, device_socket_desc, current_function, send_SpreadFactors, compute_PCR_PBD_SF);

// Sometimes, we just want send the pre-computed SpreadFactors (compute_SpreadFactors == 0).
   else if ( send_SpreadFactors == 1 )
      if ( SockSendB((unsigned char *)SAP_ptr->iSpreadFactors, SAP_ptr->num_SF_bytes, device_socket_desc) < 0 )
         { printf("ERROR: CommonCore(): Send 'PCR SpreadFactors' failed\n"); exit(EXIT_FAILURE); }

   return;
   }


// ========================================================================================================
// ========================================================================================================
// A convenience function that calls CommonCore when do_part_A is 1 to generate challenges, collect timing data 
// from the database and send challenges and exchange nonces with device, and then always does part_B of 
// CommonCore where consecutative sets of SpreadFactors are generated and sent to the device. NOTE: target_attempts 
// is FORCED to 0 when 'do_part_A' is 1. The 'return_after_each_set' is used in device authentication to
// optimize the speed of the database search, where we return and the parent stores the SpreadFactors for each
// iteration in a larger array for re-use later.

int GenChlngDeliverSpreadFactorsToDevice(int max_string_len, SRFAlgoParamsStruct *SAP_ptr, int do_part_A, 
   int target_attempts, int RANDOM, int device_socket_desc, int *done_ptr, int return_after_each_set, 
   int compute_PCR_PBD_SF, int restore_store_SF, signed char **SpreadFactors_binary_ptr, int current_function)
   {
   int set_threshold_to_zero, do_part_A_part_B_both, compute_SpreadFactors, send_SpreadFactors;
   char request_str[max_string_len];
   int i;

   struct timeval t0, t1;
   long elapsed; 

// When 1, call CommonCore to generate challenges, collect timing data from the database and send challenges to the device 
// and exchange nonces with the device. DO NOT select parameters or generate/send SpreadFactors -- that is done in the loop below.
   if ( do_part_A == 1 )
      {
      set_threshold_to_zero = 0;
      do_part_A_part_B_both = 0;

// These don't matter when do_part_A_part_B_both is 0.
      compute_SpreadFactors = 0;
      send_SpreadFactors = 0;
      target_attempts = 0;
      CommonCore(max_string_len, SAP_ptr, device_socket_desc, RANDOM, set_threshold_to_zero, target_attempts, do_part_A_part_B_both, 
         current_function, compute_SpreadFactors, send_SpreadFactors, compute_PCR_PBD_SF);
      }

// We need to iterate here calling SelectParams and computing SpreadFactors until device indicates that it has generated enough bits. 
// Calls CommonCore doing ONLY part B, which selects paramters and computes and sends SpreadFactors.
   while (1)
      {

// Here we only select parameters, set the LFSR_seed_high and compute and send SpreadFactors. Feed the device with this information until 
// it indicates it has completed its operation. Do part_B only.
      set_threshold_to_zero = 0;
      do_part_A_part_B_both = 1;

// We do NOT compute or send ANY SpreadFactors if current function is DA or LL_Enroll and PBD mode is requested.
      if ( (current_function == FUNC_DA || current_function == FUNC_LL_ENROLL) && SAP_ptr->param_PCR_or_PBD_or_PO == SF_MODE_PBD )
         {
         compute_SpreadFactors = 0;
         send_SpreadFactors = 0;
         }
      else
         {
         compute_SpreadFactors = 1;
         send_SpreadFactors = 1;
         }

// Restore SpreadFactors if incoming *SpreadFactors_binary_ptr is NON-NULL. In this case, we've already computed them previously and
// want to re-use them.
      if ( compute_SpreadFactors == 1 && restore_store_SF == 0 && SpreadFactors_binary_ptr != NULL )
         {

// Be sure to restore the fSpreadFactors since they are used in the AddSpreadFactors software routine.
         for ( i = 0; i < SAP_ptr->num_SF_words; i++ )
            {
            SAP_ptr->iSpreadFactors[i] = (*SpreadFactors_binary_ptr)[target_attempts*SAP_ptr->num_SF_words + i];
            SAP_ptr->fSpreadFactors[i] = (float)SAP_ptr->iSpreadFactors[i]/(float)SAP_ptr->iSpreadFactorScaler;
            }
         compute_SpreadFactors = 0;
         }

      CommonCore(max_string_len, SAP_ptr, device_socket_desc, RANDOM, set_threshold_to_zero, target_attempts, do_part_A_part_B_both, 
         current_function, compute_SpreadFactors, send_SpreadFactors, compute_PCR_PBD_SF);

// Store the SpreadFactors if they are being computed assuming that we'll need them again.
      if ( compute_SpreadFactors == 1 && restore_store_SF == 1 )
         {
         if ( (*SpreadFactors_binary_ptr = (signed char *)realloc(*SpreadFactors_binary_ptr, 
            ((target_attempts+1) * SAP_ptr->num_SF_words) * sizeof(signed char))) == NULL )
            { printf("ERROR: GenChlngDeliverSpreadFactorsToDevice(): Failed to allocate storage for SpreadFactors_binary!\n"); exit(EXIT_FAILURE); }

         memcpy(&((*SpreadFactors_binary_ptr)[target_attempts*SAP_ptr->num_SF_words]), SAP_ptr->iSpreadFactors, SAP_ptr->num_SF_words);
//         for ( i = 0; i < SAP_ptr->num_SF_words; i++ )
//            (*SpreadFactors_binary_ptr)[target_attempts*SAP_ptr->num_SF_words + i] = SAP_ptr->iSpreadFactors[i];
         }

      target_attempts++;

// Continue to compute and send new SpreadFactors for each increment of the LFSR seed as needed by the device. 
      if ( SAP_ptr->DEBUG_FLAG == 1 )
         {
         printf("\tReceiving more SpreadFactors command\n");
         gettimeofday(&t0, 0);
         }
      if ( SockGetB((unsigned char *)request_str, max_string_len, device_socket_desc) == -1 )
         { printf("ERROR: GenChlngDeliverSpreadFactorsToDevice(): Receive SpreadFactors request failed!\n"); exit(EXIT_FAILURE); }
      if ( strcmp(request_str, "SPREAD_FACTORS DONE") == 0 )
         {
         *done_ptr = 1;
         break;
         }
      if ( SAP_ptr->DEBUG_FLAG == 1 )
         { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tTOTAL EXEC TIME %ld us\n\n", (long)elapsed); }

      if ( return_after_each_set == 1 )
         break;
      }

   return target_attempts;
   }


// ========================================================================================================
// ========================================================================================================
// We MUST save the individual curves to separate files here (unlike ZED analysis) because we keep adding to them as 
// authentications progress. 

void WriteAuthenPointToFile(int max_string_len, char *outfile_name, int authen_num, char *wfm_header_str, float fval)
   {
   FILE *OUTFILE;

   if ( authen_num == 0 && (OUTFILE = fopen(outfile_name, "w")) == NULL )
      { printf("ERROR: WriteAuthenPointToFile(): Data file '%s' open failed for writing!\n", outfile_name); exit(EXIT_FAILURE); }
   else if ( authen_num != 0 && (OUTFILE = fopen(outfile_name, "a")) == NULL )
      { printf("ERROR: WriteAuthenPointToFile(): Data file '%s' open failed for appending!\n", outfile_name); exit(EXIT_FAILURE); }

// We don't know the authenticating chip number here (unlike the ZED TV analysis). We print a message above and will need to make 
// note in the paper how many, if any, authentications fail.
   if ( authen_num == 0 )
      fprintf(OUTFILE, "%s", wfm_header_str);
   fprintf(OUTFILE, "%d\t%f\n", authen_num, fval);

   fclose(OUTFILE);
 
   return;
   }


// ========================================================================================================
// ========================================================================================================
// Find a match in the database to the SAP_ptr->KEK_authentication_nonce using the XMR_SHD helper data sent
// by the device. Note that multiple calls to CommonCore will LIKELY be needed to generate the full 
// authentication nonce. However, we can abort on any mismatches after the first call. We must find an exact
// match. SAP_ptr->chip_num is set to the chip number in the database on a successful match, otherwise
// the chip number remains at -1 (failure to authenticate device). Database search: find the chip whose data 
// produces a match to the n bits of the KEK_authentication_nonce. NOTE: PCR and PBD SpreadFactors are NOW
// computed by the device and transmitted to the server.

//#define DEBUG3 1

void KEK_DA_SKE_FindMatch(int max_string_len, SRFAlgoParamsStruct *SAP_ptr, int received_XMR_SHD_num_bytes, 
   unsigned char *SKE_authen_XMR_SHD, signed char *authen_SpreadFactors_binary, int current_function,
   int do_scaling)
   {
   int enroll_or_regen, SBS_num_bits, SHD_num_bytes, current_num_strong_bits, do_part_A_part_B_both, set_threshold_to_zero; 
   unsigned char KEK_authentication_nonce_reproduced[SAP_ptr->num_KEK_authen_nonce_bits/8];
   int chip_num, target_attempts, num_strong_bits;
   int bits_remaining, num_minority_bit_flips;
   unsigned short Threshold;
   int i, j;

   int check_all_chips, num_mismatches, true_minority_bit_flips; 

   int num_chips;

// FIX ME -- should be 0.
   static int authen_num = 0;

// 10_14_2022: Moved this data to ../ANALYSIS/...
   char KEK_Authen_base_dir[max_string_len];
   char KEK_SHD_base_dir[max_string_len];

// Directory for multiple ZYBO simultaneously running.
   strcpy(KEK_Authen_base_dir, "../ANALYSIS/PROTOCOL_V3.0_TDC/KEK_Authentication_data");
   strcpy(KEK_SHD_base_dir, "../ANALYSIS/PROTOCOL_V3.0_TDC/KEK_Authentication_SHD");

// Directory for individual ZYBO experiments (check for bugs in the multi-threading). IF YOU ADD THIS IN, BE SURE TO DELETE THE FILES
// IN THIS RESULTS DIR AND SET THE 'authen_num = 1' above.
//   strcpy(KEK_Authen_base_dir, "../ANALYSIS/PROTOCOL_V3.0_TDC/KEK_Authentication_data_INDIVID");
//   strcpy(KEK_SHD_base_dir, "../ANALYSIS/PROTOCOL_V3.0_TDC/KEK_Authentication_SHD_INDIVID");

// There are 4 basic components of information for SKE (only two for FSB). The number of strong bits (NSB), the number of mismatches (NMM),
// the number of minority bit flips (NMBF) and the number of true minority bit flis (NTBF).
   AuthenDataStruct *ADS = NULL;

   num_chips = SAP_ptr->num_chips;

// Sanity check. We need at least 4 chips in the DB
   if ( num_chips < 4 )
      { printf("ERROR: KEK_DA_SKE_FindMatch(): Must have at least 4 chips in the DB => %d!\n", num_chips); exit(EXIT_FAILURE); }

   if ( (ADS = (AuthenDataStruct *)calloc(num_chips, sizeof(AuthenDataStruct))) == NULL )
      { printf("ERROR: KEK_DA_SKE_FindMatch(): Failed to allocate ADS!\n"); exit(EXIT_FAILURE); }

// Set this to 1 to do all comparisons, which is more robust authentication method but takes longer. If set to 0, then we break out of the
// inner loop that searches chunks of the KEK_authentication_nonce_reproduced bitstring, AND we select the first chip that has 0 mismatches
// and has num_minority_bit_flips less than a threshold, e.g. 15. This makes this routine twice as fast because we only need to look at half 
// the chips in the DB on average. If we are saving PARCE file stats, force this routine to check all chips.
   check_all_chips = 1;
   if ( SAP_ptr->do_save_PARCE_COBRA_file_stats == 1 )
      check_all_chips = 1;

#ifdef DEBUG3
printf("KEK_DA_SKE_FindMatch(): Called with %d bytes of device-generated XMR_SHD!\n", received_XMR_SHD_num_bytes); fflush(stdout);
#endif

// Sanity check
   if ( (received_XMR_SHD_num_bytes % (SAP_ptr->num_required_PNDiffs/8)) != 0 )
      { 
      printf("ERROR: KEK_DA_SKE_FindMatch(): received_XMR_SHD_num_bytes %d MUST be evenly divisible by %d!\n", 
         received_XMR_SHD_num_bytes, SAP_ptr->num_required_PNDiffs/8); 
      exit(EXIT_FAILURE); 
      }

#ifdef DEBUG3
PrintHeaderAndHexVals("ORIGINAL AUTHENTICATION NONCE:\n", SAP_ptr->num_KEK_authen_nonce_bits/8, (unsigned char *)SAP_ptr->KEK_authentication_nonce, 32);
#endif

// 10_11_2022: The SAP_ptr data structure is shared. I don't think we should allow more than one task to operate on it at the same time.
// NO, IT IS NOT. Each task has it's own copy of an element from the SAP array.
//   pthread_mutex_lock(SAP_ptr->Authentication_mutex_ptr);

// =================================================================================================================================
// =================================================================================================================================
   int num_pos_vals = 0;
   int num_neg_vals = 0;
   int num_zero_vals = 0;
   int PND_num_inspect = 0;

   int PND_num; 

   enroll_or_regen = 1;
   for ( chip_num = 0; chip_num < num_chips; chip_num++ )
      {

// Run the SRF engine and compute the fPNDco for this chip. 
      SAP_ptr->chip_num = chip_num;

// Sanity check.
      if ( do_scaling == 1 && SAP_ptr->ChipScalingConstantNotifiedArr[SAP_ptr->chip_num] == 1 && SAP_ptr->ChipScalingConstantArr[SAP_ptr->chip_num] == 0.0 )
         { printf("ERROR: KEK_DA_SKE_FindMatch(): ChipScalingConstantNotifiedArr[x] is 1 but ChipScalingConstantArr[x] value is 0.0"); exit(EXIT_FAILURE); }

#ifdef DEBUG3
printf("KEK_DA_SKE_FindMatch(): Checking chip %d\n", SAP_ptr->chip_num); fflush(stdout);
#endif
      ADS[chip_num].index = chip_num;
      ADS[chip_num].NSB = 0;
      ADS[chip_num].NMM = 0.0;
      ADS[chip_num].NMBF = 0.0;
      ADS[chip_num].NTBF = 0.0;
      ADS[chip_num].CC = 0.0;

// ASSUME that the timing vals (PNR and PNF) have already been allocated and stored in the SAP fields based on a challenge 
// and the XOR_nonce has been set with the first call to CommonCore with do_part_A_part_B_both set to 0. Since multiple
// calls to CommonCore have been made to give updated SpreadFactors to the device for each iteration, we must call CommonCore 
// here with do_part_A_part_B_both set to 1, which will select parameters and set the LFSR_seed_high according to the number
// of target_attempts. 
      current_num_strong_bits = 0;
      num_mismatches = 0;
      num_minority_bit_flips = 0;
      true_minority_bit_flips = 0;
      bits_remaining = SAP_ptr->num_KEK_authen_nonce_bits;

// Assume multiple iterations of KEK were needed to generate enough SKE_authen_XMR_SHD helper data to encode the entire nonce.
      target_attempts = 0;
      while ( current_num_strong_bits < SAP_ptr->num_KEK_authen_nonce_bits )
         {

// Call CommonCore to reset the parameters (LFSR_seed_high) based on XOR_nonce and target_attempts. Do not compute or send SpreadFactors.
         int compute_SpreadFactors = 0;
         int send_SpreadFactors = 0;

// Do NOT compute PCR (or PBD) SF (which is irrelevant because compute_SpreadFactors is 0).
         int compute_PCR_PBD_SF = 0;

         do_part_A_part_B_both = 1;

         set_threshold_to_zero = 0;
         CommonCore(max_string_len, SAP_ptr, 0, 0, set_threshold_to_zero, target_attempts, do_part_A_part_B_both, current_function, 
            compute_SpreadFactors, send_SpreadFactors, compute_PCR_PBD_SF);

// Use SpreadFactors already collected by parent. 
         for ( i = 0, j = target_attempts * SAP_ptr->num_SF_words; i < SAP_ptr->num_SF_words; i++, j++ )
            {
            SAP_ptr->iSpreadFactors[i] = authen_SpreadFactors_binary[j];

// Note: iSpreadFactorScaler is either 2 (or 1), depending on the TRIMCODE_CONSTANT (if <= 32, it is 2, else 1).
            SAP_ptr->fSpreadFactors[i] = (float)authen_SpreadFactors_binary[j]/(float)SAP_ptr->iSpreadFactorScaler;
            }

// SAME server-generated SF is used for EVERY chip (NOTE THESE are (signed char)). Validated this with SHD on chip.
#ifdef DEBUG3
//PrintHeaderAndHexVals("SERVER: SF:\n", SAP_ptr->num_SF_words, (unsigned char *)SAP_ptr->iSpreadFactors, 32);
#endif


// Do SRF Engine operations in software. Note, SF are signed char now and are computed to move PNDco (using PNDc) to the nearest lower multiple
// of TrimCodeConstant. AddSpreadFactors in this routine will remove the DC bias by repeatedly adding (if PNDco is negative) or subtracting
// (if PNDco is positve) TrimCodeConstant while the PNDco is < -TrimCodeConstant/2 or > TrimCodeConstant/2.
         DoSRFComp(max_string_len, SAP_ptr, (target_attempts == 0));

// 10_23_2022: Newest (correct) version which uses ScalingConstants.
         if ( do_scaling == 1 && SAP_ptr->ChipScalingConstantNotifiedArr[SAP_ptr->chip_num] == 1 )
            {
            for ( PND_num = 0; PND_num < SAP_ptr->num_required_PNDiffs; PND_num++ )
               {

#ifdef DEBUG3
float temp = (float)((int)(SAP_ptr->fPNDco[PND_num] * SAP_ptr->ChipScalingConstantArr[SAP_ptr->chip_num] * 16.0))/16.0;
if ( SAP_ptr->chip_num >= 4 && SAP_ptr->chip_num < 8 && target_attempts == 0 && PND_num < 50 )
   printf("Chip %3d\tScaling Constant %f\tDevice SHD bit [%d] %d\tOrig fPNDco %f\tScaled fPNDco %f\tBit val %d\n", 
      SAP_ptr->chip_num, SAP_ptr->ChipScalingConstantArr[SAP_ptr->chip_num], 
      PND_num, GetBitFromByte((SKE_authen_XMR_SHD + target_attempts*SAP_ptr->num_required_PNDiffs/8)[PND_num/8], PND_num % 8),
      SAP_ptr->fPNDco[PND_num], temp, (int)(temp >= 0));
fflush(stdout);
#endif
               SAP_ptr->fPNDco[PND_num] = (float)((int)(SAP_ptr->fPNDco[PND_num] * SAP_ptr->ChipScalingConstantArr[SAP_ptr->chip_num] * 16.0))/16.0;
               }

#ifdef DEBUG3
printf("Chip %3d\tScaled fPNDco with ScalingConstant %f\n", SAP_ptr->chip_num, SAP_ptr->ChipScalingConstantArr[SAP_ptr->chip_num]); fflush(stdout);
#endif
            }

// The KEK_FSB_SKE() routine in common.c wants the raw bitstring (computed with Threshold set to 0). Run the SingleHelpBitGen algorithm with the 
// Threshold set to 0.
         Threshold = 0;
         SBS_num_bits = SingleHelpBitGen(SAP_ptr->num_required_PNDiffs, SAP_ptr->fPNDco, SAP_ptr->device_SBS, SAP_ptr->device_SHD, 
            &SHD_num_bytes, Threshold);

// Sanity check. With Threshold set to 0, the number of strong bits is the same size as the SHD.
         if ( SBS_num_bits != SAP_ptr->num_required_PNDiffs )
            { 
            printf("ERROR: KEK_DA_SKE_FindMatch(): Number of bits in raw bitstring must be %d -- found %d!\n", 
               SAP_ptr->num_required_PNDiffs, SBS_num_bits); 
            exit(EXIT_FAILURE); 
            }

// Sanity check. The number of iterations here should NEVER exceed what the device did to generate the received_XMR_SHD_num_bytes.
         if ( target_attempts*SAP_ptr->num_required_PNDiffs/8 >= received_XMR_SHD_num_bytes )
            { printf("ERROR: KEK_DA_SKE_FindMatch(): server target attempts exceed size of SKE_authen_XMR_SHD!\n"); exit(EXIT_FAILURE); }

// SAME device-generated SHD (helper data) is used for EVERY chip. Validated this with SHD on chip.
#ifdef DEBUG3
//PrintHeaderAndHexVals("SERVER: XMR_SHD_chunk:\n", SAP_ptr->num_required_PNDiffs/8, (unsigned char *)(SKE_authen_XMR_SHD + target_attempts*SAP_ptr->num_required_PNDiffs/8), 32);
#endif

// The PRODUCT of KEK_FSB_SKE() during regeneration mode is KEK_authentication_nonce_reproduced, which uses the XMR SHD helper data generated 
// during enrollment and the raw bitstring device_SBS. Each iteration generates a portion of the KEK_authentication_nonce. It returns a portion of 
// KEK_authentication_nonce_reproduced that was generated. Note that 'bits_remaining' during regeneration is used to exit the loop early when the
// number of bits produced reaches the max (same thing that happens during enrollment when we are encoding KEK_authentication_nonce). 
//
// We record some statistics to help make the chip selection process robust. In cases where there are NO disagreements between the actual nonce
// and the regenerated nonce, then num_minority_bit_flips counts the number of bits in the XMR scheme that disagree with the majority vote.
// Also when num_mismatches is zero (computed below and also in this routine) then num_minority_bit_flips == true_minority_bit_flips. When num_mismatches 
// is > 0 (some majority vote bits disagree with the true nonce), then true_minority_bit_flips needs to be used to determine how many bits disgree with 
// an XMR encoded version of the true nonce. In either case, num_minority_bit_flips and true_minority_bit_flips count the number of bits that mismatch 
// and should be used in the statistics to show distinguishability.
         int do_mismatch_count = 1;
         int FSB_or_SKE = 1;
         num_strong_bits = KEK_FSB_SKE(SAP_ptr->num_required_PNDiffs, SAP_ptr->XMR_val, 
            SKE_authen_XMR_SHD + target_attempts*SAP_ptr->num_required_PNDiffs/8, SAP_ptr->device_SBS, NULL, bits_remaining, 
            KEK_authentication_nonce_reproduced, enroll_or_regen, do_mismatch_count, &num_minority_bit_flips, current_num_strong_bits, 
            SAP_ptr->KEK_authentication_nonce, &true_minority_bit_flips, FSB_or_SKE, chip_num, 0);
         bits_remaining -= num_strong_bits;


if ( target_attempts == 0 )
   {
   if ( SAP_ptr->fPNDco[PND_num_inspect] > 0.0 )
      num_pos_vals++;
   if ( SAP_ptr->fPNDco[PND_num_inspect] < 0.0 )
      num_neg_vals++;
   if ( SAP_ptr->fPNDco[PND_num_inspect] == 0.0 )
      num_zero_vals++;
#ifdef DEBUG3
   printf("fSpreadFactor[%d] %8.4f\tiSpreadFactor[%d] %3d\n", PND_num_inspect, SAP_ptr->fSpreadFactors[PND_num_inspect], PND_num_inspect, SAP_ptr->iSpreadFactors[PND_num_inspect]); fflush(stdout);
#endif
   }


#ifdef DEBUG3
if ( chip_num == 0 )
printf("\tCHIP x: Target attempts %d for chip %d number of strong bits to match %d\n", target_attempts, SAP_ptr->chip_num, num_strong_bits); 
fflush(stdout);
#endif

#ifdef DEBUG3
PrintHeaderAndHexVals("ORIGINAL AUTHENTICATION NONCE:\n", SAP_ptr->num_KEK_authen_nonce_bits/8, (unsigned char *)SAP_ptr->KEK_authentication_nonce, 32);
#endif

// Count the number of mismatches. Do NOT try to match bits beyond the last KEK_authentication_nonce bit.
         for ( i = current_num_strong_bits, j = 0; i < current_num_strong_bits + num_strong_bits && i < SAP_ptr->num_KEK_authen_nonce_bits; i++, j++ )
            if ( GetBitFromByte(KEK_authentication_nonce_reproduced[j/8], j % 8) != GetBitFromByte(SAP_ptr->KEK_authentication_nonce[i/8], i % 8) )
               {

#ifdef DEBUG3
//if ( chip_num == 0 )
printf("\tMISMATCH for chip %d on bit %d\n", SAP_ptr->chip_num, i); 
fflush(stdout);
#endif

// ***** NOTE: THE DEVICE IS STILL COMPUTING PCR AND COMPUTING XMR_SHD USING PCR OFFSETS EVEN THOUGH WE ARE USING POPULATION ONLY HERE ON THE 
// SERVER -- WE WILL NOT BE ABLE TO GET 0 MISMATCHES BECAUSE OF THIS. I would need to disable disable PCR mode on the device by making 
// some changes, e.g., do NOT invoke the DA authentication function in the hardware.
               num_mismatches++;
               if ( check_all_chips == 0 )
                  break;
               }

// DEBUG ONLY, BUT NEED TO KEEP TRACK of how many KEK_authentication_nonce bit have been reproduced (current_num_strong_bits). 
         current_num_strong_bits = JoinBytePackedBitStrings(current_num_strong_bits, &(SAP_ptr->DA_nonce_reproduced), num_strong_bits, 
            KEK_authentication_nonce_reproduced);

// Keep updating these on multiple iterations.
         ADS[chip_num].NSB = current_num_strong_bits;
         ADS[chip_num].NMM = (float)num_mismatches;
         ADS[chip_num].NMBF += (float)num_minority_bit_flips;
         ADS[chip_num].NTBF += (float)true_minority_bit_flips;

         if ( num_strong_bits == 0 )
            { printf("ERROR: Chip %d\tNumber of strong bits is 0!\n", chip_num); exit(EXIT_FAILURE); }

// Compute the CC. Smaller is better here, where NMM and NTBF are both zero is the best achievable.
// Note that true_minority_bit_flips INCORPORATES the number of mismatches so we do NOT need add them to the numerator here. It is identical
// to num_minority_bit_flips when there are NO mismatches but when there is a mismatch(es), then the complement of the minority is added in.
//         CC[chip_num] = ADS[chip_num].NTBF/(float)ADS[chip_num].NSB*100.0;
         ADS[chip_num].CC = ADS[chip_num].NTBF + ADS[chip_num].NMM;

// If we exit the bit-check loop early, a bit was found that mismatched. Here we break again from the 'chunk' loop. Note, if multiple 
// iterations are used (very likely), then we can also exit the above loop when we process the last of the KEK_authentication_nonce bits 
// (and find a match). This represents an authentication success so DO NOT EXECUTE this break under those conditions.
         if ( j < num_strong_bits && i != SAP_ptr->num_KEK_authen_nonce_bits )
            if ( check_all_chips == 0 )
               break;

// Set number of bits matched to max size when we processed all bits in the authentication nonce. Not sure if this is needed now???
         if ( i == SAP_ptr->num_KEK_authen_nonce_bits )
            current_num_strong_bits = SAP_ptr->num_KEK_authen_nonce_bits;

#ifdef DEBUG3
if ( num_mismatches == 0 )
   printf("\tMATCHED on iteration %2d for chip %3d\tTotal bits matched %5d\tWith bits remaining %4d\tCummulative number matching %5d\tCummulative minority bit flips %4d\n", 
      target_attempts, SAP_ptr->chip_num, num_strong_bits, bits_remaining, current_num_strong_bits, num_minority_bit_flips); 
fflush(stdout);
#endif

         target_attempts++;
         }

#ifdef DEBUG3
if ( chip_num < 4 )
PrintHeaderAndHexVals("Nonce Reproduced:\n", current_num_strong_bits/8, (unsigned char *)SAP_ptr->DA_nonce_reproduced, 32);
#endif

// Free the DA_nonce_reproduced if it was allocated. The DA_nonce_reproduced is NOT USED by this routine. DEBUG ONLY.
      if ( SAP_ptr->DA_nonce_reproduced != NULL )
         free(SAP_ptr->DA_nonce_reproduced);
      SAP_ptr->DA_nonce_reproduced = NULL;

#ifdef DEBUG3
if ( num_mismatches == 0 )
   printf("\tMATCHED***\tAuthen num %d\tFor chip %3d\tTotal bits mismatched %5d\tFrom total bits compared %5d\tWith bits remaining %4d\tTotal minority bit flips %d\n",
      authen_num, SAP_ptr->chip_num, num_mismatches, current_num_strong_bits, bits_remaining, num_minority_bit_flips); 
else
   {
   printf("\t\tMISMATCHED\tAuthen num %d\tFor chip %3d\tTotal bits mismatched %5d\tFrom total bits compared %5d\tWith bits remaining %4d\tTotal minority bit flips %d\t",
      authen_num, SAP_ptr->chip_num, num_mismatches, current_num_strong_bits, bits_remaining, num_minority_bit_flips); 

// If we are NOT doing all comparisons, then this fraction is meaningless since we exit on the first mismatch above. In which case, don't print it.
   if ( check_all_chips == 1 )
      {
      printf("Fraction %5.2f\t", (float)num_mismatches/(float)current_num_strong_bits*100);

// Flag those less than 10% different.
      if ( (float)num_mismatches/(float)current_num_strong_bits*100 < 10.0 )
         printf("*\n");
      else
         printf("\n");
      }
   else
      printf("\n");
   fflush(stdout);
   }
#endif

// Handle case with no mismatches! Break out of the chip search loop early IF check_all_chips is 0.
      if ( num_mismatches == 0 )
         {

// When we select the first chip that has a CC less than the threshold, then this routine is fast because we only need to look at half the 
// chips in the DB on average.
         if ( check_all_chips == 0 && ADS[chip_num].CC <= CC_SKE_AUTHEN_THRESHOLD )
            break;
         }
      }

#ifdef DEBUG
printf("BALANCE: PND_num %4d\tnum_pos_vals %4d\tnum_neg_vals %4d\tnum_zero_vals %4d\n", PND_num_inspect, num_pos_vals, num_neg_vals, num_zero_vals); fflush(stdout); 
#endif

// 10_11_2022: The SAP_ptr data structure is shared. I don't think we should allow more than one task to operate on it at the same time.
// NO, IT IS NOT. Each task has it's own copy of an element from the SAP array.
//   pthread_mutex_unlock(SAP_ptr->Authentication_mutex_ptr);


// ==============================================
// ==============================================
// Compute stats. First sort on CC in ascending order. We want the SMALLEST at the top of the list.
   qsort(ADS, num_chips, sizeof(AuthenDataStruct), ADS_CC_AscendCompareFunc); 

#ifdef DEBUG3
for ( chip_num = 0; chip_num < num_chips; chip_num++ )
   printf("Cnter %3d\tChip %3d\tCC %.0f\n", chip_num, ADS[chip_num].index, ADS[chip_num].CC);
#endif

#ifdef DEBUG3
printf("\n\nPCC ANALYSIS\n"); fflush(stdout);
#endif
   int do_PCH = 1;

   float AE_PCC, NE_PCC; 
   float first_diff_CC, second_diff_CC; 

// NTBF
   first_diff_CC = ADS[1].CC - ADS[0].CC;
   second_diff_CC = ADS[2].CC - ADS[1].CC;

#ifdef DEBUG
   float third_diff_CC;
   third_diff_CC = ADS[3].CC - ADS[2].CC;
#endif

// Use the SMALLEST CCs are better since they are computed as the NTBF -- smaller number of bit flips is better. 
   if ( do_PCH == 0 )
      {
      AE_PCC = first_diff_CC;
      NE_PCC = second_diff_CC;
      }
   else
      {
      if ( ADS[1].CC != 0.0 )
         AE_PCC = first_diff_CC/ADS[1].CC*100.0;
      else
         { printf("ERROR: SKE: ADS[1].CC divisor is 0.0 -- UNEXPECTED!\n"); exit(EXIT_FAILURE); }

// NE PCC to authentic, use percentage change of second and third. I did NOT do this for Cobra MDPI paper. I only reported 
// the AE_PCC for the authentic chip
      if ( ADS[2].CC != 0.0 )
         NE_PCC = second_diff_CC/ADS[2].CC*100.0;
      else
         { printf("ERROR: SKE: ADS[2].CC divisor is 0.0! -- UNEXPECTED!\n"); exit(EXIT_FAILURE); }
      }

#ifdef DEBUG
   float AE_PCC_metric, NE_PCC_metric;
   float NE2_PCC = third_diff_CC/ADS[3].CC*100.0;
   AE_PCC_metric = AE_PCC - NE_PCC;
   NE_PCC_metric = NE_PCC - NE2_PCC;
printf("SKE\tAuthen %d\tMode %d\tXMR %2d\tLFSR low %4d\tLFSR high %4d\tRC %3d\tSC %2d\tTH %d\tTCC %2d\tWinning NSB %d\n",
   authen_num, SAP_ptr->param_PCR_or_PBD_or_PO, SAP_ptr->XMR_val, SAP_ptr->param_LFSR_seed_low, SAP_ptr->param_LFSR_seed_high,
   SAP_ptr->param_RangeConstant, SAP_ptr->param_SpreadConstant, SAP_ptr->param_Threshold, SAP_ptr->param_TrimCodeConstant, ADS[0].NSB); fflush(stdout);
printf("\t\tAE PCC %.2f\tNE PCC %.2f\tCC ranking: 1st %3d (%.3f)\t2nd %3d (%.3f)\t3rd %3d (%.3f)\tDIFFS: 1st %.0f\t2nd %.0f\t3rd %.0f\t1st NMM %.0f\t2nd NMM %.0f\t3rd NMM %.0f\tAEM %.3f\tNEM %.3f\n",
   AE_PCC, NE_PCC, ADS[0].index, ADS[0].CC, ADS[1].index, ADS[1].CC, ADS[2].index, ADS[2].CC, 
   first_diff_CC, second_diff_CC, third_diff_CC, ADS[0].NMM, ADS[1].NMM, ADS[2].NMM,
   AE_PCC_metric, NE_PCC_metric);
fflush(stdout);
#endif


// SUCCESSFUL AUTHENTICATION
   SAP_ptr->chip_num = -1;
   if ( AE_PCC > PCC_SKE_AUTHEN_THRESHOLD )
      { 

// Information available during TESTING ONLY.
      if ( SAP_ptr->my_chip_num == -1 || ADS[0].index == SAP_ptr->my_chip_num )
         {
         SAP_ptr->chip_num = ADS[0].index;

         SQLIntStruct PUF_instance_index_struct;
         char InstanceName[500];
         char Dev[500];
         char Placement[500];
         char ID_str[max_string_len];

// Get the PUFInstance name using the chip_num stored in the SAP_ptr (which is the chip_num associated with the bitstring). First get 
// a list of all PUFInstance ids. Use '%' for * and '_' for ?
         GetPUFInstanceIDsForInstanceName(max_string_len, SAP_ptr->database_NAT, &PUF_instance_index_struct, "%");

// Sanity check
         if ( PUF_instance_index_struct.num_ints == 0 )
            { printf("ERROR: SaveDBBitstringInfo(): No PUFInstances found!\n"); exit(EXIT_FAILURE); }

// The chip name is stored in the PUFInstance database under the following id. Get the string information from the PUFInstance table.
// ONLY 500 characters allocated for these strings above.
         InstanceName[0] = '\0'; Dev[0] = '\0'; Placement[0] = '\0';
         GetPUFInstanceInfoForID(max_string_len, SAP_ptr->database_NAT, PUF_instance_index_struct.int_arr[SAP_ptr->chip_num], InstanceName, 
            Dev, Placement);
         strcpy(ID_str, "Instance Name: ");
         strcat(ID_str, InstanceName);
         strcat(ID_str, "  Device Name: ");
         strcat(ID_str, Dev);
         strcat(ID_str, "  Placement Name: ");
         strcat(ID_str, Placement);

         if ( SAP_ptr->ChipScalingConstantArr != NULL )
            printf("\tSKE SUCCESSFUL AUTHENTICATION: Chip %3d\tPersonalized ScalingConstant %f (Scaling? %d)\tID %s!\n", 
               SAP_ptr->chip_num, SAP_ptr->ChipScalingConstantArr[SAP_ptr->chip_num], do_scaling, ID_str); 
         else
            printf("\tSKE SUCCESSFUL AUTHENTICATION: Chip %3d\tID %s!\n", SAP_ptr->chip_num, ID_str); 
         fflush(stdout);

         if ( PUF_instance_index_struct.int_arr != NULL )
            free(PUF_instance_index_struct.int_arr); 

         if ( do_scaling == 1 && SAP_ptr->ChipScalingConstantNotifiedArr[SAP_ptr->chip_num] == 1 && 
            (int)(SAP_ptr->ChipScalingConstantArr[SAP_ptr->chip_num]*1000.0) != (int)(SAP_ptr->my_scaling_constant*1000.0) )
            {

// Do NOT exit here. I will NOT normally need to receive 'my_scaling_constant' from the device -- ONLY FOR TESTING. During GenLLK (when this function is called
// as opposed to test mode), I do NOT send chip DEBUG information to the server and so this data structure element is un-initialized. However,
// ChipScalingConstantArr IS FILLED IN and we can assume the device is ACTUALLY using the scaling constant.
            printf("WARNING: Chip %3d\tPersonalized ScalingConstant %f DOES NOT MATCH value received %f!\n", 
               SAP_ptr->chip_num, SAP_ptr->ChipScalingConstantArr[SAP_ptr->chip_num], SAP_ptr->my_scaling_constant);
//            exit(EXIT_FAILURE); 
            }
         }
      }

// FAILED AUTHENTICATION
   if ( SAP_ptr->chip_num == -1 )
      { printf("\tFAILED AUTHENTICATION!\n"); fflush(stdout); }

// ==============================================
// ==============================================
// Save stats to file if requested.
   if ( SAP_ptr->do_save_PARCE_COBRA_file_stats == 1 )
      {
      char outfile_name[max_string_len];
      char wfm_header_str[max_string_len];

      pthread_mutex_lock(SAP_ptr->FileStat_mutex_ptr);

// The first file gives the smallest CC that was found. We don't know the authenticating chip number here (unlike the ZED TV analysis) --
// actually we do now since I set the authenticating IP and bitstream number in the SAP structure. We print a message above and will need 
// to make note in the paper how many, if any, authentications fail.
if ( 0 )
   {
      sprintf(outfile_name, "%s/KEK_SKE_RC_%d_SF_%d_TH_%d_XMR_%d_first_smallest_CC.xy", KEK_Authen_base_dir, 
         SAP_ptr->param_RangeConstant, SAP_ptr->param_SpreadConstant, SAP_ptr->param_Threshold, SAP_ptr->XMR_val); 
      sprintf(wfm_header_str, "SKE_XMR_%d_first_smallest_CCs\n", SAP_ptr->XMR_val);
      WriteAuthenPointToFile(max_string_len, outfile_name, authen_num, wfm_header_str, ADS[0].CC);

// The second file gives the second smallest CC that was found.
      sprintf(outfile_name, "%s/KEK_SKE_RC_%d_SF_%d_TH_%d_XMR_%d_second_smallest_CC.xy", KEK_Authen_base_dir, 
         SAP_ptr->param_RangeConstant, SAP_ptr->param_SpreadConstant, SAP_ptr->param_Threshold, SAP_ptr->XMR_val); 
      sprintf(wfm_header_str, "SKE_XMR_%d_second_smallest_CCs\n", SAP_ptr->XMR_val);
      WriteAuthenPointToFile(max_string_len, outfile_name, authen_num, wfm_header_str, ADS[1].CC);

// The third file gives the third smallest CC that was found.
      sprintf(outfile_name, "%s/KEK_SKE_RC_%d_SF_%d_TH_%d_XMR_%d_third_smallest_CC.xy", KEK_Authen_base_dir, 
         SAP_ptr->param_RangeConstant, SAP_ptr->param_SpreadConstant, SAP_ptr->param_Threshold, SAP_ptr->XMR_val); 
      sprintf(wfm_header_str, "SKE_XMR_%d_third_smallest_CCs\n", SAP_ptr->XMR_val);
      WriteAuthenPointToFile(max_string_len, outfile_name, authen_num, wfm_header_str, ADS[2].CC);

// The fourth file gives the average CC. 
      float ave_CC = 0.0; 
      for ( chip_num = 0; chip_num < num_chips; chip_num++ )
         ave_CC += ADS[chip_num].CC;
      ave_CC /= num_chips;

      sprintf(outfile_name, "%s/KEK_SKE_RC_%d_SF_%d_TH_%d_XMR_%d_ave_CC.xy", KEK_Authen_base_dir, 
         SAP_ptr->param_RangeConstant, SAP_ptr->param_SpreadConstant, SAP_ptr->param_Threshold, SAP_ptr->XMR_val); 
      sprintf(wfm_header_str, "SKE_XMR_%d_ave_CCs\n", SAP_ptr->XMR_val);
      WriteAuthenPointToFile(max_string_len, outfile_name, authen_num, wfm_header_str, ave_CC);
   }

// The fifth file gives the AE_PCC.
      sprintf(outfile_name, "%s/KEK_SKE_RC_%d_SF_%d_TH_%d_XMR_%d_AE_PCC.xy", KEK_Authen_base_dir, 
         SAP_ptr->param_RangeConstant, SAP_ptr->param_SpreadConstant, SAP_ptr->param_Threshold, SAP_ptr->XMR_val); 
      sprintf(wfm_header_str, "SKE_XMR_%d_AE_PCCs\n", SAP_ptr->XMR_val);
//      WriteAuthenPointToFile(max_string_len, outfile_name, authen_num, wfm_header_str, AE_PCC);
      WriteAuthenPointToFile(max_string_len, outfile_name, authen_num, wfm_header_str, AE_PCC);

// The sixth file gives the NE PCC.
      sprintf(outfile_name, "%s/KEK_SKE_RC_%d_SF_%d_TH_%d_XMR_%d_NE_PCC.xy", KEK_Authen_base_dir, 
         SAP_ptr->param_RangeConstant, SAP_ptr->param_SpreadConstant, SAP_ptr->param_Threshold, SAP_ptr->XMR_val); 
      sprintf(wfm_header_str, "SKE_XMR_%d_NE_PCCs\n", SAP_ptr->XMR_val);
//      WriteAuthenPointToFile(max_string_len, outfile_name, authen_num, wfm_header_str, NE_PCC);
      WriteAuthenPointToFile(max_string_len, outfile_name, authen_num, wfm_header_str, NE_PCC);

      sprintf(outfile_name, "%s/KEK_SKE_RC_%d_SF_%d_TH_%d_XMR_%d_AE_NTBF.xy", KEK_Authen_base_dir, 
         SAP_ptr->param_RangeConstant, SAP_ptr->param_SpreadConstant, SAP_ptr->param_Threshold, SAP_ptr->XMR_val); 
      sprintf(wfm_header_str, "SKE_XMR_%d_AE_NTBFs\n", SAP_ptr->XMR_val);
      WriteAuthenPointToFile(max_string_len, outfile_name, authen_num, wfm_header_str, ADS[0].NTBF);

// The eigth file gives the NE number of mismatches.
      sprintf(outfile_name, "%s/KEK_SKE_RC_%d_SF_%d_TH_%d_XMR_%d_NE_NTBF.xy", KEK_Authen_base_dir, 
         SAP_ptr->param_RangeConstant, SAP_ptr->param_SpreadConstant, SAP_ptr->param_Threshold, SAP_ptr->XMR_val); 
      sprintf(wfm_header_str, "SKE_XMR_%d_NE_NTBFs\n", SAP_ptr->XMR_val);
      WriteAuthenPointToFile(max_string_len, outfile_name, authen_num, wfm_header_str, ADS[1].NTBF);

// Unlock mutex.
      pthread_mutex_unlock(SAP_ptr->FileStat_mutex_ptr);
      }

// 10_18_2022: Save the SHD for the authenicating device ONLY. Be careful here -- this generates a lot of data. This is shared 'static' variable
// and this routine is multi-threaded, so it should be semiphone protected. But it is done right at startup when only one device_regeneration.elf
// file is run -- I start them one-at-a-time.
   if ( SAP_ptr->do_save_SKE_SHD == 1 )
      {
      char outfile_name[max_string_len];
      static int create_or_append[MAX_CHIPS];

      if ( SAP_ptr->num_chips > MAX_CHIPS )
         { printf("ERROR: KEK_DA_SKE_FindMatch(): 'SAP_ptr->num_chips' is GREATER THAN 'MAX_CHIPS' -- increase in program!\n"); exit(EXIT_FAILURE); }

      if ( authen_num == 0 )
         for ( chip_num = 0; chip_num < num_chips; chip_num++ )
            create_or_append[chip_num] = 0;

// If we fail the authentication, do NOT save the SHD. We do NOT know which file to write it to.
      if ( SAP_ptr->chip_num == -1 )
         { printf("WARNING: KEK_DA_SKE_FindMatch(): FAILED TO AUTHENTICATE -- SKIP SAVING SHD TO FILE!\n"); }
      else
         {

// Technically I do NOT need semaphores here because I write a file specific to each chip, and the chip can NOT be doing two
// authentications at the same time. Leaving it in for now although it does slow things down a bit.
         pthread_mutex_lock(SAP_ptr->FileStat_mutex_ptr);

         sprintf(outfile_name, "%s/Chip_%d_KEK_SKE_RC_%d_SF_%d_TH_%d_XMR_%d_SHD.txt", KEK_SHD_base_dir, SAP_ptr->chip_num, 
            SAP_ptr->param_RangeConstant, SAP_ptr->param_SpreadConstant, SAP_ptr->param_Threshold, SAP_ptr->XMR_val); 
         WriteASCIIBitstringToFile(max_string_len, outfile_name, create_or_append[SAP_ptr->chip_num], received_XMR_SHD_num_bytes*8, 
            SKE_authen_XMR_SHD);
         create_or_append[SAP_ptr->chip_num] = 1;

         pthread_mutex_unlock(SAP_ptr->FileStat_mutex_ptr);
         }
      }


printf("\tKEK_DA_SKE_FindMatch(): DONE -- chip_num identified %d\n", SAP_ptr->chip_num); fflush(stdout);
#ifdef DEBUG
#endif

   if ( ADS != NULL )
      free(ADS);
   ADS = NULL;

   authen_num++; 

//exit(EXIT_SUCCESS);

   return;
   }


// ========================================================================================================
// ========================================================================================================
// Server authenticates device in a privacy-preserving fashion -- DATABASE search. SKE authentication sends
// an authentication nonce to the device, along with pure PopOnly SF (for any mode). The device does the
// bit flipping on the PopOnly, and then computes PCR leaving PNDco that are zero under PopOnly SF AT ZERO,
// -- they are NOT changed. The device sends the bit-flip modified PopOnly or computed PCR back here to the
// server, along with the XMR helper data. A database search is carried out that finds the best match, e.g.,
// to the regenerated nonce or counting minority bit flips. Before the bit-flip and handling the zero case
// for PCR, I had this working with device-generated PCR and then using the PopOnly SF here but that's not
// working now.

void KEK_DeviceAuthentication_SKE(int max_string_len, SRFAlgoParamsStruct *SAP_ptr, int device_socket_desc, 
   int RANDOM)
   {
   char request_str[max_string_len];
   int target_attempts;

   unsigned char *SKE_authen_XMR_SHD;
   int received_XMR_SHD_num_bytes; 

   int do_part_A; 
   int current_function;

// For SKE, if scaling is enabled, the number of bit flip errors in the ZED experiments increases. I'm on the fence regarding whether to use
// scaling for PARCE authentication. The downside of NOT using scaling is that the HD leaks information about the identity of the chip 
// authenticating (as it does for COBRA). The VTS data results had this control flag set to 1. Starting a ZYBO run here with it set to 0. 
//
// 11_12_2022: RE-ENABLING IT. Tried WITHOUT scaling and got same results -- higher reliability but same margin between worst case AE 
// and NE (about 17% in BOTH cases). Benefit with scaling ENABLED is anonymous SHD.
// !!!!!!!!!!!!!! NOTE: YOU MUST DO THE SAME THING IN device_regen_funcs.c to ENABLE/DISABLE THIS. !!!!!!!!!!!!!!
   int do_scaling = 0;

   struct timeval t0, t1;
   long elapsed; 

// Device computes SF when PCR mode is active. But they bias the re-generation of the nonce. Setting this to 1 overwrites the device-computed PCR
// with the server computed Pop-only SF (which are computed and sent to the device).
// ***** NOTE: WHEN THE DEVICE COMPUTES PCR AND XMR_SHD AND WE USE PopOnly HERE ON THE SERVER -- WE WILL NOT BE ABLE TO GET 0 MISMATCHES. 

// We send the device pure PopOnly SF. Setting this to zero ensures the device gets unmodified PopOnly SF computed in the ComputePxxSpreadFactors 
// routine for ANY mode. 
   int prev_do_PO_dist_flip = SAP_ptr->do_PO_dist_flip; 
   SAP_ptr->do_PO_dist_flip = 0;

// 10_22_2022: From ZED experiments, I found that PopOnly, No flip with personalized range factors works best. Forcing that here.
   int prev_PCR_PBD_PO_mode = SAP_ptr->param_PCR_or_PBD_or_PO;
   SAP_ptr->param_PCR_or_PBD_or_PO = SF_MODE_POPONLY;


   printf("\t\t\t******************* DEVICE SKE MODE AUTHENTICATION BEGINS  ******************* \n\n"); fflush(stdout);

// If the device computes its own and sends to the server, the server runs KEK SKE regeneration using the device's XMR_SHD. If the server database PNDc are 
// highly correlated to those re-produced by the device (they are supposed to be), then applying the correct PCR SF maximizes the number of XMR SBS bits
// that match with the nonce that is encoded. Ideally, device-generated PCR can eliminate filtering of fPNDco all together, which would happen if we 
// reduce the size of the random Spreadfactor to exclude the weak bit regions, i.e., -9 to -3 and 3 to 9, with the margin set to 3. I need to try this 
// as an experiment.
   current_function = FUNC_DA;

// Set XMR best for SKE.
   SAP_ptr->XMR_val = DEVICE_SKE_AUTHEN_XMR_VAL; 

// DA_nonce_reproduced is allocated by JoinBytePackedBitStrings automatically. Just make sure it is NULL initially.
   if ( SAP_ptr->DA_nonce_reproduced != NULL )
      free(SAP_ptr->DA_nonce_reproduced);
   SAP_ptr->DA_nonce_reproduced = NULL;

// Generate and send the device the KEK_authentication_nonce.
   if ( read(RANDOM, SAP_ptr->KEK_authentication_nonce, SAP_ptr->num_KEK_authen_nonce_bits/8) == -1 )
      { printf("ERROR: KEK_DeviceAuthentication_SKE(): Read /dev/urandom failed!\n"); exit(EXIT_FAILURE); }

// 12_2_20220: Original version sends the KEK_authentication_nonce in plain form to the device.
//   if ( do_two_way_encryption == 0 )
   if ( SockSendB(SAP_ptr->KEK_authentication_nonce, SAP_ptr->num_KEK_authen_nonce_bits/8, device_socket_desc) < 0 )
      { printf("ERROR: KEK_DeviceAuthentication_SKE(): Device KEK_authentication_nonce send failed\n"); exit(EXIT_FAILURE); }

// 12_2_222: Latest thoughts here after writing up the background section of SiRF_Authentication is to send an encrypted version of the 
// KEK_authentication_nonce to the device. Note this is ONLY possible if we drop anonymity since we do NOT know how to encrypt the nonce
// i.e., which device data set should be used. Note if we run COBRA first, we'll have the ID that we need for SKE. 
// The order of operations is:
//    1) Get the ID of the authenticating device
//    2) Encrypt the nonce, i.e., generate SF and HD, and send encrypted nonce, SF and HD to device
//    3) Device decrypts nonce
//    4) Device either a) re-encrypts nonce using same challenge and SF and generates new HD or b) gets a new challenge and SF from server.
//    5) Device sends encrypted nonce and HD to server
//    6) Server does NOT need to search (since ID was sent earlier by device) so it decryptes and compares to a threshold
//   else
// Not doing this now since I don't want to drop anonymity. Probably better just to do a PUF-based encryption paper as a follow-up to
// PUF-based authentication.

// For repeated attempts, we MUST reset this to 0 because it is set to -1 if we fail.
   SAP_ptr->chip_num = 0;

   do_part_A = 1;
   target_attempts = 0;

// Store the SpreadFactors in a separate array for re-use in KEK_DA_SKE_FindMatch() below.
   signed char *authen_SpreadFactors_binary = NULL;
   int done = 0;
   while ( done == 0 )
      {

// GenChlngDelverSpreadFactorsToDevice is a convenience function that calls CommonCore when do_part_A is 1 to generate challenges, collect 
// timing data from the database and exchange nonces, and then always does part_B of CommonCore where consecutative sets of PopOnly SF 
// are generated and sent to the device. NOTE: target_attempts is FORCED to 0 when 'do_part_A' is 1. NOTE: this function sets CommonCore 
// parameters as follows: set_threshold_to_zero = 0 and send_SpreadFactors = 1.

// Compute and send pure PopOnly SF to device in any mode (except PBD which requires NO SpreadFactors). NOTE: With the current_function 
// set to FUNC_DA, ComputeSendSpreadFactors() WILL NOT call ComputePxxSpreadFactors ANYWAY EVEN IF compute_PCR_PBD_SF is set to 1 -- 
// there is an 'if stmt' that prevents it.
      int compute_PCR_PBD_SF = 0;
      int restore_store_SF = 0;

// This causes GenChlngDeliverSpreadFactorsToDevice to return after computing and sending SpreadFactors to the device. Here in Device 
// Authentication, we need to do this because we USED TO overwrite the returned device SF with the population values when force_pop_SF is 1 -- 
// see below (which we don't do any longer). This is NOT necessary any longer. 
      int return_after_each_set = 1;

      target_attempts = GenChlngDeliverSpreadFactorsToDevice(max_string_len, SAP_ptr, do_part_A, target_attempts, RANDOM, device_socket_desc, 
         &done, return_after_each_set, compute_PCR_PBD_SF, restore_store_SF, NULL, current_function);

// Do NOT do part A on subsequent calls.
      do_part_A = 0;

// DEBUG
#ifdef DEBUG
//if ( target_attempts == 1 )
   printf("\tMODE: %d\tFIRST LFSR seed low %u\tLFSR seed high %u\tRangeConstant %d\tSpreadConstant %d\tThreshold %u\tXMR %d\n\n", 
      SAP_ptr->param_PCR_or_PBD_or_PO, SAP_ptr->param_LFSR_seed_low, SAP_ptr->param_LFSR_seed_high, SAP_ptr->param_RangeConstant, 
      SAP_ptr->param_SpreadConstant, SAP_ptr->param_Threshold, SAP_ptr->XMR_val); fflush(stdout);
#endif

#ifdef DEBUG
printf("\tKEK_DeviceAuthentication_SKE(): Current number of target_attempts with device %d\tStore authen SpreadFactors of size %d\n", 
   target_attempts, target_attempts * SAP_ptr->num_SF_words); fflush(stdout);
#endif

// This fetch is not needed if our mode is PopOnly, no flip (which is the current mode) since they are not changed by the device.
      if ( (authen_SpreadFactors_binary = (signed char *)realloc(authen_SpreadFactors_binary, (target_attempts * SAP_ptr->num_SF_words) * 
         sizeof(signed char))) == NULL )
         { printf("ERROR: Failed to allocate storage for authen_SpreadFactors_binary!\n"); exit(EXIT_FAILURE); }

      if ( SockGetB((unsigned char *)(authen_SpreadFactors_binary + (target_attempts - 1)*SAP_ptr->num_SF_words), 
         SAP_ptr->num_SF_bytes, device_socket_desc) != SAP_ptr->num_SF_bytes )
         { printf("ERROR: KEK_DeviceAuthentication_SKE(): Receive authen_SpreadFactors_binary chunk %d failed\n", target_attempts); exit(EXIT_FAILURE); }

// 10_22_2022: Verified, Forcing PopOnly, No flip on the device so the SF returned should be identical. No need for the device to return them back
// to the server in this case.
#ifdef DEBUG
int cur_SF_offset = (target_attempts - 1)*SAP_ptr->num_SF_words;
PrintHeaderAndHexVals("Server Compute PURE SF:\n", 32, (unsigned char *)SAP_ptr->iSpreadFactors, 32);
PrintHeaderAndHexVals("Device returned -- MUST BE THE SAME NOW:\n", 32, (unsigned char *)(&(authen_SpreadFactors_binary[cur_SF_offset])), 32);
#endif

      }

#ifdef DEBUG
printf("KEK_DeviceAuthentication_SKE(): Getting SKE_authen_XMR_SHD %d bytes from device!\n", target_attempts * SAP_ptr->num_required_PNDiffs/8); fflush(stdout);
#endif

// Expect to receive (target_attempts * SAP_ptr->num_required_PNDiffs/8) bytes of SKE_authen_XMR_SHD from device. Allocate space. 
// In terms of statistics, the KEK_authentication_nonce is a random number so no need to store and analyze that in the RunTime database.
// However, we might want to look at the SKE_authen_XMR_SHD. Fixed this with personalized range constants so that it DOES have equal numbers 
// of 0's and 1's. 
   if ( (SKE_authen_XMR_SHD = (unsigned char *)calloc((target_attempts * SAP_ptr->num_required_PNDiffs/8), sizeof(unsigned char))) == NULL )
      { printf("ERROR: KEK_DeviceAuthentication_SKE(): Allocation for 'SKE_authen_XMR_SHD' failed!\n"); exit(EXIT_FAILURE); }

// NOTE: WE ALWAYS receive XMR here. 
   if ( (received_XMR_SHD_num_bytes = SockGetB(SKE_authen_XMR_SHD, target_attempts * SAP_ptr->num_required_PNDiffs/8, device_socket_desc)) != 
      target_attempts * SAP_ptr->num_required_PNDiffs/8 )
      { 
      printf("ERROR: KEK_DeviceAuthentication_SKE(): Receive 'SKE_authen_XMR_SHD' request failed -- expected %d!\n", 
         target_attempts * SAP_ptr->num_required_PNDiffs/8); 
      exit(EXIT_FAILURE); 
      }

// Save design information and the XMR_SHD to the RunTime database if user requests it. WE ARE NOW STORING SHD to a seperate file for analysis by
// a C program developed for the ZED experiments (which does NOT use a database). It could be done here too.
   if ( SAP_ptr->do_save_bitstrings_to_RT_DB == 1 )
      {
      pthread_mutex_lock(SAP_ptr->RT_DB_mutex_ptr);
      SaveDBBitstringInfo(max_string_len, SAP_ptr, SKE_authen_XMR_SHD, received_XMR_SHD_num_bytes, FUNC_DA);
      pthread_mutex_unlock(SAP_ptr->RT_DB_mutex_ptr);
      }

// ****************************************
// ****************************************
   if ( SAP_ptr->DEBUG_FLAG == 1 )
      printf("\nDA.1) CHIP SEARCH LOOP\n\n");

// Find an exact match to the KEK_authentication_nonce using this helper data.
   KEK_DA_SKE_FindMatch(max_string_len, SAP_ptr, received_XMR_SHD_num_bytes, SKE_authen_XMR_SHD, authen_SpreadFactors_binary, 
      current_function, do_scaling);
   
// Free up temporary storage. 
   if ( authen_SpreadFactors_binary != NULL )
      free(authen_SpreadFactors_binary); 

// Check that at least one chip succeeded with 0 mismatches and number of matching bits larger than threshold
   if ( SAP_ptr->chip_num == -1 )
      printf("\t### DA FAILURE ###\n\n");
   else
      printf("\t*** DA SUCCESS ***\n\n");
   fflush(stdout);

   if ( SAP_ptr->DEBUG_FLAG == 1 )
      {
      printf("\tWaiting device's 'DONE' signal\n");
      gettimeofday(&t0, 0);
      }

// Wait for device to send 'ACK'
   int num_recv_bytes;
   if ( (num_recv_bytes = SockGetB((unsigned char *)request_str, max_string_len, device_socket_desc)) != 4 )
      { printf("KEK_DeviceAuthentication_SKE(): Receive 'ACK' request failed -- received %d bytes, expected 4!\n", num_recv_bytes); exit(EXIT_FAILURE); }
   if ( strcmp(request_str, "ACK") != 0 )
      { printf("KEK_DeviceAuthentication_SKE(): Did NOT receive 'ACK' from device!\n"); exit(EXIT_FAILURE); }
   if ( SAP_ptr->DEBUG_FLAG == 1 )
      { gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec; printf("\tElapsed %ld us\n\n", (long)elapsed); }
   fflush(stdout);

// Free allocated storage
   if ( SKE_authen_XMR_SHD != NULL )
      free(SKE_authen_XMR_SHD); 

// Reset to 'normal' values.
   SAP_ptr->XMR_val = XMR_VAL;
   SAP_ptr->param_RangeConstant = RANGE_CONSTANT; 

// 10_22_2022: From ZED experiments, I found that PopOnly, No flip with personalized range factors works best. Forcing that here.
   SAP_ptr->do_PO_dist_flip = prev_do_PO_dist_flip;
   SAP_ptr->param_PCR_or_PBD_or_PO = prev_PCR_PBD_PO_mode; 

   return;
   }


// ========================================================================================================
// ========================================================================================================
// KEK-based authentication only

int KEK_ClientServerAuthen(int max_string_len, SRFAlgoParamsStruct *SAP_ptr, int client_socket_desc, int RANDOM)
   {
   char request_str[max_string_len];
   int retries;

   struct timeval t1, t2;
   long elapsed; 
   gettimeofday(&t2, 0);

// =========================================
// Device Authentication
// Not needed any longer -- just for testing both Cobra and non-Cobra modes
   int prev_COBRA_mode = SAP_ptr->do_COBRA;
   retries = 0;

// Get device request for COBRA or SKE authentication.
   if ( SockGetB((unsigned char *)request_str, MAX_STRING_LEN, client_socket_desc) < 0 )
      { printf("ERROR: KEK_ClientServerAuthen(): Failed to get 'SKE' or 'COBRA' authentication mode!\n"); exit(EXIT_FAILURE); }
   SAP_ptr->do_COBRA = 0;

   while ( retries < MAX_DA_RETRIES )
      {

// SKE mode authentication of device to the server. Device Authentication returns SAP_ptr->chip_num = -1 IF IT FAILS.
      KEK_DeviceAuthentication_SKE(max_string_len, SAP_ptr, client_socket_desc, RANDOM);

      if ( SAP_ptr->chip_num != -1 )
         sprintf(request_str, "SUCCESS %d", SAP_ptr->chip_num);
      else
         sprintf(request_str, "FAILURE %d", SAP_ptr->chip_num);

// Send status to device. If failure, it will retry.
      if ( SockSendB((unsigned char *)request_str, strlen(request_str) + 1, client_socket_desc) < 0 )
         { printf("KEK_ClientServerAuthen(): Failed to send '%s' to device!\n", request_str); exit(EXIT_FAILURE); }

// We always need to free up the vectors and masks generated for any of our operations, and the timing data fetched from the database. 
      if ( SAP_ptr->database_NAT != NULL )
         {
         FreeVectorsAndMasks(&(SAP_ptr->num_vecs), &(SAP_ptr->num_rise_vecs), &(SAP_ptr->first_vecs_b), &(SAP_ptr->second_vecs_b), &(SAP_ptr->masks_b));
         FreeAllTimingValsForChallenge(&(SAP_ptr->num_chips), &(SAP_ptr->PNR), &(SAP_ptr->PNF));
         }

      if ( SAP_ptr->chip_num != -1 )
         break;

      retries++;

printf("KEK_ClientServerAuthen(): FAILED AUTHENTICATION! ATTEMPT %d\n", retries); fflush(stdout);
#ifdef DEBUG
#endif
      }

// Not needed any longer -- just for testing both Cobra and non-Cobra modes
   SAP_ptr->do_COBRA = prev_COBRA_mode;

// DA failure if this occurs.
   if ( retries == MAX_DA_RETRIES )
      { printf("ERROR: KEK_ClientServerAuthen(): KEK_DeviceAuthentication FAILED with %d retries!\n", MAX_DA_RETRIES); fflush(stdout); return 0; }

   gettimeofday(&t1, 0); elapsed = (t1.tv_sec-t2.tv_sec)*1000000 + t1.tv_usec-t2.tv_usec; printf("\tElapsed: DEVICE FSB AUTHENTICATION %ld us\n\n", (long)elapsed);

#ifdef DEBUG
printf("KEK_ClientServerAuthen(): Device authentication chip number found in DB %d\n\n", SAP_ptr->chip_num);
#endif

   return 1;
   }


