# 現行バージョンのOS・ミドルウェアを使い、Docker化する

基本的には、MobaSiFのインストール手順を読み替えて、Dockerfileを作っていきました。
ここでは、Dockerfileと、開発環境で使うdocker-imageについて説明します。

## サーバ構成

よくある構成のように、データベースサーバとWebアプリケーションサーバとで分けます。

- mobasif(Webアプリケーションサーバ)
- mariadb(データベースサーバ)

## 方針

### mobasif

Dockerイメージに含めるものは、基本的に変更の少ないOSとミドルウェアのみです。
アプリケーションのコードと、PerlのモジュールはDockerイメージには含めません。
例外的に、アプリケーションコードに近い、MobaSiFの独自のモジュールはそれほど更新がないはずなので、インストールします。

### mariadb

mariadbは公式のイメージを使うことにします。特にDockerfileは用意しません。

## mobasifのDockerfile

ここから、mobasifのDockerfileを説明していきます。

### CentOS

CentOSの公式イメージを元に作成します。

```Dockerfile
FROM centos:7
```

### systemd

CentOSのイメージそのままではsystemdが使えません。
[公式ページ](https://hub.docker.com/_/centos)の中程の`Dockerfile for systemd base image`記述に従って設定します。

```Dockerfile
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
```

### ビルド用

MobaSiF添付のXSモジュールをコンパイルするために使用します。

```Dockerfile
RUN yum install -y wget
RUN yum groupinstall -y "Development Tools"
```

### Apache

CentOSのyumリポジトリに管理されているApache HTTP Serverと、mod_fcgidをインストールし、ポート80でアクセスできるようにしています。
また、systemdに登録し、それ経由で起動します。

```Dockerfile
RUN yum install -y httpd
RUN yum install -y mod_fcgid
EXPOSE 80
RUN systemctl enable httpd.service
```

### MariaDB

CentOSのyumリポジトリに管理されているMariaDBとその開発キットをインストールしています。
MobaSiFは本来MySQLでしたが、yumリポジトリに含まれなくなってしまったために、互換性のあるMariaDBに変更しています。

```
RUN yum install -y mariadb mariadb-devel
```

### Perl

CentOSのyumリポジトリに管理されているPerl、開発キットをインストールしています。
また、パッケージ管理としてcartonを利用することにしたいため、cpanminusとcartonもインストールしています。

```Dockerfile
# perl
RUN yum install -y perl  # TODO: latest
RUN yum install -y perl-devel
RUN yum install -y perl-App-cpanminus
RUN cpanm Carton
```

### MobaSiF

MobaSiFのためのApacheの設定を読み込むように設定ファイルに追記しています。
また、XSモジュールをコンパイルしてインストールします。これらはあまりアップデートされないためです。
ただし、XSモジュールのうち、Mcodeは64bit環境でコンパイルできなかったために除外しました。詳しくは後述します。

```Dockerfile
RUN echo "Include /usr/local/lib/mobalog/conf/httpd.conf" >> /etc/httpd/conf/httpd.conf
COPY src/xs /tmp/xs
RUN cd /tmp/xs && ./makexs MobaConf && ./makexs MTemplate && ./makexs SoftbankEncode && ./makexs HTMLFast && ./makexs Kcode
```

#### Mcodeの除去

Mcodeとはガラケーの絵文字変換モジュール、MobaSiFのアイデンティティのひとつです。
ですが、これからMobaSiFを使う人、もしくはMobaSiFを使っていてDocker化したいと思っている人がいたとして、使うのかと言われると使わないと思います。Cのソースコードをつぶさに読んで64bit対応しても全く生かされないので、削除してしまうことにしました。
試行錯誤した軌跡は[こちら](https://github.com/ken1flan/mobasif_sample/pull/12)です。

`grep -ri mcode pm` で検索して、見つかった箇所を取り除いていきました。
削除した作業の詳細は[こちら](https://github.com/ken1flan/mobasif_sample/pull/16)です。

### テスト

```
COPY yum.repos.d/google-chrome.repo /etc/yum.repos.d
RUN yum install -y libpng libpng-devel google-chrome-stable
RUN yum install -y nmap-ncat
```



## docker-compose


## MCodeの除去
