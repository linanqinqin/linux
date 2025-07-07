/* linanqinqin */

// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2025 Nanqinqin Li <linanqinqin@princeton.edu>. All Rights Reserved.
 */
#include <vdso/lame.h>

void __vdso_lame_entry(void) {
	return;
}

int __vdso_lame_add(int x, int y) {
	int last_commit = 0xfc4f0b9; // the SHA for last commit so that userspace knows which version is invoked 
	return x+y+last_commit;
}

int lame_add(int, int)
	__attribute__((weak, alias("__vdso_lame_add")));

/* end */