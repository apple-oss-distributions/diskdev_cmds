/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * "Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License."
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*-
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/stat.h>
#include <sys/vnode.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/unpcb.h>
#include <sys/sysctl.h>
#include <sys/filedesc.h>
#define	KERNEL
#include <sys/file.h>
#include <hfs/hfs_cnode.h>
#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
//#undef KERNEL
//#define NFS
#include <sys/mount.h>
#undef KERNEL
#include <nfs/nfsproto.h>
#include <nfs/rpcv2.h>
#define	KERNEL
#include <nfs/nfs.h>
#undef KERNEL
#include <nfs/nfsnode.h>
#undef NFS
#define time t1         /* hack, hack, hack */
#include <miscfs/devfs/devfsdefs.h>
#undef  time

#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>

#include <ctype.h>
#include <errno.h>
#include <kvm.h>
#include <limits.h>
#include <nlist.h>
#include <paths.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define	TEXT	-1
#define	CDIR	-2
#define	RDIR	-3
#define	TRACE	-4

typedef struct devs {
	struct	devs *next;
	long	fsid;
	ino_t	ino;
	char	*name;
} DEVS;
DEVS *devs;

struct  filestat {
	long	fsid;
	long	fileid;
	mode_t	mode;
	u_long	size;
	dev_t	rdev;
};

#ifdef notdef
struct nlist nl[] = {
	{ "" },
};
#endif

int 	fsflg,	/* show files on same filesystem as file(s) argument */
	pflg,	/* show files open by a particular pid */
	uflg;	/* show files open by a particular (effective) user */
int 	checkfile; /* true if restricting to particular files or filesystems */
int	nflg;	/* (numerical) display f.s. and rdev as dev_t */
int	vflg;	/* display errors in locating kernel data objects etc... */

#define dprintf	if (vflg) fprintf

struct file **ofiles;	/* buffer of pointers to file structures */
int maxfiles;
#define ALLOC_OFILES(d)	\
	if ((d) > maxfiles) { \
		free(ofiles); \
		ofiles = malloc((d) * sizeof(struct file *)); \
		if (ofiles == NULL) { \
			fprintf(stderr, "fstat: %s\n", strerror(errno)); \
			exit(1); \
		} \
		maxfiles = (d); \
	}

/*
 * a kvm_read that returns true if everything is read 
 */
#define KVM_READ(kaddr, paddr, len) \
	(kvm_read(kd, (u_long)(kaddr), (char *)(paddr), (len)) == (len))

kvm_t *kd;

int ufs_filestat(), nfs_filestat(), hfs_filestat(), devfs_filestat();
void dofiles(), getinetproto(), socktrans();
int getfname();
void usage(), vtrans();

int
main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	register struct passwd *passwd;
	struct kinfo_proc *p, *plast;
	int arg, ch, what;
	char *memf, *nlistf;
	char buf[_POSIX2_LINE_MAX];
	int cnt;

	arg = 0;
	what = KERN_PROC_ALL;
	nlistf = memf = NULL;
	while ((ch = getopt(argc, argv, "fnp:u:vN:M:")) != EOF)
		switch((char)ch) {
		case 'f':
			fsflg = 1;
			break;
		case 'M':
			memf = optarg;
			break;
		case 'N':
			nlistf = optarg;
			break;
		case 'n':
			nflg = 1;
			break;
		case 'p':
			if (pflg++)
				usage();
			if (!isdigit(*optarg)) {
				fprintf(stderr,
				    "fstat: -p requires a process id\n");
				usage();
			}
			what = KERN_PROC_PID;
			arg = atoi(optarg);
			break;
		case 'u':
			if (uflg++)
				usage();
			if (!(passwd = getpwnam(optarg))) {
				fprintf(stderr, "%s: unknown uid\n",
				    optarg);
				exit(1);
			}
			what = KERN_PROC_UID;
			arg = passwd->pw_uid;
			break;
		case 'v':
			vflg = 1;
			break;
		case '?':
		default:
			usage();
		}

	if (*(argv += optind)) {
		for (; *argv; ++argv) {
			if (getfname(*argv))
				checkfile = 1;
		}
		if (!checkfile)	/* file(s) specified, but none accessable */
			exit(1);
	}

	ALLOC_OFILES(256);	/* reserve space for file pointers */

	if (fsflg && !checkfile) {	
		/* -f with no files means use wd */
		if (getfname(".") == 0)
			exit(1);
		checkfile = 1;
	}

	/*
	 * Discard setgid privileges if not the running kernel so that bad
	 * guys can't print interesting stuff from kernel memory.
	 */
	if (nlistf != NULL || memf != NULL)
		setgid(getgid());

	if ((kd = kvm_openfiles(nlistf, memf, NULL, O_RDONLY, buf)) == NULL) {
		fprintf(stderr, "fstat: %s\n", buf);
		exit(1);
	}
