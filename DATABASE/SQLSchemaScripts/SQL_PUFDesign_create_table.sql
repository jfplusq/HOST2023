PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS PUFDesign ( 
   id INTEGER PRIMARY KEY,
   netlist_name TEXT NOT NULL UNIQUE,
   synthesis_name TEXT NOT NULL,
   num_PIs INTEGER NOT NULL,
   num_POs INTEGER NOT NULL
   );

CREATE UNIQUE INDEX PUFDesign_netlist_syn_index ON PUFDesign (netlist_name, synthesis_name);
CREATE UNIQUE INDEX PUFDesign_ID_index ON PUFDesign (ID);
