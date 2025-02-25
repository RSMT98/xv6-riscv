.data

test_template: .asciz "|%s|\n"
answer_template: .asciz "%d\n"
msg_buffer_overflow: .asciz "Buffer overflow\n"
msg_empty_string: .asciz "An empty string was entered\n"
msg_no_space: .asciz "There is no space\n"
msg_multiple_spaces: .asciz "There are multiple spaces\n"
msg_no_first_num: .asciz "There is no first number\n"
msg_no_second_num: .asciz "There is no second number\n"
msg_incorrect_first_number: .asciz "Incorrect format of the first number\n"
msg_incorrect_second_number: .asciz "Incorrect format of the second number\n"
msg_read_error: .asciz "Read error\n"

.text
.global main

#define BUFFER_SIZE 256

main:
    # allocating memory and saving return address
    addi sp, sp, -BUFFER_SIZE - 64
    sd ra, BUFFER_SIZE+56(sp)    
    sd s0, BUFFER_SIZE+48(sp)
    sd s1, BUFFER_SIZE+40(sp)
    sd s2, BUFFER_SIZE+32(sp)
    sd s3, BUFFER_SIZE+24(sp)
    sd s4, BUFFER_SIZE+16(sp)
    sd s5, BUFFER_SIZE+8(sp)
    sd s6, BUFFER_SIZE+0(sp)

    # a0 = pointer to buffer
    # a1 = buffer size
    mv a0, sp         
    li a1, BUFFER_SIZE
    call gets

    # checking for gets error
    beqz a0, read_error

    # replacing '\n' with '\0'
    mv t0, sp                
replace_newline_loop:
    lb t1, 0(t0)             
    beqz t1, replace_newline_done  
    addi t2, t1, -10         
    beqz t2, found_newline
    addi t0, t0, 1           
    j replace_newline_loop
found_newline:
    sb zero, 0(t0)           
replace_newline_done:

    # calculating string length
    mv a0, sp
    call strlen
    mv s1, a0                

    # empty string check
    beqz s1, empty_string_error

    # buffer overflow check
    li t0, BUFFER_SIZE-1
    beq s1, t0, buffer_overflow_error

    # test output
    la a0, test_template
    mv a1, sp
    call printf

    # finding space character
    mv a0, sp
    call find_space
    beqz a0, no_space_error
    mv s2, a0                

    # checking for multiple spaces
    addi a0, s2, 1
    call find_space
    bnez a0, multiple_spaces_error

    sb zero, 0(s2)

    # checking first number presence
    mv a0, sp
    call strlen
    beqz a0, no_first_num_error

    # checking second number presence
    addi a0, s2, 1
    call strlen
    beqz a0, no_second_num_error

    # checking correctness of the first number
    mv a0, sp
    call is_number_valid
    beqz a0, incorrect_first_number_error

    # checking correctness of the second number
    addi a0, s2, 1
    call is_number_valid
    beqz a0, incorrect_second_number_error

    # s5 = first number
    mv a0, sp
    call atoi
    mv s5, a0                

    # s6 = second number
    addi a0, s2, 1
    call atoi
    mv s6, a0           

    # outputting the sum
    la a0, answer_template
    add a1, s5, s6
    call printf

    li a0, 0
    j handle_exit

# custom error handlers
read_error:
    la a1, msg_read_error
    j handle_error

buffer_overflow_error:
    la a1, msg_buffer_overflow
    j handle_error

empty_string_error:
    la a1, msg_empty_string
    j handle_error

no_space_error:
    la a1, msg_no_space
    j handle_error

multiple_spaces_error:
    la a1, msg_multiple_spaces
    j handle_error

no_first_num_error:
    la a1, msg_no_first_num
    j handle_error

no_second_num_error:
    la a1, msg_no_second_num
    j handle_error

incorrect_first_number_error:
    la a1, msg_incorrect_first_number
    j handle_error

incorrect_second_number_error:
    la a1, msg_incorrect_second_number
    j handle_error

handle_error:
    mv a0, a1
    call strlen
    mv a2, a0               
    li a0, 2                 
    mv a1, a1                
    call write
    li a0, 1                 

handle_exit:
    # restoring registers and restoring stack
    ld ra, BUFFER_SIZE+56(sp)      
    ld s0, BUFFER_SIZE+48(sp)
    ld s1, BUFFER_SIZE+40(sp)
    ld s2, BUFFER_SIZE+32(sp)
    ld s3, BUFFER_SIZE+24(sp)
    ld s4, BUFFER_SIZE+16(sp)
    ld s5, BUFFER_SIZE+8(sp)
    ld s6, BUFFER_SIZE+0(sp)
    addi sp, sp, BUFFER_SIZE + 64  
    call exit

# helper functions
strlen:
    mv t0, a0
    li t1, 0
strlen_loop:
    lb t2, 0(t0)
    beqz t2, strlen_end
    addi t1, t1, 1
    addi t0, t0, 1
    j strlen_loop
strlen_end:
    mv a0, t1
    ret

find_space:
    mv t0, a0
find_space_loop:
    lb t1, 0(t0)
    beqz t1, find_space_not_found
    li t2, ' '
    beq t1, t2, find_space_found
    addi t0, t0, 1
    j find_space_loop
find_space_found:
    mv a0, t0
    ret
find_space_not_found:
    li a0, 0
    ret

is_number_valid:
    mv t0, a0
is_number_valid_loop:
    lb t1, 0(t0)
    beqz t1, is_number_valid_ok
    li t2, '0'
    blt t1, t2, is_number_valid_fail
    li t2, '9'
    bgt t1, t2, is_number_valid_fail
    addi t0, t0, 1
    j is_number_valid_loop
is_number_valid_ok:
    li a0, 1
    ret
is_number_valid_fail:
    li a0, 0
    ret

atoi:
    li t0, 0
    li t1, 10
atoi_loop:
    lb t2, 0(a0)
    beqz t2, atoi_end
    addi t2, t2, -'0'
    mul t0, t0, t1
    add t0, t0, t2
    addi a0, a0, 1
    j atoi_loop
atoi_end:
    mv a0, t0
    ret
