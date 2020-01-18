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
