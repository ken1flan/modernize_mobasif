# サンプルアプリケーション「モバログ」

一般的なWebアプリケーションの一般的と思われる、情報の参照、会員登録、ログイン・ログアウト、会員のみの情報登録が含まれるものをと考え、記事管理アプリケーション、「モバログ」を作成しました。

ソースは[こちら](https://github.com/ken1flan/mobasif_sample)で管理しています。
[https://github.com/ken1flan/mobasif_sample](https://github.com/ken1flan/mobasif_sample)

## 前提条件

Docker Composeが使えること

## インストール

```console
$ git clone git@github.com:ken1flan/mobasif_sample.git
$ docker-compose up
$ docker-compose exec mobasif /usr/local/lib/mobalog/bin/setup.sh
```

ブラウザで http://localhost にアクセスすると、モバログのトップページが見られます。

## 機能詳細

- 会員登録
- ログイン、ログアウト
- 自分の記事の一覧、詳細、作成、編集、削除
- 記事の一覧、詳細

### 会員登録

### ログイン、ログアウト

### 自分の記事の一覧、詳細、作成、編集、削除

### 記事の一覧、詳細
