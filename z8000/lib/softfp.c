/* IEEE software arithmetic, round to nearest/even. Words are most significant
 * first. Only target integer arithmetic is used; no host floating arithmetic. */
static copy4(d,s)
unsigned short *d,*s;
{ int i; for(i=0;i<4;i++) d[i]=s[i]; }

static shrjam(m,n)
unsigned short *m;
int n;
{
	unsigned sticky,carry,next;
	int i;
	if(n>56) n=56;
	while(n--) {
		sticky=m[3]&1; carry=0;
		for(i=0;i<4;i++) {
			next=(m[i]&1)<<15;
			m[i]=(m[i]>>1)|carry;
			carry=next;
		}
		m[3] |= sticky;
	}
}

static shl1(m)
unsigned short *m;
{
	unsigned carry,next;
	int i;
	carry=0;
	for(i=3;i>=0;i--) {
		next=m[i]>>15;
		m[i]=(m[i]<<1)|carry;
		carry=next;
	}
}

static cmp4(a,b)
unsigned short *a,*b;
{
	int i;
	for(i=0;i<4;i++) {
		if(a[i]>b[i]) return 1;
		if(a[i]<b[i]) return -1;
	}
	return 0;
}

static add4(a,b)
unsigned short *a,*b;
{
	unsigned long sum;
	unsigned carry;
	int i;
	carry=0;
	for(i=3;i>=0;i--) {
		sum=(unsigned long)a[i]+b[i]+carry;
		a[i]=sum;
		carry=sum>>16;
	}
}

static sub4(a,b)
unsigned short *a,*b;
{
	unsigned borrow,next,t;
	int i;
	borrow=0;
	for(i=3;i>=0;i--) {
		t=a[i]; next=t<b[i] || (borrow && t==b[i]);
		a[i]=t-b[i]-borrow;
		borrow=next;
	}
}

daddcore(out,a,b)
unsigned short *out,*a,*b;
{
	unsigned short x[4],y[4],tmp[4];
	unsigned sx,sy,sign,carry;
	unsigned long sum;
	int ex,ey,e,i,c;
	sx=a[0]&0x8000; sy=b[0]&0x8000;
	ex=(a[0]>>4)&2047; ey=(b[0]>>4)&2047;
	if(ex==2047 || ey==2047) {
		if(ex==2047 && ((a[0]&15)||a[1]||a[2]||a[3])) {
			copy4(out,a); out[0]|=8; return;
		}
		if(ey==2047 && ((b[0]&15)||b[1]||b[2]||b[3])) {
			copy4(out,b); out[0]|=8; return;
		}
		if(ex==2047 && ey==2047 && sx!=sy) {
			out[0]=0x7ff8; out[1]=out[2]=out[3]=0; return;
		}
		copy4(out,ex==2047?a:b); return;
	}
	copy4(x,a); copy4(y,b);
	x[0]&=15; y[0]&=15;
	if(ex) x[0]|=16; else ex=1;
	if(ey) y[0]|=16; else ey=1;
	for(i=0;i<3;i++) { shl1(x); shl1(y); }
	if(ex<ey) {
		copy4(tmp,x); copy4(x,y); copy4(y,tmp);
		e=ex; ex=ey; ey=e; sign=sx; sx=sy; sy=sign;
	}
	e=ex; sign=sx;
	shrjam(y,ex-ey);
	if(sx==sy) {
		add4(x,y);
		if(x[0]&256) { shrjam(x,1); e++; }
	} else {
		c=cmp4(x,y);
		if(c<0) {
			copy4(tmp,x); copy4(x,y); copy4(y,tmp); sign=sy;
		}
		sub4(x,y);
		if(!(x[0]||x[1]||x[2]||x[3])) {
			out[0]=out[1]=out[2]=out[3]=0; return;
		}
		while(!(x[0]&128) && e>1) { shl1(x); e--; }
	}
	/* Three guard/round/sticky bits, then ties to the even significand. */
	if((x[3]&7)>4 || ((x[3]&7)==4 && (x[3]&8))) {
		carry=8;
		for(i=3;i>=0;i--) {
			sum=(unsigned long)x[i]+carry; x[i]=sum; carry=sum>>16;
		}
		if(x[0]&256) { shrjam(x,1); e++; }
	}
	/* Discard GRS bits without jamming them into the result. */
	for(i=3;i>0;i--) x[i]=(x[i]>>3)|(x[i-1]<<13);
	x[0]>>=3;
	if(e>=2047) { out[0]=sign|0x7ff0; out[1]=out[2]=out[3]=0; return; }
	if(!(x[0]&16)) e=0;
	copy4(out,x); out[0]=sign|(e<<4)|(x[0]&15);
}

/* Normalized 53-bit significand and unbiased exponent. */
static unpack(m,a)
unsigned short *m,*a;
{
	int e;
	copy4(m,a); m[0]&=15;
	e=(a[0]>>4)&2047;
	if(e) { m[0]|=16; return e-1023; }
	e = -1022;
	if(m[0]||m[1]||m[2]||m[3])
		while(!(m[0]&16)) { shl1(m); e--; }
	return e;
}

