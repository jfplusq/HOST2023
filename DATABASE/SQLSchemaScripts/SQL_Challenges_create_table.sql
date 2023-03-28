PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS Challenges ( 
   id INTEGER PRIMARY KEY,
   Name TEXT NOT NULL UNIQUE, 
   NumVecs INTEGER NOT NULL, 
   NumRiseVecs INTEGER NOT NULL, 
   NumPNs INTEGER NOT NULL, 
   NumRisePNs INTEGER NOT NULL, 
   PUFDesign_id INTEGER NOT NULL, 
   FOREIGN KEY (PUFDesign_id) REFERENCES PUFDesign(id) ON UPDATE CASCADE ON DELETE CASCADE
   );

CREATE UNIQUE INDEX Challenges_name_index ON Challenges (Name);
CREATE UNIQUE INDEX Challenges_ID_index ON Challenges (ID);