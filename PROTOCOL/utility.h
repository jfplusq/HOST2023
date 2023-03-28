// ========================================================================================================
// ========================================================================================================
// ********************************************** utility.h ***********************************************
// ========================================================================================================
// ========================================================================================================
//
//--------------------------------------------------------------------------------
// Company: IC-Safety, LLC and University of New Mexico
// Engineer: Professor Jim Plusquellic
// Exclusive License: IC-Safety, LLC
// Copyright: Univ. of New Mexico
//--------------------------------------------------------------------------------

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>  
#include <sys/mman.h>
#include <math.h>

#ifndef TIMING_STRUCTS
typedef struct
   {
   int vecpair_id;
   int PO_num;
   char rise_or_fall;
   float *PNs;
   } TimingValCacheStruct;
#define TIMING_STRUCTS
#endif

// Scratch pad string size
#define MAX_STRING_LEN 2048

// HELP currently uses 2048 PNR and 2048 PNF to create 2048 PNDiffs. 
#define NUM_REQUIRED_PNDIFFS 2048

float Round(float d);
float ComputeMean(int num_vals, float *vals);
float ComputeMedian(int num_vals, float *vals);
float ComputeStdDev(int num_vals, float mean, float *vals);
int GetBitFromByte(unsigned char byte, int bit_pos);
void SetBitInByte(unsigned char *byte_ptr, int bit_val, int bit_pos);

void ASCIIByteToBin(unsigned char *binary_byte_ptr, char *ascii_str);
void BinByteToASCII(unsigned char binary_byte, char *ascii_str);

void ConvertBinVecMaskToASCII(int num_PI_POs, unsigned char *vec_mask_bin, char *vec_mask_asc);
void ConvertASCIIVecMaskToBinary(int num_PI_POs, char *vec_mask_asc, unsigned char *vec_mask_bin);
void WriteASCIIBitstringToFile(int max_string_len, char *outfile_name, int create_or_append, int num_bits, 
   unsigned char *bitstring_binary);
