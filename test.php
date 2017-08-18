<?php
$key =  "OEZ6IPPKDE5JMY7Z";
$googleAuth = new GoogleAuthenticator();
echo $googleAuth->generateSecretKey(1);
echo PHP_EOL;
echo $googleAuth->generateCode($key);
echo PHP_EOL;
echo $googleAuth->getQRBarcodeURL('http://www.baidu.com','465708481@qq.com',$key);
echo PHP_EOL;
$flag = $googleAuth->validate($key,781844);
var_dump($flag);
$data = '123456';
echo $googleAuth->base32_encode($data);
$data = 'G@EZDGNBVGY';
var_dump( $googleAuth->base32_decode($data));