#ifdef notdef
	if (kvm_nlist(kd, nl) != 0) {
		fprintf(stderr, "fstat: no namelist: %s\n", kvm_geterr(kd));
		exit(1);
	}
#endif
	if ((p = kvm_getprocs(kd, what, arg, &cnt)) == NULL) {
		fprintf(stderr, "fstat: %s\n", kvm_geterr(kd));
		exit(1);
	}
	if (nflg)
		printf("%s",
"USER     CMD          PID   FD INUM            MODE   SZ|DV R/W DEV");
	else
		printf("%s",
"USER     CMD          PID   FD INUM     MODE          SZ|DV R/W MOUNT");
	if (checkfile && fsflg == 0)
		printf(" NAME\n");
	else
		putchar('\n');

	for (plast = &p[cnt]; p < plast; ++p) {
		if (p->kp_proc.p_stat == SZOMB)
			continue;
		dofiles(p);
	}
	exit(0);
}

char	*Uname, *Comm;
int	Pid;

#define PREFIX(i) printf("%-8.8s %-10.10s %5d", Uname, Comm, Pid); \
	switch(i) { \
	case TEXT: \
		printf(" text"); \
		break; \
	case CDIR: \
		printf("   wd"); \
		break; \
	case RDIR: \
		printf(" root"); \
		break; \
	case TRACE: \
		printf("   tr"); \
		break; \
	default: \
		printf(" %4d", i); \
		break; \
	}

/*
 * print open files attributed to this process
 */
void
dofiles(kp)
	struct kinfo_proc *kp;
{
	int i, last;
	struct file file;
	struct filedesc filed;
	struct proc p;
	struct extern_proc *xp = &kp->kp_proc;
	struct eproc *ep = &kp->kp_eproc;

	extern char *user_from_uid();

	Uname = user_from_uid(ep->e_ucred.cr_uid, 0);
	Pid = xp->p_pid;
	Comm = xp->p_comm;

	if (!KVM_READ(ep->e_paddr, &p, sizeof (p))) {
		dprintf(stderr, "can't read proc at %x for pid %d\n",
			ep->e_paddr, Pid);
		return;
	}

	if (p.p_fd == NULL)
		return;
	if (!KVM_READ(p.p_fd, &filed, sizeof (filed))) {
		dprintf(stderr, "can't read filedesc at %x for pid %d\n",
			p.p_fd, Pid);
		return;
	}
	/*
	 * root directory vnode, if one
	 */
	if (filed.fd_rdir)
		vtrans(filed.fd_rdir, RDIR, FREAD);
	/*
	 * current working directory vnode
	 */
	vtrans(filed.fd_cdir, CDIR, FREAD);
	/*
	 * ktrace vnode, if one
	 */
	if (xp->p_tracep)
		vtrans(xp->p_tracep, TRACE, FREAD|FWRITE);
	/*
	 * open files
	 */
#define FPSIZE	(sizeof (struct file *))
	ALLOC_OFILES(filed.fd_lastfile+1);
	if (!KVM_READ(filed.fd_ofiles, ofiles,
	    (filed.fd_lastfile+1) * FPSIZE)) {
		dprintf(stderr,
		    "can't read file structures at %x for pid %d\n",
		    filed.fd_ofiles, Pid);
		return;
	}

	for (i = 0; i <= filed.fd_lastfile; i++) {
		if (ofiles[i] == NULL)
			continue;
		if (!KVM_READ(ofiles[i], &file, sizeof (struct file))) {
			dprintf(stderr, "can't read file %d at %x for pid %d\n",
				i, ofiles[i], Pid);
			continue;
		}
		if (file.f_type == DTYPE_VNODE)
			vtrans((struct vnode *)file.f_data, i, file.f_flag);
		else if (file.f_type == DTYPE_SOCKET) {
			if (checkfile == 0)
				socktrans((struct socket *)file.f_data, i);
		}
		else {
			dprintf(stderr, 
				"unknown file type %d for file %d of pid %d\n",
				file.f_type, i, Pid);
		}
	}
}

