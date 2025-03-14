#ifndef ARCH_X86_KVM_SRE_H
#define ARCH_X86_KVM_SRE_H

#include <linux/types.h>
#include <linux/hashtable.h>
#include <linux/spinlock.h>
#include <linux/kvm_host.h>  // For gpa_t

#define SRE_HASH_BITS 16

// Per-GPA metadata structure
struct sre_flags {
    gpa_t gpa;              // Guest Physical Address (key)
    bool is_sre;            // True if page is "slow" (flash-backed)
    bool is_ept;            // True if regular EPT violation
    // atomic_t access_count;  // For promotion/demotion policies
    struct hlist_node node; // Hash table linkage
};


// exposed APIs
void sre_flags_init(void);  
void sre_flags_cleanup(void); 
struct sre_flags *sre_flags_lookup(gpa_t gpa); 

#endif /* ARCH_X86_KVM_SRE_H */
