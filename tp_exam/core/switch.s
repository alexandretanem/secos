.text

.global switch_task
.type switch_task, "function"

.global switch_first
.type switch_first, "function"

switch_task:
	push 	%ebp
	# Save esp
	mov 	%esp, (%edx)
	# Change cr3 to new pgd @
	mov	%ecx, %cr3
	# Change stack
	mov 	%eax, %esp
	
	# Now in new task
	pop	%ebp
	ret

switch_first:
	# Change cr3 to new pgd @
	mov	%edx, %cr3
	# Change stack
	mov 	%eax, %esp
	# Now in new task
	pop	%ebp	
	ret