void
vtrans(vp, i, flag)
	struct vnode *vp;
	int i;
	int flag;
{
	struct vnode vn;
	struct filestat fst;
	char rw[3], mode[15];
	char *badtype = NULL, *filename, *getmnton(), *getmntfstype();

	filename = badtype = NULL;
	if (!KVM_READ(vp, &vn, sizeof (struct vnode))) {
		dprintf(stderr, "can't read vnode at %x for pid %d\n",
			vp, Pid);
		return;
	}
	if (vn.v_type == VNON || vn.v_tag == VT_NON)
		badtype = "none";
	else if (vn.v_type == VBAD)
		badtype = "bad";
	else
		switch (vn.v_tag) {
		case VT_UFS:
			if (!ufs_filestat(&vn, &fst))
				badtype = "error";
			break;
		case VT_NFS:
			if (!nfs_filestat(&vn, &fst))
				badtype = "error";
			break;
		case VT_HFS:
			if (strcmp(getmntfstype(vn.v_mount), "afpfs") == 0)
				badtype = "afpfs";
			else if (!hfs_filestat(&vn, &fst))
				badtype = "error";
			break;
		case VT_DEVFS:
			if (!devfs_filestat(&vn, &fst))
				badtype = "error";
			break;
		default: {
			static char unknown[10];
			sprintf(badtype = unknown, "?(%x)", vn.v_tag);
			break;;
		}
	}
	if (checkfile) {
		int fsmatch = 0;
		register DEVS *d;

		if (badtype)
			return;
		for (d = devs; d != NULL; d = d->next)
			if (d->fsid == fst.fsid) {
				fsmatch = 1;
				if (d->ino == fst.fileid) {
					filename = d->name;
					break;
				}
			}
		if (fsmatch == 0 || (filename == NULL && fsflg == 0))
			return;
	}
	PREFIX(i);
	if (badtype) {
		(void)printf(" -         -  %10s    -\n", badtype);
		return;
	}
	if (nflg)
		(void)sprintf(mode, "%o", fst.mode);
	else
		strmode(fst.mode, mode);
	(void)printf(" %8d %11s", fst.fileid, mode);
	switch (vn.v_type) {
	case VBLK:
	case VCHR: {
		char *name;

		if (nflg || ((name = devname(fst.rdev, vn.v_type == VCHR ? 
		    S_IFCHR : S_IFBLK)) == NULL))
			printf("   %2d,%-2.2d", major(fst.rdev), minor(fst.rdev));
		else
			printf(" %7.7s", name);
		break;
	}
	default:
		printf(" %7d", fst.size);
	}
	rw[0] = '\0';
	if (flag & FREAD)
		strcat(rw, "r");
	if (flag & FWRITE)
		strcat(rw, "w");
	printf(" %-2.2s ", rw);
	if (nflg)
		(void)printf(" %2d,%-2d", major(fst.fsid), minor(fst.fsid));
	else
		(void)printf(" %s", getmnton(vn.v_mount));
	if (filename && !fsflg)
		printf(" %s", filename);
	putchar('\n');
}

int
ufs_filestat(vp, fsp)
	struct vnode *vp;
	struct filestat *fsp;
{
	struct inode inode;

	if (!KVM_READ(VTOI(vp), &inode, sizeof (inode))) {
		dprintf(stderr, "can't read inode at %x for pid %d\n",
			VTOI(vp), Pid);
		return 0;
	}
	fsp->fsid = inode.i_dev & 0xffff;
	fsp->fileid = (long)inode.i_number;
	fsp->mode = (mode_t)inode.i_mode;
	fsp->size = (u_long)inode.i_size;
	fsp->rdev = inode.i_rdev;

	return 1;
}

int
nfs_filestat(vp, fsp)
	struct vnode *vp;
	struct filestat *fsp;
{
	struct nfsnode nfsnode;
	register mode_t mode;

	if (!KVM_READ(VTONFS(vp), &nfsnode, sizeof (nfsnode))) {
		dprintf(stderr, "can't read nfsnode at %x for pid %d\n",
			VTONFS(vp), Pid);
		return 0;
	}
	fsp->fsid = nfsnode.n_vattr.va_fsid & 0xffff;
	fsp->fileid = nfsnode.n_vattr.va_fileid;
	fsp->size = nfsnode.n_size;
	fsp->rdev = nfsnode.n_vattr.va_rdev;
	mode = (mode_t)nfsnode.n_vattr.va_mode;
	switch (vp->v_type) {
	case VREG:
		mode |= S_IFREG;
		break;
	case VDIR:
		mode |= S_IFDIR;
		break;
	case VBLK:
		mode |= S_IFBLK;
		break;
	case VCHR:
		mode |= S_IFCHR;
		break;
	case VLNK:
		mode |= S_IFLNK;
		break;
	case VSOCK:
		mode |= S_IFSOCK;
		break;
	case VFIFO:
		mode |= S_IFIFO;
		break;
	default:
		break;
	};
	fsp->mode = mode;

