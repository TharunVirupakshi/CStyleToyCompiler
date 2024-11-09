1 || 2 && !3;


/*
0: if 1 goto 2
1: goto 6
2: if_false 1 goto 6
3: goto 4
4: t0 = 1
5: goto 7
6: t0 = 0
*/




// a = (b || str) && !(a++);
// a++;
// a;

// {
//     int a = 1;
//     a = 2;
//     b = 2;
// }
// a = 1



// string a="str";




// int foo(int a, int b, int c) return 1; // Missing return

// foo(1 ,1, 1);


// int foo1(int a, int b, char c, string str) return c; // Wrong return type
// // int b = 2;
// foo1(1, b, 'c', "a"); // VALID

// void foo3(int c){
//     return 1; // invalid
// }

// void foo4(); //No return valid

// int foo5(){

//     // Nested return stmts
//     if(b) return 'c'; //Invalid
//     for(;;){
//         if(b) return 'c';
//         return "str"; //Invalid
//     }

//     // Does not affect foo5()
//     string foo6(){
//         return 1; //Invalid
//         return "str";
//     }

//     if(foo(1, 2, 3)) return foo6(); // Invalid

//     return 1; // Valid
// }



// int a = 3 + 'c';
// a = 3 + "str"; //INVALID
// a = 3 && "str"; //VALID
// char c = "str" == !"str"; //VALID, == return 1 or 0
// "str" <= "str"; //INVALID
// string d = "str" + 1; //INVALID
// d = "str";





// int main() {
//     // Valid cases with automatic type promotion (char to int)
//     char c1 = 'a';
//     int i1 = 10;
    
//     int result1 = c1 + i1;          // char + int -> promoted to int
//     int result2 = i1 - c1;          // int - char -> promoted to int
//     int result3 = c1 * 2;           // char * int -> promoted to int
//     int result4 = 10 / c1;          // int / char -> promoted to int

//     char c2 = 'b';
//     int result5 = i1 && c2;         // Logical operator should be allowed between int and char (both are non-zero)
//     result5 = "str" && "str";       // Should work as str are non-zero values
//     int result6 = c2 < i1;          // Comparison operator should work due to promotion
    

//     char result8 = c1 || i1;        // Logical operator should work

//     // Increment/Decrement (valid for int and char)
//     c1++;                           // Valid: char can be incremented
//     i1--;                           // Valid: int can be decremented
    
//     // Assignments and promotions
//     c1 = i1 + 5;                    // Valid: int is implicitly cast to char
//     i1 = c1 + 5;                    // Valid: char promoted to int
//     int result9 = (c1 + i1) * 2;    // Nested expressions with promotion


//     // INVALID CASES
//     result1 = c1 && "string";   // Error: logical AND with incompatible types (char and string)
//     result2 = i1 < "another";   // Error: comparison between int and string
//     result4 = i1 / "str";       // Error: division with incompatible type (int and string)

//     int foo(int a) return a;
//     char foo2() return 'c';
//     string foo3() return "str";

//     result2 = i1 <= foo3();
//     result2 = foo2() <= foo3(); // Error: Cannot c
//     result2 = foo2() <= foo();
//     result2 = foo2() && foo3(); // Valid: Logical returns int type
//     result2 = foo2() == foo3(); // Error: cannot compare string with other types
//     result2 = foo3() == foo3(); // Valid: comp operator return type is int, != and == are the only op allowed for str
//     result2 = foo3() <= foo3();  // Error: not allowed
//     result2 = !foo3() <= !foo3();  // Valid: ! returns int

//     return 0;
// }






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
