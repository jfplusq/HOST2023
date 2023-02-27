# 2_27_2023
# =========
# 1) Make a DATABASE directory in the same directory as ProvisionData (called the BASE directory)
mkdir DATABASE

# 2) cd into DATABASE
cd DATABASE

# 3) Copy all the files in the DATABASE directory on the website here. NOTE: There is a SQLSchemaScripts subdirectory!
# (whatever copy method you use)

# 4) Make an output directory
mkdir output

# 5) Change back to BASE directory.
cd ..

# 6) Make a PROTOCOL directory in the same directory as ProvisionData
mkdir PROTOCOL

# 7) cd into PROTOCOL
cd PROTOCOL

# 8) Copy all the files in the PROTOCOL directory on the website here
# (whatever copy method you use)

# 9) Change directory back to DATABASE
cd ../DATABASE

# 10) Compile the programs.
make -f Makefile_AddChallengeDB
make -f Makefile_EnrollDB
make -f Makefile_add_PUFDesign_challengeDB

# 11) Create NAT_Master_TDC.db (non-anonymous timing database) (Note: You must have sqlite3 installed)
# (NOTE: If you need to start over with the database creation process, start by removing the databases and re-running these scripts, e.g., rm *.db)
sqlite3 NAT_Master_TDC.db < SQLSchemaScripts/SQL_PUFDesign_create_table.sql
sqlite3 NAT_Master_TDC.db < SQLSchemaScripts/SQL_PUFInstance_create_table.sql
sqlite3 NAT_Master_TDC.db < SQLSchemaScripts/SQL_VecPairs_create_table.sql
sqlite3 NAT_Master_TDC.db < SQLSchemaScripts/SQL_Vectors_create_table.sql
sqlite3 NAT_Master_TDC.db < SQLSchemaScripts/SQL_TimingVals_create_table.sql
sqlite3 NAT_Master_TDC.db < SQLSchemaScripts/SQL_PathSelectMasks_create_table.sql
sqlite3 NAT_Master_TDC.db < SQLSchemaScripts/SQL_Challenges_create_table.sql
sqlite3 NAT_Master_TDC.db < SQLSchemaScripts/SQL_ChallengeVecPairs_create_table.sql

sqlite3 Challenges.db < SQLSchemaScripts/SQL_PUFDesign_create_table.sql
sqlite3 Challenges.db < SQLSchemaScripts/SQL_PUFInstance_create_table.sql
sqlite3 Challenges.db < SQLSchemaScripts/SQL_VecPairs_create_table.sql
sqlite3 Challenges.db < SQLSchemaScripts/SQL_Vectors_create_table.sql
sqlite3 Challenges.db < SQLSchemaScripts/SQL_TimingVals_create_table.sql
sqlite3 Challenges.db < SQLSchemaScripts/SQL_PathSelectMasks_create_table.sql
sqlite3 Challenges.db < SQLSchemaScripts/SQL_Challenges_create_table.sql
sqlite3 Challenges.db < SQLSchemaScripts/SQL_ChallengeVecPairs_create_table.sql

# 12) Add the provisioning data to the database (AT LEAST the four files that you created from your board. Eventually, you will add them all).
#     NOTE: Change the 'C_Jim_204' to your filename prefix!
#     NOTE: ALWAYS USE ZYBO, even if you have a CORA board in the following!
#     NOTE: CHECK output/enrollDB... file for errors.
   enrollDB NAT_Master_TDC.db SR_RFM_V4_TDC SRFSyn1 ZYBO P1 Z_Jim_64 392 32 ../CHALLENGES/SR_RFM_V4_Random_Rise_1000Vs_Fall_1000Vs_NumSeeds_10_vecs 0 ../ProvisionData/Z_Jim_64_SR_RFM_V4_TDC_P1_25C_1.00V_NCs_2000_E_PUFNums.txt > output/enrollDB_SR_RFM_V4_TDC_SRFSyn1_Cx_Vx.txt
   enrollDB NAT_Master_TDC.db SR_RFM_V4_TDC SRFSyn1 ZYBO P2 Z_Jim_64 392 32 ../CHALLENGES/SR_RFM_V4_Random_Rise_1000Vs_Fall_1000Vs_NumSeeds_10_vecs 0 ../ProvisionData/Z_Jim_64_SR_RFM_V4_TDC_P2_25C_1.00V_NCs_2000_E_PUFNums.txt >> output/enrollDB_SR_RFM_V4_TDC_SRFSyn1_Cx_Vx.txt
   enrollDB NAT_Master_TDC.db SR_RFM_V4_TDC SRFSyn1 ZYBO P3 Z_Jim_64 392 32 ../CHALLENGES/SR_RFM_V4_Random_Rise_1000Vs_Fall_1000Vs_NumSeeds_10_vecs 0 ../ProvisionData/Z_Jim_64_SR_RFM_V4_TDC_P3_25C_1.00V_NCs_2000_E_PUFNums.txt >> output/enrollDB_SR_RFM_V4_TDC_SRFSyn1_Cx_Vx.txt
   enrollDB NAT_Master_TDC.db SR_RFM_V4_TDC SRFSyn1 ZYBO P4 Z_Jim_64 392 32 ../CHALLENGES/SR_RFM_V4_Random_Rise_1000Vs_Fall_1000Vs_NumSeeds_10_vecs 0 ../ProvisionData/Z_Jim_64_SR_RFM_V4_TDC_P4_25C_1.00V_NCs_2000_E_PUFNums.txt >> output/enrollDB_SR_RFM_V4_TDC_SRFSyn1_Cx_Vx.txt

