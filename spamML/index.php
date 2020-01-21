<!DOCTYPE html>
<meta http-equiv="Content-Type" content="text/html;charset=UTF-8">
<html>
    <head>
        <title>
            SpamML
        </title>

        <link href="https://fonts.googleapis.com/css?family=Ubuntu&display=swap" rel="stylesheet">

        <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/css/bootstrap.min.css">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js"></script>
        <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/js/bootstrap.min.js"></script>
        <link rel="stylesheet" type="text/css" href="style.css">
        <link rel='icon' href='favicon.ico' type='image/x-icon'>
        <script src="index_script.js"></script> 
    </head>

    <body>
        <div id="emailContainer" class="col-sm-4">
            <div id="topBar">
                <div id="letterCircle" class="col-sm-4">
                    S
                </div>
                <span id="userContainer" class="col-sm-8">
                    SpamML - AlgoExpert contest
                </span>
            </div>
            <!-- Formular login -->
            <div id="credsAndIntel">
                <form>
                    <div class="form-group">
                        <label for="emailInput">Email address</label>
                        <input type="email" class="form-control" id="emailInput" aria-describedby="emailHelp" placeholder="Enter email">
                        <small id="emailHelp" class="form-text text-muted">Your password will remain safe.</small>
                    </div>
                    <div class="form-group">
                        <label for="passwordInput">Password</label>
                        <input type="password" class="form-control" id="passwordInput" placeholder="Password">
                    </div>
                    <div id="submitCredsButton">  
                        <a id="sendCreds" href="">Submit</a>
                    </div>
                    <div class="loader"></div>
                </form>
                <div id="goodToKnow">
                    <h1> Welcome user! </h1> 
                    <p>SpamML is a machine learning spam detector build on top of a custom HTTP async web server and created for the AlgoExpert Winter 2020 contest. 
                    You can either paste the body of the email in the textarea or connect your GMAIL to the app (experimental feature, use the test credentials given in the registration message).<p>
                </div>
            </div>

            <div id="logoutButtonContainer">
                <a id="logoutButton" href="">Logout</a>
            </div>
            <div id="paginationContainer">
            </div>



        </div>
        
        <div id="contentContainer" class="col-sm-8">
            <div id="spamTextContainer">
                <textarea  placeholder="Paste the content of your email message here ..." id="spamTextarea" class="form-control" rows="5" id="comment"></textarea>
            </div>
            <div id="submitSearchButton">  
                <a id="runSpamButton" href="">Run spam analysis</a>
            </div>

            <div id="resultContainer">
                Your result will appear here!
            </div>
        </div>

    </body>
</html>
