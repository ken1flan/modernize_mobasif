
= クッキー


自分の記事を登録するアプリケーションなんだから、当然ログインしないとダメだよね。
いつもどおりに作りますか、クッキーにセッションIDを保存して…。



…クッキーどこ？



そんなものはありませんでした。MobaSiFはガラケー用のWebアプリケーションフレームワークで、当時のガラケーではクッキーは一般的ではありませんでした。



そうはいっても、多くの開発者に愛されてそこから実装する羽目になるとは…。


== 実装


@<href>{https://perldoc.jp/docs/modules/CGI-2.89/CGI/Cookie.pod,CGI::Cookie}という便利なモジュールがあるので、全面的にお世話になります…。



各処理の最初で、クッキーをグローバルなモジュール変数@<tt>{$_::C}に保存しておきます。


//emlist[][perl]{
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
//}


また、@<tt>{Response::output}でレスポンスを返すときに、ヘッダに@<tt>{$_::C}の内容を記入して、ブラウザに伝えています。


//emlist[][perl]{
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
//}
