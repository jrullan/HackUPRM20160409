#!/usr/bin/env php
<?php

// Set no time limit
set_time_limit ( 0 );


require_once('./websockets.php');
require_once ('./functions.php');

class echoServer extends WebSocketServer
{
    //protected $maxBufferSize = 1048576; //1MB... overkill for an echo server, but potentially plausible for other applications.

    protected function validateUser($user, $message){
        $token ='';
        //$query = "SELECT token FROM devices WHERE token = '$token' " ;
       //print_r($user);



        $parameters = json_decode($message);
       // print "user id: " . $user->id . "  Token: " . $parameters->token ."\n";
       // print_r($this);
        if (isset($parameters->token)){
            if(trim($parameters->token) != '') {
                $this->token[$user->id] = $parameters->token;
            }
        }

    }

    protected function process($user, $message)
    {

        //$this->send($user, "hola : " . $message);

        // Verificamos que el device esta en la base de datos y lo actualizamos. !!

        $this->validateUser($user, $message);

        print "Data From User: " . $message . "\n";
        $this->protocol($user, $message);
    }

    protected function connected($user)
    {
        // Do nothing: This is just an echo server, there's no need to track the user.
        // However, if we did care about the users, we would probably have a cookie to
        // parse at this step, would be looking them up in permanent storage, etc
        //verify that the client is the the database..
        
        
        

        

    }

    protected function closed($user)
    {
        // Do nothing: This is where cleanup would go, in case the user had any sort of
        // open files or other objects associated with them.  This runs after the socket
        // has been closed, so there is no need to clean up the socket itself here.
    }


    protected function protocol ($user, $message ){

        //Rompemos el mensaje porque llegan en un json !!

        $parameters = json_decode($message);

       // print_r($parameters). "\n" ;

        switch ($parameters->cmd ) {
            case "ID":
                $this->send($user, $this->createMsg('ACK','',"Connected"));
                break;

            case "msg":
                $this->send($user, $this->createMsg('ACK','',"MSG Received"));
                break;
            case "getUsers":
                $this->send($user, $this->createMsg('users','',$this->getUsers()  ));
                break;
            case "digitalWriteOn":
                $this->send($this->users[$parameters->client], $this->createMsg($parameters->cmd,'',$parameters->msg ));
                print " ENVIANDO MENSAJE A: " .$parameters->client ;
                break;
            case "digitalWriteOff":
                $this->send($this->users[$parameters->client], $this->createMsg($parameters->cmd,'',$parameters->msg ));
                print " ENVIANDO MENSAJE A: " .$parameters->client ;
                break;
        }



        switch ($parameters->msg ){



            case "totalUsers":
                $this->send($user, "Total de Usuarios");
                break;
            case "identify":
                break;

/*
            default:
                print $message . "\n" ;
                $this->send($user, $this->createMsg('msg','',$message));
*/
        }
    }
    
    protected function getUsers(){

        $clients = array();


        foreach ($this->users as $key => $value) {

            $clients[] = $value->id  ;


        }

        //print_r($this->users);
        return json_encode($clients);
}

    protected function createMsg($cmd,$client,$msg){
        //create the json that is going to be send to the client.
        $parameters = array();
        $parameters["cmd"] = $cmd;
        $parameters["msg"] = $msg;
        $parameters["client"] = $client;
        //$parameters["token"] = $token;

        print json_encode($parameters,JSON_UNESCAPED_SLASHES) . "\n";
        return json_encode($parameters,JSON_UNESCAPED_SLASHES);


    }


}




$server = 'vidal.ws';
$serverIP = gethostbyname($server);
$internalIP = "10.0.0.10";

print $serverIP;
//$echo = new echoServer($server,"9000");
$echo = new echoServer($internalIP,"443");


try {
  $echo->run();
}
catch (Exception $e) {
  $echo->stdout($e->getMessage());
}
