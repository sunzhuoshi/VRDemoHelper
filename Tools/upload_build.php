<?php
set_error_handler(function($errno, $errstr, $errfile, $errline) {
    error_log("$errstr, errno: $errno, errfile: $errfile, errline: $errline");
    throw new ErrorException($errstr, $errno, 0, $errfile, $errline);
});
    
function respond($code, $message) {
    header("HTTP/1.0 $code $message");
}
    
try {
    if ('PUT' == $_SERVER('REQUEST_METHOD')) {
        if (isset($_GET['build_file'])) {
            // remove folders if any(security reason)
            $splitted = preg_split('/[/\\\\]/', $_GET['build_file']);
            $c = count($splitted);
                
            if ($c) {
                $build_file = $splitted[$c - 1];
                $build_file_path = 'builds/' . $build_file;
                if (!file_exists($build_file_path)) {
                    $fp = fopen('builds/' . $build_file, 'w');
                    while ($data = fread($putdata, 1024)) {
                        fwrite($fp, $data);
                    }
                    fclose($fp);
                    fclose($putdata);
                    echo 'ok';
                }
                else {
                    respond(400, 'Bad Request: file already exists');
                }
            }
            else {
                respond(400, 'Bad Request: invalid parameter value');
            }
        }
        else {
            respond(400, 'Bad Request: build_file missed');
        }
    else {
        respond(400, 'Bad Request: http method unsupported');
    }
}
catch (Exception $e) {
    respond(500, 'Internal Server Error');
}
?>
