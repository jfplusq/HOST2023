// ========================================================================================================
// ========================================================================================================
// ********************************************** utility.c ***********************************************
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

#include "utility.h"


// ===========================================================================================================
// ===========================================================================================================
// Largest integer number NOT greater than argument. So floor(-0.6 + 0.5) = floor(-0.1) = -1.0. So this function
// rounds AWAY from 0, which is exactly what we want when arg is negative.

float Round(float d) 
   { return (float)floor((double)d + 0.5); }


// ===========================================================================================================
// ===========================================================================================================

float ComputeMean(int num_vals, float *vals) 
   {
   int i; 
   double mean;

   if ( num_vals == 0 )
      { printf("ERROR: ComputeMean(): num_vals is 0!\n"); exit(EXIT_FAILURE); }

   mean = 0.0;
   for ( i = 0; i < num_vals; i++ )
      { mean += (double)vals[i]; }
   return (float)(mean/(double)num_vals);
   }


// ===========================================================================================================
// ===========================================================================================================
// qsort: Return -x if ele1 < ele2, 0 if ele1 == ele2 and +x if ele1 > ele2. 

int MedianCompareFunc(const void *v1, const void *v2)
   {
   if ( *(float *)v1 - *(float *)v2 == 0.0 )
      return 0;
   else if ( *(float *)v1 - *(float *)v2 < 0.0 )
      return -1;
   else 
      return 1;
   }


// ===========================================================================================================
// ===========================================================================================================
// Compute the median value by sorting and returning the value of the middle value.

float ComputeMedian(int num_vals, float *vals)
   {
   float *median_vals;
   float median;
   int i;

   if ( num_vals == 0 )
      { printf("ERROR: ComputeMedian(): Number of values MUST be > 0!\n"); exit(EXIT_FAILURE); }

   if ( num_vals == 1 )
      return vals[0];

   if ( (median_vals = (float *)malloc(num_vals*sizeof(float))) == NULL )
      { printf("ERROR: ComputeMedian(): Malloc FAILED!\n"); exit(EXIT_FAILURE); }

// Copy  array to temporary array so we can sort it without disturbing the array passed in.
   for ( i = 0; i < num_vals; i++ )
      median_vals[i] = vals[i];

   qsort(median_vals, num_vals, sizeof(float), MedianCompareFunc);

// If even number, compute average of two middle samples, otherwise return middle value;
   if ( num_vals % 2 == 0 )
      median = (median_vals[num_vals/2 - 1] + median_vals[num_vals/2])/2;
   else
      median = median_vals[num_vals/2];

#ifdef DEBUG
   for ( i = 0; i < num_vals; i++ )
      printf("\t%d) Median val %f\n", i, median_vals[i]);
   printf("MEDIAN => %f\n", median);
   fflush(stdout);
#endif

   free(median_vals); 

   return median;
   }


// ===========================================================================================================
// ===========================================================================================================

float ComputeStdDev(int num_vals, float mean, float *vals)
   {
   int i; 
   double std_dev;

   if ( num_vals <= 1 )
      { printf("ERROR: ComputeStdDev(): num_vals is <= 1!\n"); exit(EXIT_FAILURE); }

   std_dev = 0.0;
   for ( i = 0; i < num_vals; i++ )
      { std_dev += pow((double)mean - (double)vals[i], (double)2); }
   return (float)sqrt(std_dev/(double)(num_vals - 1));
   }


// ===========================================================================================================
// ===========================================================================================================
// This routine takes a char (a byte) and a bit position between 0 and 7 and returns the value of the bit
// at that position as 0 or 1. It is used to make use of compact bitstring arrays. 

int GetBitFromByte(unsigned char byte, int bit_pos)
   {

// Note this is equal to 0 or SOMETHING LARGER THAN 0 WHICH IS NOT NECESSARILY 1 so we MUST compare to 0 or
// >0 -- NOT to 1.
   return ((byte & (1 << bit_pos)) == 0) ? 0 : 1;
   }


// ===========================================================================================================
// ===========================================================================================================
// This routine takes a char (a byte), a bit value and a bit position between 0 and 7 and overwrites the bit 
// in the byte with the new bit. The byte is returned.

void SetBitInByte(unsigned char *byte_ptr, int bit_val, int bit_pos)
   {
   *byte_ptr = (bit_val == 0) ? (*byte_ptr) & ~(1 << bit_pos) : (*byte_ptr) | (1 << bit_pos);
   }


// ========================================================================================================
// ========================================================================================================
// ASCII '0'/'1' string to binary.

void ASCIIByteToBin(unsigned char *binary_byte_ptr, char *ascii_str)
   {
   int i; 

   *binary_byte_ptr = 0;
   for ( i = 0; i < 8; i++ )
      if ( ascii_str[i] == '1' )
         *binary_byte_ptr *= 2 + 1;
      else
         *binary_byte_ptr *= 2;
   return;
   }


// ========================================================================================================
// ========================================================================================================
// Binary byte to ASCII '0'/'1' string 

