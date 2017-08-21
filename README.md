# php_google_authenticator

php_google_authenticator是google身份验证器服务端php实现，可用于网站后台动态令牌验证。

## 从源码安装

1. 解压缩源码包
    ```
    tar -xf php_google_authenticator.tar.gz
    cd php_google_authenticator
    ```

2. 编译

    PHP扩展
    ```
    {php_bin_dir}/phpize
    ./configure --with-php-config={php_bin_dir}/php-config
    make
    ```
3. 安装配置

    安装PHP扩展至PHP目录
    ```
    make install
    ```

    编辑配置文件`php.ini`，增加下面配置信息。
    ```
    extension=php_google_authenticator.so
    google_authenticator.window_size = 3     
    ```

## 使用

1.函数原型

```php
/**
 * Class GoogleAuthenticator
 */
class GoogleAuthenticator
{
    /**
     * 校验验证码
     * @param string $secretKey  用户密钥
     * @param int $code   用户输入的验证码
     * @return bool
     */
    public function validate($secretKey,$code)
    {
        return false;
    }

    /**
     * @param $encodeStr
     * @return string
     */
    public function base32_encode($encodeStr)
    {
        return '';
    }

    /**
     * @param $decodeStr
     * @return string
     */
    public function base32_decode($decodeStr)
    {
        return '';
    }

    /**
     * 生成密钥
     * $token为null时，随机生成密钥，传入固定的字符串时 可以生成固定的密钥，如:根据用户名生成
     * @param null $token
     * @return string
     */
    public function generateSecretKey($token=null)
    {
        return '';
    }

    /**
     * 根据输入的密钥生成验证码，同google身份验证器客户端功能
     * @param $secretKey
     * @return int
     */
    public function generateCode($secretKey)
    {
        return 0;
    }

    /**
     * 生成一个google身份验证器，识别的字符串，只需要把该方法返回值生成二维码扫描就可以了
     * @param string $user 用户名
     * @param string $secretKey 用户密钥
     * @return string
     */
    public function getQRBarcode($user,$secretKey)
    {
        return '';
    }


    /**
     * 生成二维码链接，访问此链接用google身份验证器扫描即可
     * @param string $host 用于验证的网站域名
     * @param string $user 用户名
     * @param string $secretKey  用户密钥
     * @return string
     */
    public function getQRBarcodeURL($host,$user,$secretKey)
    {
        return '';
    }
}
```

2.生成用户密钥

```php
<?php
$googleAuth = new GoogleAuthenticator();
//随机生成
echo $googleAuth->generateSecretKey();
echo PHP_EOL;
//根据固定串生成
echo $googleAuth->generateSecretKey('465708481@qq.com');
echo PHP_EOL;
```
Output:
````
[root@localhost ~]# phpdev test.php 
OQ3BASVCUUTM2VNH
6AEUMY5VUJSV6ZI3
[root@localhost ~]# phpdev test.php 
UVD6Y5R4OLE72SUD
6AEUMY5VUJSV6ZI3
````

3.验证输入
```php
<?php
$key  =  "6AEUMY5VUJSV6ZI3";
$code = 781844 ;
$googleAuth = new GoogleAuthenticator();
$flag = $googleAuth->validate($key,$code);
var_dump($flag); 
```
Output:

````
[root@localhost ~]# phpdev test.php 
bool(false)
````
