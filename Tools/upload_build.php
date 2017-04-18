<?php
// TODO: deal with PUT only
if (isset($_GET['build_file'])) {
  // TODO: remove folder names if any
  $build_file = $_GET['build_file'];
  $putdata = fopen('php://input', 'r');
  // TODO: check if it is file and exists
  $fp = fopen('builds/' . $build_file, 'w');
  while ($data = fread($putdata, 1024)) {
    fwrite($fp, $data);
  }
  fclose($fp);
  fclose($putdata);
  echo 'ok';
}
else {
  header('HTTP/1.0 400 Bad Request');
}
?>