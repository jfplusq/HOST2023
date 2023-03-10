// ========================================================================================================
// ========================================================================================================
// **************************************** verifier_regen_funcs.h ****************************************
// ========================================================================================================
// ========================================================================================================
//
//--------------------------------------------------------------------------------
// Company: IC-Safety, LLC and University of New Mexico
// Engineer: Professor Jim Plusquellic
// Exclusive License: IC-Safety, LLC
// Copyright: Univ. of New Mexico
//--------------------------------------------------------------------------------

#include "commonDB.h"

void FreeAllTimingValsForChallenge(int *num_PUF_instances_ptr, float ***PNR_ptr, float ***PNF_ptr);

void ComputeSendSpreadFactors(int max_string_len, SRFAlgoParamsStruct *SAP_ptr, int device_socket_desc, int current_function,
   int send_SpreadFactors, int compute_PCR_SF);

void DoSRFComp(int max_string_len, SRFAlgoParamsStruct *SAP_ptr, int do_dump);

int SingleHelpBitGen(int max_PNDiffs, float *fPNDco, unsigned char *SBS, unsigned char *SHD, int *HD_num_bytes_ptr, 
   unsigned short Threshold);

int KEK_ClientServerAuthen(int max_string_len, SRFAlgoParamsStruct *SAP_ptr, int client_socket_desc, int RANDOM);
