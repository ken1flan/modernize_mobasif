# テスト

## モジュールのテスト

Test::Specを使います。

```perl
# test/pm/UserData.t
use Test::Spec;

use MobaConf;
use Test::Spec;
use Data::Dumper;

use UserData;

use CGI::Session;
use DA;
use Func::User;

describe 'UserData' => sub {
  describe 'getInfo' => sub {
    my $dbh = DA::getHandle($_::DB_USER_W);
    my $user;

    before all => sub {
      Func::User::create({ nickname => 'foobar', email => 'foobar@example.com', password => 'password' });
      DA::commit();

      $user = Func::User::find_by_email('foobar@example.com');
    };

    after all => sub {
      $dbh->do('TRUNCATE TABLE user_data;');
    };

    before each => sub {
      $_::S = new CGI::Session("driver:File", undef, {Directory=> $_::SESSION_DIR});
    };

    context 'セッションにuser_idが保存されていないとき' => sub {
      it 'ユーザ情報が空であること' => sub {
        my $user_data = new UserData();
        $user_data->getInfo();

        ok(!defined($user_data->{USER_ID}));
      };
    };

    context 'セッションに登録されていないuser_idが保存されているとき' => sub {
      before each => sub {
        $_::S->param('user_id', 99999);
      };

      it 'ユーザ情報が空であること' => sub {
        my $user_data = new UserData();
        $user_data->getInfo();

        ok(!defined($user_data->{USER_ID}));
      };
    };

    context 'セッションに登録されているuser_idが保存されているとき' => sub {
      before each => sub {
        $_::S->param('user_id', $user->{user_id});
      };

      it 'ユーザ情報が格納されていること' => sub {
        my $user_data = new UserData();
        $user_data->getInfo();

        ok($user_data->{USER_ID} == $user->{user_id});
      };
    };
  };
};

runtests unless caller;
```

## ブラウザを使ったテスト

Test::Specの他に、Mechanize::Chrome を使います。

```perl
use strict;
use warnings;
use utf8;

use Log::Log4perl qw(:easy);
use WWW::Mechanize::Chrome;
use Test::Spec;

use MobaConf;

use DA;
use Func::User;

describe 'Session' => sub {
  my $mech;

  before each => sub {
    $mech = WWW::Mechanize::Chrome->new(headless=> 1);

    Func::User::create({ nickname => 'foobar', email => 'foobar@example.com', password => 'password' });
    DA::commit();
  };

  after each => sub {
    my $dbh = DA::getHandle($_::DB_USER_W);
    $dbh->do('TRUNCATE TABLE user_data;');
  };

  it 'ログイン、ログアウトができること' => sub {
    # ログインページを訪れる。
    $mech->get('http://127.0.0.1/session/new');

    # 誤った認証情報を入力する。
    $mech->set_fields(email=> 'foobar@example.com', password=> 'passw0rd');
    $mech->click('login');
    ok($mech->content =~ 'Eメールまたはパスワードが間違っています。');

    # 正しい認証情報を入力する。
    $mech->get_set_value(name=> 'password', value=> 'password');
    $mech->click('login');
    ok($mech->content =~ 'ログインしました。');

    # トップページを訪れる。
    $mech->get('http://127.0.0.1/');
    ok($mech->content =~ 'foobar');

    # ログアウト
    $mech->get('http://127.0.0.1/session/destroy');
    ok($mech->content =~ 'ログアウトしました。');
  };

};
runtests unless caller;
```

## CircleCIでテスト実行

```yml
# .circleci/config.yml
version: 2.1

executors:
  my-executor:
    docker:
      - image: ken1flan/mobasif_sample
      - image: mariadb:10.3
        name: mariadb
        environment:
          MYSQL_ALLOW_EMPTY_PASSWORD: yes

jobs:
  prepare:
    executor: my-executor
    working_directory: /usr/local/lib/mobalog
    steps:
      - checkout
      - run:
          name: Set data dir permission
          command: chmod -R a+w data
      - restore_cache:
          key: cpanfile-cache-{{ .Environment.CI_CACHE_KEY }}-{{ checksum "cpanfile.snapshot" }}
      - run:
          name: carton install
          command: carton install
      - run:
          name: compile template
          command: MOBA_DIR=`pwd` carton exec script/tool/compile_template
      - save_cache:
          key: cpanfile-cache-{{ .Environment.CI_CACHE_KEY }}-{{ checksum "cpanfile.snapshot" }}
          paths:
            - local
      - persist_to_workspace:
          root: .
          paths:
            - ./

  test:
    executor: my-executor
    working_directory: /usr/local/lib/mobalog
    steps:
      - attach_workspace:
          at: .
      - run:
          name: start httpd
          command: /usr/sbin/httpd
      - run:
          name: waiting for mariadb to be ready
          command: |
            for i in `seq 20`; do
              nc -z mariadb 3306 && echo Success && exit 0
              echo -n .
              sleep 1
            done
            echo Failed waiging for mariadb to be ready && exit 1
      - run:
          name: create database
          command: mysql -h mariadb -u root < conf/createdb.sql
      - run:
          name: test
          command: |
            carton exec prove -r test

workflows:
  version: 2
  build:
    jobs:
      - prepare
      - test:
          requires:
            - build
```
