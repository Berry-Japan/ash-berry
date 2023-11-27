#!/bin/sh -
#
# Copyright (c) 1991 The Regents of the University of California.
# All rights reserved.
#
# This code is derived from software contributed to Berkeley by
# Kenneth Almquist.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#	This product includes software developed by the University of
#	California, Berkeley and its contributors.
# 4. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
#	@(#)mktokens	5.1 (Berkeley) 3/7/91
#
#	/b/source/CVS/src/bin/sh/mt,v 1.3 1993/03/23 00:28:48 cgd Exp

# The following is a list of tokens.  The second column is nonzero if the
# token marks the end of a list.  The third column is the name to print in
# error messages.

cat > /tmp/ka$$ <<\!
TEOF	1	end of file
TNL	0	newline
TSEMI	0	";"
TBACKGND 0	"&"
TAND	0	"&&"
TOR	0	"||"
TPIPE	0	"|"
TLP	0	"("
TRP	1	")"
TENDCASE 1	";;"
TENDBQUOTE 1	"`"
TREDIR	0	redirection
TWORD	0	word
TIF	0	"if"
TTHEN	1	"then"
TELSE	1	"else"
TELIF	1	"elif"
TFI	1	"fi"
TWHILE	0	"while"
TUNTIL	0	"until"
TFOR	0	"for"
TDO	1	"do"
TDONE	1	"done"
TBEGIN	0	"{"
TEND	1	"}"
TCASE	0	"case"
TESAC	1	"esac"
!
nl=`wc -l /tmp/ka$$`
exec 
awk "-F "  '{print $1 "#define " NR-1}' /tmp/ka$$

rm /tmp/ka$$
