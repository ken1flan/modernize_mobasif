# セッション

先の章でクッキーを実装しました。
クッキーができれば、セッションも準備ができます。
クッキーにセッションを識別する値を持たせ、サーバ側でそのセッションに紐づくセンシティブな情報を持たせます。ここではセッションの値をファイルに保存していますが、データベースに保存することもできます。

## 実装

[CGI::Session](https://perldoc.jp/docs/modules/CGI-Session-3.11/Session.pod)という便利なモジュールがあるので、基本的にそれを使っています。こちらではそれを適宜呼び出すのみです。

リクエストのクッキーを変数に復元した後に、そこに書かれたセッションを復元します。
復元のキーはクッキーに保存してある`session_id`で、この値がなければ新しくセッションを作成し、セッションIDをクッキーに保存しています。

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
	$_::S = new CGI::Session("driver:File", $session_id,
		{Directory=> $_::SESSION_DIR});
	$session_id = $_::S->id() unless (defined($session_id));
	$_::C->{session_id} = new CGI::Cookie(-name => 'session_id',
		-value => $session_id, -expires => '+1y');
	$_::S->expires('+1y');
}
# :
# :
```
