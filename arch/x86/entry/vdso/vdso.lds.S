/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Linker script for 64-bit vDSO.
 * We #include the file to define the layout details.
 *
 * This file defines the version script giving the user-exported symbols in
 * the DSO.
 */

#define BUILD_VDSO64

#include "vdso-layout.lds.S"

/*
 * This controls what userland symbols we export from the vDSO.
 */
VERSION {
	LINUX_2.6 {
	global:
		/* linanqinqin */
		lame_handle_array;
		__vdso_lame_entry;
		lame_add;
		__vdso_lame_add;
		/* end */
		clock_gettime;
		__vdso_clock_gettime;
		gettimeofday;
		__vdso_gettimeofday;
		getcpu;
		__vdso_getcpu;
		time;
		__vdso_time;
		clock_getres;
		__vdso_clock_getres;
#ifdef CONFIG_X86_SGX
		__vdso_sgx_enter_enclave;
#endif
		getrandom;
		__vdso_getrandom;
	local: *;
	};
}
