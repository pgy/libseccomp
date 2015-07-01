/**
 * Seccomp Library test program
 *
 * This test triggers a bug in libseccomp erroneously allowing the close()
 * syscall on x32 instead of 'KILL'ing it, as it should do for unsupported
 * architectures.
 *
 * Copyright (c) 2012 Red Hat <pmoore@redhat.com>
 * Authors: Paul Moore <pmoore@redhat.com>
 *          Mathias Krause <minipli@googlemail.com>
 */

/*
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of version 2.1 of the GNU Lesser General Public License as
 * published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses>.
 */

#include <errno.h>
#include <unistd.h>

#include <seccomp.h>

#include "util.h"

int main(int argc, char *argv[])
{
	int rc;
	struct util_options opts;
	scmp_filter_ctx ctx = NULL;

	rc = util_getopt(argc, argv, &opts);
	if (rc < 0)
		goto out;

	ctx = seccomp_init(SCMP_ACT_ALLOW);
	if (ctx == NULL)
		return ENOMEM;

	rc = seccomp_arch_remove(ctx, SCMP_ARCH_NATIVE);
	if (rc != 0)
		goto out;

	/* add x86-64 and x86 (in that order!) but explicitly leave out x32 */
	rc = seccomp_arch_add(ctx, SCMP_ARCH_X86_64);
	if (rc != 0)
		goto out;
	rc = seccomp_arch_add(ctx, SCMP_ARCH_X86);
	if (rc != 0)
		goto out;

	rc = seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1), SCMP_SYS(close), 0);
	if (rc != 0)
		goto out;

	rc = util_filter_output(&opts, ctx);
	if (rc)
		goto out;

out:
	seccomp_release(ctx);
	return (rc < 0 ? -rc : rc);
}