	return 1;
}

int
hfs_filestat(vp, fsp)
	struct vnode *vp;
	struct filestat *fsp;
{
	struct cnode cnode;
	struct filefork fork = {0};
	struct filefork *activefork;

	if (!KVM_READ(vp->v_data, &cnode, sizeof (cnode))) {
		dprintf(stderr, "can't read cnode at %x for pid %d\n",
			vp->v_data, Pid);
		return 0;
	}

	if (vp->v_type == VDIR)
		activefork = NULL;
	else if (cnode.c_rsrc_vp == vp)
		activefork = cnode.c_rsrcfork;
	else
		activefork = cnode.c_datafork;

	if (activefork && !KVM_READ(activefork, &fork, sizeof (fork))) {
		dprintf(stderr, "can't read file fork at %x for pid %d\n",
			activefork, Pid);
		return 0;
	}

	fsp->fsid = cnode.c_dev & 0xffff;
	fsp->fileid = (long)cnode.c_fileid;
	fsp->mode = (mode_t)cnode.c_mode;
	fsp->size = (vp->v_type == VDIR)
			? (u_long)cnode.c_nlink * 128
			: (u_long)fork.ff_size;
	if (vp->v_type == VBLK || vp->v_type == VCHR)
		fsp->rdev = cnode.c_rdev;
	else
		fsp->rdev = 0;
	return 1;
}

int
devfs_filestat(vp, fsp)
	struct vnode *vp;
	struct filestat *fsp;
{
	struct devnode devnode;

	if (!KVM_READ(VTODN(vp), &devnode, sizeof (devnode))) {
		dprintf(stderr, "can't read devnode at %x for pid %d\n",
			VTODN(vp), Pid);
		return 0;
	}

	fsp->fsid = 0;
	fsp->fileid = (long)vp->v_data;
	fsp->mode = (mode_t)devnode.dn_mode;
	fsp->size = (u_long)devnode.dn_len;
	fsp->rdev = devnode.dn_typeinfo.dev;

	return 1;
}

struct mtab {
	struct mtab *next;
	struct mount *m;
	char mntonname[MNAMELEN];
	char fstypename[MFSNAMELEN];
} *mhead = NULL;

struct mtab *
getmntinfo(m)
	struct mount *m;
{
	struct mount mount;
	register struct mtab *mt;

	for (mt = mhead; mt != NULL; mt = mt->next)
		if (m == mt->m)
			return (mt);
	if (!KVM_READ(m, &mount, sizeof(struct mount))) {
		fprintf(stderr, "can't read mount table at %x\n", m);
		return (NULL);
	}
	if ((mt = malloc(sizeof (struct mtab))) == NULL) {
		fprintf(stderr, "fstat: %s\n", strerror(errno));
		exit(1);
	}
	mt->m = m;
	bcopy(&mount.mnt_stat.f_mntonname[0],  &mt->mntonname[0],  MNAMELEN);
	bcopy(&mount.mnt_stat.f_fstypename[0], &mt->fstypename[0], MFSNAMELEN);
	mt->next = mhead;
	mhead = mt;
	return (mt);
}

char *
getmnton(m)
	struct mount *m;
{
	register struct mtab *mt;

	mt = getmntinfo(m);
	return (mt == NULL) ? NULL : mt->mntonname;
}

char *
getmntfstype(m)
	struct mount *m;
{
	register struct mtab *mt;

	mt = getmntinfo(m);
	return (mt == NULL) ? NULL : mt->fstypename;
}

