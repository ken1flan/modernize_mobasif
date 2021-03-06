# ログイン

セッションができたので、ログイン処理をかけるようになりました。
よくある手順のとおりに作っていきます。

## 実装

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
