<?php

function map_in_boundaries($value)
{
    if ($value > 0.93)
        return 0.93;
    
    if ($value < 0.02)
        return 0.02;

    return $value;
}


if ($argc < 2 || empty($argv[1])) {
    $res = array(
        "status" => "FAILURE"
    );
    
    echo json_encode($res);
    exit(1);
}

//TODO: don't hardcode it.
$database_path = "spamML/spamML_trained_database.db";

class MyDB extends SQLite3 {
    function __construct($database_path) {
        $this->open($database_path);
    }
}

$db = new MyDB($database_path);
if(!$db) {
    echo $db->lastErrorMsg();
    $res = array(
        "status" => "FAILURE"
    );
    
    echo json_encode($res);
    exit(1);
}


$parsed_input = array();
$to_compute = strtolower($argv[1]);

$token = strtok($to_compute, ":;\"/()\{\}[]|><*-+$\%*^!?/-.,\n ");
while ($token !== false)
{
    array_push($parsed_input, $token);
    $token = strtok(":;\"/()\{\}[]|><*-+$\%*^!?/-.,\n ");
}

$count_query = 'SELECT resource_count FROM spam_ham_count WHERE resource_name=\'%s\';';
$word_count_query = 'SELECT resource_count FROM word_count WHERE resource_name=\'%s\';';
$select_query = 'SELECT frequency FROM %s WHERE word=\'%s\';';

$ret = $db->query(sprintf($word_count_query, "ham_count"));
$ham_global_count = $ret->fetchArray(SQLITE3_ASSOC)["resource_count"];
if ($ham_global_count == 0) {
    $res = array(
        "status" => "FAILURE"
    );
    
    echo json_encode($res);
    exit(1);
}
$ret = $db->query(sprintf($word_count_query, "spam_count"));
$spam_global_count = $ret->fetchArray(SQLITE3_ASSOC)["resource_count"];
if ($spam_global_count == 0) {
    $res = array(
        "status" => "FAILURE"
    );
    
    echo json_encode($res);
    exit(1);
}


// same thing not to be multiplied twice 
$spamicity_as_num = 1; 
$spamicity_as_den = 1;
$not_spamicity_as_den = 1;

foreach ($parsed_input as $word) {
    if (strlen($word) <= 5) {
        continue;
    }

    $ret = $db->query(sprintf($select_query, "spam_words", $word));
    if (!$ret)
    continue;
    
    $spam_count = $ret->fetchArray(SQLITE3_ASSOC)["frequency"];
    if (empty($spam_count))
    continue;
    
    
    $ret = $db->query(sprintf($select_query, "ham_words", $word));
    if (!$ret)
    continue;
    $ham_count = $ret->fetchArray(SQLITE3_ASSOC)["frequency"];
    if (empty($ham_count))
    continue;
    
    $word_spamicity =  ($spam_count / $spam_global_count) / ($spam_count / $spam_global_count + $ham_count / $ham_global_count);
    $word_spamicity = map_in_boundaries($word_spamicity);
    
    $spamicity_as_num *= $word_spamicity;
    $spamicity_as_den = $spamicity_as_num;
    
    $not_spamicity_as_den *= (1 - $word_spamicity);
    
    if ($spamicity_as_num < 0.1 || $not_spamicity_as_den < 0.1) {
        $spamicity_as_num *= 1000; 
        $spamicity_as_den *= 1000;
        $not_spamicity_as_den *= 1000;
    }
}
// echo "RES: $spamicity_as_num    $not_spamicity_as_den\n";
$classification_result = $spamicity_as_num /($spamicity_as_num + $not_spamicity_as_den);
$date = date('Y-m-d H:i:s');

$res = array(
    "classificationResult" => $classification_result,
    "timestamp" => $date,
    "status" => "SUCCESS"
);

echo json_encode($res);

?> 