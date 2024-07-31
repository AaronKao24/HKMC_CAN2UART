

int indexof_word(char *ppcmd, int len)
{
	int i ;
	for (i = 0; i <len; i++) {
		if (isalnum(ppcmd[i])) {
			return i ;
		}
	}
	return -1;
}

int indexof_sp_char(char *ppcmd, int len)
{
	int i ;
	for (i = 0; i <len; i++) {
		if ((ppcmd[i] == ' ')||(ppcmd[i] == '\t')||(ppcmd[i] == '\r')||(ppcmd[i] == '\n')) {
			return i ;
		}
	}
	return -1;
}
int str_to_hex_ary(char *ppcmd,unsigned char * hex_buf,int len)
{
	return 0 ;
}
long hex_to_long(char *ppcmd, int len)
{
	char h_digit ;
	int i ;
	long val = 0 ;
	if ((ppcmd[0] == '0') && ((ppcmd[1] == 'x') || (ppcmd[1] == 'X'))) {
		ppcmd += 2 , len -= 2 ; // Hex 字串綴字
	}
	if((ppcmd[len-1] == 'H') || (ppcmd[len-1] == 'h')) {
		len -= 1 ; // Hex 字串綴字
	}
	for (i = 0; i <len; i++) {
		if (isdigit(ppcmd[i])) {
			val = (val * 16) + (ppcmd[i]-'0') ;
			continue ;
		}
		h_digit = toupper(ppcmd[i]) ;
		if ( isxdigit(h_digit) ) { // 0x61
			val = (val * 16) + (h_digit-0x37) ;
		}
	}
	return val ;
}
