var emailRegex = /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
var passwordRegex = /(?!^$)([^\s])/;

var defaultTopMessage = "SpamML - AlgoExpert contest";

var emailLock = false;
var passLock = false;

var received_response;
var received_emails;
var current_user;
var hardLock;

var current_page = 0;
var emails_count = -1;
const entries_per_page = 4; 


$(document).ready(function(){
    $("#runSpamButton").click(function(event){
        event.preventDefault();
        spam_content = $("#spamTextarea").val();
        $.ajax({
            type: 'POST',
            url: 'classification.php', 
            data: spam_content
        }).done(function(data){
            received_response = JSON.parse(data);
            percentage = Math.round(received_response.classificationResult * 100 * 100) / 100;
            
            if (percentage > 90) 
                message = "Spam alert! This email has a high chance of being spam. (" + percentage + "\%).";
            else if(percentage > 60)
                message = "Possible spam. This email has a chance of being spam. (" + percentage + "\%).";
            else
                message = "This email is not a spam. The resulted chance is low. (" + percentage + "\%).";

            if (received_response.status == "FAILURE")
                alert("Server encounered problem");
            $("#resultContainer").text(message);
            
        })
        .fail(function() {
            alert("Request failed...");
        });
    });
});


$(document).ready(function(){
    $('.mailContent').click(function() {
        $('.activeMessageContainer').removeClass("activeMessageContainer");
        $(this).addClass("activeMessageContainer");
        $("#spamTextarea").val($(this).text());
    });
});


function prepare_string(malformed_string)
{
    let tmp = malformed_string.replace(/\=\?UTF-8\?Q\?/g, "");
    tmp = tmp.replace(/_/g, " ");
    tmp = tmp.replace(/\?=/g, "");
    tmp = tmp.replace(/=..=..=../g, "");
    tmp = tmp.replace(/=([A-Fa-f0-9]{2})/g, function(m, g1) {
        return String.fromCharCode(parseInt(g1, 16));
      });
    return tmp;
}

function paginate()
{
    $("#paginationContainer").empty();
    for (let i = current_page * entries_per_page; i < (current_page + 1) * entries_per_page && i < received_emails.length; ++i) {
        let message = "<div class=\"row messageBoxContainer\"><div id=\"mailFrame" + i + "\" class=\"col-sm-10 mailContent\">" + 
                        prepare_string(received_emails[i].date) + "  <b>" + 
                        prepare_string(received_emails[i].sender) + "</b></br>" + 
                        prepare_string(received_emails[i].subject) + "</div></div>";
        $("#paginationContainer").append(message);
    }
    if (current_page != 0) {
        $("#paginationContainer").append("<div id=\"prevButtonContainer\" class=\"col-md-3\"><a id=\"prevButton\" href=\"\">Prev</a></div>");
        $("#prevButton").css("text-decoration", "none");
        $("#prevButton").css("color", "#143A41");
        $("#prevButton").css("user-select", "none");
        
        $("#prevButtonContainer").css("background-color", "#88BDBC");
        $("#prevButtonContainer").css("text-align", "center");
        $("#prevButtonContainer").css("border-radius", "1vh");
        $("#prevButtonContainer").css("font-size", "3.5vh");
        $("#prevButtonContainer").css("font-weight", "bold");
        $("#prevButtonContainer").css("margin-top", "5%");
        $("#prevButtonContainer").css("width", "25%");
        $("#prevButtonContainer").css("display", "inline-block");

       
        $('#prevButton').click(function() {
            current_page--;
            paginate();
            return false;
        });
    }
    
    if ((current_page + 1) * entries_per_page <= received_emails.length) {
        $("#paginationContainer").append("<div id=\"nextButtonContainer\" class=\"col-md-3\"><a id=\"nextButton\" href=\"\">Next</a></div>");
        
        $("#nextButton").css("text-decoration", "none");
        $("#nextButton").css("color", "#143A41");
        $("#nextButton").css("user-select", "none");
        
        /*Maybe change to class */
        $("#nextButtonContainer").css("background-color", "#88BDBC");
        $("#nextButtonContainer").css("text-align", "center");
        $("#nextButtonContainer").css("border-radius", "1vh");
        $("#nextButtonContainer").css("font-size", "3.5vh");
        $("#nextButtonContainer").css("font-weight", "bold");
        $("#nextButtonContainer").css("margin-top", "5%");
        $("#prevButtonContainer").css("width", "25%");
        $("#nextButtonContainer").css("display", "inline-block");
        $("#nextButtonContainer").css("float", "right");
        $("#nextButtonContainer").css("margin-right", "13%");

        $('#nextButton').click(function() {
            current_page++;
            paginate();
            return false;
        });
    }

    
    $('.mailContent').click(function() {
        $('.activeMessageContainer').removeClass("activeMessageContainer");
        let indx = parseInt($(this).attr('id').replace("mailFrame", ""));
        $(this).addClass("activeMessageContainer");
        $("#spamTextarea").val(prepare_string(received_emails[indx].content));
    });

    $('#logoutButton').click(function() {
        $("#credsAndIntel").css("display", "block");
        $("#paginationContainer").css("display", "none");
        $("#logoutButton").css("display", "none");
    });
}


$(document).ready(function(){
    $("#sendCreds").click(function(event){
        event.preventDefault();

        current_user = $("#emailInput").val();
        credentials = {
            email: current_user,
            password: $("#passwordInput").val()
        }
        $.ajax({
            type: 'POST',
            url: 'mailer.php', 
            data: credentials
            // timeout: 14000
        }).done(function(data){
            received_emails = JSON.parse(data);
            $("#credsAndIntel").css("display", "none");
            $("#logoutButtonContainer").css("display", "block");
            $("#paginationContainer").css("display", "block");
            $("#letterCircle").text(current_user[0].toUpperCase());
            $("#userContainer").text(current_user)
            current_page = 0;
            paginate();
            
        })
        .fail(function() {
            alert("Request failed...");
            toogle_spinner_off();

        });
        toogle_spinner_on();

    });
});


$(document).ready(function() {
    $("#emailInput").on("change paste keyup", function() {
        if (! emailRegex.test($(this).val())) {
            $("#sendCreds").css("pointer-events", "none");
            $("#sendCreds").css("color", "#6e7d91");
            emailLock = false;
        }
        else {
            emailLock = true;
            if (emailLock && passLock && (!hardLock)) {
                $("#sendCreds").css("pointer-events", "auto");
                $("#sendCreds").css("color", "#143A41");
            }
        }
    });

    $("#passwordInput").on("change paste keyup", function() {
        if (! passwordRegex.test($(this).val())) {
            $("#sendCreds").css("pointer-events", "none");
            $("#sendCreds").css("color", "#6e7d91");
            passLock = false;
        }
        else {
            passLock = true;
            if (emailLock && passLock && (!hardLock)) {
                $("#sendCreds").css("pointer-events", "auto");
                $("#sendCreds").css("color", "#143A41");
            }
        }
    });
});


function toogle_spinner_off()
{
    $(".loader").css("display", "none");
    $("#sendCreds").css("pointer-events", "auto");
    $("#sendCreds").css("color", "#143A41");
    hardLock = false;
}

function toogle_spinner_on()
{
    $(".loader").css("display", "block");
    $("#sendCreds").css("pointer-events", "none");
    $("#sendCreds").css("color", "#6e7d91");
    hardLock = true;
}
