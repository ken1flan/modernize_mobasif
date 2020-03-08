
= 現行バージョンのOS・ミドルウェアを使い、Docker化する


基本的には、MobaSiFのインストール手順を読み替えて、Dockerfileを作っていきました。
ここでは、Dockerfileと、開発環境で使うdocker-compose.ymlについて説明します。


== コンテナ構成


よくある構成のように、データベースサーバとWebアプリケーションサーバとで分けます。

 * mobasif(Webアプリケーションサーバ)
 * mariadb(データベースサーバ)


== Dockerイメージの方針

=== mobasif


Dockerイメージに含めるものは、基本的に変更の少ないOSとミドルウェアのみです。
アプリケーションのコードと、PerlのモジュールはDockerイメージには含めません。
例外的に、アプリケーションコードに近い、MobaSiFの独自のモジュールはそれほど更新がないはずなので、インストールします。


=== mariadb


mariadbは公式のイメージを使うことにします。特にDockerfileは用意しません。


== mobasifのDockerfile


ここから、mobasifのDockerfileを説明していきます。


=== CentOS


CentOSの公式イメージを元に作成します。


//emlist[][Dockerfile]{
FROM centos:7
//}

=== systemd


CentOSのイメージそのままではsystemdが使えません。
@<href>{https://hub.docker.com/_/centos,公式ページ}の中程の@<tt>{Dockerfile for systemd base image}記述に従って設定します。


//emlist[][Dockerfile]{
RUN (cd /lib/systemd/system/sysinit.target.wants/; for i in *; do [ $i == \
systemd-tmpfiles-setup.service ] || rm -f $i; done); \
rm -f /lib/systemd/system/multi-user.target.wants/*;\
rm -f /etc/systemd/system/*.wants/*;\
rm -f /lib/systemd/system/local-fs.target.wants/*; \
rm -f /lib/systemd/system/sockets.target.wants/*udev*; \
rm -f /lib/systemd/system/sockets.target.wants/*initctl*; \
rm -f /lib/systemd/system/basic.target.wants/*;\
rm -f /lib/systemd/system/anaconda.target.wants/*;
VOLUME [ "/sys/fs/cgroup" ]

# (省略)

CMD ["/usr/sbin/init"]
//}

=== ビルド用


MobaSiF添付のXSモジュールをコンパイルするために使用します。


//emlist[][Dockerfile]{
RUN yum install -y wget
RUN yum groupinstall -y "Development Tools"
//}

=== Apache


CentOSのyumリポジトリに管理されているApache HTTP Serverと、mod@<b>{fcgidをインストールし、ポート80でアクセスできるようにしています。MobaSiFのドキュメントではmod}fastcgiでしたが、すでになくなっていたため、mod_fcgidに変更しています。
また、systemdに登録し、それ経由で起動します。


//emlist[][Dockerfile]{
RUN yum install -y httpd
RUN yum install -y mod_fcgid
EXPOSE 80
RUN systemctl enable httpd.service
//}

=== MariaDB


CentOSのyumリポジトリに管理されているMariaDBとその開発キットをインストールしています。
MobaSiFは本来MySQLでしたが、yumリポジトリに含まれなくなってしまったために、互換性のあるMariaDBに変更しています。


//emlist{
RUN yum install -y mariadb mariadb-devel
//}

=== Perl


CentOSのyumリポジトリに管理されているPerl、開発キットをインストールしています。
また、パッケージ管理としてcartonを利用することにしたいため、cpanminusとcartonもインストールしています。


//emlist[][Dockerfile]{
# perl
RUN yum install -y perl  # TODO: latest
RUN yum install -y perl-devel
RUN yum install -y perl-App-cpanminus
RUN cpanm Carton
//}

=== MobaSiF


MobaSiFのためのApacheの設定を読み込むように設定ファイルに追記しています。
また、XSモジュールをコンパイルしてインストールします。これらはあまりアップデートされないためです。
ただし、XSモジュールのうち、Mcodeは64bit環境でコンパイルできなかったために除外しました。詳しくは後述します。


//emlist[][Dockerfile]{
RUN echo "Include /usr/local/lib/mobalog/conf/httpd.conf" >> \
  /etc/httpd/conf/httpd.conf
COPY src/xs /tmp/xs
RUN cd /tmp/xs && ./makexs MobaConf && ./makexs MTemplate && \
  ./makexs SoftbankEncode && ./makexs HTMLFast && ./makexs Kcode
//}

==== Apacheの設定


Apacheのデフォルトの設定ファイルの最後で読み込むようにさせた@<tt>{conf/httpd.conf}です。
アプリケーションを@<tt>{/usr/local/lib/mobalog}に配置していますが、その中の@<tt>{fcgi}ディレクトリでCGIを実行できるようにしています。
また、@<tt>{htdocs}は画像などの静的ファイルが置かれているディレクトリでこちらも読み込みできるように設定しています。こちらはCGIを実行できるようにしてありません。


//emlist{
# conf/httpd.conf
<VirtualHost *>
ServerName   lvh.me
DocumentRoot /usr/local/lib/mobalog/htdocs
Alias  /fcgi /usr/local/lib/mobalog/fcgi
Include      /usr/local/lib/mobalog/conf/rewrite.conf

LogFormat    "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-agent}i\"\
  \"%{x-dcmguid}i\" \"%{x-up-subno}i\" \"%{x-jphone-uid}i\" " custom
CustomLog    /usr/local/lib/mobalog/data/log/access_log custom
ErrorLog     /usr/local/lib/mobalog/data/log/error_log

<Directory /usr/local/lib/mobalog/htdocs>
  Order allow,deny
  Allow from all
  Require all granted
</Directory>

<Directory /usr/local/lib/mobalog/fcgi>
  SetHandler fcgid-script
  Options +ExecCGI

  Order allow,deny
  Allow from all
  Require all granted
</Directory>
</VirtualHost>
//}

==== Mcodeの除去


Mcodeとはガラケーの絵文字変換モジュール、MobaSiFのアイデンティティのひとつです。
ですが、これからMobaSiFを使う人、もしくはMobaSiFを使っていてDocker化したいと思っている人がいたとして、使うのかと言われると使わないと思います。Cのソースコードをつぶさに読んで64bit対応しても全く生かされないので、削除してしまうことにしました。
試行錯誤した軌跡は@<href>{https://github.com/ken1flan/mobasif_sample/pull/12,こちら}です。



@<tt>{grep -ri mcode pm} で検索して、見つかった箇所を取り除いていきました。
削除した作業の詳細は@<href>{https://github.com/ken1flan/mobasif_sample/pull/16,こちら}です。


=== テスト


テストで使うモジュールをインストールしています。
E2EテストでHeadless Chromeを使うのでChromeと、Chromeで表示した際に画像を保尊することもあると思ったのでPNGのライブラリをインストールしています。
Chromeは標準のyumリポジトリにないので、追加のリポジトリを設定しています。


//emlist{
COPY yum.repos.d/google-chrome.repo /etc/yum.repos.d
RUN yum install -y libpng libpng-devel google-chrome-stable
RUN yum install -y nmap-ncat
//}

== docker-compose


マシンをよくある構成のようにWebアプリケーションサーバ(mobasif)とデータベースサーバ(mariadb)で分割したので、@<tt>{docker}のみで対応しようとすると、起動が大変です。docker-composeを使って、マシンの構成や起動時の設定をまとめ、起動しやすくします。


=== mobasif


Webアプリケーションサーバのmobasifは、前述のDockerfileに記述された内容のイメージを使います。カレントディレクトリを@<tt>{/usr/local/lib/mobalog}にマウントし、自PCから直接編集したものが反映されるようになっています。


//emlist[][yml]{
# docker-compose.yml
version: '2'
services:
  mobasif:
    image: ken1flan/mobasif_sample
    ports:
      - "80:80"
    volumes:
      - .:/usr/local/lib/mobalog
    depends_on:
      - mariadb
    privileged: true
#  :
# 省略
#  :
//}

=== mariadb


データベースサーバのmariadbは、MariaDB公式のDockerイメージを使います。
起動時の設定を制御するために、環境変数を設定したり、MariaDBの起動時に自動で読み込まれる@<tt>{/etc/mysql/mariadb.conf.d}にファイルをマウントしたりしています。


//emlist[][yml]{
# docker-compose.yml
version: '2'
services:
#  :
# 省略
#  :
  mariadb:
    image: mariadb:10.3-bionic
    env_file: ./mariadb/.env
    ports:
      - "3306:3306"
    volumes:
      - ./mariadb/etc/mysql/mariadb.conf.d:/etc/mysql/mariadb.conf.d
//}