void
socktrans(sock, i)
	struct socket *sock;
	int i;
{
	static char *stypename[] = {
		"unused",	/* 0 */
		"stream", 	/* 1 */
		"dgram",	/* 2 */
		"raw",		/* 3 */
		"rdm",		/* 4 */
		"seqpak"	/* 5 */
	};
#define	STYPEMAX 5
	struct socket	so;
	struct protosw	proto;
	struct domain	dom;
	struct inpcb	inpcb;
	struct unpcb	unpcb;
	int len;
	char dname[32], *strcpy();

	PREFIX(i);

	/* fill in socket */
	if (!KVM_READ(sock, &so, sizeof(struct socket))) {
		dprintf(stderr, "can't read sock at %x\n", sock);
		goto bad;
	}

	/* fill in protosw entry */
	if (!KVM_READ(so.so_proto, &proto, sizeof(struct protosw))) {
		dprintf(stderr, "can't read protosw at %x", so.so_proto);
		goto bad;
	}

	/* fill in domain */
	if (!KVM_READ(proto.pr_domain, &dom, sizeof(struct domain))) {
		dprintf(stderr, "can't read domain at %x\n", proto.pr_domain);
		goto bad;
	}

	if ((len = kvm_read(kd, (u_long)dom.dom_name, dname,
	    sizeof(dname) - 1)) < 0) {
		dprintf(stderr, "can't read domain name at %x\n",
			dom.dom_name);
		dname[0] = '\0';
	}
	else
		dname[len] = '\0';

	if ((u_short)so.so_type > STYPEMAX)
		printf("* %s ?%d", dname, so.so_type);
	else
		printf("* %s %s", dname, stypename[so.so_type]);

	/* 
	 * protocol specific formatting
	 *
	 * Try to find interesting things to print.  For tcp, the interesting
	 * thing is the address of the tcpcb, for udp and others, just the
	 * inpcb (socket pcb).  For unix domain, its the address of the socket
	 * pcb and the address of the connected pcb (if connected).  Otherwise
	 * just print the protocol number and address of the socket itself.
	 * The idea is not to duplicate netstat, but to make available enough
	 * information for further analysis.
	 */
	switch(dom.dom_family) {
	case AF_INET:
		getinetproto(proto.pr_protocol);
		if (proto.pr_protocol == IPPROTO_TCP ) {
			if (so.so_pcb) {
				if (kvm_read(kd, (u_long)so.so_pcb,
				    (char *)&inpcb, sizeof(struct inpcb))
				    != sizeof(struct inpcb)) {
					dprintf(stderr, 
					    "can't read inpcb at %x\n",
					    so.so_pcb);
					goto bad;
				}
				printf(" %x", (int)inpcb.inp_ppcb);
			}
		}
		else if (so.so_pcb)
			printf(" %x", (int)so.so_pcb);
		break;
	case AF_UNIX:
		/* print address of pcb and connected pcb */
		if (so.so_pcb) {
			printf(" %x", (int)so.so_pcb);
			if (kvm_read(kd, (u_long)so.so_pcb, (char *)&unpcb,
			    sizeof(struct unpcb)) != sizeof(struct unpcb)){
				dprintf(stderr, "can't read unpcb at %x\n",
				    so.so_pcb);
				goto bad;
			}
			if (unpcb.unp_conn) {
				char shoconn[4], *cp;

				cp = shoconn;
				if (!(so.so_state & SS_CANTRCVMORE))
					*cp++ = '<';
				*cp++ = '-';
				if (!(so.so_state & SS_CANTSENDMORE))
					*cp++ = '>';
				*cp = '\0';
				printf(" %s %x", shoconn,
				    (int)unpcb.unp_conn);
			}
		}
		break;
	default:
		/* print protocol number and socket address */
		printf(" %d %x", proto.pr_protocol, (int)sock);
	}
	printf("\n");
	return;
bad:
	printf("* error\n");
}

/*
 * getinetproto --
 *	print name of protocol number
 */
void
getinetproto(number)
	int number;
{
	char *cp;

	switch(number) {
	case IPPROTO_IP:
		cp = "ip"; break;
	case IPPROTO_ICMP:
		cp ="icmp"; break;
	case IPPROTO_GGP:
		cp ="ggp"; break;
	case IPPROTO_TCP:
		cp ="tcp"; break;
	case IPPROTO_EGP:
		cp ="egp"; break;
	case IPPROTO_PUP:
		cp ="pup"; break;
	case IPPROTO_UDP:
		cp ="udp"; break;
	case IPPROTO_IDP:
		cp ="idp"; break;
	case IPPROTO_RAW:
		cp ="raw"; break;
	default:
		printf(" %d", number);
		return;
	}
	printf(" %s", cp);
}

int
getfname(filename)
	char *filename;
{
	struct stat statbuf;
	DEVS *cur;

	if (stat(filename, &statbuf)) {
		fprintf(stderr, "fstat: %s: %s\n", filename, strerror(errno));
		return(0);
	}
	if ((cur = malloc(sizeof(DEVS))) == NULL) {
		fprintf(stderr, "fstat: %s\n", strerror(errno));
		exit(1);
	}
	cur->next = devs;
	devs = cur;

	cur->ino = statbuf.st_ino;
	cur->fsid = statbuf.st_dev & 0xffff;
	cur->name = filename;
	return(1);
}

void
usage()
{
	(void)fprintf(stderr,
 "usage: fstat [-fnv] [-p pid] [-u user] [-N system] [-M core] [file ...]\n");
	exit(1);
}
