CREATE TABLE spam_ham_count (
    resource_name TEXT NOT NULL,
    resource_count INTEGER NOT NULL DEFAULT 0,
    UNIQUE(resource_name),

    --
    CHECK(resource_count >= 0)
);

INSERT INTO spam_ham_count (resource_name) VALUES ('spam_count');
INSERT INTO spam_ham_count (resource_name) VALUES ('ham_count');

CREATE TABLE spam_words (
    word TEXT NOT NULL,
    frequency INTEGER NOT NULL,
    --
    UNIQUE(word),
    CHECK(frequency >= 0)
);


CREATE TABLE ham_words (
    word TEXT NOT NULL,
    frequency INTEGER NOT NULL,
    --
    UNIQUE(word),
    CHECK(frequency >= 0)
);


CREATE TABLE word_count (
    resource_name TEXT NOT NULL,
    resource_count INTEGER NOT NULL DEFAULT 0,
    UNIQUE(resource_name),

    --
    CHECK(resource_count >= 0)
);

INSERT INTO word_count (resource_name) VALUES ('spam_count');
INSERT INTO word_count (resource_name) VALUES ('ham_count');