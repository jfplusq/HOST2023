PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS Vectors ( 
   id INTEGER PRIMARY KEY,
   vector BLOB NOT NULL UNIQUE
   );

CREATE UNIQUE INDEX Vectors_vector_index ON Vectors (vector);
CREATE UNIQUE INDEX Vectors_ID_index ON Vectors (ID);
