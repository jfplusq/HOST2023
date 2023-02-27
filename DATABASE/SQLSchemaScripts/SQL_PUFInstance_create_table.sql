PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS PUFInstance ( 
   id INTEGER PRIMARY KEY,
   Instance_name TEXT NOT NULL,
   Dev TEXT NOT NULL,
   Placement TEXT NOT NULL,
   EnrollDate TEXT NOT NULL,
   PUFDesign_id INTEGER NOT NULL,
   FOREIGN KEY (PUFDesign_id) REFERENCES PUFDesign(id) ON UPDATE CASCADE ON DELETE CASCADE
   );

CREATE UNIQUE INDEX PUFInstance_IName_Dev_Placement_index ON PUFInstance (Instance_name, Dev, Placement);
CREATE UNIQUE INDEX PUFInstance_ID_index ON PUFInstance (ID);
