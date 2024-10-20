// Variable declarations
int x, y, z;
char a;
string b;

// Assignment
x = 10;
y = 5;
z = x + y;
a = 'c';
a = "string";

// Arithmetic operations
x = x - y;
y = y * 2;
z = z / x;
z = x + (y*9)/2 * -3;

// Unary operations
x = -x;   // Unary minus
y = -y;   // Unary minus on another variable
a++;
++a;
--a;
a--;


// Combination of unary and binary
z = -(x + y);    // Unary minus on an expression

// not supported yet
// x = +9;          // Unary plus (just for testing, even though it has no effect)
// x = +a;


// Conditional statements
if (x > y) {
    x = x + 1;   // if block with multiple statements
    
    int a;
    int x, y, z;
    char a;
    string b;
    a++; a--;

    /* Multi-line comment 
       inside if block */
    
    if (-(y < z) == -1)   // nested if
        y = z;   
    else
        z = y + x;
}

// Another if statement without a block
if (x == 0)
    x = y;


// Loops 
while (x < 100) {
    x = x + 1;
    y = y + 2;
}
while(1);
while(1){
    while (1)
    {
        /* code */
    }   
}

for(;;);
for(int a=2; ;);
for(int a=2, b=2; a<b ; b++);
for(int a=2, b=2; a<b ; b++){
    c++;
    for(a=2, b=3, 1+1, -(2+3);;); // for_init with only exp_list
    for(;;){
        while(1);
        return;
    }

    return a;
}

// Empty statement
;

// return 
return ;
return 1;

// Empty block
{ }
