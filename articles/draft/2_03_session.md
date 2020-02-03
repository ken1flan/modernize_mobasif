# セッション

自分の記事を登録するアプリケーションなんだから、当然ログインしないとダメだよね。
いつもどおりに作りますか、クッキーにセッションIDを保存して…。

…クッキーどこ？

そんなものはありません。
MobaSiFはガラケー用のWebアプリケーションフレームワークです。

そこから実装する羽目になるとは…。

## 実装

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