static pack(out,m,e,sign)
unsigned short *out,*m;
int e;
unsigned sign;
{
	unsigned carry;
	unsigned long sum;
	int i;
	if(e < -1022) { shrjam(m,-1022-e); e = -1022; }
	if((m[3]&7)>4 || ((m[3]&7)==4 && (m[3]&8))) {
		carry=8;
		for(i=3;i>=0;i--) { sum=(unsigned long)m[i]+carry; m[i]=sum; carry=sum>>16; }
	}
	if(m[0]&256) { shrjam(m,1); e++; }
	if(e>1023) { out[0]=sign|0x7ff0; out[1]=out[2]=out[3]=0; return; }
	for(i=3;i>0;i--) m[i]=(m[i]>>3)|(m[i-1]<<13);
	m[0]>>=3;
	copy4(out,m);
	out[0]=sign|((m[0]&16)?((e+1023)<<4):0)|(m[0]&15);
}

static zero(a)
unsigned short *a;
{ return !((a[0]&32767)||a[1]||a[2]||a[3]); }

static nan(a)
unsigned short *a;
{ return (a[0]&0x7ff0)==0x7ff0 && ((a[0]&15)||a[1]||a[2]||a[3]); }

static special(out,a,b,div)
unsigned short *out,*a,*b;
int div;
{
	int ia,ib,za,zb;
	unsigned sign;
	ia=(a[0]&0x7ff0)==0x7ff0; ib=(b[0]&0x7ff0)==0x7ff0;
	za=zero(a); zb=zero(b); sign=(a[0]^b[0])&32768;
	if(nan(a)||nan(b)||(div?(ia&&ib)||(za&&zb):(ia&&zb)||(ib&&za))) {
		out[0]=0x7ff8; out[1]=out[2]=out[3]=0; return 1;
	}
	if(div?(ia||zb):(ia||ib)) {
		out[0]=sign|0x7ff0; out[1]=out[2]=out[3]=0; return 1;
	}
	if(div?(za||ib):(za||zb)) {
		out[0]=sign; out[1]=out[2]=out[3]=0; return 1;
	}
	return 0;
}

dsubcore(out,a,b)
unsigned short *out,*a,*b;
{ unsigned short t[4]; copy4(t,b); t[0]^=32768; daddcore(out,a,t); }

dnegcore(out,a)
unsigned short *out,*a;
{ copy4(out,a); out[0]^=32768; }

dmulcore(out,a,b)
unsigned short *out,*a,*b;
{
	unsigned short x[4],y[4],z[8];
	unsigned carry,sticky,next;
	unsigned long t;
	int e,i,j,n;
	if(special(out,a,b,0)) return;
	e=unpack(x,a)+unpack(y,b);
	for(i=0;i<8;i++) z[i]=0;
	for(i=3;i>=0;i--) {
		carry=0;
		for(j=3;j>=0;j--) {
			t=(unsigned long)x[i]*y[j]+z[i+j+1]+carry;
			z[i+j+1]=t; carry=t>>16;
		}
		z[i]=carry;
	}
	n=49;
	if(z[1]&512) { n++; e++; }
	while(n--) {
		sticky=z[7]&1; carry=0;
		for(i=0;i<8;i++) { next=(z[i]&1)<<15; z[i]=(z[i]>>1)|carry; carry=next; }
		z[7]|=sticky;
	}
	for(i=0;i<4;i++) x[i]=z[i+4];
	pack(out,x,e,(unsigned)((a[0]^b[0])&32768));
}

ddivcore(out,a,b)
unsigned short *out,*a,*b;
{
	unsigned short x[4],y[4],q[4];
	int e,i;
	if(special(out,a,b,1)) return;
	e=unpack(x,a)-unpack(y,b);
	if(cmp4(x,y)<0) { shl1(x); e--; }
	for(i=0;i<4;i++) q[i]=0;
	for(i=0;i<56;i++) {
		shl1(q);
		if(cmp4(x,y)>=0) { sub4(x,y); q[3]|=1; }
		shl1(x);
	}
	if(x[0]||x[1]||x[2]||x[3]) q[3]|=1;
	pack(out,q,e,(unsigned)((a[0]^b[0])&32768));
}

/* Relations are passed explicitly so unordered comparisons preserve C's
 * NaN rules rather than depending on subtracting the operands. */
dcompare(a,b,op)
unsigned short *a,*b;
int op;
{
	int c;
	unsigned short x[4],y[4];
	if(nan(a)||nan(b)) return op==1;
	copy4(x,a); copy4(y,b); x[0]&=32767; y[0]&=32767;
	c=cmp4(x,y);
	if(zero(a)&&zero(b)) c=0;
	else if((a[0]^b[0])&32768) c=(a[0]&32768)?-1:1;
	else if(a[0]&32768) c = -c;
	switch(op) {
	case 0: return c==0;
	case 1: return c!=0;
	case 2: return c<0;
	case 3: return c<=0;
	case 4: return c>0;
	case 5: return c>=0;
	}
	return 0;
}

