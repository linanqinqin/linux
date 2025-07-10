/* linanqinqin */

// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2025 Nanqinqin Li <linanqinqin@princeton.edu>. All Rights Reserved.
 */
#include <vdso/lame.h>
#include <asm/vdso/lame_data.h>

/* Thread-local storage for LAME data */
__thread struct lame_thread_data lame_tls_data __attribute__((visibility("hidden")));

__attribute__((naked)) void __vdso_lame_entry(void) {
	asm volatile(
		"incq %r13\n"  /* Increment r13 register */
		"iretq\n"
	);
}

int __vdso_lame_add(int x, int y) {
	int last_commit = 0x95a9354; // the SHA for last commit so that userspace knows which version is invoked 
	return x+y+last_commit;
}

int lame_add(int, int)
	__attribute__((weak, alias("__vdso_lame_add")));

/* end */