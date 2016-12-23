/* checksum.asm */
global checksum_ip
global checksum_tcp
section .text
checksum_ip :
	push rbp
	mov rbp,rsp
	push rcx
	push rbx
	push rdx

	mov rax,0
	mov rbx,0
	mov rdx,0
	mov rcx,10
	for :
		mov dh,[rdi+rbx]
		mov dl,[rdi+rbx+1]
		add eax,edx
		add rbx,2
	loop for

	andadd:
	mov edx,0
	mov edx,eax
	shr edx,16
	cmp dx,0
	je end 

	mov rcx,0
	mov cx,ax
	add edx,ecx	
	mov eax,edx
	jmp andadd

	end :
	mov ebx,0xffff;
	sub ebx,eax
	mov eax,ebx

	pop rdx
	pop rbx
	pop rcx
	pop rbp
	ret	

checksum_tcp :
	push rbp
	mov rbp,rsp
	push rbx
	push rcx
	push rdx

	mov rax,0
	mov rdx,0
	mov rbx,12
	mov rcx,14
	for_2 :
		mov dh,[rdi+rbx]
		mov dl,[rdi+rbx+1]
		add eax,edx
		add rbx,2
	loop for_2

	mov rdx,0
	mov dh,0
	mov dl,[rdi+9]
	add eax,edx

	mov rdx,0
	mov dh,[rdi+2]
	mov dl,[rdi+3]
	sub dx,20
	add eax,edx

	andadd_2 :
	mov edx,0
	mov edx,eax
	shr edx,16
	cmp edx,0
	je end_2

	mov ecx,0
	mov cx,ax
	add ecx,edx
	mov eax,ecx
	jmp andadd_2 

	end_2 :
	mov ecx,0xffff
	mov edx,eax
	sub ecx,edx
	mov eax,ecx

	pop rdx
	pop rcx
	pop rbx
	pop rbp
	ret 