void BinByteToASCII(unsigned char binary_byte, char *ascii_str)
   {
   int i; 
   
   for ( i = 0; i < 8; i++ )
      if ( (binary_byte & (unsigned char)(1 << (7 - i))) == 0 )
         ascii_str[i] = '0';
      else
         ascii_str[i] = '1';
   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// Convert a binary vec/mask to an ASCII vec/mask. Assume storage is allocated in caller. NOTE: mask_asc is 
// num_POs + 1 in size (for NULL character). NOTE that the ORDER is reversed so that low addresses in the ASCII 
// version correspond to high order bits in the binary vector/mask. This also works for packed SHD, SBS, DHD, 
// etc.

void ConvertBinVecMaskToASCII(int num_PI_POs, unsigned char *vec_mask_bin, char *vec_mask_asc)
   {
   int PI_PO_num;

   for ( PI_PO_num = 0; PI_PO_num < num_PI_POs; PI_PO_num++ )
      vec_mask_asc[num_PI_POs - PI_PO_num - 1] = (GetBitFromByte(vec_mask_bin[PI_PO_num/8], PI_PO_num % 8) == 1 ) ? '1' : '0';
   vec_mask_asc[num_PI_POs] = '\0';

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// Covert ASCII vec/mask to binary version. Note that ASCII vector is specified from char[0] to char[num_PIs] 
// in high order bit to low order bit order. Reverse this order in the the unsigned char binary byte array by 
// starting the load at the highest byte and working toward the lowest byte. The hardware transfer by the chip
// to the PL side loads the data in this order. Note that the ASCII version is convenient because when printed, 
// the high-order byte is printed on the left (which is the natural way to read the vector). But the left-most 
// character of the ASCII version for the high-order bit has the lowest address so we need to reverse it. 
// Storage MUST be allocated in caller. NOTE: 'num_PIs' MUST BE DIVISIBLE BY 8.

void ConvertASCIIVecMaskToBinary(int num_PI_POs, char *vec_mask_asc, unsigned char *vec_mask_bin)
   {
   int bit_cnter, byte_cnter, mod_8_val;

   if ( (num_PI_POs % 8) != 0 )
      { 
      printf("ERROR: ConvertASCIIVecMaskToBinary(): number of ASCII chars to convert MUST be a multiple of 8 => %d!\n", 
         num_PI_POs); exit(EXIT_FAILURE); 
      }

   byte_cnter = num_PI_POs/8 - 1;
   for ( bit_cnter = 0; bit_cnter < num_PI_POs; )
      {

// Sanity check
      if ( byte_cnter < 0 )
         { 
         printf("ERROR: ConvertASCIIVecMaskToBinary(): Program error: 'byte_cnter' %d is LESS THAN 0!\n", byte_cnter); 
         exit(EXIT_FAILURE); 
         }

// Zero out the unsigned char byte on multiples of 8 bits.
      mod_8_val = bit_cnter % 8;
      if ( mod_8_val == 0 )
         vec_mask_bin[byte_cnter] = 0;

// Keep adding in bits to the current byte at the appropriate bit position in the byte. Left to right processing of the line needs to make 
// the first (left-most) ASCII bit in file 'line' the high order bit in the binary byte.
      if ( vec_mask_asc[bit_cnter] == '1' )
         vec_mask_bin[byte_cnter] += (unsigned char)(1 << (7 - mod_8_val));

// Increment to the next ASCII bit.
      bit_cnter++;

// After processing 8 ASCII bits, decrement the unsigned char index.
      if ( (bit_cnter % 8) == 0 )
         {
#ifdef DEBUG
printf("%02X ", vec_ptr[byte_cnter]); fflush(stdout);
#endif
         byte_cnter--;
         }
      }

#ifdef DEBUG
printf("\n"); fflush(stdout);
#endif

   return;
   }


// ===========================================================================================================
// ===========================================================================================================
// 10_10_2022: Save ASCII '0'/'1' bitstring to a file so we can do a statistical analysis on them. 

void WriteASCIIBitstringToFile(int max_string_len, char *outfile_name, int create_or_append, int num_bits, 
   unsigned char *bitstring_binary)
   {
   int bit_num;
   FILE *OUTFILE;

   if ( create_or_append == 0 )
      {
      if ( (OUTFILE = fopen(outfile_name, "w")) == NULL )
         { printf("ERROR: WriteASCIIBitstringToFile(): Data file '%s' open failed for writing!\n", outfile_name); exit(EXIT_FAILURE); }
      }
   else
      if ( (OUTFILE = fopen(outfile_name, "a")) == NULL ) 
         { printf("ERROR: WriteASCIIBitstringToFile(): Data file '%s' open failed for writing!\n", outfile_name); exit(EXIT_FAILURE); }

// Save only the exposed SHD sent by the device. Doing this analysis in CobraAnalyzeSHD.c now -- NOT HERE.
   for ( bit_num = 0; bit_num < num_bits; bit_num++ )
      fprintf(OUTFILE, "%d", GetBitFromByte(bitstring_binary[bit_num/8], bit_num % 8));
   fprintf(OUTFILE, "\n");
   fclose(OUTFILE);

   return;
   }
