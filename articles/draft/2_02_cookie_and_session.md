# クッキー、セッションとログイン

自分の記事を登録するアプリケーションなんだから、当然ログインしないとダメだよね。
いつもどおりに作りますか、クッキーにセッションIDを保存して…。

…クッキーどこ？

そんなものはありません。
MobaSiFはガラケー用のWebアプリケーションフレームワークです。

そこから実装する羽目になるとは…。

## クッキー

CGI::Cookieという便利なモジュールがあるので、全面的にお世話になります…。

各処理の最初で、クッキーをグローバルなモジュール変数`$_::C`に保存しておきます。

```perl
# pm/Main.pm
# :
# :
sub main {
	my $t1 = Time::HiRes::time();
	#-------------------------
	# 初期化
	my $func = '';
	eval {
    # :
    # :
		#-------------------------------
		# Cookie設定
		my %cookies = fetch CGI::Cookie;
		$_::C = \%cookies;
    # :
```

また、`Response::output`でレスポンスを返すときに、ヘッダの位置に`$_::C`の内容を記入して、ブラウザに伝えています。

```perl
# pm/Response.pm
# :
# :
sub output {
	my ($rHtml, $cache) = @_;
	my $html = ${$rHtml};

	# content-type は内容を見て決定
	my $charset = 'Shift_JIS';  # TODO: 自動判別にする。
	print "Content-type: text/html; charset=$charset\r\n";

	# cookieを設定
	_print_cookies($_::C);

	$html = '' if ($ENV{REQUEST_METHOD} eq 'HEAD');

	my $len = length($html);
	print "Content-length: $len\r\n";

	print "Connection: close\r\n";
	print "\r\n$html";
}
# :
# :
sub _print_cookies {
	my ($cookies) = @_;

	foreach (keys %$cookies) {
		print 'Set-Cookie:', $cookies->{$_}, "\r\n";
	}
}
# :
# :
```

## セッション

クッキーができれば、セッションも準備ができます。
クッキーにセッションを識別する値を持たせ、サーバ側でそのセッションに紐づくセンシティブな情報を持たせます。ここではセッションの値をファイルに保存していますが、データベースに保存することもできます。

```perl
# pm/Main.pm
# :
# :
		#-------------------------------
		# Cookie設定
		my %cookies = fetch CGI::Cookie;
		$_::C = \%cookies;

		#-------------------------------
		# セッション設定
		_restore_or_create_session();
# :
# :
sub _restore_or_create_session {
	CGI::Session->name('session_id');
	my $session_id = $_::C->{session_id} ? $_::C->{session_id}->value : undef;
	$_::S = new CGI::Session("driver:File", $session_id, {Directory=> $_::SESSION_DIR});
	$session_id = $_::S->id() unless (defined($session_id));
	$_::C->{session_id} = new CGI::Cookie(-name => 'session_id', -value => $session_id, -expires => '+1y');
	$_::S->expires('+1y');
}
# :
# :
```

## ログイン

セッションができたので、ログイン処理をかけるようになりました。
ユーザのログインID(ここではメールアドレス)とパスワードが送られてきたら、メールアドレスでユーザ情報を取得し、そこに保存されているパスワードと照合します。問題がなければセッションにユーザIDを保存します。
セキュリティ的にはログインが成功したときに、一度セッションIDを振り直すと、セッションIDを盗み出してユーザがログインするのを待っていた悪意のある人をがっかりさせることができるのですが…今度やります。

```perl
# pm/Page/Session.pm
package Page::Session;
# :
# :
sub pageCreate {
	my $func = shift;
	my $rhData = {};

	my $template_name;
	if (Session::create($_::F->{email}, $_::F->{password})) {
		$template_name = 'session/create';
	} else {
		$rhData->{Err} = 1;
		$rhData->{email} = $_::F->{email};
		$template_name = 'session/new';
	}

	my $html = HTMLTemplate::insert($template_name, $rhData);
	Response::output(\$html);
}
# :
# :
```

```perl
# pm/Session.pm
package Session;
# :
# :
sub create {
  my ($email, $password) = @_;

  my $user = Func::User::find_by_email($email);
  return 0 unless (defined($user));

  my $pwd = Text::Password::SHA->new();
  return 0 unless ($pwd->verify($password, $user->{hashed_password}));

  # TODO: Reset session_id
  $_::S->param("user_id", $user->{user_id});

  return 1;
}
# :
# :
```

## ログイン中のユーザ情報取得

ログインが成功してセッションにユーザIDが保存されたら、次にアクセスしてきたときにそこからユーザ情報を取るようにします。

これで、ログインができるようになりました。

```perl
# pm/Pagn/Main.pm
package Page::Main;
# :
# :
		# Cookie設定
		my %cookies = fetch CGI::Cookie;
		$_::C = \%cookies;

		# セッション設定
		_restore_or_create_session();

		$_::F = new Request();  # リクエストパラメータを取得
		$_::U = new UserData(); # ユーザ情報を取得
# :
# :
```

```perl
# pm/UserData.pm
package UserData;
# :
# :
sub new {
# :
# :
	$self->getInfo() if (!$_::BYPASS_FUNC{$_::F->{f}});

	return($self);
}
# :
# :
sub getInfo {
	my ($self) = @_;

	my $user_id = $_::S->param('user_id');
	return undef unless defined($user_id);

	my $user = Func::User::find($user_id);
	return undef unless defined($user);

	$self->{USER_ID} = $user->{user_id};
	$self->{NICKNAME} = $user->{nickname};
	$self->{USER_ST} = $user->{user_st};
	$self->{SERV_ST} = $user->{serv_st};
	$self->{REG_MODEL} = $user->{model_name};

	return undef;
};
```
