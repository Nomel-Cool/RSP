CREATE DATABASE IF NOT EXISTS stable_sequences;
USE stable_sequences;
CREATE TABLE MainMap (
    id INT AUTO_INCREMENT PRIMARY KEY,
    key1 INT NOT NULL,
    key2 INT NOT NULL
);
CREATE TABLE VectorList (
    id INT AUTO_INCREMENT PRIMARY KEY,
    main_map_id INT NOT NULL,
    vector_index INT NOT NULL,
    satisfiction_rate DOUBLE NOT NULL DEFAULT 0,
    FOREIGN KEY (main_map_id) REFERENCES MainMap(id)
);
CREATE TABLE PairList (
    id INT AUTO_INCREMENT PRIMARY KEY,
    vector_list_id INT NOT NULL,
    pair1 INT NOT NULL,
    pair2 INT NOT NULL,
    FOREIGN KEY (vector_list_id) REFERENCES VectorList(id)
);
