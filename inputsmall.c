

int a = 3 + 'c';
a = 3 + "str"; //INVALID
a = 3 && "str"; //VALID
char c = "str" == !"str"; //VALID, == return 1 or 0
"str" <= "str"; //INVALID
string d = "str" + 1; //INVALID
d = "str";





int main() {
    // Valid cases with automatic type promotion (char to int)
    char c1 = 'a';
    int i1 = 10;
    
    int result1 = c1 + i1;          // char + int -> promoted to int
    int result2 = i1 - c1;          // int - char -> promoted to int
    int result3 = c1 * 2;           // char * int -> promoted to int
    int result4 = 10 / c1;          // int / char -> promoted to int

    char c2 = 'b';
    int result5 = i1 && c2;         // Logical operator should be allowed between int and char (both are non-zero)
    result5 = "str" && "str";       // Should work as str are non-zero values
    int result6 = c2 < i1;          // Comparison operator should work due to promotion
    

    char result8 = c1 || i1;        // Logical operator should work

    // Increment/Decrement (valid for int and char)
    c1++;                           // Valid: char can be incremented
    i1--;                           // Valid: int can be decremented
    
    // Assignments and promotions
    c1 = i1 + 5;                    // Valid: int is implicitly cast to char
    i1 = c1 + 5;                    // Valid: char promoted to int
    int result9 = (c1 + i1) * 2;    // Nested expressions with promotion


    // INVALID CASES
    result1 = c1 && "string";   // Error: logical AND with incompatible types (char and string)
    result2 = i1 < "another";   // Error: comparison between int and string
    result4 = i1 / "str";       // Error: division with incompatible type (int and string)


    return 0;
}






// int a;
// char b;
// a = a+'c'; // type mismatch
// b = 'b';
// b = 1 + 2 * 'c' / 3 + 'd';


// int a = 1;
// int b;
// int z = a * (b + 1) && 1;

// char d = 'c';
// string str = "str";
// int a, b ,c;

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
