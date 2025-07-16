#include <linux/linkage.h>
#include "lame_data_asm.h"

.code64
.section .text

    .align 16
SYM_CODE_START(__vdso_lame_entry)
    

    /* Return from interrupt */
    iretq
SYM_CODE_END(__vdso_lame_entry) 