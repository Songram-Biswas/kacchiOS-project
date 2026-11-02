.global context_switch

/* 
 * void context_switch(uint32_t* old_esp, uint32_t new_esp);
 * Stack Frame:
 *   [esp + 8] = new_esp
 *   [esp + 4] = old_esp pointer
 *   [esp]     = return address
 */
context_switch:
    /* ১. রানিং প্রসেসের রেজিস্টারগুলো সেভ করা */
    pushal

    /* ২. পুরানো প্রসেসের stack pointer (ESP) সেভ করা */
    movl 36(%esp), %eax       /* eax = old_esp pointer */
    testl %eax, %eax
    jz .load_new
    movl %esp, (%eax)         /* *old_esp = %esp */

.load_new:
    /* ৩. নতুন প্রসেসের stack pointer (ESP) লোড করা */
    movl 40(%esp), %esp       /* %esp = new_esp */

    /* ৪. নতুন প্রসেসের রেজিস্টার রিস্টোর করে জাম্প করা */
    popal
    ret