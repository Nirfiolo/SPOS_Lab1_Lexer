# SPOS Lab1 Lexer

## Example

### Input
```cpp
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
Id: 0   Type: int             Line:    0[0   ] Symbol id:      Symbol:
Id: 1   Type: Id              Line:    0[4   ] Symbol id: 0    Symbol: |main|
Id: 2   Type: (               Line:    0[9   ] Symbol id:      Symbol:
Id: 3   Type: )               Line:    0[10  ] Symbol id:      Symbol:
Id: 4   Type: {               Line:    1[1   ] Symbol id:      Symbol:
Id: 5   Type: Id              Line:    2[4   ] Symbol id: 1    Symbol: |std|
Id: 6   Type: ::              Line:    2[7   ] Symbol id:      Symbol:
Id: 7   Type: Id              Line:    2[9   ] Symbol id: 2    Symbol: |cout|
Id: 8   Type: String          Line:    2[17  ] Symbol id: 3    Symbol: |"Hi, world!\n"|
Id: 9   Type: ;               Line:    2[32  ] Symbol id:      Symbol:
Id: 10  Type: return          Line:    3[4   ] Symbol id:      Symbol:
Id: 11  Type: IntNumber       Line:    3[11  ] Symbol id: 4    Symbol: |0|
Id: 12  Type: ;               Line:    3[13  ] Symbol id:      Symbol:
Id: 13  Type: }               Line:    4[1   ] Symbol id:      Symbol:
```
