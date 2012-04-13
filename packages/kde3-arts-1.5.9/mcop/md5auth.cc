    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
   
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#include <config.h>

#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#elif HAVE_SYS_TIME_H
# include <sys/time.h>
#else
# include <time.h>
#endif

#include "md5.h"
#include "md5auth.h"
#include "debug.h"

// MD5_BINARY_LEN is defined in md5.h
#define MD5_COOKIE_LEN (MD5_BINARY_LEN*2)
#define MD5_MANGLE_LEN (MD5_COOKIE_LEN*2)



struct random_info {
	struct timeval tv;
	int pid;
	struct utsname un;
	char dev_urandom[MD5_BINARY_LEN];
	char seed[MD5_COOKIE_LEN+1];
	int number;
};

static char md5_seed[MD5_COOKIE_LEN+1];
static char md5_cookie[MD5_COOKIE_LEN+1];
static int md5_init = 0;
static int md5_random_cookie_number = 0;

static char *md5_to_ascii_overwrite(char *md5)
{
	char outascii[MD5_COOKIE_LEN+1];
	char *result;
	int i;

	// convert to readable text
	for(i = 0; i < MD5_BINARY_LEN; i++) {
		sprintf(&outascii[i*2],"%02x",md5[i] & 0xff);
	}
	outascii[MD5_COOKIE_LEN] = 0;
	result = strdup(outascii);

	// try to zero out traces
	memset(md5,0,MD5_BINARY_LEN);
	memset(outascii,0,MD5_COOKIE_LEN);

	// well, here it is
	return result;
}

char *arts_md5_auth_mangle(const char *cookie)
{
	char mangle[MD5_MANGLE_LEN+1];
	char out[MD5_BINARY_LEN];
	assert(md5_init);
        
	// concat the two cookies and mangle them to a new one
	strncpy(mangle,md5_cookie, MD5_COOKIE_LEN);
	mangle[MD5_COOKIE_LEN] = 0;
	strncat(mangle,cookie, MD5_COOKIE_LEN);
	mangle[MD5_MANGLE_LEN] = 0;
	arts_md5sum((unsigned char *)mangle,MD5_MANGLE_LEN,out);
        
	// try to zero out traces
	memset(mangle,0,MD5_MANGLE_LEN);

	return md5_to_ascii_overwrite(out);
}

char *arts_md5_auth_mkcookie()
{
	struct random_info r;
	char out[MD5_BINARY_LEN];
	int rndfd;

	memset(&r,0,sizeof(struct random_info));

	// collect some "random" system information
	gettimeofday(&r.tv,0);
	r.pid = getpid();
	uname(&r.un);

	// linux random device ; if that works, we have good random anyway, the
	// above are just helpers for the case that it doesn't
	rndfd = open("/dev/urandom",O_RDONLY);
	if(rndfd != -1)
	{
		read(rndfd,r.dev_urandom,MD5_BINARY_LEN);
		close(rndfd);
	}

	// ensure that two cookies that are  requested very shortly after each
	// other (so that it looks like "at the same time") won't be the same
	r.number = ++md5_random_cookie_number;

	// this is some seed from a file which is updated sometimes with a
	// new "arts_md5_auth_mkcookie()" after initialization
	strncpy(r.seed,md5_seed,MD5_COOKIE_LEN);

	// build hash value of all information
	arts_md5sum((unsigned char *)&r,sizeof(struct random_info),out);

	// zero out traces and return
	memset(&r,0,sizeof(struct random_info));
	return md5_to_ascii_overwrite(out);
}

const char *arts_md5_auth_cookie()
{
	assert(md5_init);
	return md5_cookie;
}

static int md5_load_cookie(const char *filename, char *cookie)
{
	int fd = open(filename,O_RDONLY);
	int i;

	if(fd != -1) {
		struct stat st;
		for(i=0;i<5;i++) {
			fstat(fd,&st);
			if(st.st_size == MD5_COOKIE_LEN) {
				lseek(fd, 0, SEEK_SET); 
				if(read(fd,cookie,MD5_COOKIE_LEN) == MD5_COOKIE_LEN)
				{
					cookie[MD5_COOKIE_LEN] = 0;
					close(fd);
					return 1;
				}
			}
			arts_warning("MCOP: "
					"authority file has wrong size (just being written?)");
			sleep(1);
		}
	}
	return 0;
}

void arts_md5_auth_init_seed(const char *seedname)
{
	// don't care if it works - no harm is being done if it doesn't
	md5_load_cookie(seedname,md5_seed);

	/*
	 * maxage ensures that not everybody will try to update the seed
	 * at the same time, while it will take at most 5 hours between
	 * updates (if there are any initialization calls)
	 */
	struct stat st;
	int maxage = 300 + (getpid() & 0xfff)*4;
	int lstat_result = lstat(seedname,&st);
	if(lstat_result != 0 || (time(0) - st.st_mtime) > maxage)
	{
		int fd = open(seedname,O_TRUNC|O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR);
		if(fd != -1) {
			char *cookie = arts_md5_auth_mkcookie();
			write(fd,cookie,strlen(cookie));
			memset(cookie,0,strlen(cookie));
			free(cookie);
			close(fd);
		}
	}
}

bool arts_md5_auth_set_cookie(const char *cookie)
{
	if(strlen(cookie) != MD5_COOKIE_LEN)
		return false;

	strncpy(md5_cookie,cookie,MD5_COOKIE_LEN);
	md5_cookie[MD5_COOKIE_LEN] = 0;
	md5_init = 1;
	return true;
}