# ZYBO
   enrollDB NAT_Master_TDC.db SR_RFM_V4_TDC SRFSyn1 ZYBO P1 C_Jim_204 392 32 ../CHALLENGES/SR_RFM_V4_Random_Rise_1000Vs_Fall_1000Vs_NumSeeds_10_vecs 0 ../ProvisionData/C_Jim_204_SR_RFM_V4_TDC_P1_25C_1.00V_NCs_2000_E_PUFNums.txt >> output/enrollDB_SR_RFM_V4_TDC_SRFSyn1_Cx_Vx.txt
   enrollDB NAT_Master_TDC.db SR_RFM_V4_TDC SRFSyn1 ZYBO P2 C_Jim_204 392 32 ../CHALLENGES/SR_RFM_V4_Random_Rise_1000Vs_Fall_1000Vs_NumSeeds_10_vecs 0 ../ProvisionData/C_Jim_204_SR_RFM_V4_TDC_P2_25C_1.00V_NCs_2000_E_PUFNums.txt >> output/enrollDB_SR_RFM_V4_TDC_SRFSyn1_Cx_Vx.txt
   enrollDB NAT_Master_TDC.db SR_RFM_V4_TDC SRFSyn1 ZYBO P3 C_Jim_204 392 32 ../CHALLENGES/SR_RFM_V4_Random_Rise_1000Vs_Fall_1000Vs_NumSeeds_10_vecs 0 ../ProvisionData/C_Jim_204_SR_RFM_V4_TDC_P3_25C_1.00V_NCs_2000_E_PUFNums.txt >> output/enrollDB_SR_RFM_V4_TDC_SRFSyn1_Cx_Vx.txt
   enrollDB NAT_Master_TDC.db SR_RFM_V4_TDC SRFSyn1 ZYBO P4 C_Jim_204 392 32 ../CHALLENGES/SR_RFM_V4_Random_Rise_1000Vs_Fall_1000Vs_NumSeeds_10_vecs 0 ../ProvisionData/C_Jim_204_SR_RFM_V4_TDC_P4_25C_1.00V_NCs_2000_E_PUFNums.txt >> output/enrollDB_SR_RFM_V4_TDC_SRFSyn1_Cx_Vx.txt


# 13) Add challenge to NAT_Master_TDC.db
add_challengeDB NAT_Master_TDC.db SR_RFM_V4_TDC SRFSyn1 Master1_OptKEK_TVN_0.00_WID_1.75 ../CHALLENGES/SR_RFM_V4_Random_Rise_1000Vs_Fall_1000Vs_NumSeeds_10 ../CHALLENGES/optKEK_qualifing_path_TVN_0.00_WID_1.75_SetSize_64000 1 > output/add_challengeDB_SRFSyn1_Master1_OptKEK_TVN_0.00_WID_1.75.txt

# 14) Make a copy of the NAT_Master_TDC.db to AT_Master_TDC.db (anonymous timing database)
cp NAT_Master_TDC.db AT_Master_TDC.db

# 15) Add challenges to Challenges.db
add_PUFDesign_challengeDB Challenges.db SR_RFM_V4_TDC SRFSyn1 CORA P1 Cxx 392 32 ../CHALLENGES/SR_RFM_V4_Random_Rise_1000Vs_Fall_1000Vs_NumSeeds_10_vecs 0 > output/add_challengeDB_SRFSyn1_Master1_OptKEK_TVN_0.00_WID_1.75_Challenges.txt
add_challengeDB Challenges.db SR_RFM_V4_TDC SRFSyn1 Master1_OptKEK_TVN_0.00_WID_1.75 ../CHALLENGES/SR_RFM_V4_Random_Rise_1000Vs_Fall_1000Vs_NumSeeds_10 ../CHALLENGES/optKEK_qualifing_path_TVN_0.00_WID_1.75_SetSize_64000 0 > output/add_challengeDB_SRFSyn1_Master1_OptKEK_TVN_0.00_WID_1.75_Challenges.txt



