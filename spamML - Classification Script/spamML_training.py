#!/usr/env python3

import os
import sqlite3
from sqlite3 import Error
from sqlescapy import sqlescape

target_directory = "raw_dataset"
ml_database = "spamML_trained_database.db"

count = 1
total_number = len(os.listdir(target_directory))

def tokenize_words(email_content):
    # print(email_content)
    for char in ':;\"/()\{\}[]|><*-+$\%*^!?/-.,\n':
        email_content = email_content.replace(char, ' ')

    email_content = email_content.lower()
    return email_content.split()


def create_connection(db_file):
    conn = None
    try:
        conn = sqlite3.connect(db_file)
    except Error as e:
        print(e)
 
    return conn


def increment_word_count(conn, word, spam):
    # print(word)
    table = "spam_words" if spam else "ham_words"

    select_clause = "SELECT 1 FROM %s WHERE word='%s' LIMIT 1" % (table, sqlescape(word))  

    cursor = conn.cursor()
    cursor.execute(select_clause)
    if cursor.fetchall():
        sql_clause = "UPDATE %s SET frequency=frequency + 1 WHERE word='%s'" % (table, sqlescape(word))
    else:
        sql_clause = "INSERT INTO %s(word, frequency) VALUES('%s', 1)" % (table, sqlescape(word))
    cursor.execute(sql_clause)

    return cursor.lastrowid


def increment_files_count(conn, spam):
    sql_clause = "UPDATE spam_ham_count SET resource_count=resource_count + 1 WHERE resource_name='spam_count'" if spam \
                 else "UPDATE spam_ham_count SET resource_count=resource_count + 1 WHERE resource_name='ham_count'"

    cursor = conn.cursor()
    cursor.execute(sql_clause)

    return cursor.lastrowid


if __name__=='__main__':
    conn = create_connection(ml_database)

    for current_directory in os.listdir(target_directory):
        print("[Current directory] : " + current_directory)
        ham_dir = target_directory + "/" + current_directory + "/ham"
        spam_dir = target_directory + "/" + current_directory + "/spam"

        ham_dirsize =  len(os.listdir(ham_dir))
        spam_dirsize =  len(os.listdir(spam_dir))

        f_count = 1
        for filename in os.listdir(ham_dir):
            print("File: " + str(f_count) + "/" + str(ham_dirsize))
            with conn:
                increment_files_count(conn, False)
            with open(ham_dir + "/" + filename) as current_file:
                word_list = tokenize_words(current_file.read())
                for word in word_list:
                    with conn:
                        increment_word_count(conn, sqlescape(word), False)
            f_count += 1

        
        f_count = 1
        for filename in os.listdir(spam_dir):
            print("File: " + str(f_count) + "/" + str(spam_dirsize))
            with conn:
                increment_files_count(conn, True)
            print(spam_dir + "/" + filename)
            
            with open(spam_dir + "/" + filename) as current_file:
                word_list = tokenize_words(current_file.read())
                for word in word_list:
                    with conn:
                        increment_word_count(conn, sqlescape(word), True)
            f_count += 1
