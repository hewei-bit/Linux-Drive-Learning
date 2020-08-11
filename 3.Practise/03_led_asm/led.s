.text
.global _start
_start:
	//配置复用功能
	ldr r0,=0xC001E020  
	ldr r1,[r0]
	ldr r2,=3<<26
	mvn r3,r2
	and r1,r1,r3
	str r1,[r0]

	ldr r0,=0xC001C020 
	ldr r1,[r0]
	ldr r2,=1<<14
	orr r1,r1,r2
	str r1,[r0]

	ldr r0,=0xC001C020 
	ldr r1,[r0]
	ldr r2,=1<<16
	orr r1,r1,r2
	str r1,[r0]

	ldr r0,=0xC001C024 
	ldr r1,[r0]
	ldr r2,=1<<2
	orr r1,r1,r2
	str r1,[r0]

	//允许输出电平
	ldr r0,=0xC001E004  
	ldr r1,[r0]
	ldr r2,=1<<13
	orr r1,r1,r2
	str r1,[r0]

	ldr r0,=0xC001C004  
	ldr r1,[r0]
	ldr r2,=1<<7
	orr r1,r1,r2
	str r1,[r0]

	ldr r0,=0xC001C004  
	ldr r1,[r0]
	ldr r2,=1<<8
	orr r1,r1,r2
	str r1,[r0]

	ldr r0,=0xC001C004  
	ldr r1,[r0]
	ldr r2,=1<<17
	orr r1,r1,r2
	str r1,[r0]

loop:
	ldr r0,=0xC001E000
	ldr r1,[r0]
	ldr r2,=1<<13
	mvn r3,r2
	and r1,r1,r3
	str r1,[r0]

	bl delay

	ldr r0,=0xC001E000
	ldr r1,[r0]
	ldr r2,=1<<13 
	orr r1,r1,r2
	str r1,[r0]

	ldr r0,=0xC001C000
	ldr r1,[r0]
	ldr r2,=1<<17
	mvn r3,r2
	and r1,r1,r3
	str r1,[r0]

	bl delay

	ldr r0,=0xC001C000
	ldr r1,[r0]
	ldr r2,=1<<17 
	orr r1,r1,r2
	str r1,[r0]

	ldr r0,=0xC001C000
	ldr r1,[r0]
	ldr r2,=1<<8
	mvn r3,r2
	and r1,r1,r3
	str r1,[r0]

	bl delay

	ldr r0,=0xC001C000
	ldr r1,[r0]
	ldr r2,=1<<8 
	orr r1,r1,r2
	str r1,[r0]


	ldr r0,=0xC001C000
	ldr r1,[r0]
	ldr r2,=1<<7
	mvn r3,r2
	and r1,r1,r3
	str r1,[r0]

	bl delay

	ldr r0,=0xC001C000
	ldr r1,[r0]
	ldr r2,=1<<7 
	orr r1,r1,r2
	str r1,[r0]

	b loop

delay:
	push {r0,lr}
	ldr r0,=0x2000000
delay_loop:
	sub r0,r0,#1
	cmp r0,#0
	bne delay_loop
	pop {r0,lr}
	bx lr

.end