ftodcore(out,a)
unsigned short *out,*a;
{
	unsigned short m[4];
	int e,i;
	unsigned sign;
	sign=a[0]&32768; e=(a[0]>>7)&255;
	if(e && e!=255) {
		out[0]=sign|((e+896)<<4)|((a[0]>>3)&15);
		out[1]=((a[0]&7)<<13)|(a[1]>>3); out[2]=a[1]<<13; out[3]=0; return;
	}
	if(e==255) { out[0]=sign|0x7ff0|(((a[0]&127)||a[1])?8:0); out[1]=out[2]=out[3]=0; return; }
	m[0]=m[1]=0; m[2]=a[0]&127; m[3]=a[1];
	if(e) { m[2]|=128; e-=127; }
	else {
		e = -126;
		if(m[2]||m[3]) while(!(m[2]&128)) { shl1(m); e--; }
	}
	for(i=0;i<32;i++) shl1(m);
	pack(out,m,e,sign);
}

dtofcore(out,a)
unsigned short *out,*a;
{
	unsigned short m[4];
	unsigned carry,sign;
	unsigned long t;
	int e,i;
	sign=a[0]&32768;
	if((a[0]&0x7ff0)==0x7ff0) { out[0]=sign|0x7f80|(nan(a)?64:0); out[1]=0; return; }
	e=unpack(m,a); shrjam(m,26);
	if(e < -126) { shrjam(m,-126-e); e = -126; }
	if((m[3]&7)>4 || ((m[3]&7)==4 && (m[3]&8))) {
		carry=8;
		for(i=3;i>=0;i--) { t=(unsigned long)m[i]+carry; m[i]=t; carry=t>>16; }
	}
	if(m[2]&2048) { shrjam(m,1); e++; }
	if(e>127) { out[0]=sign|0x7f80; out[1]=0; return; }
	m[3]=(m[3]>>3)|(m[2]<<13); m[2]>>=3;
	out[0]=sign|((m[2]&128)?((e+127)<<7):0)|(m[2]&127); out[1]=m[3];
}

itodcore(out,a,width,uns)
unsigned short *out,*a;
int width,uns;
{
	unsigned short m[4];
	unsigned long v;
	unsigned sign;
	int e,i;
	sign=uns?0:(a[0]&32768);
	if(width==1) { v=a[0]; if(sign) v=0x10000L-v; }
	else { v=((unsigned long)a[0]<<16)|a[1]; if(sign) v=(~v+1)&0xffffffffL; }
	m[0]=m[1]=0; m[2]=v>>16; m[3]=v;
	e=52;
	if(v) while(!(m[0]&16)) { shl1(m); e--; }
	else e=0;
	for(i=0;i<3;i++) shl1(m);
	pack(out,m,e,sign);
}

dtoicore(out,a,width,uns)
unsigned short *out,*a;
int width,uns;
{
	unsigned short m[4];
	unsigned long v;
	int e,i;
	e=unpack(m,a);
	/* Out-of-range C conversions are undefined; return a deterministic zero. */
	if(e<0 || e>=width*16) v=0;
	else {
		for(i=0;i<52-e;i++) {
			m[3]=(m[3]>>1)|(m[2]<<15); m[2]=(m[2]>>1)|(m[1]<<15);
			m[1]=(m[1]>>1)|(m[0]<<15); m[0]>>=1;
		}
		v=((unsigned long)m[2]<<16)|m[3];
		if(a[0]&32768) v=(~v+1)&0xffffffffL;
	}
	if(width==1) out[0]=v;
	else { out[0]=v>>16; out[1]=v; }
}

dfopcore(out,a,b,op,single)
unsigned short *out,*a,*b;
int op,single;
{
	unsigned short x[4],y[4],z[4];
	if(single) { ftodcore(x,a); ftodcore(y,b); }
	else { copy4(x,a); copy4(y,b); }
	switch(op) {
	case 0: daddcore(z,x,y); break;
	case 1: dsubcore(z,x,y); break;
	case 2: dmulcore(z,x,y); break;
	case 3: ddivcore(z,x,y); break;
	}
	if(single) dtofcore(out,z); else copy4(out,z);
}

dafcore(out,a,b,op,single)
unsigned short *out,*a,*b;
int op,single;
{
	unsigned short x[4],z[4];
	if(single) { ftodcore(x,a); dfopcore(z,x,b,op,0); dtofcore(out,z); }
	else dfopcore(out,a,b,op,0);
	if(single) { a[0]=out[0]; a[1]=out[1]; }
	else copy4(a,out);
}

postcore(out,a,delta,single)
unsigned short *out,*a;
int delta,single;
{
	unsigned short x[4],one[4],z[4];
	if(single) { out[0]=a[0]; out[1]=a[1]; ftodcore(x,a); }
	else { copy4(out,a); copy4(x,a); }
	one[0]=delta<0?0xbff0:0x3ff0; one[1]=one[2]=one[3]=0;
	daddcore(z,x,one);
	if(single) dtofcore(a,z); else copy4(a,z);
}
