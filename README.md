# SPOS Lab1 Lexer

## Example

### Input
```cpp
#include <iostream>

int main() 
{ 
    std::cout << "Hi, world!\n";
    return 0; 
}
```

### Output
```
No errors

Tokens:
( #include        , 0    )
( int                    )
( Id              , 1    )
( (                      )
( )                      )
( {                      )
( Id              , 2    )
( ::                     )
( Id              , 3    )
( <<                     )
( String          , 4    )
( ;                      )
( return                 )
( IntNumber       , 5    )
( ;                      )
( }                      )

Symbol table:
Index: 0   Symbol: |#include <iostream>|
Index: 1   Symbol: |main|
Index: 2   Symbol: |std|
Index: 3   Symbol: |cout|
Index: 4   Symbol: |"Hi, world!\n"|
Index: 5   Symbol: |0|

Tokens (full info):
Id: 0   Type: #include        Line:    0[0   ] Symbol id: 0    Symbol: |#include <iostream>|
Id: 1   Type: int             Line:    2[0   ] Symbol id:      Symbol: 
Id: 2   Type: Id              Line:    2[4   ] Symbol id: 1    Symbol: |main|
Id: 3   Type: (               Line:    2[9   ] Symbol id:      Symbol: 
Id: 4   Type: )               Line:    2[10  ] Symbol id:      Symbol: 
Id: 5   Type: {               Line:    3[1   ] Symbol id:      Symbol: 
Id: 6   Type: Id              Line:    4[4   ] Symbol id: 2    Symbol: |std|
Id: 7   Type: ::              Line:    4[7   ] Symbol id:      Symbol: 
Id: 8   Type: Id              Line:    4[9   ] Symbol id: 3    Symbol: |cout|
Id: 9   Type: <<              Line:    4[14  ] Symbol id:      Symbol: 
Id: 10  Type: String          Line:    4[17  ] Symbol id: 4    Symbol: |"Hi, world!\n"|
Id: 11  Type: ;               Line:    4[32  ] Symbol id:      Symbol: 
Id: 12  Type: return          Line:    5[4   ] Symbol id:      Symbol: 
Id: 13  Type: IntNumber       Line:    5[11  ] Symbol id: 5    Symbol: |0|
Id: 14  Type: ;               Line:    5[13  ] Symbol id:      Symbol: 
Id: 15  Type: }               Line:    6[1   ] Symbol id:      Symbol: 
```
