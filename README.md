# SpamML

**[March 2020 update] Won special mention
https://www.algoexpert.io/swe-project-contests/winter-2020

**AlgoExpert winter 2020 submission.**

https://www.youtube.com/watch?v=5-in3VOlvu4&feature=youtu.be

A machine-learning based spam detector created as a web application.
It runs on top of an asynchronous HTTP web server written in C for Linux systems, with multiplexing I/O capabilities. It is able to serve static content and dynamic pages (PHP pages).

Required for this project:
- HTTP parser (included): https://github.com/nodejs/http-parser
- libaio (sudo apt install libaio1 libaio-dev)
- PHP (v7.2 recommended)
- Bootstrap
- JQuery
- Sqlite3
- php7.2-imap
- gcc / make

## How to run
Run `./setup-dev-env.sh` script, which will check for missing dependencies, install them if necessary and then build the async server. The command to start the server is `./async_server` with the required cli arguments. Use `./async_server -h` for a detailed list of arguments.

![Help option](https://github.com/Nurckye/SpamML/blob/master/presentation_images/AWS_help.png)

To run the SpamML app you can use (spamML with lowercase 's'):

`./async_server -a -p 1234 -v -t spamML`

Which will run on port 1234, on the automatic mode (default) for the target directory spamML. '-v' stands for verbose and will tell the program to print all the logs on stdin.

![Logs](https://github.com/Nurckye/SpamML/blob/master/presentation_images/AWS_logs.png)

## Server implementation
The server executes a routine as follows - In the main loop, epoll is used to avoid busy waiting for connections. When a new connection appears, an event is triggered and the server is signaled to start the routine and parse the http header of the content. It checks if the resource path exists and if it does decides what type the desired resource is. After that it sends the right response header back to the client and starts processing the file. 

If it is a static file, like a picture or a javascript script it will be sent with the zero-copy mechanism in chunks of BUFFER_SIZE size to allow new connections to be processed in case the response is big, not to produce a delay for other clients. 

If the file is a dynamic one (PHP), an unnamed pipe is created and then a php process is forked, which will output the processed content of the file to the previously created pipe. In parallel, a new asynchronous context is created on the reading end of the pipe for the parent process. Using the libaio library, asynchronous operations can be done on the pipe (reading in chunks of BUFFER_SIZE), which will avoid blocking, therefore allowing processing other connections faster. 

## SpamML implementation 
SpamML is a web app that runs on async server previously described. The database was trained with a bussiness email dataset using a Python script (included). Over 40000 pre-labeled emails were used to build the classifier. 

It uses an adaptation of the Naive Bayes algorithm to compute the probability of an email being spam or ham.

Resources used:
- https://en.wikipedia.org/wiki/Naive_Bayes_spam_filtering
- https://en.wikipedia.org/wiki/Naive_Bayes_classifier

The single page interface: 


![InterfaceNotLogged](https://github.com/Nurckye/SpamML/blob/master/presentation_images/AWS_interface2.png)

The user interacts with the app trough the web interface, which exposes a textarea where the suspicious email can be pasted. When the request is made the server calls the `classification.php` script, which returns the probability after parsing the email. 
I also implemented a small Gmail client (activate less secure apps) with the IMAP protocol, which can be used to read the inbox content inside the application (takes about 10-15 seconds to connect and download the most recent 30 emails). 

For the client side of the app I opted for a simple interface, build with Bootstrap and JQuery.

![InterfaceLogged](https://github.com/Nurckye/SpamML/blob/master/presentation_images/AWS_interface.png)

## Final thoughts 

This application is heavily IO bound due to the amount of queries that have to be done and the delay the email link would produce, so the asynchronous implementation comes to rescue. A multithreaded implementation would have been more resource consuming due to the fact that threads are more expensive to create and destroy than asynchronous operations.

Example of usage:

![funEx](https://github.com/Nurckye/SpamML/blob/master/presentation_images/AWS_spamexample.png)
