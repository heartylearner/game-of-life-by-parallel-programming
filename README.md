# game-of-life-by-parallel-programming
The rule of the game is widespread on the internet.
Your input file should align with the following format:
m=rows n=columns epoch
a11 a12 .... a1n
.
.
.
am1 ........ amn
with aij belong to {'.','O'}
ex:  
3 5 4  
O..O.  
O....  
.O.O.  
and the result will be   
.O...  
.O...  
.O...  
in the output file  

you can try with ./main -t 1~20 infile outfile
with 1~20 represent how many thread you want to use.
