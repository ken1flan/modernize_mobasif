# フラッシュメッセージ

```perl
# pm/Flash.pm
package Flash;
  # :
  # (省略)
  # :
sub set {
  my ($message, $type) = @_;
  my $out_message = Encode::from_to($message, 'euc-jp', 'shiftjis');

  $_::S->param('flash_message', $message);
  $_::S->param('flash_type', $type);
}

sub get {
  return ($_::S->param('flash_message'), $_::S->param('flash_type'));
}

sub clear {
	$_::S->clear(['flash_message', 'flash_type']);
}
```

```perl
# pm/HTMLTemplate.pm
package HTMLTemplate;
  # :
  # (省略)
  # :
sub insert {
	my ($name, $rhParams, $rhParams2) = @_;
  # :
  # (省略)
  # :
	# フラッシュメッセージ
	($rhParams2->{FLASH_MESSAGE}, $rhParams2->{FLASH_TYPE}) = Flash::get();
	Flash::clear();

	# テンプレート処理

	# my $type = lc($ENV{MB_CARRIER_UA});
  my $type = 'p';
	my $html = MTemplate::insert(
		"$_::HTML_BIN_DIR/_system/$name.bin.$type",
			$rhParams, $rhParams2, $_::DEFAULT_CONFIG);

	return($html);
}
  # :
  # (省略)
  # :
```

```html
<!-- template/inc_html.txt -->
    :
    (省略)
    :
$INCDEF:navbar$
    :
    (省略)
    :
$ if (FLASH_MESSAGE) { $
  <div class="row py-3">
    <div class="col">
      <div class="alert alert-$=h:FLASH_TYPE$">
        $=h:FLASH_MESSAGE$
      </div>
    </div>
  </div>
$ } $
$/INCDEF$
    :
    (省略)
    :
```
