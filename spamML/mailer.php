<?php

// set_time_limit(4000); 

$res = urldecode($argv[1]);

list($req_email, $req_pass) = explode("&", $res);
$username = explode("=", $req_email)[1];
$password  = explode("=", $req_pass)[1];

// Connect to gmail
$imapPath = '{imap.gmail.com:993/imap/ssl}INBOX';

// try to connect 
$inbox = imap_open($imapPath, $username, $password) or die('Cannot connect to Gmail: ' . imap_last_error());

$emails = imap_sort($inbox, SORTDATE, 1);

$output = array();
$i = 0;
foreach($emails as $mail) {
    $i = $i + 1;
    if ($i > 25)
        break;
        
    $headerInfo = imap_headerinfo($inbox, $mail);

    $emailStructure = imap_fetchstructure($inbox, $mail);
    $mail_content = imap_fetchbody($inbox, $mail, 1); 
    // else
        // $mail_content = "";
    // echo quoted_printable_decode($headerInfo->subject);
    $current_email_field = array(
        "subject" => $headerInfo->subject, 
        "date" => $headerInfo->date,
        "sender" => $headerInfo->fromaddress,
        "content" => $mail_content
    );
    array_push($output, $current_email_field);
}

echo json_encode($output);

imap_expunge($inbox);
imap_close($inbox);
?>