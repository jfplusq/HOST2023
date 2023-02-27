PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS PathSelectMasks ( 
   id INTEGER PRIMARY KEY,
   vector_str TEXT NOT NULL UNIQUE
   );

CREATE UNIQUE INDEX PathSelectMasks_vector_str_index ON PathSelectMasks (vector_str);
CREATE UNIQUE INDEX PathSelectMasks_ID_index ON PathSelectMasks (ID);
