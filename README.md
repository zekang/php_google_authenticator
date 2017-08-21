# php_google_authenticator

php_google_authenticator是Google验证器服务端php实现，可用于网站后台动态令牌验证。

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
    ```

## 使用

