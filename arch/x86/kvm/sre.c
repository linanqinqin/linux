/*
 * Slow Request Exception emulation module
 *
 *
 * Author:
 *   linanqinqin   <>
 */

#include <linux/slab.h>  // For kmalloc/kfree

#include "sre.h"


// Define hash table and spinlock
DEFINE_HASHTABLE(sre_metadata_hash, SRE_HASH_BITS);
DEFINE_SPINLOCK(sre_flags_spinlock);

// initialize the sre_flags hashmap
void sre_flags_init(void) {  
    hash_init(sre_metadata_hash);  
}  

// cleanup the entire sre_flags hashmap
void sre_flags_cleanup(void) {  
    struct sre_flags *entry;  
    struct hlist_node *tmp;  
    unsigned long bkt;  

    hash_for_each_safe(sre_metadata_hash, bkt, tmp, entry, node) {  
        hash_del(&entry->node);  
        kfree(entry);  
    }  
}  

// Add a GPA to the hash table
static struct sre_flags *sre_flags_new(gpa_t gpa) {
    struct sre_flags *entry = kmalloc(sizeof(*entry), GFP_KERNEL);
    if (!entry) return NULL;

    entry->gpa = gpa;
    entry->is_sre = false;          // is_sre should be initialized to false; the flash emulation layer sets it later 
    entry->is_ept = true;           // is_ept should be initialized to true, since the first occurence is always a regular EPT violation 
    // atomic_set(&entry->access_count, 0);    // probably does not need atomic here at init

    spin_lock(&sre_flags_spinlock);
    hash_add(sre_metadata_hash, &entry->node, gpa);
    spin_unlock(&sre_flags_spinlock);

    return entry;
}

// Remove a GPA from the hash table
// static void sre_flags_remove(gpa_t gpa) {
//     struct sre_flags *entry;
//     hash_for_each_possible(sre_metadata_hash, entry, node, gpa) {
//         if (entry->gpa == gpa) {
//             spin_lock(&sre_flags_spinlock);
//             hash_del(&entry->node);
//             spin_unlock(&sre_flags_spinlock);
//             kfree(entry);
//             break;
//         }
//     }
// }

// Lookup sre_flags for a GPA
struct sre_flags *sre_flags_lookup(gpa_t gpa) {  
    struct sre_flags *entry;  

    // Check if entry exists  
    hash_for_each_possible(sre_metadata_hash, entry, node, gpa) {  
        if (entry->gpa == gpa) {  
            return entry;  
        }  
    }  

    // Allocate new entry if not found  
    entry = sre_flags_new(gpa);
    
    return entry;  
}  
