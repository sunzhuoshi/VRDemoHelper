<?php
$UPLOAD_DIR = 'builds/';
$MAX_FILE_SIZE = 10000000;   // 10mb

$ALLOW_FILE_TYPES = array(
   'application/octet-stream' => 1
);

date_default_timezone_set('UTC');

$build_file = $_FILES['build_file'];
$build_file_name = basename($build_file['name']);
$build_file_size = $build_file['size'];
$build_file_type = $build_file['type'];
$build_file_upload_path = $UPLOAD_DIR . $build_file_name;

$ERROR_NO = 0;
$ERROR_TOO_LARGE_FILE_SERVER = 1;
$ERROR_TOO_LARGE_FILE = 2;
$ERROR_INVALID_FILE_TYPE = 3;
$ERROR_INVALID_PARAM = 4;
$ERROR_UNKNOWN = -1;

$detail_error_message = '';

$ERROR_MESSAGES = array(
    $ERROR_NO => 'ok',
    $ERROR_TOO_LARGE_FILE_SERVER => 'too large file(server)',
    $ERROR_TOO_LARGE_FILE => "too large file($build_file_size / $MAX_FILE_SIZE)",
    $ERROR_INVALID_FILE_TYPE => "invalid file type($build_file_type)",
    $ERROR_INVALID_PARAM => 'invalid parameter',
    $ERROR_UNKNOWN => 'unknown error'
);

$result_code = $ERROR_NO;

if (!$build_file) {
    $result_code = $ERROR_INVALID_PARAM;
}
else {
    $file_error = $build_file['error'];
    switch($file_error) {
        case 1:
            $result_code = $ERROR_TOO_LARGE_FILE_SERVER;
            break;
        case 0:
            if ($image_file_size > $MAX_FILE_SIZE)  {
                $result_code = $ERROR_TOO_LARGE_FILE;
            }
            else if (!array_key_exists($build_file_type, $ALLOW_FILE_TYPES)) {
                $result_code = $ERROR_INVALID_FILE_TYPE;
            }
            else {
                if (move_uploaded_file($build_file['tmp_name'], $build_file_upload_path)) {
                    $referrer = $_SERVER['REQUEST_URI'];
                    $endIndex = strrpos($referrer, '/');
                    if (0 <= $endIndex) {
                        $referrer = substr($referrer, 0, $endIndex + 1);
                        $build_file_url = 'http://'. $_SERVER['SERVER_NAME'] . $referrer . 'buids/' .  $build_file_name;
                    }
                    else {
                        $result_code = $ERROR_UNKNOWN;
                    }
                }
                else {
                    $result_code = $ERROR_UNKNOWN;
                }
            }
            break;
        default:
            $result_code = $ERROR_UNKNOWN;
            $detail_error_message = ", file error: $file_error";
            break;
    }
}

$response = array(
    'code' => $result_code,
    'msg' => $ERROR_MESSAGES[$result_code] . $detail_error_message
);

if ($result_code == $ERROR_NO) {
    $response['url'] = $build_file_url;
}

// JSON_UNESCAPED_SLASHES not supported by php 5.3;
echo json_encode($response, 0);

