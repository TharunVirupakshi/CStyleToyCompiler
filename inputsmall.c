// Variable declarations
int a = 10;
char b = 'x';
string s = "hello";

// Correct type usage
int c = a + 5;          // Valid: both sides are int
char d = b;             // Valid: char assigned to char
string greeting = s;    // Valid: string assigned to string

// Type mismatch errors
int wrongType1 = b;     // Valid: assigning char to int
char wrongType2 = a;    // Valid: assigning int to char
int mismatch = s;       // Error: assigning string to int

// Usage of undeclared variable
int undeclaredTest = x; // Error: 'x' is undeclared

// Binary expressions
int sum = a + b;        // Valid: can add int and char
int product = a * c;    // Valid: int * int

// Unary expressions
int neg = -a;           // Valid: unary minus on int
int invalidUnary = -s;  // Error: unary minus on string


// Mixed expressions
int complexExpr = a + b * c - 10; // Error: type mismatch in expression
int correctExpr = a * (c + 1);    // Valid: all int operations

// Function definitions
int myFunc(int x) {
    return x + 5;
}

int anotherFunc(string str) {
    return str;          // Error: return type mismatch, string to int
}

int nestedFuncCalls() {
    return myFunc(myFunc(a)); // Valid: nested function calls with int return
}

int invalidNestedCalls() {
    return myFunc(greeting); // Error: argument type mismatch
}

// Edge cases
int edgeCase1 = a + undeclaredVar; // Error: undeclared variable in expression
int edgeCase2 = 5 + b - s;         // Error: mixing int, char, and string in expression

// Final assignment validation
a = 20;                   // Valid: assigning int to int
a = s;                    // Error: assigning string to int





// int a;
// char b;
// a = a+'c'; // type mismatch
// b = 'b';
// b = 1 + 2 * 'c' / 3 + 'd';


// // int a = 1;
// // int b;
// // int z = a * (b + 1) && 1;

// // char d = 'c';
// // string str = "str";
// // int a, b ,c;

// // {
// //     int a = x;
// //     int y;
// //     int a;
// // }

// // string str; // Duplicate

// if(1) return 1;

// if(1) int a=1; 
// else{
//     int a=1;
// }

// for(int a=1; ; ){
//     a = 'c';
// }



// return;
// return z;
// char c=2;
// for(int a=1; a < 10, c=3; a++){
//     for(;;);
// }

// while(c == 2);

// int a;
// int foo(int a){

//     int foo1(int a){
//         // int foo1(int a){
//         //     int b;
//         // };
//         int b;
//         {
//             int c;
//             int foo1(int a) return a;
//         }
//     };

// };

// int a;

// int foo(int a) {
//     int b;
//     return a;
// }
// // void foo(int a, int b, int c){

// // }

// // int foo2(int a) return a + foo1(2);

// // int foo3(int a){
// //     int b = 2 + a;
// //     a++;
// //     while(c == 2); 
// //     return b;
// // }

// // foo();
// // foo1(1);
// // foo2(b);

// // for(int a=1; a < 10; a++, b++){
// //     b = 1;
// // }
